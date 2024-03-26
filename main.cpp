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


    /*
     * If the stuff below is allocated on the stack, its destructor tries to destroy an already-null MainWindow (which has deleted itself due to going out of scope),
     * causing a crash on exit.
     * Therefore, heap for all.
    */
    QGoodWindow* gw = new QGoodWindow(nullptr);
    MainWindow* w = new MainWindow();
    QGoodCentralWidget* gcw = new QGoodCentralWidget(gw);

    w->ParentGoodWin = gw;
    gw->setCentralWidget(gcw);
    w->resize(1300,1115);
    gcw->setCentralWidget(w);
    gw->setAppDarkTheme();
    gcw->setTitleBarColor(QColor::fromRgbF(0.6f,0.6f,0.6f));
    gw->show();



    int returncode = a.exec();


    // This causes a cascade that deletes everything including the MainWindow.
    delete gw;


    return returncode;
}
