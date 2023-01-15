#pragma once
#include <QObject>
#include <QPointer>

class QWebSocket;

class HomeAssistantService : public QObject
{
    Q_OBJECT

public:
    HomeAssistantService(QObject* parent = nullptr);

    ~HomeAssistantService();

    void Connect();

    void Disconnect();

signals:
    void Connected();

    void Disconnected();

private slots:

    void OnWebSocketConnected();

    void OnWebSocketDisconnected();

    void OnWebSocketTextMessageReceived(const QString& message);

private:
    enum class HAConnectionState
    {
        DISCONNECTED,
        CONNECTING,
        AUTHENTICATING,
        CONNECTED
    };

    QPointer<QWebSocket> _webSocket;

    HAConnectionState _haConnectionState = HAConnectionState::DISCONNECTED;

    void SendJsonObject(const QJsonObject& obj);
};

