#include "HomeAssistantService.h"
#include <QWebSocket>
#include <QUrl>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMetaEnum>
#include "WinApi.h"
#include <QDebug>

static const auto s_haWebSocketURL = "ws://192.168.1.3:8123/api/websocket";

HomeAssistantService::HomeAssistantService(QObject* parent) : QObject(parent)
{
    _webSocket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);
    _pingTimer = new QTimer(this);
    _pingTimer->setInterval(5000);

    QObject::connect(_webSocket, &QWebSocket::connected, this, &HomeAssistantService::OnWebSocketConnected);
    QObject::connect(_webSocket, &QWebSocket::disconnected, this, &HomeAssistantService::OnWebSocketDisconnected);
    QObject::connect(_webSocket, &QWebSocket::textMessageReceived, this, &HomeAssistantService::OnWebSocketTextMessageReceived);
    QObject::connect(_pingTimer, &QTimer::timeout, this, &HomeAssistantService::OnPingTimerTimeout);
}

HomeAssistantService::~HomeAssistantService()
{
    Disconnect();
}

void HomeAssistantService::Connect()
{
    switch (_haConnectionState)
    {
    case HAConnectionState::DISCONNECTED:
        qDebug("Connecting to %s", s_haWebSocketURL);
        _webSocket->open(QUrl(s_haWebSocketURL));
        _haConnectionState = HAConnectionState::CONNECTING;
        return;
    }
    qCritical() << "Unexpected state" << _haConnectionState;
}

void HomeAssistantService::Disconnect()
{
    switch (_haConnectionState)
    {
    case HAConnectionState::AUTHENTICATING:
    case HAConnectionState::CONNECTED:
        _webSocket->close();
        return;
    }
    qCritical() << "Unexpected state" << _haConnectionState;
}

void HomeAssistantService::OnWebSocketConnected()
{
    qDebug("Websocket connected");
    _haNextMessageId = 1;
}

void HomeAssistantService::OnWebSocketDisconnected()
{
    _pingTimer->stop();
    _haNextMessageId = 0;
    _haConnectionState = HAConnectionState::DISCONNECTED;
    qDebug("Websocket diconnected");
    emit Disconnected();
}

void HomeAssistantService::OnWebSocketTextMessageReceived(const QString& message)
{
    qDebug() << "Websocket text message received: " << message;

    auto jDoc = QJsonDocument::fromJson(message.toUtf8());
    switch (_haConnectionState)
    {
    case HAConnectionState::CONNECTING:
        if (jDoc["type"].toString() == "auth_required")
        {
            try {
                auto token = WinApi_ReadAuthToken();
                QJsonObject jObj;
                jObj["type"] = "auth";
                jObj["access_token"] = QString::fromStdString(token);
                SendJsonObject(jObj);
                _haConnectionState = HAConnectionState::AUTHENTICATING;
            }
            catch (const WinApiError& ex)
            {
                emit ServiceErrored(QString::fromLocal8Bit(ex.what()), ex.errorCode());
            }
        }
        break;
    case HAConnectionState::AUTHENTICATING:
        if (jDoc["type"].toString() == "auth_ok")
        {
            qInfo("Connected to Home Assistant. Version %s", jDoc["ha_version"].toString().toUtf8().constData());
            _haConnectionState = HAConnectionState::CONNECTED;
            _pingTimer->start();
            emit Connected();
        }
        break;
    case HAConnectionState::CONNECTED:
        if (jDoc["type"].toString() == "result")
        {
            auto success = jDoc["success"].toBool();
            emit ResultReceived(jDoc["id"].toInt(), success, success ? jDoc["result"] : jDoc["error"]);
        }
        else if (jDoc["type"].toString() == "event")
        {
            emit EventReceived(jDoc["id"].toInt(), jDoc["event"].toObject());
        }
        break;
    default:
        qCritical() << "Unexpected state" << _haConnectionState;
    }
}

void HomeAssistantService::OnPingTimerTimeout()
{
    QJsonObject jObj;
    jObj["type"] = "ping";

    SendCommand(jObj);
}

int HomeAssistantService::CallService(const QString& domain, const QString& service, const QJsonObject& target, const QJsonObject& serviceData)
{
    QJsonObject jObj;
    jObj["type"] = "call_service";
    jObj["domain"] = domain;
    jObj["service"] = service;
    if (!target.isEmpty())
    {
        jObj["target"] = target;
    }
    if (!serviceData.isEmpty())
    {
        jObj["service_data"] = serviceData;
    }
    return SendCommand(jObj);
}

int HomeAssistantService::CallService(const QString& domain, const QString& service, const QString& targetEntity, const QJsonObject& serviceData)
{
    return CallService(domain, service, QJsonObject{ {"entity_id", targetEntity} }, serviceData);
}

int HomeAssistantService::FetchStates()
{
    QJsonObject jObj;
    jObj["type"] = "get_states";
    return SendCommand(jObj);
}

int HomeAssistantService::SubscribeToEvents(const QString& eventType)
{
    QJsonObject jObj;
    jObj["type"] = "subscribe_events";
    jObj["event_type"] = eventType;
    return SendCommand(jObj);
}

void HomeAssistantService::SendJsonObject(const QJsonObject& obj)
{
    auto message = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    qDebug() << "Sending websocket text message:" << message;
    _webSocket->sendTextMessage(message);
    _webSocket->flush();
}

int HomeAssistantService::SendCommand(const QJsonObject& obj)
{
    auto id = _haNextMessageId++;
    auto commandObj = obj;
    commandObj["id"] = id;
    SendJsonObject(commandObj);
    return id;
}

