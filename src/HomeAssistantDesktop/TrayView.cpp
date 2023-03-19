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

    _connectedMenu.addAction(humidifierAction);
    _connectedMenu.addAction(testPlugAction);
    _connectedMenu.addAction(bedroomLightAction);
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

void TrayView::OnConnectionStateChanged(bool connected)
{
    _sysTrayIcon->setIcon(connected ? _connectedIcon : _disconnectedIcon);
    _sysTrayIcon->setContextMenu(nullptr);
    _sysTrayIcon->setContextMenu(connected ? &_connectedMenu : &_disconnectedMenu);
}