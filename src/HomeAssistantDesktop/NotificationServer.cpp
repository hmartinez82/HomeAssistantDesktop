#include "NotificationServer.h"
#include <QHostAddress>
#include <QHttpServerRequest>
#include <QMap>
#include <functional>

using namespace std;
using namespace std::placeholders;

NotificationServer::NotificationServer(QObject* parent) : QObject(parent)
{
	std::function<void(const QHttpServerRequest&, QHttpServerResponder&&)> handler = 
		bind(&NotificationServer::ProcessRequest, this, _1, _2);
	auto routed = _server.route("/", QHttpServerRequest::Method::Post, handler);
	if (!routed)
	{
		qWarning() << "Failed to route POST requests to /";
	}
	else
	{
		qInfo() << "Routed POST requests to /";
	}
}


bool NotificationServer::Start()
{
	auto ret = _server.listen(QHostAddress("192.168.1.10"), 9123) != 0;
	if (!ret)
	{
		qWarning() << "Failed to start REST server to receive Home Assistant Notifications";
	}
	else
	{
		qInfo() << "Started REST server to receive Home Assistant Notifications";
	}
	return ret;
}

void NotificationServer::ProcessRequest(const QHttpServerRequest& request, QHttpServerResponder&& responder)
{
	const auto& body = request.body();

	qDebug() << "Received notification payload: " << body;

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

	emit NotificationReceived(map["title"], map["message"]);
}