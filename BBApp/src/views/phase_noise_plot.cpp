#include "phase_noise_plot.h"

PhaseNoisePlot::PhaseNoisePlot(Session *sPtr, QWidget *parent) :
    GLSubView(sPtr, parent),
    textFont(12),
    divFont(12),
    startDecade(2),
    stopDecade(6)
{
    makeCurrent();

    glShadeModel(GL_SMOOTH);
    glClearDepth(1.0);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    context()->format().setDoubleBuffer(true);

    glGenBuffers(1, &traceBufferObject);

    doneCurrent();
}

PhaseNoisePlot::~PhaseNoisePlot()
{
    makeCurrent();

    // Delete GL objects
    glDeleteBuffers(1, &traceBufferObject);

    doneCurrent();
}

void PhaseNoisePlot::resizeEvent(QResizeEvent *)
{
    SetGraticuleDimensions(QPoint(60, 50),
                           QPoint(width() - 80, height() - 100));
}

void PhaseNoisePlot::paintEvent(QPaintEvent *)
{
    makeCurrent();

    glQClearColor(GetSession()->colors.background);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glEnableClientState(GL_VERTEX_ARRAY);

    BuildGraticule();

    SetGraticuleDimensions(QPoint(60, 50),
                           QPoint(width() - 80, height() - 100));

    if(grat_sz.x() >= 600) {
        textFont = GLFont(14);
    } else if(grat_sz.x() <= 350) {
        textFont = GLFont(8);
    } else {
        int mod = (600 - grat_sz.x()) / 50;
        textFont = GLFont(13 - mod);
    }

    // Calculate dimensions for the presence of the title bar
    if(!GetSession()->GetTitle().isNull()) {
        grat_ul -= QPoint(0, 20);
        grat_sz -= QPoint(0, 20);
    }

    glViewport(0, 0, width(), height());

    DrawGraticule();
    DrawTraces();

    glDisable(GL_DEPTH_TEST);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //DrawMarkers();
    //DrawGratText();

    swapBuffers();
    doneCurrent();
}

void PhaseNoisePlot::DrawTraces()
{
    // Prep viewport
    glPushAttrib(GL_VIEWPORT_BIT);
    glViewport(grat_ll.x(), grat_ll.y(),
               grat_sz.x(), grat_sz.y());

    // Prep modelview
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Ortho
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);

    // Nice lines, doesn't smooth quads
    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glLineWidth(GetSession()->prefs.trace_width);

    normalize_trace(&trace, normalizedTrace, grat_sz);
    DrawTrace(&trace, normalizedTrace, traceBufferObject);

    // Disable nice lines
    glLineWidth(1.0);
    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glPopAttrib();
}

void PhaseNoisePlot::DrawTrace(const Trace *t, const GLVector &v, GLuint vbo)
{
    if(v.size() < 1) {
        return;
    }

    glColor3f(0.0, 0.0, 0.0);

    // Put the trace in the vbo
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, v.size()*sizeof(float),
                 &v[0], GL_DYNAMIC_DRAW);
    glVertexPointer(2, GL_FLOAT, 0, INDEX_OFFSET(0));

    // Draw fill
    glDrawArrays(GL_TRIANGLE_STRIP, 0, v.size() / 2);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Draw lines
    glVertexPointer(2, GL_FLOAT, 16, INDEX_OFFSET(0));
    glDrawArrays(GL_LINE_STRIP, 0, v.size() / 4);
    glVertexPointer(2, GL_FLOAT, 16, INDEX_OFFSET(8));
    glDrawArrays(GL_LINE_STRIP, 0, v.size() / 4);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Unbind array
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void PhaseNoisePlot::BuildGraticule()
{
    int divisions = (stopDecade - startDecade);

    std::vector<float> points;

    for(int i = 0; i <= 10; i++) {
        points.push_back(0.0);
        points.push_back(0.1 * i);
        points.push_back(1.0);
        points.push_back(0.1 * i);
    }

    for(int d = 0; d < divisions; d++) {
        float offset = (float)d / (float)divisions;
        for(int i = 0; i < 10; i++) {
            double x = log10(i) / (double)divisions + offset;
            points.push_back(x);
            points.push_back(0.0);
            points.push_back(x);
            points.push_back(1.0);
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, gratVBO);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(float),
                 &points[0], GL_DYNAMIC_DRAW);

    innerGratPoints = points.size();
}

void PhaseNoisePlot::DrawGratText()
{

}

void PhaseNoisePlot::DrawMarkers()
{

}
