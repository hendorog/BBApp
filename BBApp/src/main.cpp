#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    // Step out when the program breaks
    //_CrtSetBreakAlloc( 531753 );

    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    return a.exec();
}
