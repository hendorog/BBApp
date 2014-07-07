#include "demod_spectrum_plot.h"

DemodSpectrumPlot::DemodSpectrumPlot(Session *sPtr, QWidget *parent) :
    GLSubView(sPtr, parent),
    textFont("Arial", 14)
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

    fft = std::unique_ptr<FFT>(new FFT(1024, false));
}

DemodSpectrumPlot::~DemodSpectrumPlot()
{
    makeCurrent();

    glDeleteBuffers(1, &traceVBO);
    glDeleteBuffers(1, &gratVBO);
    glDeleteBuffers(1, &gratBorderVBO);

    doneCurrent();
}

void DemodSpectrumPlot::resizeEvent(QResizeEvent *)
{
    grat_ll = QPoint(60, 50);
    grat_ul = QPoint(60, size().height() - 50);

    grat_sz = QPoint(size().width() - 80,
                     size().height() - 100);
}

void DemodSpectrumPlot::paintEvent(QPaintEvent *)
{
    makeCurrent();

    glClearColor(1.0, 1.0, 1.0, 1.0);
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

    DrawSpectrum();

    swapBuffers();
    doneCurrent();
}

void DemodSpectrumPlot::DrawSpectrum()
{
    spectrum.clear();

    const IQSweep &sweep = GetSession()->iq_capture;

    // May need to resize the fft if it goes below 1024
    int fftSize = fft->Length() // Here


    postTransform.resize(1024);


    FFT fff(1024, false);
    fff.Transform(&GetSession()->iq_capture.sweep[0], &postTransform[0]);
    for(int i = 0; i < 1024; i++) {
        postTransform[i].re /= 1024.0;
        postTransform[i].im /= 1024.0;
    }

    for(int i = 0; i < 1024; i++) {
        spectrum.push_back((double)i / 1024.0);
        double mag = postTransform[i].re * postTransform[i].re + postTransform[i].im * postTransform[i].im;
        mag = 10.0 * log10(mag);
        spectrum.push_back(mag);
    }

    glViewport(grat_ll.x(), grat_ll.y(), grat_sz.x(), grat_sz.y());
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    //glScalef(1, 1, 1);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1.0, -100.0, 0.0, -1, 1);

    // Nice lines, doesn't smooth quads
    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glLineWidth(GetSession()->prefs.trace_width);

    qglColor(QColor(255, 0, 0));
    DrawTrace(spectrum);
    DrawPlotText();

    // Disable nice lines
    glLineWidth(1.0);
    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

void DemodSpectrumPlot::DrawTrace(const GLVector &v)
{
    if(v.size() < 2) {
        return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, traceVBO);
    glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(float),
                 &v[0], GL_DYNAMIC_DRAW);
    glVertexPointer(2, GL_FLOAT, 0, INDEX_OFFSET(0));

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawArrays(GL_LINE_STRIP, 0, v.size() / 2);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void DemodSpectrumPlot::DrawPlotText()
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

    str = "Center Freq " + ds->CenterFreq().GetFreqString();
    DrawString(str, textFont, QPoint(grat_ll.x(), grat_ll.y() - 30), LEFT_ALIGNED);
    str = "Span " + ds->Bandwidth().GetFreqString();
    DrawString(str, textFont, QPoint(grat_ll.x() + grat_sz.x(), grat_ll.y() - 30), RIGHT_ALIGNED);

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glPopAttrib();
}
