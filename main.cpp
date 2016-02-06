#include "dialog.h"
#include "ui_dialog.h"
#include "RenderWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Dialog w;

    RenderWindow r;
    r.setBaseSize(QSize(640,480));

    QObject::connect(w.ui->fullScreenBut, SIGNAL(clicked()), &r, SLOT(toggleFullScreen()));
    QObject::connect(w.ui->quitBut, SIGNAL(clicked()), &a, SLOT(quit()));
    QObject::connect(&r, SIGNAL(computedFPS(int)), &w, SLOT(updateFPSLabel(int)));
    QObject::connect(&r, SIGNAL(computedRenderTime(int)), &w, SLOT(updateRenderTimeLabel(int)));
    QObject::connect(w.ui->pauseBut, SIGNAL(clicked(bool)), &r, SLOT(togglePause()));
    QObject::connect(w.ui->noneRadio, SIGNAL(clicked(bool)), &r, SLOT(setRenderNormal()));
    QObject::connect(w.ui->tripleRadio, SIGNAL(clicked(bool)), &r, SLOT(setRenderMode3x()));
    QObject::connect(w.ui->octupleRadio, SIGNAL(clicked(bool)), &r, SLOT(setRenderMode8x()));
    QObject::connect(w.ui->dodekaQuadrupleRadio, SIGNAL(clicked(bool)), &r, SLOT(setRenderMode24x()));
    r.show();
    w.show();
    w.focusWidget();

    return a.exec();
}
