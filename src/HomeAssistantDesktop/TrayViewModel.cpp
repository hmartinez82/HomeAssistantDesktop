#include "TrayViewModel.h"
#include <QCoreApplication>
#include <QJsonArray>
#include <algorithm>
#include "HomeAssistantService.h"
#include "WinApi.h"

static const QString HUMIDIFIER_ENTITY_ID = "switch.humidifier";
static const QString TESTPLUG_ENTITY_ID = "switch.testplug";
static const QString BEDROOMLIGHT_ETNTITY_ID = "switch.bedroom_light";

TrayViewModel::TrayViewModel(HomeAssistantService* haService, QObject *parent) : _haService(haService), QObject{parent}
{
    QObject::connect(_haService, &HomeAssistantService::Connected, this, &TrayViewModel::OnHAConnected);
    QObject::connect(_haService, &HomeAssistantService::Disconnected, this, &TrayViewModel::OnHADisconnected);
    QObject::connect(_haService, &HomeAssistantService::ResultReceived, this, &TrayViewModel::OnHAResultReceived);
    QObject::connect(_haService, &HomeAssistantService::EventReceived, this, &TrayViewModel::OnHAEventReceived);
}

void TrayViewModel::QuitApplication()
{   
    qApp->quit();
}

void TrayViewModel::SetHumidifierState(bool on)
{
    qInfo() << "Setting humidifier state to" << (on ? "on" : "off");
    _haService->CallService("switch", QString("turn_%1").arg(on ? "on" : "off"), HUMIDIFIER_ENTITY_ID);
}

bool TrayViewModel::GetHumidifierState()
{
    return _humidifierState;
}

void TrayViewModel::SetTestPlugState(bool on)
{
    qInfo() << "Setting test plug state to" << (on ? "on" : "off");
    _haService->CallService("switch", QString("turn_%1").arg(on ? "on" : "off"), TESTPLUG_ENTITY_ID);
}

bool TrayViewModel::GetTestPlugState()
{
    return _testPlugState;
}

void TrayViewModel::SetBedroomLightState(bool on)
{
    qInfo() << "Setting bedroom light state to" << (on ? "on" : "off");
    _haService->CallService("switch", QString("turn_%1").arg(on ? "on" : "off"), BEDROOMLIGHT_ETNTITY_ID);
}

bool TrayViewModel::GetBedroomLightState()
{
    return _bedroomLightState;
}

void TrayViewModel::OnHAConnected()
{
    // Listen to state changed events
    _stateChangedEventId = _haService->SubscribeToEvents("state_changed");

    // Load current state
    _fetchStateCommandId = _haService->FetchStates();

    emit HomeAsssitantConnectionStateChanged(true);
}

void TrayViewModel::OnHADisconnected()
{
    emit HomeAsssitantConnectionStateChanged(false);
}

void TrayViewModel::OnHAResultReceived(int id, bool success, const QJsonValue& result)
{
    using namespace std;

    if (!success)
    {
        return;
    }

    if (id == _fetchStateCommandId)
    {
        auto entities = result.toArray();

        auto it = find_if(cbegin(entities), cend(entities), [&](const QJsonValue& v) {
            return v["entity_id"].toString() == TESTPLUG_ENTITY_ID;
        });
        if (it != cend(entities))
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

        if (it != cend(entities))
        {
            auto newState = (*it)[QLatin1String("state")].toString() == "on";
            if (newState != _humidifierState)
            {
                _humidifierState = newState;
                emit HumidifierStateChanged(newState);
            }
        }

        it = find_if(cbegin(entities), cend(entities), [&](const QJsonValue& v) {
            return v["entity_id"].toString() == BEDROOMLIGHT_ETNTITY_ID;
            });

        if (it != cend(entities))
        {
            auto newState = (*it)[QLatin1String("state")].toString() == "on";
            if (newState != _bedroomLightState)
            {
                _bedroomLightState = newState;
                emit BedroomLightStateChanged(newState);
            }
        }
    }
}

void TrayViewModel::OnHAEventReceived(int id, const QJsonObject& event)
{
    if (id == _stateChangedEventId)
    {
        auto data = event["data"].toObject();
        auto newState = data["new_state"][QLatin1String("state")].toString() == "on";
        auto entity = data["entity_id"].toString();

        if (entity == TESTPLUG_ENTITY_ID)
        {
            if (newState != _testPlugState)
            {
                _testPlugState = newState;
                qInfo() << "Test plug state changed to" << (newState ? "on" : "off");
                emit TestPlugStateChanged(newState);
            }
        }
        else if (entity == HUMIDIFIER_ENTITY_ID)
        {
            if (newState != _humidifierState)
            {
                _humidifierState = newState;
                qInfo() << "Humidifier state changed to" << (newState ? "on" : "off");
                emit HumidifierStateChanged(newState);
            }
        }
    }
}