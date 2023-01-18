#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QMessageBox>
#include <QWidget>
#include <QWindow>
#include "HomeAssistantService.h"
#include "TrayViewModel.h"
#include "TrayView.h"
#include "WinApi.h"

void HandleWinApiError(const QString& message, int errorCode)
{
    QMessageBox::critical(nullptr, QString("Authentication token error"),
                          QString("%1. Code: 0x%2").arg(message).arg((uint)errorCode, 8, 16), QMessageBox::Ok);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    WinApi_Shutdown();
    WinApi_Initialize();

    QCommandLineParser parser;
    QCommandLineOption setTokenOptions("t", "Set auth token", "token");
    parser.addOption(setTokenOptions);
    parser.process(a);

    int ret = 0;
    if (parser.isSet(setTokenOptions))
    {
        auto token = parser.value(setTokenOptions);
        try
        {
            WinApi_StoreAuthToken(token.toStdString());
        }
        catch (const WinApiError& ex)
        {
            HandleWinApiError(QString::fromLocal8Bit(ex.what()), ex.errorCode());
            return ex.errorCode();
        }
    }
    else
    {
        HomeAssistantService haService;
        TrayViewModel viewModel(&haService);
        TrayView view(&viewModel);

        QObject::connect(&haService, &HomeAssistantService::ServiceErrored, HandleWinApiError);

        haService.Connect();

        ret = a.exec();
    }
    WinApi_Shutdown();
    return ret;
}
