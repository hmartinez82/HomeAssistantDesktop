#ifndef TRAYVIEWMODEL_H
#define TRAYVIEWMODEL_H

#include <QObject>
#include <QPointer>
#include <QNetworkAccessManager>
#include <QNetworkRequest>

class TrayViewModel : public QObject
{
    Q_OBJECT
public:
    explicit TrayViewModel(QObject *parent = nullptr);

    void QuitApplication();

    void SetHumidifierState(bool on);

    bool GetHumidifierState();

    void SetTestPlugState(bool on);

    bool GetTestPlugState();

signals:
    void HumidifierStateChanged(bool state);

    void TestPlugStateChanged(bool state);

private slots:
    void OnLoadcurrentStateRequestFinished();

private:
    QPointer<QNetworkAccessManager> _networkManager;

    bool _humidifierState = false;

    bool _testPlugState = false;

    void LoadCurrentState();

    QNetworkRequest CreateBaseRequest(const QString& path);
};

#endif // TRAYVIEWMODEL_H
