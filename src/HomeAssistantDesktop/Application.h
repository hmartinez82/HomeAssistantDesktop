#pragma once
#include <QApplication>
#include <cpptrace/forward.hpp>

class Application : public QApplication
{
public:
    using QApplication::QApplication;
    bool notify(QObject* receiver, QEvent* event) override;

    static void CppTracePrinter(const cpptrace::stacktrace& trace);
};
