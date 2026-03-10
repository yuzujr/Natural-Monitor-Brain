#include "SimulatorWidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SimulatorWidget w;
    w.show();
    return a.exec();
}
