#pragma once

#include <QSystemTrayIcon>
#include <QMenu>
#include <QPointer>

class DualMenuSystemTrayIcon : public QSystemTrayIcon {
    Q_OBJECT

public:
    DualMenuSystemTrayIcon(QObject* parent = nullptr);

    DualMenuSystemTrayIcon(const QIcon& icon, QObject* parent = nullptr);

    void setMenu(QMenu* menu);

    void setAlternateMenu(QMenu* menu, Qt::KeyboardModifiers modifiers);

private slots:
    void handleActivation(QSystemTrayIcon::ActivationReason reason);

private:
    QPointer<QMenu> _regularMenu;

    QPointer<QMenu> _alternateMenu;

	Qt::KeyboardModifiers _modifiers = Qt::NoModifier;
};