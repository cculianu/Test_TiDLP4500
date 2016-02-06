#include "dialog.h"
#include "ui_dialog.h"
#include <QCloseEvent>
#include <QHideEvent>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    connect(ui->noneRadio, SIGNAL(clicked(bool)), ui->reverseChk, SLOT(setDisabled(bool)));
    connect(ui->tripleRadio, SIGNAL(clicked(bool)), ui->reverseChk, SLOT(setEnabled(bool)));
    connect(ui->octupleRadio, SIGNAL(clicked(bool)), ui->reverseChk, SLOT(setEnabled(bool)));
    connect(ui->dodekaQuadrupleRadio, SIGNAL(clicked(bool)), ui->reverseChk, SLOT(setEnabled(bool)));
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::closeEvent(QCloseEvent *e)
{
    qApp->quit();
    e->accept();
}

void Dialog::hideEvent(QHideEvent *e)
{
    qApp->quit();
    e->ignore();
}

void Dialog::updateFPSLabel(int fps)
{
    ui->fpsLbl->setText(QString::number(fps));
}

void Dialog::updateRenderTimeLabel(int time_us)
{
    ui->renderTimeLbl->setText(QString::number(double(time_us/1000.0)));
}
