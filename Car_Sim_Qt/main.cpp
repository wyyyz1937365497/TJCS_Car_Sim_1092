#include "Car_Sim_Qt.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Car_Sim_Qt window;
    window.show();
    return app.exec();
}
