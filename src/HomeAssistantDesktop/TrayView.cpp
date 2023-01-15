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
    auto humidifierAction = new QAction("Humidifier");
    humidifierAction->setCheckable(true);
    humidifierAction->setChecked(_viewModel->GetHumidifierState());
    connect(humidifierAction, &QAction::triggered, this, &TrayView::OnHumidifierActionToggled);
    connect(_viewModel, &TrayViewModel::HumidifierStateChanged, humidifierAction, &QAction::setChecked);

    auto testPlugAction = new QAction("Test Plug");
    testPlugAction->setCheckable(true);
    testPlugAction->setChecked(_viewModel->GetTestPlugState());
    connect(testPlugAction, &QAction::triggered, this, &TrayView::OnTestPlugActionToggled);
    connect(_viewModel, &TrayViewModel::TestPlugStateChanged, testPlugAction, &QAction::setChecked);

    auto quitAction = new QAction("Quit");
    connect(quitAction, &QAction::triggered, this, &TrayView::OnQuitActionTriggered);

    auto menu = new QMenu();
    menu->addAction(humidifierAction);
    menu->addAction(testPlugAction);
    menu->addSeparator();
    menu->addAction(quitAction);

    auto icon = qApp->style()->standardIcon(QStyle::SP_FileDialogListView);

    auto trayIcon = new QSystemTrayIcon(icon, this);
    trayIcon->setContextMenu(menu);
    trayIcon->show();
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
