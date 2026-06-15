#pragma once

#include <QHttpServer>

class NotificationServer : public QObject
{
	Q_OBJECT

public:
	NotificationServer(QObject* parent = nullptr);

	~NotificationServer() = default;

	bool Start();

signals:
	void NotificationReceived(const QString& title, const QString& message);

private slots:
	void ProcessHARequest(const QHttpServerRequest& request, QHttpServerResponder& responder);

	void ProcessWebHookRequest(const QHttpServerRequest& request, QHttpServerResponder& responder);

private:
	QHttpServer _haServer;
	QHttpServer _webHookServer;
};

