#include <QApplication>
#include "InterfaceGrafica.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    
    InterfaceGrafica janelaPrincipal;
    janelaPrincipal.show();

    return a.exec();
}