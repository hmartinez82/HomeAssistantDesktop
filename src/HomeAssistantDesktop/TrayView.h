#ifndef TRAYVIEW_H
#define TRAYVIEW_H

#include <QObject>
#include <QPointer>

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

private:
    QPointer<TrayViewModel> _viewModel;

    void InitializeComponents();
};

#endif // TRAYVIEW_H
