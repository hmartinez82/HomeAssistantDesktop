#include "Application.h"
#include <cpptrace/cpptrace.hpp>
#include <cpptrace/from_current.hpp>
#include <format>

void Application::CppTracePrinter(const cpptrace::stacktrace& trace)
{
    qCritical("Stack trace (most recent call first):");

    std::size_t i = 0;
    for (const auto& frame : trace.frames) {
        const bool hasLine = frame.line != cpptrace::nullable<std::uint32_t>::null();
        qCritical("%s", std::format(
            "  #{:<3} {:#018x} in {} at {}:{}",
            i++,
            frame.raw_address,
            frame.symbol.empty() ? "<unknown>" : frame.symbol,
            frame.filename.empty() ? "<unknown>" : frame.filename,
            hasLine ? std::to_string(frame.line.raw_value) : "?"
        ).c_str());
    }
}

bool Application::notify(QObject* receiver, QEvent* event)
{
    bool result = false;
    cpptrace::try_catch(
        [&] {
            result = QApplication::notify(receiver, event);
        },
        [&](const std::exception& e) {
            qCritical()
                << "Unhandled exception in slot/event handler\n"
                << "  receiver :" << receiver->metaObject()->className()
                << receiver->objectName() << "\n"
                << "  event    :" << event->type() << "\n"
                << "  what()   :" << e.what();
            CppTracePrinter(cpptrace::from_current_exception());
            // result stays false — swallow and keep running
            // alternatives: throw; to re-raise, or QApplication::quit();
        },
        [&]() {
            qCritical()
                << "Unhandled exception in slot/event handler\n"
                << "  receiver :" << receiver->metaObject()->className()
                << receiver->objectName() << "\n"
                << "  event    :" << event->type();
            CppTracePrinter(cpptrace::from_current_exception());
        }
    );
    return result;
}
