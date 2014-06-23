#ifndef DEMOD_IQ_TIME_PLOT_H
#define DEMOD_IQ_TIME_PLOT_H

#include <QOpenGLFunctions>
#include <QGLWidget>

#include "lib/bb_lib.h"
#include "model/session.h"

class DemodIQTimePlot : public QGLWidget, public QOpenGLFunctions {
    Q_OBJECT

public:
    DemodIQTimePlot(Session *session, QWidget *parent = 0);
    ~DemodIQTimePlot();

protected:
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);

private:
    void DrawIQLines();
    void DrawTrace(const GLVector &v);

    Session *session_ptr;

    QPoint grat_sz, grat_ul, grat_ll;
    GLVector grat, gratBorder;
    GLuint gratVBO, gratBorderVBO;

    // I/Q traces
    GLVector traces[2];
    GLuint traceVBO;

public slots:

private slots:

signals:

private:
    DISALLOW_COPY_AND_ASSIGN(DemodIQTimePlot)
};

#endif // DEMOD_IQ_TIME_PLOT_H
