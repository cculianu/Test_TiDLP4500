#include "RenderWindow.h"
#include "Util.h"
#include <QKeyEvent>
#include <QCloseEvent>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QWidget>
#include <QApplication>
#include <math.h>

RenderWindow::RenderWindow(QWindow *parent)
    : QOpenGLWindow(NoPartialUpdate, parent), g(0)
{
    lastFPSFC = frameCount = 0;
    tLastFPS = 0.; renderTimeAccum = 0.;
    paused = false;
    fbo = 0;
    fragShader = 0;
    render_mode = Normal;
    mode = MovingObjects;
    is_reverse = false;
    time_scale = 1.0f;
    mgtex = 0;
    phase = 0.f; spatial_freq = 5.f; temp_freq = 1.f, angle = 0.f;

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
    makeCurrent();
    if (mgtex) g->glDeleteTextures(1, &mgtex), mgtex = 0;
    if (fbo) delete fbo, fbo = 0;
    if (fragShader) delete fragShader, fragShader = 0;
    if (g) delete g, g = 0;
}

void RenderWindow::initializeGL()
{
    qDebug("OpenGL: %d.%d", context()->format().majorVersion(), context()->format().minorVersion());

    if (!g) {
        g = new QOpenGLFunctions_1_5;
        g->initializeOpenGLFunctions();
    }
    if (!fragShader) fragShader = new QOpenGLShaderProgram(this);
    initFragShader();
    initSquares();
    initMGTex();

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
void RenderWindow::initMGTex()
{
    if (mgtex) g->glDeleteTextures(1, &mgtex), mgtex = 0;
    g->glGenTextures(1, &mgtex);
    const int NCOLORS = 256;
    GLubyte pix[NCOLORS];
    for (int i = 0; i < NCOLORS; ++i) {
        pix[i] = qRound(((::cosf(GLfloat(i)/GLfloat(NCOLORS-1)*2.0f*M_PI)+1.0f)/2.0f) * 255);
    }
    g->glBindTexture(GL_TEXTURE_1D, mgtex);
    g->glTexImage1D(GL_TEXTURE_1D, 0, GL_LUMINANCE, NCOLORS, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, pix);
    g->glBindTexture(GL_TEXTURE_1D, 0);
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

        if (!fragShader->bind()) qWarning("Error binding frag shader");
        fragShader->setUniformValue("renderMode", int(render_mode));

        for (int k = 0; k < nSubframes; ++k) {

            setColorMask(k);

            if (mode == MovingObjects) {

                g->glEnableClientState(GL_VERTEX_ARRAY);

                for (int i = 0; i < squares.size(); ++i) {
                    Square & s = squares[i];
                    g->glPushMatrix();

                    fragShader->setUniformValue("sampleTexture", false);
                    fragShader->setUniformValue("color", QVector4D(s.intensity, s.intensity, s.intensity, 1.0));
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

            } else if (mode == MovingGrating) {

                float dT = (1.f/60.f) * (time_scale/float(nSubframes));
                phase += dT * temp_freq;
                phase = fmod(phase, 1.0);

                fragShader->setUniformValue("color", QVector4D(1.,1.,1., 1.0));
                fragShader->setUniformValue("sampleTexture", true);

                g->glPushMatrix();

                g->glTranslatef( 0.5f, 0.5f, 0);
                g->glRotatef( angle, 0.0, 0.0, 1.0 );


                g->glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
                g->glEnable(GL_TEXTURE_1D);
                g->glBindTexture(GL_TEXTURE_1D, mgtex);
                g->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                g->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                g->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                g->glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);

                g->glEnableClientState(GL_VERTEX_ARRAY);
                g->glEnableClientState(GL_TEXTURE_COORD_ARRAY);

                GLfloat v[] = { -.5f,-.5f, .5f,-.5f, .5f,.5f, -.5f,.5f };
                const float bw = (spatial_freq <= 0. ? 1.0 : 1.0/spatial_freq);
                GLfloat t[] = { 0.f+phase, 1.f/bw+phase, 1.f/bw+phase, 0.f+phase };

                g->glVertexPointer(2, GL_FLOAT, 0, v);
                g->glTexCoordPointer(1, GL_FLOAT, 0, t);

                g->glDrawArrays(GL_QUADS, 0, 4);

                g->glDisableClientState(GL_VERTEX_ARRAY);
                g->glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                g->glDisable(GL_TEXTURE_1D);

                g->glPopMatrix();
            }
            unsetColorMask();
        }

        fragShader->release();

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

    fragShader->setUniformValue("subFrame", k);

    if (render_mode == Mode3x) {
        g->glColorMask(k==0,k==1,k==2,GL_TRUE);
    } else if (render_mode != Normal) {
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

void RenderWindow::initFragShader()
{
    bool wasOK =
    fragShader->addShaderFromSourceFile(QOpenGLShader::Fragment,":/shaders/subframeshader.fsh");

    if (!wasOK) {
        qWarning("Error compiling shader: %s", fragShader->log().toUtf8().constData());
        return;
    }
    if (!fragShader->link()) {
        qWarning("Error linking shader: %s", fragShader->log().toUtf8().constData());
        return;
    }
}
