#include "demod_iq_time_plot.h"

DemodIQTimePlot::DemodIQTimePlot(Session *session, QWidget *parent) :
    GLSubView(session, parent),
    textFont(14),
    divFont(12)
{
    for(int i = 0; i < 11; i++) {
        grat.push_back(0.0);
        grat.push_back(0.1 * i);
        grat.push_back(1.0);
        grat.push_back(0.1 * i);
    }

    for(int i = 0; i < 11; i++) {
        grat.push_back(0.1 * i);
        grat.push_back(0.0);
        grat.push_back(0.1 * i);
        grat.push_back(1.0);
    }

    gratBorder.push_back(0.0);
    gratBorder.push_back(0.0);
    gratBorder.push_back(1.0);
    gratBorder.push_back(0.0);
    gratBorder.push_back(1.0);
    gratBorder.push_back(1.0);
    gratBorder.push_back(0.0);
    gratBorder.push_back(1.0);
    gratBorder.push_back(0.0);
    gratBorder.push_back(0.0);

    makeCurrent();

    glShadeModel(GL_SMOOTH);
    glClearDepth(1.0);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    context()->format().setDoubleBuffer(true);

    glGenBuffers(1, &traceVBO);
    glGenBuffers(1, &gratVBO);
    glGenBuffers(1, &gratBorderVBO);

    glBindBuffer(GL_ARRAY_BUFFER, gratVBO);
    glBufferData(GL_ARRAY_BUFFER, grat.size()*sizeof(float),
                 &grat[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, gratBorderVBO);
    glBufferData(GL_ARRAY_BUFFER, gratBorder.size()*sizeof(float),
                 &gratBorder[0], GL_STATIC_DRAW);

    doneCurrent();
}

DemodIQTimePlot::~DemodIQTimePlot()
{
    makeCurrent();

    glDeleteBuffers(1, &traceVBO);
    glDeleteBuffers(1, &gratVBO);
    glDeleteBuffers(1, &gratBorderVBO);

    doneCurrent();
}

void DemodIQTimePlot::resizeEvent(QResizeEvent *)
{
    grat_ll = QPoint(60, 50);
    grat_ul = QPoint(60, size().height() - 50);

    grat_sz = QPoint(size().width() - 80,
                     size().height() - 100);
}

void DemodIQTimePlot::paintEvent(QPaintEvent *)
{
    makeCurrent();

    glQClearColor(GetSession()->colors.background);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glEnableClientState(GL_VERTEX_ARRAY);

    if(grat_sz.x() >= 600) {
        textFont = GLFont(14);
    } else if(grat_sz.x() <= 350) {
        textFont = GLFont(8);
    } else {
        int mod = (600 - grat_sz.x()) / 50;
        textFont = GLFont(13 - mod);
    }

    int textHeight = textFont.GetTextHeight() + 2;
    grat_ll.setY(textHeight * 2);
    grat_ul.setY(height() - (textHeight*2));
    grat_sz.setY(height() - (textHeight*4));

    glViewport(0, 0, width(), height());

    // Model view for graticule
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(grat_ll.x(), grat_ll.y(), 0);
    glScalef(grat_sz.x(), grat_sz.y(), 1.0);

    // Projection for graticule
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, size().width(), 0, size().height(), -1, 1);

    glLineWidth(GetSession()->prefs.graticule_width);
    glQColor(GetSession()->colors.graticule);

    // Draw inner grat
    if(GetSession()->prefs.graticule_stipple) {
        glLineStipple(1, 0x8888);
        glEnable(GL_LINE_STIPPLE);
    }

    glBindBuffer(GL_ARRAY_BUFFER, gratVBO);
    glVertexPointer(2, GL_FLOAT, 0, INDEX_OFFSET(0));
    glDrawArrays(GL_LINES, 0, grat.size()/2);

    if(GetSession()->prefs.graticule_stipple) {
        glDisable(GL_LINE_STIPPLE);
    }

    // Border
    glBindBuffer(GL_ARRAY_BUFFER, gratBorderVBO);
    glVertexPointer(2, GL_FLOAT, 0, INDEX_OFFSET(0));
    glDrawArrays(GL_LINE_STRIP, 0, gratBorder.size()/2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glLineWidth(1.0);

    DrawIQLines();

    glDisable(GL_DEPTH_TEST);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    DrawPlotText();

    swapBuffers();
    doneCurrent();
}

void DemodIQTimePlot::DrawIQLines()
{
    traces[0].clear();
    traces[1].clear();

    const IQSweep &sweep = GetSession()->iq_capture;
    const std::vector<complex_f> &iq = sweep.iq;

    if(sweep.iq.size() <= 0) return;

    glColor3f(1.0, 0.0, 0.0);

    for(int i = 0; i < sweep.sweepLen; i++) {
        traces[0].push_back(i);
        traces[0].push_back(iq[i].re);
        traces[1].push_back(i);
        traces[1].push_back(iq[i].im);
    }

    float max = 0.0;
    for(int i = 0; i < sweep.sweepLen; i++) {
        max = bb_lib::max2(max, iq[i].re);
        max = bb_lib::max2(max, iq[i].im);
    }
    yScale = bb_lib::next_multiple_of(0.05, max);
    if(yScale - max < 0.01) yScale += 0.1;
    if(yScale > 1.5) yScale = 0.75;

    glViewport(grat_ll.x(), grat_ll.y(), grat_sz.x(), grat_sz.y());
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, sweep.sweepLen - 1, -yScale, yScale, -1, 1);

    // Nice lines
    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glLineWidth(GetSession()->prefs.trace_width);

    qglColor(QColor(255, 0, 0));
    DrawTrace(traces[0]);
    qglColor(QColor(0, 255, 0));
    DrawTrace(traces[1]);

    // Disable nice lines
    glLineWidth(1.0);
    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

void DemodIQTimePlot::DrawTrace(const GLVector &v)
{
    if(v.size() < 2) {
        return;
    }

    // Put the trace in the vbo
    glBindBuffer(GL_ARRAY_BUFFER, traceVBO);
    glBufferData(GL_ARRAY_BUFFER, v.size()*sizeof(float),
                 &v[0], GL_DYNAMIC_DRAW);
    glVertexPointer(2, GL_FLOAT, 0, INDEX_OFFSET(0));

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawArrays(GL_LINE_STRIP, 0, v.size() / 2);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Unbind array
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void DemodIQTimePlot::DrawPlotText()
{
    int ascent = textFont.FontMetrics().ascent();
    int textHeight = textFont.GetTextHeight();

    glPushAttrib(GL_VIEWPORT_BIT);
    glViewport(0, 0, width(), height());

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width(), 0, height(), -1, 1);

    const IQSweep &sweep = GetSession()->iq_capture;
    const DemodSettings *ds = GetSession()->demod_settings;
    QString str;

    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_QUADS);
    glVertex2i(grat_ul.x() + 5, grat_ul.y() + 2);
    glVertex2i(grat_ul.x() + 5 + ascent, grat_ul.y() + 2);
    glVertex2i(grat_ul.x() + 5 + ascent, grat_ul.y() + 2 + ascent);
    glVertex2i(grat_ul.x() + 5, grat_ul.y() + 2 + ascent);
    glVertex2i(grat_ul.x() + 5, grat_ul.y() + 2);
    glEnd();

    glColor3f(0.0, 1.0, 0.0);
    glBegin(GL_QUADS);
    glVertex2i(grat_ul.x() + 45, grat_ul.y() + 2);
    glVertex2i(grat_ul.x() + 45 + ascent, grat_ul.y() + 2);
    glVertex2i(grat_ul.x() + 45 + ascent, grat_ul.y() + 2 + ascent);
    glVertex2i(grat_ul.x() + 45, grat_ul.y() + 2 + ascent);
    glVertex2i(grat_ul.x() + 45, grat_ul.y() + 2);
    glEnd();

    glQColor(GetSession()->colors.text);

    DrawString("I", textFont, QPoint(grat_ul.x() + 25, grat_ul.y() + 2), LEFT_ALIGNED);
    DrawString("Q", textFont, QPoint(grat_ul.x() + 65, grat_ul.y() + 2), LEFT_ALIGNED);

    str = "IF Bandwidth " + ds->Bandwidth().GetFreqString();
    DrawString(str, textFont, QPoint(grat_ll.x() + 5, grat_ll.y() - textHeight), LEFT_ALIGNED);
    str = "Capture Len " + ds->SweepTime().GetString();
    DrawString(str, textFont, QPoint(grat_ll.x() + grat_sz.x() - 5, grat_ll.y() - textHeight), RIGHT_ALIGNED);
    //str = "Sample Rate " + QVariant(40.0 / (1 << ds->DecimationFactor())).toString() + " MS/s";
    str = "Sample Rate " + getSampleRateString(sweep.descriptor.sampleRate);
    DrawString(str, textFont, QPoint(grat_ll.x() + grat_sz.x() - 5, grat_ul.y() + 2), RIGHT_ALIGNED);

    for(int i = 0; i <= 10; i++) {
        int x_loc = grat_ul.x() - 2,
                y_loc = grat_ul.y() - (i * grat_sz.y()/10.0) - 5;
        str.sprintf("%.2f", yScale - (i * yScale/5.0));
        DrawString(str, divFont, x_loc, y_loc, RIGHT_ALIGNED);
    }

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glPopAttrib();
}
