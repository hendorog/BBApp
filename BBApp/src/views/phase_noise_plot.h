#ifndef PHASE_NOISE_PLOT_H
#define PHASE_NOISE_PLOT_H

#include "gl_sub_view.h"

class PhaseNoisePlot : public GLSubView {
    Q_OBJECT

public:
    PhaseNoisePlot(Session *sPtr, QWidget *parent = 0);
    ~PhaseNoisePlot();

    void SetDecades(int start, int stop)
    {
        startDecade = start;
        stopDecade = stop;
    }

    Trace trace;

protected:
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);

private:
    void BuildGraticule();
    void DrawTraces();
    void DrawTrace(const Trace *t, const GLVector &v, GLuint vbo);
    void DrawGratText();
    void DrawMarkers();

    GLuint traceBufferObject;
    GLVector normalizedTrace;

    GLFont textFont, divFont;
    int startDecade, stopDecade;

private:
    Q_DISABLE_COPY(PhaseNoisePlot)
};

#endif // PHASE_NOISE_PLOT_H
