#include "demod_spectrum_plot.h"

DemodSpectrumPlot::DemodSpectrumPlot(Session *sPtr, QWidget *parent) :
    GLSubView(sPtr, parent),
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

    fft = std::unique_ptr<FFT>(new FFT(MAX_FFT_SIZE, false));
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

    DrawSpectrum();

    glDisable(GL_DEPTH_TEST);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    DrawPlotText();

    swapBuffers();
    doneCurrent();
}

void DemodSpectrumPlot::DrawSpectrum()
{
    spectrum.clear();

    const IQSweep &sweep = GetSession()->iq_capture;
    const DemodSettings *ds = GetSession()->demod_settings;
    double ref, botRef;

    if(sweep.iq.size() < 2) {
        return;
    }

    if(ds->InputPower().IsLogScale()) {
        ref = ds->InputPower().ConvertToUnits(AmpUnits::DBM);
        botRef = ref - 100.0;
    } else {
        ref = ds->InputPower();
        botRef = 0;
    }

    // May need to resize the fft if it goes below MAX_FFT_SIZE
    int fftSize = bb_lib::min2(MAX_FFT_SIZE, sweep.sweepLen);
    fftSize = bb_lib::round_down_power_two(fftSize);

    if(fftSize != fft->Length()) {
        fft = std::unique_ptr<FFT>(new FFT(fftSize, false));
    }

    postTransform.resize(MAX_FFT_SIZE);

    fft->Transform(&sweep.iq[0], &postTransform[0]);
    for(int i = 0; i < fftSize; i++) {
        postTransform[i].re /= fftSize;
        postTransform[i].im /= fftSize;
    }

    // Create mW scale first
    for(int i = 0; i < fftSize; i++) {
        spectrum.push_back((double)i);
        double mag = postTransform[i].re * postTransform[i].re +
                postTransform[i].im * postTransform[i].im;
        spectrum.push_back(mag);
    }

    // Convert to dBm or mV
    if(ds->InputPower().IsLogScale()) {
        for(int i = 1; i < spectrum.size(); i += 2) {
            spectrum[i] = 10.0 * log10(spectrum[i]);
        }
    } else {
        for(int i = 1; i < spectrum.size(); i += 2) {
            spectrum[i] = sqrt(spectrum[i] * 50000.0);
        }
    }

    glViewport(grat_ll.x(), grat_ll.y(), grat_sz.x(), grat_sz.y());
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, fftSize - 1, botRef, ref, -1, 1);

    // Nice lines, doesn't smooth quads
    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glLineWidth(GetSession()->prefs.trace_width);

    qglColor(QColor(255, 0, 0));
    DrawTrace(spectrum);

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

    const DemodSettings *ds = GetSession()->demod_settings;
    QString str;

    glQColor(GetSession()->colors.text);

    str = "Center " + ds->CenterFreq().GetFreqString();
    DrawString(str, textFont, QPoint(grat_ll.x() + 5, grat_ll.y() - textHeight), LEFT_ALIGNED);
    str = "Span " + Frequency(ds->SampleRate()).GetFreqString(3, true);
    DrawString(str, textFont, QPoint(grat_ll.x() + grat_sz.x() - 5, grat_ll.y() - textHeight), RIGHT_ALIGNED);
    str = "FFT Size " + QVariant(fft->Length()).toString() + " pts";
    DrawString(str, textFont, grat_ul.x() + grat_sz.x() - 5, grat_ul.y() + 2, RIGHT_ALIGNED);
    DrawString("Div 10 dB", textFont, QPoint(grat_ul.x() + 5, grat_ul.y() + 2), LEFT_ALIGNED);

    double botVal, step;

    if(ds->InputPower().IsLogScale()) {
        botVal = ds->InputPower() - 100.0;
        step = 10.0;
    } else {
        botVal = 0;
        step = ds->InputPower() / 10.0;
    }

    for(int i = 0; i <= 10; i += 2) {
        int x_pos = grat_ul.x() - 2, y_pos = (grat_sz.y() / 10) * i + grat_ll.y() - 5;
        str.sprintf("%.2f", botVal + step * i);
        DrawString(str, divFont, x_pos, y_pos, RIGHT_ALIGNED);
    }

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glPopAttrib();
}
