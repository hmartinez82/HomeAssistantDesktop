#include "TrayView.h"
#include <QAction>
#include <QApplication>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QStyle>
#include "TrayViewModel.h"

TrayView::TrayView(TrayViewModel* viewModel, QObject *parent) : QObject{parent}, _viewModel(viewModel)
{
    InitializeComponents();
}

void TrayView::InitializeComponents()
{
    auto humidifierAction = new QAction("Humidifier", this);
    humidifierAction->setCheckable(true);
    humidifierAction->setChecked(_viewModel->GetHumidifierState());
    connect(humidifierAction, &QAction::triggered, this, &TrayView::OnHumidifierActionToggled);

    auto testPlugAction = new QAction("Test Plug", this);
    testPlugAction->setCheckable(true);
    testPlugAction->setChecked(_viewModel->GetTestPlugState());
    connect(testPlugAction, &QAction::triggered, this, &TrayView::OnTestPlugActionToggled);

    auto quitAction = new QAction("Quit", this);
    connect(quitAction, &QAction::triggered, this, &TrayView::OnQuitActionTriggered);

    auto bedroomLightAction = new QAction("Bedroom Light", this);
    bedroomLightAction->setCheckable(true);
    bedroomLightAction->setChecked(_viewModel->GetBedroomLightState());
    connect(bedroomLightAction, &QAction::triggered, this, &TrayView::OnBedroomLightActionToggled);

    auto kitchenLightAction = new QAction("Kitchen Light", this);
    kitchenLightAction->setCheckable(true);
    kitchenLightAction->setChecked(_viewModel->GetKitchenLightState());
    connect(kitchenLightAction, &QAction::triggered, this, &TrayView::OnKitchenLightActionToggled);

    auto co2Action = new QAction("CO₂");
    _connectedMenu.addAction(co2Action);
    _connectedMenu.addSeparator();
    _connectedMenu.addAction(humidifierAction);
    _connectedMenu.addAction(testPlugAction);
    _connectedMenu.addAction(bedroomLightAction);
    _connectedMenu.addAction(kitchenLightAction);
    _connectedMenu.addSeparator();
    _connectedMenu.addAction(quitAction);

    _disconnectedMenu.addAction("Connecting...");
    _disconnectedMenu.addSeparator();
    _disconnectedMenu.addAction(quitAction);

    _connectedIcon = qApp->style()->standardIcon(QStyle::SP_FileDialogListView);
    _disconnectedIcon = qApp->style()->standardIcon(QStyle::SP_MessageBoxWarning);

    _sysTrayIcon = new QSystemTrayIcon(_disconnectedIcon, this);
    _sysTrayIcon->setContextMenu(&_disconnectedMenu);
    _sysTrayIcon->show();

    connect(_viewModel, &TrayViewModel::HumidifierStateChanged, humidifierAction, &QAction::setChecked);
    connect(_viewModel, &TrayViewModel::TestPlugStateChanged, testPlugAction, &QAction::setChecked);
    connect(_viewModel, &TrayViewModel::HomeAsssitantConnectionStateChanged, this, &TrayView::OnConnectionStateChanged);
    connect(_viewModel, &TrayViewModel::NotificationReceived, this, &TrayView::ShowNotification);
    connect(_viewModel, &TrayViewModel::CO2ValueChanged, [=] (float value)  {
        co2Action->setText(QString("CO₂ (%1) ppm").arg(value, 0, 'f', 1));
    });
}

void TrayView::OnQuitActionTriggered(bool)
{
    _viewModel->QuitApplication();
}

void TrayView::OnHumidifierActionToggled(bool checked)
{
    _viewModel->SetHumidifierState(checked);
}

void TrayView::OnTestPlugActionToggled(bool checked)
{
    _viewModel->SetTestPlugState(checked);
}

void TrayView::OnBedroomLightActionToggled(bool checked)
{
    _viewModel->SetBedroomLightState(checked);
}

void TrayView::OnKitchenLightActionToggled(bool checked)
{
    _viewModel->SetKitchenLightState(checked);
}

void TrayView::OnConnectionStateChanged(bool connected)
{
    _sysTrayIcon->setIcon(connected ? _connectedIcon : _disconnectedIcon);
    _sysTrayIcon->setContextMenu(nullptr);
    _sysTrayIcon->setContextMenu(connected ? &_connectedMenu : &_disconnectedMenu);
}

void TrayView::OnCO2ValueChanged(float value)
{

}

void TrayView::ShowNotification(const QString& title, const QString& message)
{
    _sysTrayIcon->showMessage(title, message);
}