#include "tg_trace_view.h"
#include "mainwindow.h"

#include <QMouseEvent>

TGPlot::TGPlot(Session *sPtr, QWidget *parent) :
    GLSubView(sPtr, parent),
    textFont(12),
    divFont(12),
    tgStepSize(0)
{
    makeCurrent();

    glShadeModel(GL_SMOOTH);
    glClearDepth(1.0);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    context()->format().setDoubleBuffer(true);

    glGenBuffers(1, &traceVBO);

    doneCurrent();
}

TGPlot::~TGPlot()
{
    makeCurrent();
    glDeleteBuffers(1, &traceVBO);
    doneCurrent();
}

void TGPlot::resizeEvent(QResizeEvent *)
{
    SetGraticuleDimensions(QPoint(60, 50),
                           QPoint(width() - 80, height() - 100));
}

void TGPlot::mousePressEvent(QMouseEvent *e)
{
    if(PointInGrat(e->pos())) {
        // Make point relative to upper left of graticule
        int x_pos = e->pos().x() - grat_ul.x();

        if(x_pos < 0 || x_pos > grat_sz.x()) {
            return;
        }

        GetSession()->trace_manager->PlaceMarker((double)x_pos / grat_sz.x());

    }

    QGLWidget::mousePressEvent(e);
}

void TGPlot::mouseMoveEvent(QMouseEvent *e)
{
    if(PointInGrat(e->pos())) {
        const SweepSettings *s = GetSession()->sweep_settings;
        double x, xScale, y, yScale;

        xScale = s->Span() / grat_sz.x();
        x = s->Start() + xScale * (e->pos().x() - grat_ll.x());

        if(s->RefLevel().IsLogScale()) {
            yScale = (s->Div() * 10.0) / grat_sz.y();
        } else {
            yScale = s->RefLevel() / grat_sz.y();
        }

        y = s->RefLevel() - (e->pos().y() - grat_ll.y()) * yScale;
        MainWindow::GetStatusBar()->SetCursorPos(
                    Frequency(x).GetFreqString() + "  " +
                    Amplitude(y, s->RefLevel().Units()).GetString());

    } else {
        MainWindow::GetStatusBar()->SetCursorPos("");
    }
}

void TGPlot::paintEvent(QPaintEvent *)
{
    makeCurrent();

    glQClearColor(GetSession()->colors.background);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glEnableClientState(GL_VERTEX_ARRAY);

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

    BuildGraticule();
    DrawGraticule();
    DrawTraces();

    glDisable(GL_DEPTH_TEST);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    RenderMarkers();
    RenderText();

    swapBuffers();
    doneCurrent();
}

// Build/reload just the graticule border
void TGPlot::BuildGraticule()
{
    float border[20] =
    {
        0.0, 0.0, 1.0, 0.0,
        1.0, 0.0, 1.0, 1.0,
        1.0, 1.0, 0.0, 1.0,
        0.0, 1.0, 0.0, 0.0,

        0.0, 0.0, 1.0, 0.0
    };

    double ref = GetSession()->sweep_settings->RefLevel().Val();
    double div = GetSession()->sweep_settings->Div();
    double grat_bottom = ref - 10.0*div;

    float pos = (0.0 - grat_bottom) / (ref - grat_bottom);
    if(pos < 0.0) pos = 0.0;
    if(pos > 1.0) pos = 1.0;

    border[17] = border[19] = pos;

    glBindBuffer(GL_ARRAY_BUFFER, gratBorderVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(border),
                 border, GL_DYNAMIC_DRAW);
    borderGratPoints = 20;
}

