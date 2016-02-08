#include "dialog.h"
#include "ui_dialog.h"
#include "RenderWindow.h"
#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Dialog w;

    w.setWindowIcon(QIcon(":/icons/icon.png"));

    RenderWindow r;
    r.setBaseSize(QSize(640,480));
    r.resize(640,480);

    QObject::connect(w.ui->fullScreenBut, SIGNAL(clicked()), &r, SLOT(toggleFullScreen()));
    QObject::connect(w.ui->quitBut, SIGNAL(clicked()), &a, SLOT(quit()));
    QObject::connect(&r, SIGNAL(computedFPS(int)), &w, SLOT(updateFPSLabel(int)));
    QObject::connect(&r, SIGNAL(computedRenderTime(int)), &w, SLOT(updateRenderTimeLabel(int)));
    QObject::connect(w.ui->pauseBut, SIGNAL(clicked()), &r, SLOT(togglePause()));
    QObject::connect(w.ui->noneRadio, SIGNAL(clicked()), &r, SLOT(setRenderNormal()));
    QObject::connect(w.ui->tripleRadio, SIGNAL(clicked()), &r, SLOT(setRenderMode3x()));
    QObject::connect(w.ui->octupleRadio, SIGNAL(clicked()), &r, SLOT(setRenderMode8x()));
    QObject::connect(w.ui->dodekaQuadrupleRadio, SIGNAL(clicked()), &r, SLOT(setRenderMode24x()));
    QObject::connect(w.ui->reverseChk, SIGNAL(clicked(bool)), &r, SLOT(setReverseRGB(bool)));
    QObject::connect(w.ui->timeSlider, SIGNAL(valueChanged(int)), &w, SLOT(on_timeSlider_valueChanged(int)));
    QObject::connect(w.ui->timeSlider, SIGNAL(valueChanged(int)), &r, SLOT(setTimeScale(int)));
    QObject::connect(w.ui->movingObjectsRadio, SIGNAL(clicked()), &r, SLOT(setMovingObjectsMode()));
    QObject::connect(w.ui->movingGratingRadio, SIGNAL(clicked()), &r, SLOT(setMovingGratingMode()));
    QObject::connect(w.ui->movingGratingRadio, SIGNAL(clicked(bool)), w.ui->mo_depth_test_chk, SLOT(setDisabled(bool)));
    QObject::connect(w.ui->movingGratingRadio, SIGNAL(clicked(bool)), w.ui->mo_no_fragshader_chk, SLOT(setDisabled(bool)));
    QObject::connect(w.ui->movingObjectsRadio, SIGNAL(clicked(bool)), w.ui->mo_depth_test_chk, SLOT(setEnabled(bool)));
    QObject::connect(w.ui->movingObjectsRadio, SIGNAL(clicked(bool)), w.ui->mo_no_fragshader_chk, SLOT(setEnabled(bool)));
    QObject::connect(w.ui->mo_depth_test_chk, SIGNAL(clicked(bool)), &r, SLOT(setMovingObjectsUsesDepthTest(bool)));
    QObject::connect(w.ui->mo_no_fragshader_chk, SIGNAL(clicked(bool)), &r, SLOT(setMovingObjectsNoFragShader(bool)));

    r.show();
    w.show();
    w.focusWidget();

    return a.exec();
}


