#include "TrayViewModel.h"
#include <QCoreApplication>
#include <QJsonArray>
#include <algorithm>
#include "HomeAssistantService.h"
#include "WinApi.h"

static const QString HUMIDIFIER_ENTITY_ID = "switch.humidifier";
static const QString TESTPLUG_ENTITY_ID = "switch.testplug";
static const QString BEDROOMLIGHT_ENTITY_ID = "light.bedroom_light";
static const QString KITCHENLIGHT_ENTITY_ID = "light.kitchen";
static const QString CO2_SENSOR_ENTITY_ID = "sensor.view_plus_carbon_dioxide";

TrayViewModel::TrayViewModel(HomeAssistantService* haService, QObject *parent) : _haService(haService), QObject{parent}
{
    QObject::connect(_haService, &HomeAssistantService::Connected, this, &TrayViewModel::OnHAConnected);
    QObject::connect(_haService, &HomeAssistantService::Disconnected, this, &TrayViewModel::OnHADisconnected);
    QObject::connect(_haService, &HomeAssistantService::ResultReceived, this, &TrayViewModel::OnHAResultReceived);
    QObject::connect(_haService, &HomeAssistantService::EventReceived, this, &TrayViewModel::OnHAEventReceived);
    QObject::connect(_haService, &HomeAssistantService::NotificationReceived, this, &TrayViewModel::NotificationReceived);
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
    _haService->CallService("light", QString("turn_%1").arg(on ? "on" : "off"), BEDROOMLIGHT_ENTITY_ID);
}

bool TrayViewModel::GetBedroomLightState()
{
    return _bedroomLightState;
}

bool TrayViewModel::GetKitchenLightState()
{
    return _kitchenLightState;
}

void TrayViewModel::SetKitchenLightState(bool on)
{
    qInfo() << "Setting kitchen light state to" << (on ? "on" : "off");
    _haService->CallService("light", QString("turn_%1").arg(on ? "on" : "off"), KITCHENLIGHT_ENTITY_ID);
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
            return v["entity_id"].toString() == BEDROOMLIGHT_ENTITY_ID;
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

        it = find_if(cbegin(entities), cend(entities), [&](const QJsonValue& v) {
            return v["entity_id"].toString() == KITCHENLIGHT_ENTITY_ID;
            });

        if (it != cend(entities))
        {
            auto newState = (*it)[QLatin1String("state")].toString() == "on";
            if (newState != _kitchenLightState)
            {
                _kitchenLightState = newState;
                emit KitchenLightStateChanged(newState);
            }
        }

        it = find_if(cbegin(entities), cend(entities), [&](const QJsonValue& v) {
            return v["entity_id"].toString() == CO2_SENSOR_ENTITY_ID;
            });

        if (it != cend(entities))
        {
            auto newState = (*it)[QLatin1String("state")].toString();
            auto newValue = newState.toDouble();
            if (newValue != _co2SensorValue)
            {
                _co2SensorValue = newValue;
                emit CO2ValueChanged(newValue);
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
		else if (entity == BEDROOMLIGHT_ENTITY_ID)
		{
			if (newState != _bedroomLightState)
			{
				_bedroomLightState = newState;
				qInfo() << "Bedroom light state changed to" << (newState ? "on" : "off");
				emit BedroomLightStateChanged(newState);
			}
		}
		else if (entity == KITCHENLIGHT_ENTITY_ID)
		{
			if (newState != _kitchenLightState)
			{
				_kitchenLightState = newState;
				qInfo() << "Kitchen light state changed to" << (newState ? "on" : "off");
				emit KitchenLightStateChanged(newState);
			}
		}
		else if (entity == CO2_SENSOR_ENTITY_ID)
		{
			auto newValue = data["new_state"][QLatin1String("state")].toString().toDouble();
			if (newValue != _co2SensorValue)
			{
				_co2SensorValue = newValue;
				qInfo() << "CO2 sensor value changed to" << newValue;
				emit CO2ValueChanged(newValue);
			}
		}
	}
}