#ifndef TRAYVIEWMODEL_H
#define TRAYVIEWMODEL_H

#include <QObject>
#include <QPointer>

class HomeAssistantService;

class TrayViewModel : public QObject
{
    Q_OBJECT
public:
    explicit TrayViewModel(HomeAssistantService* haService, QObject *parent = nullptr);

    void QuitApplication();

    void SetHumidifierState(bool on);

    bool GetHumidifierState();

    void SetTestPlugState(bool on);

    bool GetTestPlugState();

    void SetBedroomLightState(bool on);

    bool GetBedroomLightState();

signals:
    void HumidifierStateChanged(bool state);

    void TestPlugStateChanged(bool state);

    void BedroomLightStateChanged(bool state);

    void HomeAsssitantConnectionStateChanged(bool connected);

private slots:

    void OnHAConnected();

    void OnHADisconnected();

    void OnHAResultReceived(int id, bool success, const QJsonValue& result);

    void OnHAEventReceived(int id, const QJsonObject& event);

private:
    QPointer<HomeAssistantService> _haService;

    int _fetchStateCommandId = 0;
    
    int _stateChangedEventId = 0;

    bool _humidifierState = false;

    bool _testPlugState = false;

    bool _bedroomLightState = false;
};

#endif // TRAYVIEWMODEL_H