void TGPlot::DrawTraces()
{
    TraceManager *manager = GetSession()->trace_manager;
    SweepSettings *settings = GetSession()->sweep_settings;

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

    manager->Lock();

    // Loop through each trace
    for(int i = 0; i < TRACE_COUNT; i++) {
        // If Trace is active, normalize and draw it
        const Trace *trace = manager->GetTrace(i);

        if(trace->Active()) {
            normalize_trace(trace, traces[i], grat_sz,
                            settings->RefLevel(), settings->Div());
            DrawTrace(trace, traces[i], traceVBO);
        }
    }

    manager->Unlock();

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

void TGPlot::DrawTrace(const Trace *t, const GLVector &v, GLuint vbo)
{
    if(v.size() < 1) {
        return;
    }

    QColor c = t->Color();
    glColor3f(c.redF(), c.greenF(), c.blueF());

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

void TGPlot::RenderText()
{
    glQColor(GetSession()->colors.text);

    //glViewport(0, 0, size().width(), size().height());

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, size().width(), 0, size().height(), -1, 1);

    const SweepSettings *s = GetSession()->sweep_settings;
    TraceManager *tm = GetSession()->trace_manager;
    QVariant elapsed = time.restart();
    QString str;
    int textHeight = textFont.GetTextHeight();

    double div = s->RefLevel().IsLogScale() ? s->Div() : (s->RefLevel().Val() / 10.0);

    str = GetSession()->GetTitle();
    if(!str.isNull()) {
        DrawString(str, GLFont(20), width() / 2, height() - 22, CENTER_ALIGNED);
    }

    TgCalState tgState = GetSession()->device->GetTgCalState();
    if(tgState == tgCalStateUncalibrated) {
        str.sprintf("Uncalibrated");
    } else if(tgState == tgCalStatePending) {
        str.sprintf("Calibration on next sweep");
    } else {
        str.sprintf("");
    }
    DrawString(str, textFont, grat_ll.x() + 5, grat_ll.y() + 5, LEFT_ALIGNED);

    str.sprintf("%d pts in %d ms", tm->GetTrace(0)->Length(), elapsed.toInt());
    DrawString(str, textFont, grat_ll.x()+grat_sz.x()-5,
               grat_ll.y()-textHeight*2, RIGHT_ALIGNED);
    DrawString("Center " + s->Center().GetFreqString(), textFont,
               grat_ll.x() + grat_sz.x()/2, grat_ll.y()-textHeight, CENTER_ALIGNED);
    DrawString("Span " + s->Span().GetFreqString(), textFont,
               grat_ll.x() + grat_sz.x()/2, grat_ll.y()-textHeight*2, CENTER_ALIGNED);
    DrawString("Start " + (s->Start()).GetFreqString(), textFont,
               grat_ll.x()+5, grat_ll.y()-textHeight, LEFT_ALIGNED);
    DrawString("Stop " + (s->Stop()).GetFreqString(), textFont,
               grat_ll.x()+grat_sz.x()-5, grat_ll.y()-textHeight, RIGHT_ALIGNED);
    DrawString("Ref " + s->RefLevel().GetString(), textFont,
               grat_ll.x()+5, grat_ul.y()+textHeight, LEFT_ALIGNED);
    str.sprintf("Div %.1f", div);
    DrawString(str, textFont, grat_ul.x()+5, grat_ul.y()+2 , LEFT_ALIGNED);
    DrawString("Step " + Frequency(tgStepSize).GetFreqString(), textFont,
               grat_ll.x() + grat_sz.x()/2, grat_ul.y()+textHeight, CENTER_ALIGNED);
    DrawString("Atten --", textFont, grat_ll.x() + grat_sz.x()/2, grat_ul.y()+2, CENTER_ALIGNED);
    DrawString("VBW --", textFont, grat_ul.x()+grat_sz.x()-5,
               grat_ul.y()+textHeight, RIGHT_ALIGNED);

    // y-axis labels
    for(int i = 0; i <= 8; i += 2) {
        int x_pos = 58, y_pos = (grat_sz.y() / 10) * i + grat_ll.y() - 5;
        QString div_str;
        div_str.sprintf("%.2f", s->RefLevel() - (div*(10-i)));
        DrawString(div_str, divFont, x_pos, y_pos, RIGHT_ALIGNED);
    }

    if(tm->GetLimitLine()->Active()) {
        QPoint limitTextLoc(grat_ul.x() + (grat_sz.x() * 0.5),
                            grat_ul.y() - (grat_sz.y() * 0.25));
        if(tm->GetLimitLine()->LimitsPassed()) {
            glColor3f(0.0, 1.0, 0.0);
            DrawString("Passed", textFont, limitTextLoc, CENTER_ALIGNED);
        } else {
            glColor3f(1.0, 0.0, 0.0);
            DrawString("Failed", textFont, limitTextLoc, CENTER_ALIGNED);
        }
    }

    // Amplitude high warning
    if(GetSession()->trace_manager->LastTraceAboveReference()) {
        glColor3f(1.0, 0.0, 0.0);
        DrawString("*Warning* : Signal Level Higher Than Ref Level", textFont,
                   (grat_ul.x() + grat_sz.x()) / 2.0, grat_ul.y() - 22, CENTER_ALIGNED);
    }

    if(GetSession()->device->IsOpen()) {
        // Uncal text strings
        bool uncal = false;
        int uncal_x = grat_ul.x() + 5, uncal_y = grat_ul.y() - textHeight;
        glColor3f(1.0, 0.0, 0.0);
        if(!GetSession()->device->IsPowered()) {
            uncal = true;
            DrawString("Low Voltage", textFont, uncal_x, uncal_y, LEFT_ALIGNED);
            uncal_y -= textHeight;
        }
        if(GetSession()->device->ADCOverflow()) {
            uncal = true;
            DrawString("IF Overload", textFont, uncal_x, uncal_y, LEFT_ALIGNED);
            uncal_y -= textHeight;
        }
        if(GetSession()->device->NeedsTempCal()) {
            uncal = true;
            DrawString("Device Temp", textFont, uncal_x, uncal_y, LEFT_ALIGNED);
            uncal_y -= textHeight;
        }
        if(uncal) {
            DrawString("Uncal", textFont, grat_ul.x() - 5, grat_ul.y() + 2, RIGHT_ALIGNED);
        }
    }

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

void TGPlot::RenderMarkers()
{
    SweepSettings *s = GetSession()->sweep_settings;
    TraceManager *tm = GetSession()->trace_manager;

    // Viewport on grat, full pixel scale
    glPushAttrib(GL_VIEWPORT_BIT);
    glViewport(grat_ll.x(), grat_ll.y(), grat_sz.x(), grat_sz.y());

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, grat_sz.x(), 0, grat_sz.y(), -1, 1);

    int x_print = grat_sz.x() - 5;
    int y_print = grat_sz.y() - 20;

    tm->SolveMarkers(s);

    // Nice lines, doesn't smooth quads
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glLineWidth(1.0);

    for(int i = 0; i < MARKER_COUNT; i++) {
        Marker *m = tm->GetMarker(i);
        if(!m->Active() || !m->InView()) {
            continue;
        }

        if(m->InView()) {
            DrawMarker(m->xRatio() * grat_sz.x(),
                       m->yRatio() * grat_sz.y(), i + 1);
        }

        if(m->DeltaActive() && m->DeltaInView()) {
            DrawDeltaMarker(m->delxRatio() * grat_sz.x(),
                            m->delyRatio() * grat_sz.y(), i + 1);
        }
        // Does not have to be in view to draw the delta values
        if(m->DeltaActive()) {
            glQColor(GetSession()->colors.text);
            DrawString("Mkr " + QVariant(i+1).toString() + " Delta: " + m->DeltaText(),
                       textFont, QPoint(x_print, y_print), RIGHT_ALIGNED);
            y_print -= 20;
        } else if(m->Active()) {
            glQColor(GetSession()->colors.text);
            DrawString("Mkr " + QVariant(i+1).toString() + ": " + m->Text(),
                       textFont, QPoint(x_print, y_print), RIGHT_ALIGNED);
            y_print -= 20;
        }
    }

    // Disable nice lines
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_BLEND);

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
}

