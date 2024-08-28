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
	bool b = _server.route("/", QHttpServerRequest::Method::Post, handler);
	qDebug() << b;
}


bool NotificationServer::Start()
{
	return _server.listen(QHostAddress("192.168.1.10"), 9123) != 0;

}

void NotificationServer::ProcessRequest(const QHttpServerRequest& request, QHttpServerResponder&& responder)
{
	auto body = QString::fromUtf8(request.body());
	auto parameters = request.body().split('&');

	QMap<QString, QString> map;
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