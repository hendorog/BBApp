#include "demod_sweep_plot.h"

#include <QLayout>
#include <QMouseEvent>

DemodSweepArea::DemodSweepArea(Session *session, QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Zero-Span Plot");

    toolBar = new QToolBar(this);
    toolBar->move(0, 0);
    toolBar->layout()->setContentsMargins(0, 0, 0, 0);
    toolBar->layout()->setSpacing(0);
    toolBar->move(0, 0);

    plot = new DemodSweepPlot(session, this);
    plot->move(0, 30);
    plot->setWindowTitle("Demod Plot");

    ComboBox *demodSelect = new ComboBox();
    QStringList comboString;
    comboString << "AM Demod" << "FM Demod" << "PM Demod";
    demodSelect->insertItems(0, comboString);
    demodSelect->setFixedSize(200, 30-4);

    QPushButton *markerOff, *markerDelta;
    markerOff = new QPushButton("Marker Off");
    markerOff->setObjectName("BBPushButton");
    markerOff->setFixedSize(120, 30-4);

    markerDelta = new QPushButton("Marker Delta");
    markerDelta->setObjectName("BBPushButton");
    markerDelta->setFixedSize(120, 30-4);

    toolBar->addWidget(new FixedSpacer(QSize(10, 30)));
    toolBar->addWidget(demodSelect);
    toolBar->addWidget(new FixedSpacer(QSize(10, 30)));
    toolBar->addSeparator();
    toolBar->addWidget(new FixedSpacer(QSize(10, 30)));
    toolBar->addWidget(markerOff);
    toolBar->addWidget(markerDelta);

    connect(demodSelect, SIGNAL(activated(int)), plot, SLOT(changeDemod(int)));
    connect(markerOff, SIGNAL(clicked()), plot, SLOT(disableMarker()));
    connect(markerDelta, SIGNAL(clicked()), plot, SLOT(toggleDelta()));
}

DemodSweepPlot::DemodSweepPlot(Session *session, QWidget *parent) :
    GLSubView(session, parent),
    textFont("Arial", 14),
    divFont("Arial", 12),
    demodType(DemodTypeAM),
    markerOn(false),
    deltaOn(false)
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

DemodSweepPlot::~DemodSweepPlot()
{
    makeCurrent();

    glDeleteBuffers(1, &traceVBO);
    glDeleteBuffers(1, &gratVBO);
    glDeleteBuffers(1, &gratBorderVBO);

    doneCurrent();
}

void DemodSweepPlot::resizeEvent(QResizeEvent *)
{
    grat_ll = QPoint(60, 50);
    grat_ul = QPoint(60, size().height() - 50);

    grat_sz = QPoint(size().width() - 80,
                     size().height() - 100);
}

void DemodSweepPlot::mousePressEvent(QMouseEvent *e)
{
    QRect area(grat_ll.x(), grat_ll.y(), grat_sz.x(), grat_sz.y());
    if(area.contains(e->pos())) {
        markerOn = true;
        markerPos.setX((double)(e->pos().x() - grat_ul.x()) / grat_sz.x());
    }

    QGLWidget::mousePressEvent(e);
}

void DemodSweepPlot::paintEvent(QPaintEvent *)
{
    makeCurrent();

    glQClearColor(GetSession()->colors.background);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glEnableClientState(GL_VERTEX_ARRAY);

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

    DemodAndDraw();
    DrawPlotText();
    DrawMarkers();

    swapBuffers();
    doneCurrent();
}

