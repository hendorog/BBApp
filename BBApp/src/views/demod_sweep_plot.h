#ifndef DEMOD_SWEEP_PLOT_H
#define DEMOD_SWEEP_PLOT_H

#include "gl_sub_view.h"
#include "lib/bb_lib.h"

class DemodSweepPlot : public GLSubView {
    Q_OBJECT

public:
    DemodSweepPlot(Session *session, QWidget *parent = 0);
    ~DemodSweepPlot();

protected:
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);

private:
    void DemodAndDraw();
    void DrawPlotText();
    void DrawTrace(const GLVector &v);

    QPoint grat_sz, grat_ul, grat_ll;
    GLVector grat, gratBorder;
    GLuint gratVBO, gratBorderVBO;

    GLFont textFont, divFont;

    GLVector trace;
    GLuint traceVBO;

public slots:

private slots:

signals:

private:
    DISALLOW_COPY_AND_ASSIGN(DemodSweepPlot)
};

#endif // DEMOD_SWEEP_PLOT_H
