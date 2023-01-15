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

signals:
    void HumidifierStateChanged(bool state);

    void TestPlugStateChanged(bool state);

private slots:

    void OnHomeAssistantConnected();

    void OnResultReceived(int id, bool success, const QJsonValue& result);

private:
    QPointer<HomeAssistantService> _haService;

    int _fetchStateCommandId = 0;

    bool _humidifierState = false;

    bool _testPlugState = false;

    void LoadCurrentState();
};

#endif // TRAYVIEWMODEL_H
