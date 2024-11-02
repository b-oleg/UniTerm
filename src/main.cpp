#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("IAP RAS");
    QCoreApplication::setApplicationName("UniTerm");
    QCoreApplication::setApplicationVersion("0.4");

    MainWindow w;
    //w.setWindowTitle(QString("%1 v%2").arg(QCoreApplication::applicationName(), QCoreApplication::applicationVersion()));
    w.show();
    return a.exec();
}
