#include <QtGui/QApplication>
#include <QMetaType>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setApplicationVersion(APP_VERSION);
    a.setApplicationName("CLOP");
    a.setOrganizationName("univ-lille3.fr");
    a.setOrganizationDomain("univ-lille3.fr");

    MainWindow w;
    w.show();
    return a.exec();
}
