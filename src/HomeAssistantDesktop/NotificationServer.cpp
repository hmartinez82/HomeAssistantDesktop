#include "NotificationServer.h"
#include <QHostAddress>
#include <QHttpServerRequest>
#include <QJsonDocument>
#include <QJsonValue>
#include <QMap>
#include <QTcpServer>
#include <functional>

using namespace std;
using namespace std::placeholders;

NotificationServer::NotificationServer(QObject* parent) : QObject(parent)
{
	auto routedHA = _haServer.route("/", QHttpServerRequest::Method::Post, this, &NotificationServer::ProcessHARequest);

	QHttpServerConfiguration config;
	config.setWhitelist({ { QHostAddress("192.168.1.1"), 32 } }); // Allow only requests from Gateway

	auto routedWebHook = _webHookServer.route("/ui_webhook", QHttpServerRequest::Method::Post, this, &NotificationServer::ProcessWebHookRequest);

	if (!routedHA || !routedWebHook)
	{
		qWarning() << "Failed to route POST requests";
	}
	else
	{
		qInfo() << "Routed POST requests";
	}
}


bool NotificationServer::Start()
{
	auto tcpserver = new QTcpServer();
	if (!tcpserver->listen(QHostAddress("192.168.1.10"), 9123) || !_haServer.bind(tcpserver)) {
		delete tcpserver;
		qWarning() << "Failed to start TCP server to receive Home Assistant Notifications";
		return false;
	}

	tcpserver = new QTcpServer();
	if (!tcpserver->listen(QHostAddress("192.168.1.10"), 9124) || !_webHookServer.bind(tcpserver)) {
		delete tcpserver;
		qWarning() << "Failed to start TCP server to receive WebHook Notifications";
		return false;
	}

	qInfo() << "Started TCP servers to receive Home Assistant and WebHook Notifications";

	return true;
}

void NotificationServer::ProcessHARequest(const QHttpServerRequest& request, QHttpServerResponder& responder)
{
	const auto& body = request.body();

	qDebug() << "Received HA notification payload: " << body;

	QMap<QString, QString> map;
	auto parameters = body.split('&');
	for (const auto& param : parameters)
	{
		auto paramN = param.split('=');
		if (paramN.size() == 2)
		{
			map.insert(QString::fromUtf8(paramN[0]), QUrl::fromPercentEncoding(paramN[1].replace('+', ' ')));
		}
	}

	responder.write(QHttpServerResponder::StatusCode::Ok);

	emit NotificationReceived(map["title"], map["message"]);
}

void NotificationServer::ProcessWebHookRequest(const QHttpServerRequest& request, QHttpServerResponder& responder)
{
	const auto& body = request.body();

	qDebug() << "Received WebHook notification payload: " << body;

	auto jDoc = QJsonDocument::fromJson(body);

	responder.write(QHttpServerResponder::StatusCode::Ok);

	emit NotificationReceived(jDoc["name"].toString(), jDoc["message"].toString());

}