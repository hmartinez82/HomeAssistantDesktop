#pragma once
#include <QObject>
#include <QAbstractSocket>
#include <QPointer>
#include <QJsonObject>
#include <QTimer>

class QWebSocket;
class NotificationServer;

class HomeAssistantService : public QObject
{
    Q_OBJECT

public:
    enum class HAConnectionState : int
    {
        DISCONNECTED,
        CONNECTING,
        AUTHENTICATING,
        CONNECTED
    };
    Q_ENUM(HAConnectionState)

    HomeAssistantService(QObject* parent = nullptr);

    ~HomeAssistantService();

    void Connect();

    void Disconnect();

    bool StartNotificationServer();

    int CallService(const QString& domain, const QString& service,
                    const QJsonObject& target = QJsonObject(),
                     const QJsonObject& serviceData = QJsonObject());
    int CallService(const QString& domain, const QString& service, const QString& targetEntity,
                    const QJsonObject& serviceData = QJsonObject());

    int FetchStates();

    int SubscribeToEvents(const QString& eventType);

signals:
    void Connected();

    void Disconnected();

    void ResultReceived(int id, bool success, const QJsonValue& result);

    void EventReceived(int id, const QJsonObject& event);

    void ServiceErrored(const QString& message, int code);

    void NotificationReceived(const QString& title, const QString& message);

private slots:

    void OnWebSocketConnected();

    void OnWebSocketDisconnected();

    void OnWebSocketTextMessageReceived(const QString& message);

    void OnWebSocketError(QAbstractSocket::SocketError error);

    void OnPingTimerTimeout();

    void OnReconnectTimerTimeout();

private:

    QPointer<QWebSocket> _webSocket;

    QPointer<NotificationServer> _notificationServer;

    QPointer<QTimer> _pingTimer;

    QPointer<QTimer> _reconnectTimer;

    HAConnectionState _haConnectionState = HAConnectionState::DISCONNECTED;

    int _haNextMessageId = 0;

    void SendJsonObject(const QJsonObject& obj);

    int SendCommand(const QJsonObject& obj);

    void Reconnect();
};

