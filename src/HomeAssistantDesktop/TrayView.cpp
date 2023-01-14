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
    auto toggleAction = new QAction("Toggle");
    toggleAction->setCheckable(true);
    toggleAction->setChecked(_viewModel->GetHumidifierState());
    connect(toggleAction, &QAction::triggered, this, &TrayView::OnToggleActionToggled);
    connect(_viewModel, &TrayViewModel::HumidifierStateChanged, toggleAction, &QAction::setChecked);

    auto quitAction = new QAction("Quit");
    connect(quitAction, &QAction::triggered, this, &TrayView::OnQuitActionTriggered);

    auto menu = new QMenu();
    menu->addAction(toggleAction);
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

void TrayView::OnToggleActionToggled(bool checked)
{
    _viewModel->SetHumidifierState(checked);
}
