#include <QApplication>
#include "HomeAssistantService.h"
#include "TrayViewModel.h"
#include "TrayView.h"
#include "WinApi.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    WinApi_Shutdown();
    WinApi_Initialize();

    HomeAssistantService haService;
    haService.Connect();

    TrayViewModel viewModel;
    TrayView view(&viewModel);

    auto ret = a.exec();
    WinApi_Shutdown();
    return ret;
}
