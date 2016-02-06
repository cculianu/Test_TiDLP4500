#include "RenderWindow.h"
#include "Util.h"
#include <QKeyEvent>
#include <QCloseEvent>
#include <QOpenGLFramebufferObject>
#include <QWidget>
#include <QApplication>

RenderWindow::RenderWindow(QWindow *parent)
    : QOpenGLWindow(NoPartialUpdate, parent), g(0)
{
    lastFPSFC = frameCount = 0;
    tLastFPS = 0.; renderTimeAccum = 0.;
    paused = false;
    fbo = 0;
    render_mode = Normal;
    is_reverse = false;
    time_scale = 1.0f;

    setSurfaceType(OpenGLSurface);
    QSurfaceFormat  format;
    format.setProfile(QSurfaceFormat::CompatibilityProfile);
    format.setVersion(1,5);
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    format.setSwapInterval(1);
    QSurfaceFormat::setDefaultFormat(format);
    setFormat(format);
    setTitle("Test Ti 4500 DLP Render Window");
}

RenderWindow::~RenderWindow()
{
    delete g; g = 0;
}

void RenderWindow::initializeGL()
{
    if (!g) {
        g = new QOpenGLFunctions_1_5;
        g->initializeOpenGLFunctions();
    }
    initSquares();

    qDebug("OpenGL: %d.%d", context()->format().majorVersion(), context()->format().minorVersion());
}

void RenderWindow::resizeGL(int w, int h)
{
    (void)w; (void)h;
    makeCurrent();

    if (fbo) delete fbo;
    fbo = new QOpenGLFramebufferObject(QSize(w,h));

    g->glMatrixMode(GL_PROJECTION);
    g->glLoadIdentity();
    g->glOrtho(0.,1.,0.,1.,-1.,1);

}

void RenderWindow::initSquares()
{
    squares.clear();
    squares.resize(40);
    for (int i = 0; i < squares.size(); ++i) {
        Square &s(squares[i]);
        s.x = getRand(0.1,0.9);
        s.y = getRand(0.1,0.9);
        s.angle = getRand(-45.,45.);
        s.intensity = getRand(.5,1.0);
        s.w = getRand(0.01,0.1);
        s.h = getRand(0.01,0.1);
        s.vx = getRand(-0.005,0.005);
        s.vy = getRand(-0.005,0.005);
        s.spin = getRand(-5.,5.);
        s.intensity_delta = getRand(-0.01,0.01);
    }
}

void RenderWindow::togglePause() { paused = !paused; }

void RenderWindow::paintGL()
{
    double now = getTime();

    ++frameCount;

    if (now - tLastFPS >= 1.0) {
        int nframes = frameCount-lastFPSFC;
        int fps = qRound(nframes/(now-tLastFPS));
        emit computedFPS(fps);
        emit computedRenderTime(qRound((renderTimeAccum/nframes)*1e6));
        lastFPSFC = frameCount;
        tLastFPS = now;
        renderTimeAccum = 0.;
    }

    if (!paused) {

        if (!fbo->bind())
            qWarning("QOpenGLFrameBufferObject::bind() returned false!");

        g->glMatrixMode(GL_MODELVIEW);
        g->glLoadIdentity();

        g->glClearColor(0.f,0.f,0.f,1.f);
        g->glClear(GL_COLOR_BUFFER_BIT);

        int nSubframes = (int)render_mode;
        for (int k = 0; k < nSubframes; ++k) {

            setColorMask(k);

            g->glEnableClientState(GL_VERTEX_ARRAY);

            for (int i = 0; i < squares.size(); ++i) {
                Square & s = squares[i];
                g->glPushMatrix();

                GLubyte c[3];
                getColor(k,s.intensity, c);

                g->glColor3ub(c[0],c[1],c[2]);
                g->glTranslatef(s.x,s.y,0.f);
                g->glRotatef(s.angle, 0.f,0.f,1.f);

                GLfloat hw = s.w/2.f, hh = s.h/2.f;

                GLfloat v[] = { -hw, -hh, hw, -hh, hw, hh, -hw, hh };

                g->glVertexPointer(2, GL_FLOAT, 0, v);
                g->glDrawArrays(GL_QUADS, 0, 4);

                float timeScale = (1.0/float(nSubframes)) * time_scale;

                // animate
                s.x += s.vx * timeScale;
                s.y += s.vy * timeScale;
                s.angle += s.spin * timeScale;
                s.intensity += s.intensity_delta * timeScale;

                if (s.angle > 360.f) s.angle -= 360.f;
                if (s.angle < -360.f) s.angle += 360.f;
                // normalize & bounce
                if (s.vx > 0.f && s.x > 1.0f) s.vx = -s.vx;
                if (s.vy > 0.f && s.y > 1.0f) s.vy = -s.vy;
                if (s.vx < 0.f && s.x < 0.f) s.vx = -s.vx;
                if (s.vy < 0.f && s.y < 0.f) s.vy = -s.vy;
                if (s.intensity < 0.f && s.intensity_delta < 0.f) s.intensity_delta = -s.intensity_delta, s.intensity = 0.f;
                if (s.intensity > 1.f && s.intensity_delta > 0.f) s.intensity_delta = -s.intensity_delta, s.intensity = 1.f;

                g->glPopMatrix();
            }
            g->glDisableClientState(GL_VERTEX_ARRAY);
            unsetColorMask();
        }

        if (!fbo->release())
            qWarning("QOpenGLFramebufferObject::release() returned false!");
    } // end if !paused


    g->glEnable(GL_TEXTURE_2D);
    g->glEnableClientState(GL_VERTEX_ARRAY);
    g->glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    g->glColor4f(1.f,1.f,1.f,1.f);
    g->glBindTexture(GL_TEXTURE_2D,fbo->texture());
    g->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    g->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    g->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    g->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


    GLfloat v[] = { 0,0, 1,0, 1,1, 0,1 };
    GLfloat t[] = { 0,0, 1,0, 1,1, 0,1 };

    g->glVertexPointer(2, GL_FLOAT, 0, v);
    g->glTexCoordPointer(2, GL_FLOAT, 0, t);
    g->glDrawArrays(GL_QUADS, 0, 4);;

    g->glDisable(GL_TEXTURE_2D);
    g->glDisableClientState(GL_VERTEX_ARRAY);
    g->glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    renderTimeAccum += getTime()-now;

    update(); ///< Schedule another update after a bufswap?
}

