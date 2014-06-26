#ifndef DEMOD_IQ_TIME_PLOT_H
#define DEMOD_IQ_TIME_PLOT_H

#include "gl_sub_view.h"

#include "lib/bb_lib.h"

class DemodIQTimePlot : public GLSubView {
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
    void DrawPlotText();

    QPoint grat_sz, grat_ul, grat_ll;
    GLVector grat, gratBorder;
    GLuint gratVBO, gratBorderVBO;

    // I/Q traces
    GLVector traces[2];
    GLuint traceVBO;

    GLFont textFont;

public slots:

private slots:

signals:

private:
    DISALLOW_COPY_AND_ASSIGN(DemodIQTimePlot)
};

#endif // DEMOD_IQ_TIME_PLOT_H
