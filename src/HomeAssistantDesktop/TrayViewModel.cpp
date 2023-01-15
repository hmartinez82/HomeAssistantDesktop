#include "TrayViewModel.h"
#include <QByteArray>
#include <QGuiApplication>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QJsonDocument>
#include "WinApi.h"

static const QString HUMIDIFIER_ENTITY_ID = "switch.humidifier";
static const QString TESTPLUG_ENTITY_ID = "switch.testplug";
static const QString API_URL = "http://192.168.1.3:8123/api";

TrayViewModel::TrayViewModel(QObject *parent) : QObject{parent}
{
    _networkManager = new QNetworkAccessManager(this);

    LoadCurrentState();
}

void TrayViewModel::QuitApplication()
{   
    qApp->quit();
}

void TrayViewModel::SetHumidifierState(bool on)
{
    auto req = CreateBaseRequest(QString("/services/switch/turn_%1").arg(on ? "on" : "off"));
    _networkManager->post(req, QString("{\"entity_id\": \"%1\"}").arg(HUMIDIFIER_ENTITY_ID).toUtf8());
}

bool TrayViewModel::GetHumidifierState()
{
    return _humidifierState;
}

void TrayViewModel::LoadCurrentState()
{
    auto req = CreateBaseRequest(QString("/states/%1").arg(HUMIDIFIER_ENTITY_ID));
    auto reply = _networkManager->get(req);
    connect(reply, &QNetworkReply::finished, this, &TrayViewModel::OnLoadcurrentStateRequestFinished);

    req = CreateBaseRequest(QString("/states/%1").arg(TESTPLUG_ENTITY_ID));
    reply = _networkManager->get(req);
    connect(reply, &QNetworkReply::finished, this, &TrayViewModel::OnLoadcurrentStateRequestFinished);
}

void TrayViewModel::SetTestPlugState(bool on)
{
    auto req = CreateBaseRequest(QString("/services/switch/turn_%1").arg(on ? "on" : "off"));
    _networkManager->post(req, QString("{\"entity_id\": \"%1\"}").arg(TESTPLUG_ENTITY_ID).toUtf8());
}

bool TrayViewModel::GetTestPlugState()
{
    return _testPlugState;
}

void TrayViewModel::OnLoadcurrentStateRequestFinished()
{
    auto reply = static_cast<QNetworkReply*>(sender());
    auto data = reply->readAll();
    auto jDoc = QJsonDocument::fromJson(data);
    auto newState = jDoc["state"].toString() == "on";

    if (jDoc["entity_id"].toString() == HUMIDIFIER_ENTITY_ID)
    {
        if (newState != _humidifierState)
        {
            _humidifierState = newState;
            emit HumidifierStateChanged(newState);
        }
    }
    else if (jDoc["entity_id"].toString() == TESTPLUG_ENTITY_ID)
    {
        if (newState != _testPlugState)
        {
            _testPlugState = newState;
            emit TestPlugStateChanged(newState);
        }
    }
    reply->close();
}

QNetworkRequest TrayViewModel::CreateBaseRequest(const QString& path)
{
    auto token = WinApi_ReadAuthToken();

    QNetworkRequest req(API_URL + path);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader(QByteArray("Authorization"),
                     QByteArray("Bearer ").append(token.c_str()));

    return req;
}