void RenderWindow::setColorMask(int k)
{
    if (render_mode != Normal && is_reverse)  k = (int(render_mode)-1)-k;

    if (render_mode == Mode3x) g->glColorMask(k==0,k==1,k==2,GL_TRUE);
    else {
        g->glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        g->glEnable(GL_COLOR_LOGIC_OP);
        g->glLogicOp(GL_OR);
    }
}

void RenderWindow::unsetColorMask()
{
    g->glDisable(GL_COLOR_LOGIC_OP);
    g->glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

void RenderWindow::getColor(int k, GLfloat intensity, GLubyte c[3])
{
    if (intensity < 0.f) intensity = 0.f;
    if (intensity > 1.f) intensity = 1.f;
    if (render_mode != Normal && is_reverse)  k = (int(render_mode)-1)-k;

    GLubyte tmp = 0, shift = 0;

    switch(render_mode) {
    case Mode3x:
        c[0] = c[1] = c[2] = 0;
        c[k] = intensity*255.f; // scale to 8-bit
        break;
    case Mode8x:
        c[0] = c[1] = c[2] = 0;
        tmp = intensity*8.f; // scale to 3-bit
        tmp = tmp & 0x7; // make sure it's only bottom-three bits
        shift = (7-k);
        c[0] = (tmp>>2&0x1) << shift;
        c[1] = (tmp>>1&0x1) << shift;
        c[2] = (tmp>>0&0x1) << shift;
        break;
    case Mode24x:
        c[0] = c[1] = c[2] = 0;
        tmp = intensity > 1.f/255.f ? 0x1 : 0x0;
        tmp = tmp&0x1; // ensure just bottom 1 bit is set
        shift = (7-(k%8));
        if (k < 8) c[0] = tmp<<shift;
        else if (k < 16) c[1] = tmp<<shift;
        else if (k < 24) c[2] = tmp<<shift;
        break;
    default: c[0] = c[1] = c[2] = intensity*255.f; break;
    }
}

void RenderWindow::toggleFullScreen() {
    if (windowState() & Qt::WindowFullScreen)  showNormal();
    else showFullScreen();
}

void RenderWindow::keyPressEvent(QKeyEvent *k)
{
    if (k->key() == Qt::Key_Escape) { if (windowState()&Qt::WindowFullScreen) showNormal(); }
    else if (k->key() == Qt::Key_Space) togglePause();
    else if ( (k->key() == Qt::Key_Return || k->key() == Qt::Key_Enter)
              && (qApp->keyboardModifiers()&(Qt::AltModifier|Qt::ControlModifier)))
        toggleFullScreen();
    else QOpenGLWindow::keyPressEvent(k);
}

bool RenderWindow::event(QEvent *e)
{
    if (dynamic_cast<QCloseEvent *>(e)) { e->ignore(); return false; }
    return QOpenGLWindow::event(e);
}
