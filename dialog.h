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

public slots:
    void updateFPSLabel(int fps);
    void updateRenderTimeLabel(int time_us);

protected:
    void closeEvent(QCloseEvent *);
    void hideEvent(QHideEvent *);
private slots:
    void on_timeSlider_valueChanged(int value);
};

#endif // DIALOG_H
