#ifndef DEMOD_SWEEP_PLOT_H
#define DEMOD_SWEEP_PLOT_H

#include "gl_sub_view.h"
#include "lib/bb_lib.h"
#include "widgets/entry_widgets.h"

#include <QToolBar>

enum DemodType {
    DemodTypeAM = 0,
    DemodTypeFM = 1,
    DemodTypePM = 2
};

// AM/FM/PM Demod Plot Window
class DemodSweepPlot : public GLSubView {
    Q_OBJECT

public:
    DemodSweepPlot(Session *session, QWidget *parent = 0);
    ~DemodSweepPlot();

protected:
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);

private:
    void DemodAndDraw();
    void DrawPlotText();
    void DrawMarkers();
    void DrawTrace(const GLVector &v);
    void DrawMarker(int x, int y, int num);
    void DrawDeltaMarker(int x, int y, int num);

    QPoint grat_sz, grat_ul, grat_ll;
    GLVector grat, gratBorder;
    GLuint gratVBO, gratBorderVBO;

    GLFont textFont, divFont;

    GLVector trace;
    GLuint traceVBO;

    DemodType demodType;

    bool markerOn, deltaOn;
    float markerVal, deltaVal;
    QPointF markerPos, deltaPos;

public slots:
    void changeDemod(int newDemod) {
        demodType = (DemodType)newDemod;
        update();
    }

    void disableMarker() {
        markerOn = false;
        update();
    }

    void toggleDelta() {
        deltaOn = !deltaOn;
        if(deltaOn) {
            deltaPos = markerPos;
            deltaVal = markerVal;
        }
        update();
    }

private slots:

signals:

private:
    DISALLOW_COPY_AND_ASSIGN(DemodSweepPlot)
};

//// Container for the plot and Demod ToolBar
//class DemodSweepArea : public QWidget {
//    Q_OBJECT

//public:
//    DemodSweepArea(Session *session, QWidget *parent = 0);
//    ~DemodSweepArea() {}

//    operator QGLWidget*() {
//        return plot; }

//protected:
//    void resizeEvent(QResizeEvent *)
//    {
//        toolBar->resize(width(), 30);
//        plot->resize(width(), height() - 30);
//    }

//    void paintEvent(QPaintEvent *)
//    {
//        plot->update();
//    }

//private:
//    QToolBar *toolBar;
//    DemodSweepPlot *plot;
//};

#endif // DEMOD_SWEEP_PLOT_H
