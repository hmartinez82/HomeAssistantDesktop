#include "HomeAssistantService.h"
#include <QWebSocket>
#include <QUrl>
#include <QJsonObject>
#include <QJsonDocument>
#include "WinApi.h"

HomeAssistantService::HomeAssistantService(QObject* parent) : QObject(parent)
{
    _webSocket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);
    QObject::connect(_webSocket, &QWebSocket::connected, this, &HomeAssistantService::OnWebSocketConnected);
    QObject::connect(_webSocket, &QWebSocket::disconnected, this, &HomeAssistantService::OnWebSocketDisconnected);
    QObject::connect(_webSocket, &QWebSocket::textMessageReceived, this, &HomeAssistantService::OnWebSocketTextMessageReceived);
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
        _webSocket->open(QUrl("ws://192.168.1.3:8123/api/websocket"));
        _haConnectionState = HAConnectionState::CONNECTING;
        break;
    }
}

void HomeAssistantService::Disconnect()
{
    switch (_haConnectionState)
    {
    case HAConnectionState::AUTHENTICATING:
    case HAConnectionState::CONNECTED:
        _webSocket->close();
        _haConnectionState = HAConnectionState::DISCONNECTED;
        break;
    }
}

void HomeAssistantService::OnWebSocketConnected()
{
    _haNextMessageId = 1;
}

void HomeAssistantService::OnWebSocketDisconnected()
{
    _haNextMessageId = 0;
    emit Disconnected();
}

void HomeAssistantService::OnWebSocketTextMessageReceived(const QString& message)
{
    auto jDoc = QJsonDocument::fromJson(message.toUtf8());

    switch (_haConnectionState)
    {
    case HAConnectionState::CONNECTING:
        if (jDoc["type"].toString() == "auth_required")
        {
            QJsonObject jObj;
            jObj["type"] = "auth";
            jObj["access_token"] = QString::fromStdString(WinApi_ReadAuthToken());
            SendJsonObject(jObj);
            _haConnectionState = HAConnectionState::AUTHENTICATING;
        }
        break;
    case HAConnectionState::AUTHENTICATING:
        auto jDoc = QJsonDocument::fromJson(message.toUtf8());
        if (jDoc["type"].toString() == "auth_ok")
        {
            qDebug() << "Connected to Home Assistant, version " << jDoc["ha_version"].toString();
            _haConnectionState = HAConnectionState::CONNECTED;
            emit Connected();
        }
        break;
    }
}

void HomeAssistantService::SendJsonObject(const QJsonObject& obj)
{
    auto message = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    _webSocket->sendTextMessage(message);
    _webSocket->flush();
}


void HomeAssistantService::CallService(const QString& domain, const QString& service, const QJsonObject& target, const QJsonObject& serviceData)
{
    QJsonObject jObj;
    jObj["id"] = _haNextMessageId++;
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
    SendJsonObject(jObj);
}

void HomeAssistantService::CallService(const QString& domain, const QString& service, const QString& targetEntity, const QJsonObject& serviceData)
{
    CallService(domain, service, QJsonObject{ {"entity_id", targetEntity} }, serviceData);
}