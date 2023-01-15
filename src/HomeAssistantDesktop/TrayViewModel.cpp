#include "TrayViewModel.h"
#include <QGuiApplication>
#include <QJsonArray>
#include "HomeAssistantService.h"
#include "WinApi.h"
#include <algorithm>

static const QString HUMIDIFIER_ENTITY_ID = "switch.humidifier";
static const QString TESTPLUG_ENTITY_ID = "switch.testplug";

TrayViewModel::TrayViewModel(HomeAssistantService* haService, QObject *parent) : _haService(haService), QObject{parent}
{
    QObject::connect(_haService, &HomeAssistantService::ResultReceived, this, &TrayViewModel::OnResultReceived);
    QObject::connect(_haService, &HomeAssistantService::Connected, this, &TrayViewModel::OnHomeAssistantConnected);
}

void TrayViewModel::QuitApplication()
{   
    qApp->quit();
}

void TrayViewModel::SetHumidifierState(bool on)
{
    _haService->CallService("switch", QString("turn_%1").arg(on ? "on" : "off"), HUMIDIFIER_ENTITY_ID);
}

bool TrayViewModel::GetHumidifierState()
{
    return _humidifierState;
}

void TrayViewModel::LoadCurrentState()
{
    _fetchStateCommandId = _haService->FetchStates();
}

void TrayViewModel::SetTestPlugState(bool on)
{
    _haService->CallService("switch", QString("turn_%1").arg(on ? "on" : "off"), TESTPLUG_ENTITY_ID);
}

bool TrayViewModel::GetTestPlugState()
{
    return _testPlugState;
}

void TrayViewModel::OnHomeAssistantConnected()
{
    LoadCurrentState();
}

void TrayViewModel::OnResultReceived(int id, bool success, const QJsonValue& result)
{
    using namespace std;

    if (id == _fetchStateCommandId)
    {
        auto entities = result.toArray();

        auto it = find_if(cbegin(entities), cend(entities), [&](const QJsonValue& v) {
            return v["entity_id"].toString() == TESTPLUG_ENTITY_ID;
        });
        if (it != end(entities))
        {
            auto newState = (*it)[QLatin1String("state")].toString() == "on";
            if (newState != _testPlugState)
            {
                _testPlugState = newState;
                emit TestPlugStateChanged(newState);
            }
        }

        it = find_if(cbegin(entities), cend(entities), [&](const QJsonValue& v) {
            return v["entity_id"].toString() == HUMIDIFIER_ENTITY_ID;
        });

        if (it != end(entities))
        {
            auto newState = (*it)[QLatin1String("state")].toString() == "on";
            if (newState != _humidifierState)
            {
                _humidifierState = newState;
                emit HumidifierStateChanged(newState);
            }
        }
    }
}