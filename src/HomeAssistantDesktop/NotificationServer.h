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

private:
	QHttpServer _server;

	void ProcessRequest(const QHttpServerRequest& request, QHttpServerResponder&& responder);
};

