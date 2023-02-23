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
    void OnQuitActionTriggered(bool);

    void OnHumidifierActionToggled(bool);

    void OnTestPlugActionToggled(bool);

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
