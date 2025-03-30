#include "TrayViewModel.h"
#include <QCoreApplication>
#include <QJsonArray>
#include <algorithm>
#include "HomeAssistantService.h"
#include "ConfigurationService.h"
#include "WinApi.h"

static const QString HUMIDIFIER_ENTITY_ID = "switch.humidifier";
static const QString TESTPLUG_ENTITY_ID = "switch.testplug";
static const QString BEDROOMLIGHT_ENTITY_ID = "light.bedroom_light";
static const QString KITCHENLIGHT_ENTITY_ID = "light.kitchen";
static const QString CO2_SENSOR_ENTITY_ID = "sensor.view_plus_carbon_dioxide";
static const QStringList ENTITIES = { HUMIDIFIER_ENTITY_ID, TESTPLUG_ENTITY_ID, BEDROOMLIGHT_ENTITY_ID, KITCHENLIGHT_ENTITY_ID, CO2_SENSOR_ENTITY_ID };

TrayViewModel::TrayViewModel(ConfigurationService* configurationService, HomeAssistantService* haService, QObject *parent) :
  _configurationService(configurationService), _haService(haService), QObject{parent}
{
    QObject::connect(_haService, &HomeAssistantService::Connected, this, &TrayViewModel::OnHAConnected);
    QObject::connect(_haService, &HomeAssistantService::Disconnected, this, &TrayViewModel::OnHADisconnected);
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

bool TrayViewModel::UpdateAuthToken()
{
	return _configurationService->InputAuthToken();
}

void TrayViewModel::SetKitchenLightState(bool on)
{
    qInfo() << "Setting kitchen light state to" << (on ? "on" : "off");
    _haService->CallService("light", QString("turn_%1").arg(on ? "on" : "off"), KITCHENLIGHT_ENTITY_ID);
}

void TrayViewModel::OnHAConnected()
{
    // Listen to entities events
    _haService->SubscribeToEntities(ENTITIES);

    emit HomeAsssitantConnectionStateChanged(true);
}

void TrayViewModel::OnHADisconnected()
{
    emit HomeAsssitantConnectionStateChanged(false);
}

void TrayViewModel::OnHAEventReceived(const QJsonObject& event)
{
    auto data = event.constBegin();
    if (data == event.constEnd())
    {
        return;
    }
	auto firstChild = data->toObject();

    if (firstChild.contains(TESTPLUG_ENTITY_ID))
    {
        auto entity = firstChild[TESTPLUG_ENTITY_ID].toObject();
        if (entity.contains("+"))
        {
            entity = entity["+"].toObject();
        }
		auto newState = entity["s"].toString() == "on";
		if (newState != _testPlugState)
		{
			_testPlugState = newState;
			emit TestPlugStateChanged(newState);
		}
    }

    if (firstChild.contains(HUMIDIFIER_ENTITY_ID))
    {
        auto entity = firstChild[HUMIDIFIER_ENTITY_ID].toObject();
        if (entity.contains("+"))
        {
            entity = entity["+"].toObject();
        }
        auto newState = entity["s"].toString() == "on";
        if (newState != _humidifierState)
        {
            _humidifierState = newState;
            emit HumidifierStateChanged(newState);
        }
    }

    if (firstChild.contains(BEDROOMLIGHT_ENTITY_ID))
    {
        auto entity = firstChild[BEDROOMLIGHT_ENTITY_ID].toObject();
        if (entity.contains("+"))
        {
            entity = entity["+"].toObject();
        }
        auto newState = entity["s"].toString() == "on";
		if (newState != _bedroomLightState)
		{
			_bedroomLightState = newState;
			emit BedroomLightStateChanged(newState);
		}
    }

	if (firstChild.contains(KITCHENLIGHT_ENTITY_ID))
	{
        auto entity = firstChild[KITCHENLIGHT_ENTITY_ID].toObject();
        if (entity.contains("+"))
        {
            entity = entity["+"].toObject();
        }
        auto newState = entity["s"].toString() == "on";
		if (newState != _kitchenLightState)
		{
			_kitchenLightState = newState;
			emit KitchenLightStateChanged(newState);
		}
	}

    if (firstChild.contains(CO2_SENSOR_ENTITY_ID))
    {
        auto entity = firstChild[CO2_SENSOR_ENTITY_ID].toObject();
        if (entity.contains("+"))
        {
            entity = entity["+"].toObject();
        }
        auto newValue = entity["s"].toString().toDouble();
		if (newValue != _co2SensorValue)
		{
			_co2SensorValue = newValue;
			emit CO2ValueChanged(newValue);
		}
    }
}