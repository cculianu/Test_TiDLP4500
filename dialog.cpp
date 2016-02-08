#include "dialog.h"
#include "ui_dialog.h"
#include <QCloseEvent>
#include <QHideEvent>
#include <QKeyEvent>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    connect(ui->noneRadio, SIGNAL(clicked(bool)), ui->reverseChk, SLOT(setDisabled(bool)));
    connect(ui->tripleRadio, SIGNAL(clicked(bool)), ui->reverseChk, SLOT(setEnabled(bool)));
    connect(ui->octupleRadio, SIGNAL(clicked(bool)), ui->reverseChk, SLOT(setEnabled(bool)));
    connect(ui->dodekaQuadrupleRadio, SIGNAL(clicked(bool)), ui->reverseChk, SLOT(setEnabled(bool)));
    connect(ui->ftposxLE, SIGNAL(textChanged(QString)), this, SLOT(broadcastFTCtlsUpdate()));
    connect(ui->ftposyLE, SIGNAL(textChanged(QString)), this, SLOT(broadcastFTCtlsUpdate()));
    connect(ui->ftsizeLE, SIGNAL(textChanged(QString)), this, SLOT(broadcastFTCtlsUpdate()));
    connect(ui->ftintLE, SIGNAL(textChanged(QString)), this, SLOT(broadcastFTCtlsUpdate()));
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

void Dialog::on_timeSlider_valueChanged(int value)
{
    ui->timeScaleLbl->setText(QString::number(value));
}

void Dialog::broadcastFTCtlsUpdate() {
    int x,y,size;
    float intensity;
    bool ok = false;

    x = ui->ftposxLE->text().toInt(&ok); if (!ok) return;
    y = ui->ftposyLE->text().toInt(&ok); if (!ok) return;
    size = ui->ftsizeLE->text().toInt(&ok); if (!ok) return;
    intensity = ui->ftintLE->text().toFloat(&ok); if (!ok) return;

    emit ftrackParamsChanged(x,y,size,intensity);
}

void Dialog::keyPressEvent(QKeyEvent *e)
{
    // this is so that when user hits enter inside the line edits, the default buttons on the dialog don't get activated!!
    QWidget *w = focusWidget();
    if ( (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
         && (w==ui->ftintLE || w==ui->ftposxLE || w==ui->ftposyLE || w==ui->ftsizeLE)) {
        ui->pauseBut->setFocus();
        broadcastFTCtlsUpdate();
        e->ignore();
    } else
        QDialog::keyPressEvent(e);
}
