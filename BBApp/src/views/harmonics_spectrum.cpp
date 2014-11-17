#include "harmonics_spectrum.h"

HarmonicsSpectrumPlot::HarmonicsSpectrumPlot(Session *sPtr, QWidget *parent) :
    GLSubView(sPtr, parent)
{
    makeCurrent();

    glShadeModel(GL_SMOOTH);
    glClearDepth(1.0);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    context()->format().setDoubleBuffer(true);

    //glGenBuffers(1, &traceVBO);

    doneCurrent();
}

HarmonicsSpectrumPlot::~HarmonicsSpectrumPlot()
{
    makeCurrent();
    //glDeleteBuffers(1, &traceVBO);
    doneCurrent();
}

void HarmonicsSpectrumPlot::resizeEvent(QResizeEvent *)
{
    SetGraticuleDimensions(QPoint(60, 50),
                           QPoint(width() - 80, height() - 100));
}

void HarmonicsSpectrumPlot::paintEvent(QPaintEvent *)
{
    makeCurrent();

    glQClearColor(GetSession()->colors.background);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glEnableClientState(GL_VERTEX_ARRAY);

    DrawGraticule();

    swapBuffers();
    doneCurrent();
}
