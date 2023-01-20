#pragma once
#include <QtGlobal>

void SetupLogger(const QString& logFile);

void CustomMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);