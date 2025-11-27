#include <QApplication>
#include "InterfaceGrafica.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    
    InterfaceGrafica w;
    w.show();

    return a.exec();
}