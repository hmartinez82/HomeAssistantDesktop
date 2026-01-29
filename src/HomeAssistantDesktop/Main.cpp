#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>
#include <Windows.h>
#include <appmodel.h>
#include "HomeAssistantService.h"
#include "TrayViewModel.h"
#include "TrayView.h"
#include "WinApi.h"
#include "Logging.h"
#include "ConfigurationService.h"

void HandleServiceError(const QString& message, int errorCode)
{
	QMessageBox::critical(nullptr, QString("Authentication token error"),
		QString("%1. Code: 0x%2").arg(message).arg((uint)errorCode, 8, 16), QMessageBox::Ok);
}

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("HomeAssistantDesktop");
	a.setQuitOnLastWindowClosed(false);

    UINT32 length = 0;
    auto result = GetCurrentPackageFamilyName(&length, nullptr);
    if (result == ERROR_INSUFFICIENT_BUFFER) { //Packaged
        SetupLogger(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/HomeAssistant.log");
    }
    else { // Not packaged
        SetupLogger(QDir::toNativeSeparators(QCoreApplication::applicationDirPath() + "/HomeAssistant.log"));
    }

    ConfigurationService configService;

    if (!configService.IsAuthTokenSet() &&
        !configService.InputAuthToken())
    {
		qCritical("No authentication token provided. Exiting.");
		return -1;
	}

    HomeAssistantService haService(&configService);
    TrayViewModel viewModel(&configService , &haService);
    TrayView view(&viewModel);

    QObject::connect(&haService, &HomeAssistantService::ServiceErrored, HandleServiceError);

    haService.Connect();
    haService.StartNotificationServer();
    auto ret = a.exec();

    qInfo("Exiting");
    return ret;
}
