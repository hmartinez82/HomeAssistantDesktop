#ifndef TRAYVIEW_H
#define TRAYVIEW_H

#include <QObject>
#include <QMenu>
#include <QPointer>
#include <QIcon>

class QSystemTrayIcon;
class TrayViewModel;

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

    void OnConnectionStateChanged(bool connected);

private:
    QPointer<TrayViewModel> _viewModel;

    QPointer<QSystemTrayIcon> _sysTrayIcon;

    QMenu _connectedMenu;

    QMenu _disconnectedMenu;

    QIcon _connectedIcon;

    QIcon _disconnectedIcon;

    void InitializeComponents();
};

#endif // TRAYVIEW_H
