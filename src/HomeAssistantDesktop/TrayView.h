#ifndef TRAYVIEW_H
#define TRAYVIEW_H

#include <QObject>
#include <QMenu>
#include <QPointer>
#include <QIcon>

class QSystemTrayIcon;
class TrayViewModel;
class DualMenuSystemTrayIcon;

class TrayView : public QObject
{
    Q_OBJECT
public:
    explicit TrayView(TrayViewModel* viewModel, QObject *parent = nullptr);

signals:

private slots:
    void OnQuitActionTriggered(bool checked);

    void OnHumidifierActionToggled(bool checked);

    void OnTestPlugActionToggled(bool checked);

    void OnBedroomLightActionToggled(bool checked);

    void OnKitchenLightActionToggled(bool checked);

    void OnOfficeLightActionToggled(bool checked);

    void OnConnectionStateChanged(bool connected);

    void OnCO2ValueChanged(float value);

    void OnSetApiTokenActionTriggered();

    void OnStartWithWindowsActionTriggered(bool checked);

    void ShowNotification(const QString& title, const QString& message);

private:
    QPointer<TrayViewModel> _viewModel;

    QPointer<DualMenuSystemTrayIcon> _sysTrayIcon;

    QPointer<QAction> _co2Action;

    QMenu _connectedMenu;

    QMenu _disconnectedMenu;

    QMenu _configurationMenu;

    QIcon _connectedIcon;

    QIcon _disconnectedIcon;

    void InitializeComponents();
};

#endif // TRAYVIEW_H
