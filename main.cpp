#include "dialog.h"
#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Dialog w;
    QIcon icono(":/res/icon24.png");
    w.setWindowTitle("Conversor Video Cámaras");
    w.setWindowIcon(icono);
    a.setWindowIcon(icono);
    w.show();

    return a.exec();
}

