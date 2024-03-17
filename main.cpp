#include "mainwindow.h"

#include <QApplication>
#include <QGoodWindow>
#include <QGoodCentralWidget>

int main(int argc, char *argv[])
{
    QGoodWindow::setup();
    QApplication a(argc, argv);

    winrt::uninit_apartment();
    winrt::init_apartment();

    QGoodWindow gw;
    MainWindow w;

    w.ParentGoodWin = &gw;
    QGoodCentralWidget gcw(&gw);
    gw.setCentralWidget(&gcw);
    w.resize(1300,1115);
    gcw.setCentralWidget(&w);
    gw.setAppDarkTheme();
    gcw.setTitleBarColor(QColor::fromRgbF(0.6f,0.6f,0.6f));
    gw.show();



    return a.exec();
}
