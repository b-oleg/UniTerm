#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QCoreApplication::setOrganizationName(APP_NAME);
    QCoreApplication::setApplicationName(APP_NAME);
    QCoreApplication::setApplicationVersion(APP_VERSION);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
