#include <DualMenuSystemTrayIcon.h>
#include <QGuiApplication>

DualMenuSystemTrayIcon::DualMenuSystemTrayIcon(QObject* parent) : QSystemTrayIcon(parent)
{
     connect(this, &QSystemTrayIcon::activated, this, &DualMenuSystemTrayIcon::handleActivation);
}

DualMenuSystemTrayIcon::DualMenuSystemTrayIcon(const QIcon& icon, QObject* parent) : QSystemTrayIcon(icon, parent)
{
    connect(this, &QSystemTrayIcon::activated, this, &DualMenuSystemTrayIcon::handleActivation);
}

void DualMenuSystemTrayIcon::setMenu(QMenu* menu)
{
	_regularMenu = menu;
}

void DualMenuSystemTrayIcon::setAlternateMenu(QMenu* menu, Qt::KeyboardModifiers modifiers)
{
	_alternateMenu = menu;
	_modifiers = modifiers;
}

void DualMenuSystemTrayIcon::handleActivation(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::Context) {
        Qt::KeyboardModifiers modifiers = QGuiApplication::queryKeyboardModifiers();

        if ((modifiers & _modifiers) && !_alternateMenu.isNull()) {
            _alternateMenu->exec(QCursor::pos());
        }
        else if(!_regularMenu.isNull()) {
            _regularMenu->exec(QCursor::pos());
        }
    }
}