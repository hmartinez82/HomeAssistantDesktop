#include "Logging.h"
#include <QByteArray>
#include <QString>
#include <memory>
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/msvc_sink.h"
#include "spdlog/spdlog.h"

void SetupLogger(const QString& logFile)
{
    auto console_sink = std::make_shared<spdlog::sinks::windebug_sink_st>(true);

    auto max_size = 1048576 * 10; //10MB
    auto max_files = 20;
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_st>(logFile.toStdString(), max_size, max_files);

    auto multi_sink_logger = new spdlog::logger("multi_sink", { file_sink, console_sink });
    spdlog::set_default_logger(std::shared_ptr<spdlog::logger>(multi_sink_logger));
    spdlog::set_pattern("%Y-%m-%d %T%z [%^%l%$] %v");

#ifdef _DEBUG
    spdlog::set_level(spdlog::level::debug);
#endif // DEBUG

    qInstallMessageHandler(CustomMessageHandler);
}

void CustomMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    auto localMsg = msg.toLocal8Bit();
    spdlog::source_loc loc(context.file, context.line, context.function);

    switch (type) {
    case QtDebugMsg:
        spdlog::log(loc, spdlog::level::debug, localMsg.constData());
        break;
    case QtInfoMsg:
        spdlog::log(loc, spdlog::level::info, localMsg.constData());
        break;
    case QtWarningMsg:
        spdlog::log(loc, spdlog::level::warn, localMsg.constData());
        break;
    case QtCriticalMsg:
        spdlog::log(loc, spdlog::level::err, localMsg.constData());
        break;
    case QtFatalMsg:
        spdlog::log(loc, spdlog::level::critical, localMsg.constData());
        abort();
    }
    spdlog::default_logger()->flush();
}