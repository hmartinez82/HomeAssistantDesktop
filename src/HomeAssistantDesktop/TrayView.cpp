#include "TrayView.h"
#include <QAction>
#include <QApplication>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QStyle>
#include "TrayViewModel.h"
#include "DualMenuSystemTrayIcon.h"

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

    _co2Action = new QAction("CO₂");
    _co2Action->setEnabled(false);

    _connectedMenu.addAction(_co2Action);
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

	auto setApiTokenAction = new QAction("Set HA API Token", this);
    connect(setApiTokenAction, &QAction::triggered, this, &TrayView::OnSetApiTokenActionTriggered);

	_configurationMenu.addAction(setApiTokenAction);

    _connectedIcon = qApp->style()->standardIcon(QStyle::SP_FileDialogListView);
    _disconnectedIcon = qApp->style()->standardIcon(QStyle::SP_MessageBoxWarning);

    _sysTrayIcon = new DualMenuSystemTrayIcon(_disconnectedIcon, this);
    _sysTrayIcon->setMenu(&_disconnectedMenu);
	_sysTrayIcon->setAlternateMenu(&_configurationMenu, Qt::ControlModifier); // Use Ctrl+Right Click to show connected menu
    _sysTrayIcon->show();

    connect(_viewModel, &TrayViewModel::HumidifierStateChanged, humidifierAction, &QAction::setChecked);
    connect(_viewModel, &TrayViewModel::TestPlugStateChanged, testPlugAction, &QAction::setChecked);
    connect(_viewModel, &TrayViewModel::HomeAsssitantConnectionStateChanged, this, &TrayView::OnConnectionStateChanged);
    connect(_viewModel, &TrayViewModel::NotificationReceived, this, &TrayView::ShowNotification);
	connect(_viewModel, &TrayViewModel::CO2ValueChanged, this, &TrayView::OnCO2ValueChanged);
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
    _sysTrayIcon->setMenu(connected ? &_connectedMenu : &_disconnectedMenu);
}

void TrayView::OnCO2ValueChanged(float value)
{
    _co2Action->setText(QString("CO₂ (%1) ppm").arg(value, 0, 'f', 1));
}

void TrayView::OnSetApiTokenActionTriggered()
{
	_viewModel->UpdateAuthToken();
}

void TrayView::ShowNotification(const QString& title, const QString& message)
{
    _sysTrayIcon->showMessage(title, message);
}