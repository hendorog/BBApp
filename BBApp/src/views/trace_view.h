#ifndef TRACE_VIEW_H
#define TRACE_VIEW_H

#include <QOpenGLFunctions>
#include <QGLWidget>
#include <QTime>
#include <QFont>
#include <QFontMetrics>
#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <QApplication>

#include "../lib/bb_lib.h"
#include "persistence_view.h"

#define OFFSET(x) ((GLvoid*)x)
#define MAX_WATERFALL_LINES 128

class Session;
class SwapThread;
class Trace;

inline void glQColor(QColor c) {
    glColor3f(c.redF(), c.greenF(), c.blueF());
}
inline void glQClearColor(QColor c, float alpha = 0.0) {
    glClearColor(c.redF(), c.greenF(), c.blueF(), alpha);
}

class TraceView : public QGLWidget, public QOpenGLFunctions {
    Q_OBJECT
    friend class SwapThread;

    // margin in pixels
    static const int MARGIN = 20;

public:
    enum TextAlignment {
        LEFT_ALIGNED, RIGHT_ALIGNED, CENTER_ALIGNED
    };

    TraceView(Session *session, QWidget *parent = 0);
    ~TraceView();

protected:
    bool InitPersistFBO();

    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent *e);

    void Paint();

    void RenderGraticule();
    void RenderGratText();
    void RenderTraces();
    void DrawTrace(const Trace *t, const GLVector &v);
    void RenderMarkers();
    void DrawMarker(int x, int y, int num);
    void DrawDeltaMarker(int x, int y, int num);
    void RenderChannelPower();
    void DrawPersistence();
    void DrawLimitLines(const Trace *limitTrace, const GLVector &v);

    void AddToPersistence(const GLVector &v);
    void AddToWaterfall(const GLVector &v);
    void ClearWaterfall();
    void DrawWaterfall();

private:
    bool PointInGrat(const QPoint &p) const {
        return QRect(grat_ul.x(), height() - grat_ul.y(),
                     grat_sz.x(), grat_sz.y()).contains(p);
    }

    // Strings are subject to matrix transformations ?
    void DrawString(const QString &s, const QFont &f,
                    QPoint p, TextAlignment align);
    void DrawString(const QString &s, const QFont &f,
                    int x, int y, TextAlignment align) {
        DrawString(s, f, QPoint(x, y), align);
    }
    // Get width of string in pixels
    int GetTextWidth(const QString &s, const QFont &f) const {
        QFontMetrics metrics(f);
        return metrics.width(s);
    }
    // Flips point so (0,0) is lower left corner
    QPoint WindowToGLSpace(const QPoint &p) const {
        return QPoint(p.x() - grat_ll.x(), p.y() - grat_ll.y());
    }

    Session *session_ptr; // Pointer copy, does not own
    QTime time; // Used for sweep time display
    GLVector graticule, grat_border; // Point lists for graticule
    GLuint gratVBO, borderVBO; // GL VBOs for graticule

    QPoint grat_sz, grat_ul, grat_ll; // Dimensions of grat

    // Members for switch context is separate thread
    SwapThread *swap_thread;
    //QMutex drawMutex;
    std::mutex drawMutex;
    semaphore paintCondition;

    GLVector traces[TRACE_COUNT]; // Normalized traces
    GLuint traceVBO, textureVBO;

    QString plotTitle;
    QFont textFont, divFont;

    bool persist_on;
    bool clear_persistence; // When true, clears buffer next frame update
    GLProgram *persist_program; // Shaders for persistence
    GLuint persist_fbo; // Frame-Buffer-Object for persistent
    GLuint persist_depth; // FBO depth buffer
    GLuint persist_tex; // Offscreen persist buffer

    WaterfallState waterfall_state;
    GLuint waterfall_tex; // Waterfall spectrum texture
    std::vector<GLVector*> waterfall_verts;
    std::vector<GLVector*> waterfall_coords;

public slots:
    void enablePersistence(int enable) { persist_on = (enable == Qt::Checked); }
    void clearPersistence() { clear_persistence = true; }
    // New state must be between [0,2]
    void enableWaterfall(int new_state) {
        waterfall_state = (WaterfallState)new_state;
        update(); }
};

/*
 * Responsible for swapping the openGL buffers in a
 *  separate thread
 */
class SwapThread : public QThread {
    Q_OBJECT

public:
    SwapThread(TraceView *view)
        : trace_view(view) { running = true; }
    ~SwapThread() { Stop(); }
    void Stop() { running = false; }

protected:
    // Context gets released in main thread, moved to this thread
    // Swap buffers takes place, then context is moved back to main thread
    void run() {
        QThread *mainThread = QApplication::instance()->thread();
        QGLContext *cntx = trace_view->context();

        while(running) {
            trace_view->paintCondition.wait();
            if(!running) {
                return;
            }
            trace_view->drawMutex.lock();
            // Dont do anything unless the context belongs to this thread
            if(cntx->contextHandle()->thread() == this) {
                cntx->makeCurrent();
                cntx->swapBuffers();
                cntx->doneCurrent();
                cntx->moveToThread(mainThread);
            }

            trace_view->drawMutex.unlock();
        }
    }

    TraceView *trace_view; // Does not own
    bool running;
};

#endif // TRACE_VIEW_H
