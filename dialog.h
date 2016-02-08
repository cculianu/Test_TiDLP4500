#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();


    Ui::Dialog *ui;

signals:
    void ftrackParamsChanged(int posx, int posy, int size, float intensity);

public slots:
    void updateFPSLabel(int fps);
    void updateRenderTimeLabel(int time_us);

protected:
    void closeEvent(QCloseEvent *);
    void hideEvent(QHideEvent *);
    void keyPressEvent(QKeyEvent *);

private slots:
    void on_timeSlider_valueChanged(int value);

    void broadcastFTCtlsUpdate();
};

#endif // DIALOG_H
