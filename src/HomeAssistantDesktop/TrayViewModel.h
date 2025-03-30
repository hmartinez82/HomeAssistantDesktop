#ifndef TRAYVIEWMODEL_H
#define TRAYVIEWMODEL_H

#include <QObject>
#include <QPointer>

class ConfigurationService;
class HomeAssistantService;

class TrayViewModel : public QObject
{
    Q_OBJECT
public:
    TrayViewModel(ConfigurationService* configurationService, HomeAssistantService* haService, QObject *parent = nullptr);

    void QuitApplication();

    void SetHumidifierState(bool on);

    bool GetHumidifierState();

    void SetTestPlugState(bool on);

    bool GetTestPlugState();

    void SetBedroomLightState(bool on);

    bool GetBedroomLightState();

    void SetKitchenLightState(bool on);

    bool GetKitchenLightState();

    bool UpdateAuthToken();

signals:
    void HumidifierStateChanged(bool state);

    void TestPlugStateChanged(bool state);

    void BedroomLightStateChanged(bool state);

    void KitchenLightStateChanged(bool state);

    void HomeAsssitantConnectionStateChanged(bool connected);

    void NotificationReceived(const QString& title, const QString& message);

    void CO2ValueChanged(float value);

private slots:

    void OnHAConnected();

    void OnHADisconnected();

    void OnHAEventReceived(const QJsonObject& event);

private:
    QPointer<ConfigurationService> _configurationService;

    QPointer<HomeAssistantService> _haService;

    bool _humidifierState = false;

    bool _testPlugState = false;

    bool _bedroomLightState = false;

    bool _kitchenLightState = false;

    double _co2SensorValue = 0;
};

#endif // TRAYVIEWMODEL_H
