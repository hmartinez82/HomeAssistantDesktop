#include "HomeAssistantService.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMetaEnum>
#include <QUrl>
#include <QWebSocket>
#include "NotificationServer.h"
#include "ConfigurationService.h"
#include "WinApi.h"

static const auto HOMEASSISTANT_WS_URL = "ws://192.168.1.3:8123/api/websocket";
static const auto PING_TIMER = 60;
static const auto RECONNECT_TIMER = 15;

HomeAssistantService::HomeAssistantService(ConfigurationService* configurationService, QObject* parent) :
    _configurationService(configurationService), QObject(parent)
{
    _webSocket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);
    _notificationServer = new NotificationServer(this);
    _pingTimer = new QTimer(this);
    _pingTimer->setInterval(PING_TIMER * 1000);

    _reconnectTimer = new QTimer(this);
    _reconnectTimer->setInterval(RECONNECT_TIMER * 1000);
    _reconnectTimer->setSingleShot(true);

    QObject::connect(_webSocket, &QWebSocket::connected, this, &HomeAssistantService::OnWebSocketConnected);
    QObject::connect(_webSocket, &QWebSocket::disconnected, this, &HomeAssistantService::OnWebSocketDisconnected);
    QObject::connect(_webSocket, &QWebSocket::textMessageReceived, this, &HomeAssistantService::OnWebSocketTextMessageReceived);
    QObject::connect(_webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &HomeAssistantService::OnWebSocketError);
    QObject::connect(_notificationServer, &NotificationServer::NotificationReceived, this, &HomeAssistantService::NotificationReceived);
    QObject::connect(_pingTimer, &QTimer::timeout, this, &HomeAssistantService::OnPingTimerTimeout);
    QObject::connect(_reconnectTimer, &QTimer::timeout, this, &HomeAssistantService::OnReconnectTimerTimeout);
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
        qDebug("Connecting to %s", HOMEASSISTANT_WS_URL);
        _webSocket->open(QUrl(HOMEASSISTANT_WS_URL));
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

bool HomeAssistantService::StartNotificationServer()
{
    return _notificationServer->Start();
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

    _reconnectTimer->start();
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
                QJsonObject jObj;
                jObj["type"] = "auth";
                jObj["access_token"] = _configurationService->GetAuthToken();;
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

            _fetchStateCommandId = 0;
            _entitiyChangedEventId = 0;
            emit Connected();
        }
		else if (jDoc["type"].toString() == "auth_invalid")
		{
			qCritical("Authentication failed. Invalid token.");
			emit ServiceErrored("Authentication failed. Invalid token.", 0);
			Disconnect();
		}
        break;
    case HAConnectionState::CONNECTED:
        if (jDoc["type"].toString() == "result" && jDoc["id"].toInt() ==_fetchStateCommandId)
        {
            auto success = jDoc["success"].toBool();
            emit ResultReceived(success, success ? jDoc["result"] : jDoc["error"]);
        }
        else if (jDoc["type"].toString() == "event" && jDoc["id"].toInt() == _entitiyChangedEventId)
        {
            emit EventReceived(jDoc["event"].toObject());
        }
        break;
    default:
        qCritical() << "Unexpected state" << _haConnectionState;
    }
}

void HomeAssistantService::OnWebSocketError(QAbstractSocket::SocketError)
{
    qCritical() << "Websocket error:" << _webSocket->errorString();
}

void HomeAssistantService::OnPingTimerTimeout()
{
    QJsonObject jObj;
    jObj["type"] = "ping";

    SendCommand(jObj);
}

void HomeAssistantService::OnReconnectTimerTimeout()
{
    if (_haConnectionState == HAConnectionState::DISCONNECTED ||
        _haConnectionState == HAConnectionState::CONNECTING)
    {
        _haConnectionState = HAConnectionState::DISCONNECTED;
        _reconnectTimer->start();
        Reconnect();
    }
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

void HomeAssistantService::FetchStates()
{
    QJsonObject jObj;
    jObj["type"] = "get_states";
    _fetchStateCommandId = SendCommand(jObj);
}

void HomeAssistantService::SubscribeToEntities(const QStringList& entities)
{
    QJsonObject jObj;
    jObj["type"] = "subscribe_entities";
    jObj["entity_ids"] = QJsonArray::fromStringList(entities);
    _entitiyChangedEventId = SendCommand(jObj);
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

void HomeAssistantService::Reconnect()
{
    qInfo() << "Attempting to reconnect...";
    Connect();
}