void DemodSweepPlot::DemodAndDraw()
{
    const IQSweep &iq = GetSession()->iq_capture;
    if(iq.sweep.size() <= 1) return;

    trace.clear();

    double ref, botRef;

    if(demodType == DemodTypeAM) {
        for(int i = 0; i < iq.sweep.size(); i++) {
            trace.push_back(i);
            trace.push_back(10 * log10(iq.sweep[i].re * iq.sweep[i].re +
                                       iq.sweep[i].im * iq.sweep[i].im));
        }
        ref = GetSession()->demod_settings->InputPower().ConvertToUnits(DBM);
        botRef = ref - 100.0;
    } else if(demodType == DemodTypeFM) {
        double sr = 40.0e6 / (0x1 << GetSession()->demod_settings->DecimationFactor());
        demod_fm(iq.sweep, trace, sr);
        ref = 40.0e6 / (0x1 << GetSession()->demod_settings->DecimationFactor());
        botRef = -ref;
    } else if(demodType == DemodTypePM) {
        for(int i = 0; i < iq.sweep.size(); i++) {
            trace.push_back(i);
            trace.push_back(atan2(iq.sweep[i].im, iq.sweep[i].re));
        }
        ref = BB_PI;
        botRef = -BB_PI;
    }

    glViewport(grat_ll.x(), grat_ll.y(), grat_sz.x(), grat_sz.y());
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, trace.size()/2 - 1, botRef, ref, -1, 1);

    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glLineWidth(GetSession()->prefs.trace_width);

    qglColor(QColor(255, 0, 0));
    DrawTrace(trace);

    // Disable nice lines
    glLineWidth(1.0);
    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

void DemodSweepPlot::DrawPlotText()
{
    glPushAttrib(GL_VIEWPORT_BIT);
    glViewport(0, 0, width(), height());

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width(), 0, height(), -1, 1);

    const DemodSettings *ds = GetSession()->demod_settings;
    QString str;

    glQColor(GetSession()->colors.text);

    DrawString("Center " + ds->CenterFreq().GetFreqString(), textFont,
               grat_ul.x() + grat_sz.x()/2, grat_ll.y() - 22, CENTER_ALIGNED);
    str.sprintf("%f ms per div", ds->SweepTime() * 100.0);
    DrawString(str, textFont, grat_ll.x() + grat_sz.x() - 5, grat_ll.y() - 42, RIGHT_ALIGNED);
    str.sprintf("%d pts", GetSession()->iq_capture.len);
    DrawString(str, textFont, grat_ll.x() + grat_sz.x() - 5, grat_ll.y() - 22, RIGHT_ALIGNED);

    if(demodType == DemodTypeAM) {
        DrawString("Ref " + ds->InputPower().GetString(), textFont,
                   QPoint(grat_ul.x() + 5, grat_ul.y() + 22), LEFT_ALIGNED);
        for(int i = 0; i <= 8; i += 2) {
            int x_pos = grat_ul.x() - 2, y_pos = (grat_sz.y() / 10) * i + grat_ll.y() - 5;
            str.sprintf("%.2f", ds->InputPower().Val() - (10.0*(10-i)));
            DrawString(str, divFont, x_pos, y_pos, RIGHT_ALIGNED);
        }
    } else if(demodType == DemodTypeFM) {
        Frequency ref = 40.0e6 / ((0x1 << ds->DecimationFactor()) * 2.0);
        double div = ref / 5.0;
        DrawString("Ref " + ref.GetFreqString(), textFont,
                   QPoint(grat_ul.x() + 5, grat_ul.y() + 22), LEFT_ALIGNED);
        for(int i = 1; i <= 9; i += 2) {
            int x_pos = grat_ul.x() - 2, y_pos = (grat_sz.y() / 10) * i + grat_ll.y() - 5;
            str.sprintf("%.2fM", (-ref + i * div) * 1.0e-6);
            DrawString(str, divFont, x_pos, y_pos, RIGHT_ALIGNED);
        }
    } else if(demodType == DemodTypePM) {
        DrawString("Ref 3.14159 Deg", textFont,
                   QPoint(grat_ul.x() + 5, grat_ul.y() + 22), LEFT_ALIGNED);
        for(int i = 1; i <= 9; i += 2) {
            int x_pos = grat_ul.x() - 2, y_pos = (grat_sz.y() / 10) * i + grat_ll.y() - 5;
            str.sprintf("%.2f", - 3.14159 + i * ((2 * BB_PI) / 10.0));
            DrawString(str, divFont, x_pos, y_pos, RIGHT_ALIGNED);
        }
    }

    if(ds->TrigType() == TriggerTypeExternal || ds->TrigType() == TriggerTypeVideo) {
        if(GetSession()->iq_capture.triggered) {
            str.sprintf("Triggered");
        } else {
            str.sprintf("Armed");
        }
        DrawString(str, textFont, grat_ul.x() + grat_sz.x() / 2,
                   grat_ul.y() + 22, CENTER_ALIGNED);
    }

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glPopAttrib();

}

