#pragma once
#include <QObject>
#include <QPointer>
#include <QJsonObject>
#include <QTimer>

class QWebSocket;

class HomeAssistantService : public QObject
{
    Q_OBJECT

public:
    HomeAssistantService(QObject* parent = nullptr);

    ~HomeAssistantService();

    void Connect();

    void Disconnect();

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

private slots:

    void OnWebSocketConnected();

    void OnWebSocketDisconnected();

    void OnWebSocketTextMessageReceived(const QString& message);

    void OnPingTimerTimeout();

private:
    enum class HAConnectionState
    {
        DISCONNECTED,
        CONNECTING,
        AUTHENTICATING,
        CONNECTED
    };

    QPointer<QWebSocket> _webSocket;

    QPointer<QTimer> _pingTimer;

    HAConnectionState _haConnectionState = HAConnectionState::DISCONNECTED;

    int _haNextMessageId = 0;

    void SendJsonObject(const QJsonObject& obj);

    int SendCommand(const QJsonObject& obj);
};