void TGPlot::DrawMarker(int x, int y, int num)
{
    glQColor(GetSession()->colors.markerBackground);
    glBegin(GL_POLYGON);
    glVertex2f(x, y);
    glVertex2f(x + 10, y + 15);
    glVertex2f(x, y + 30);
    glVertex2f(x - 10, y + 15);
    glEnd();

    glQColor(GetSession()->colors.markerBorder);
    glBegin(GL_LINE_STRIP);
    glVertex2f(x, y);
    glVertex2f(x + 10, y + 15);
    glVertex2f(x, y + 30);
    glVertex2f(x - 10, y + 15);
    glVertex2f(x, y);
    glEnd();

    glQColor(GetSession()->colors.markerText);
    QString str;
    str.sprintf("%d", num);
    DrawString(str, divFont,
               QPoint(x, y + 10), CENTER_ALIGNED);
}

void TGPlot::DrawDeltaMarker(int x, int y, int num)
{
    glQColor(GetSession()->colors.markerBackground);
    glBegin(GL_POLYGON);
    glVertex2f(x, y);
    glVertex2f(x + 11, y + 11);
    glVertex2f(x + 11, y + 27);
    glVertex2f(x - 11, y + 27);
    glVertex2f(x - 11, y + 11);
    glEnd();

    glQColor(GetSession()->colors.markerBorder);
    glBegin(GL_LINE_STRIP);
    glVertex2f(x, y);
    glVertex2f(x + 11, y + 11);
    glVertex2f(x + 11, y + 27);
    glVertex2f(x - 11, y + 27);
    glVertex2f(x - 11, y + 11);
    glVertex2f(x, y);
    glEnd();

    glQColor(GetSession()->colors.markerText);
    QString str;
    str.sprintf("R%d", num);
    DrawString(str, divFont, QPoint(x, y+11), CENTER_ALIGNED);
}