void DemodSweepPlot::DrawMarkers()
{
    if(!markerOn) return;

    const DemodSettings *ds = GetSession()->demod_settings;
    QString str;
    double binSize = 1.0 / (40.0e6 / (0x1 << ds->DecimationFactor()));

    int index = (trace.size()/2) * markerPos.x();
    str = QVariant(index * binSize * 1000.0).toString() + " ms : ";
    index = index * 2 + 1;

    // Clamp to size, no out of bound indexing please
    if(index < 0) index = 1;
    if(index > trace.size()) index = trace.size();

    float markerVal = trace[index];

    if(demodType == DemodTypeAM) {
        float botRef = ds->InputPower() - 100.0;
        markerPos.setY((markerVal - botRef) / 100.0);

    } else if(demodType == DemodTypeFM) {
        float botRef = -40.0e6 / (0x1 << ds->DecimationFactor());
        markerPos.setY((markerVal - botRef) / fabs(2.0 * botRef));
    } else if(demodType == DemodTypePM) {
        float botRef = -BB_PI;
        markerPos.setY((markerVal - botRef) / BB_TWO_PI);
    }

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

    // Nice lines
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glLineWidth(1.0);

    glDisable(GL_DEPTH_TEST);
    DrawMarker(markerPos.x() * grat_sz.x(), markerPos.y() * grat_sz.y(), 1);
    if(deltaOn) {
        DrawDeltaMarker(deltaPos.x() * grat_sz.x(), deltaPos.y() * grat_sz.y(), 1);
    }

    //glLoadIdentity();
    //glOrtho(0, width(), 0, height(), -1, 1);
    DrawString(str, textFont, grat_sz.x() - 5, grat_sz.y() - 22, RIGHT_ALIGNED);

    // Disable nice lines
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_BLEND);

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();


}

void DemodSweepPlot::DrawTrace(const GLVector &v)
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

void DemodSweepPlot::DrawMarker(int x, int y, int num)
{
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_POLYGON);
    glVertex2f(x, y);
    glVertex2f(x + 10, y + 15);
    glVertex2f(x, y + 30);
    glVertex2f(x - 10, y + 15);
    glEnd();

    //glQColor(session_ptr->colors.markers);
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINE_STRIP);
    glVertex2f(x, y);
    glVertex2f(x + 10, y + 15);
    glVertex2f(x, y + 30);
    glVertex2f(x - 10, y + 15);
    glVertex2f(x, y);
    glEnd();

    glColor3f(0.0, 0.0, 0.0);
    QString str;
    str.sprintf("%d", num);
    DrawString(str, divFont,
               QPoint(x, y + 10), CENTER_ALIGNED);
}

void DemodSweepPlot::DrawDeltaMarker(int x, int y, int num)
{
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_POLYGON);
    glVertex2f(x, y);
    glVertex2f(x + 11, y + 11);
    glVertex2f(x + 11, y + 27);
    glVertex2f(x - 11, y + 27);
    glVertex2f(x - 11, y + 11);
    glEnd();

    //glQColor(session_ptr->colors.markers);
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINE_STRIP);
    glVertex2f(x, y);
    glVertex2f(x + 11, y + 11);
    glVertex2f(x + 11, y + 27);
    glVertex2f(x - 11, y + 27);
    glVertex2f(x - 11, y + 11);
    glVertex2f(x, y);
    glEnd();

    glColor3f(0.0, 0.0, 0.0);
    QString str;
    str.sprintf("R%d", num);
    DrawString(str, divFont,
               QPoint(x, y+11), CENTER_ALIGNED);
}
