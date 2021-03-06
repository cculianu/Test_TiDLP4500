#include "RenderWindow.h"
#include "Util.h"
#include <QKeyEvent>
#include <QCloseEvent>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QWidget>
#include <QApplication>
#include <math.h>
#include <QIcon>

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
    mo_no_fragshader = mo_depth_test = no_fbo = false;
    time_scale = 1.0f;
    mgtex = 0;
    phase = 0.f; spatial_freq = 5.f; temp_freq = 1.f, angle = 0.f;

    ftrack = false;
    ftrack_x = 776; ftrack_y = 4; ftrack_len = 20; ftrack_int = 1.0f;


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
    setIcon(QIcon(":/icons/icon.png"));
}

RenderWindow::~RenderWindow()
{
    makeCurrent();
    if (mgtex) g->glDeleteTextures(1, &mgtex), mgtex = 0;
    if (fbo) delete fbo, fbo = 0;
    if (fragShader) delete fragShader, fragShader = 0;
    if (g) delete g, g = 0;
}

#if defined(Q_OS_WIN) /* Windows */
typedef BOOL (APIENTRY *wglswapfn_t)(int);

static void setVSyncMode(QOpenGLContext *c, bool vsync)
{
    wglswapfn_t wglSwapIntervalEXT = (wglswapfn_t)c->getProcAddress( "wglSwapIntervalEXT" );
    if( wglSwapIntervalEXT ) {
        wglSwapIntervalEXT(vsync ? 1 : 0);
        //qDebug("VSync explicitly set to %s",vsync?"ON":"OFF");
    } else {
        qWarning("VSync could not be altered because wglSwapIntervalEXT is missing.");
    }
}
#else
static void setVSyncMode(QOpenGLContext *c, bool vsync)
{
    (void)c; (void)vsync;
    qDebug("Cannot set VSync.  This only works on Windows for now.");
}
#endif

void RenderWindow::setNoVSync(bool b)
{
    QOpenGLContext *oldContext = QOpenGLContext::currentContext();
    QSurface *oldSurface = oldContext ? oldContext->surface() : 0;

    makeCurrent();
    setVSyncMode(context(), !b);

    if (oldContext) oldContext->makeCurrent(oldSurface);
}

void RenderWindow::initializeGL()
{
    qDebug("OpenGL: %d.%d", context()->format().majorVersion(), context()->format().minorVersion());

    setNoVSync(false);

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
        emit computedVFPS(fps*int(render_mode));
        emit computedRenderTime(qRound((renderTimeAccum/nframes)*1e6));
        lastFPSFC = frameCount;
        tLastFPS = now;
        renderTimeAccum = 0.;
    }

    if (!paused) {

        if (!no_fbo && !fbo->bind())
            qWarning("QOpenGLFrameBufferObject::bind() returned false!");

        const bool enable_depth = mode == MovingObjects && mo_depth_test;

        g->glMatrixMode(GL_MODELVIEW);
        g->glLoadIdentity();

        g->glClearColor(0.f,0.f,0.f,1.f);
        g->glClear(GL_COLOR_BUFFER_BIT | (enable_depth ? GL_DEPTH_BUFFER_BIT : 0));

        if (enable_depth)
            g->glEnable(GL_DEPTH_TEST);

        const int nSubframes = (int)render_mode;

        if (mode != MovingObjects || !mo_no_fragshader) {
            // movinggrating mode always uses frag shader
            // movingobjects optionally does the color computation using getColor below..
            if (!fragShader->bind()) qWarning("Error binding frag shader");
            fragShader->setUniformValue("renderMode", int(render_mode));
        }

        for (int k = 0; k < nSubframes; ++k) {

            setColorMask(k);

            if (mode == MovingObjects) {

                g->glEnableClientState(GL_VERTEX_ARRAY);

                int i_init = 0,
                    i_incr = 1;
                float depth = 0.f, depth_incr = 0.f;
                if (enable_depth) {
                    i_init = squares.size()-1;
                    i_incr = -1;
                    depth = -0.95f;
                    depth_incr = 1.9f/float(squares.size());
                    g->glDepthRange(-1,1);
                    g->glDepthFunc(GL_LEQUAL);
                }

                for (int i = i_init; i < squares.size() && i > -1; i += i_incr, depth += depth_incr)
                {
                    Square & s = squares[i];
                    g->glPushMatrix();

                    if (mo_no_fragshader) {
                        GLubyte c[3];
                        getColor(k, s.intensity, c);
                        g->glColor3ub(c[0],c[1],c[2]);
                    } else {
                        fragShader->setUniformValue("sampleTexture", false);
                        fragShader->setUniformValue("color", QVector4D(s.intensity, s.intensity, s.intensity, 1.0));
                    }
                    g->glTranslatef(s.x,s.y,depth);
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

        if (!no_fbo && !fbo->release())
            qWarning("QOpenGLFramebufferObject::release() returned false!");

        g->glDisable(GL_DEPTH_TEST);

    } // end if !paused


    if (!no_fbo) {
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
        g->glDrawArrays(GL_QUADS, 0, 4);

        g->glDisable(GL_TEXTURE_2D);
        g->glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        g->glDisableClientState(GL_VERTEX_ARRAY);
    }

    if (ftrack) {
        float w = width(), h = height();
        float x = ftrack_x/w, y = ftrack_y/h, sx = (ftrack_len/w)*.5f, sy = (ftrack_len/h)*.5f;

        if (frameCount % 2)
            g->glColor4f(ftrack_int, ftrack_int, ftrack_int, 1.f);
        else
            g->glColor4f(0.f,0.f,0.f, 1.f);

        GLfloat v[] = { x-sx,y-sy,  x+sx,y-sy, x+sx,y+sy, x-sx,y+sy };

        g->glEnableClientState(GL_VERTEX_ARRAY);

        g->glVertexPointer(2, GL_FLOAT, 0, v);
        g->glDrawArrays(GL_QUADS, 0, 4);

        g->glColor4f(1.f,1.f,1.f,1.f);
        g->glDisableClientState(GL_VERTEX_ARRAY);
    }

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
        tmp = intensity >= 0.5f ? 0x1 : 0x0;
        tmp = tmp&0x1; // ensure just bottom 1 bit is set
        shift = (7-(k%8));
        if (k < 8) c[0] = tmp<<shift;
        else if (k < 16) c[1] = tmp<<shift;
        else if (k < 24) c[2] = tmp<<shift;
        break;
    default: c[0] = c[1] = c[2] = intensity*255.f; break;
    }
}

void RenderWindow::setFrameTrackParams(int posx, int posy, int size, float intensity)
{
    if (size <= 0 || intensity < 0.f || intensity > 1.f) return;
    ftrack_x = posx; ftrack_y = posy; ftrack_len = size; ftrack_int = intensity;
}

