#include <QOpenGLWindow>
#include <QOpenGLFunctions_1_5>
#include <QVector>


#ifndef RENDERWINDOW_H
#define RENDERWINDOW_H

class QOpenGLFramebufferObject;

class RenderWindow : public QOpenGLWindow
{
    Q_OBJECT

public:
    RenderWindow(QWindow *parent = 0);
    ~RenderWindow();

    enum RenderModes { Normal=1, Mode3x=3, Mode8x=8, Mode24x=24 };

    RenderModes renderMode() const { return render_mode; }

    bool isReverseRGB() const { return is_reverse; }

signals:
    void computedFPS(int fps);
    void computedRenderTime(int time_us);

public slots:
    void toggleFullScreen();
    void togglePause();
    void setRenderMode(RenderModes m) { render_mode = m; }
    void setRenderNormal() { render_mode = Normal; }
    void setRenderMode3x() { render_mode = Mode3x; }
    void setRenderMode8x() { render_mode = Mode8x; }
    void setRenderMode24x() { render_mode = Mode24x; }
    void setReverseRGB(bool b) { is_reverse = b; }

protected:
    void initializeGL();
    void paintGL();
    void keyPressEvent(QKeyEvent *);
    bool event(QEvent *);
    void resizeGL(int w, int h);

private:
    QOpenGLFunctions_1_5 *g;
    QOpenGLFramebufferObject *fbo;

    struct Square {
        float x,y,w,h,intensity,angle,vx,vy,intensity_delta,spin;

        Square() : x(0.f), y(0.f), w(.1f), h(.1f), intensity(.75f), angle(0.f), vx(.01f), vy(.01f), intensity_delta(0.f), spin(0.f) {}
    };

    QVector<Square> squares;

    void initSquares();

    double tLastFPS, renderTimeAccum;
    long frameCount, lastFPSFC;
    bool paused;
    RenderModes render_mode;
    bool is_reverse;

    void setColorMask(int subframe_num);
    void unsetColorMask();
    void getColor(int subframe_num, GLfloat intensity, GLubyte c[3]);
};

#endif // RENDERWINDOW_H
