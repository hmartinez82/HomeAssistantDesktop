#include <QApplication>
#include "TrayViewModel.h"
#include "TrayView.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    TrayViewModel viewModel;
    TrayView view(&viewModel);

    return a.exec();
}
