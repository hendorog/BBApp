#include "demod_sweep_plot.h"

#include <QLayout>
#include <QMouseEvent>

#include <limits>

//ReceiverStats getReceiverStats(const IQSweep &sweep)
//{
//    ReceiverStats stats;
//    const std::vector<float> &am = sweep.amWaveform;
//    const std::vector<float> &fm = sweep.fmWaveform;

//    std::vector<float> temp;
//    temp.resize(fm.size());

//    FirFilter fir(0.03, 1024); // Filter AM and FM

//    fir.Filter(&fm[0], &temp[0], fm.size());
//    fir.Reset();

//    // Start with FM
//    stats.fmPeakPlus = std::numeric_limits<double>::lowest();
//    stats.fmPeakMinus = std::numeric_limits<double>::max();
//    stats.fmRMS = 0.0;

//    float fmLastVal = 1.0;
//    double fmLastCrossing, fmFirstCrossing = 0.0;
//    bool fmFirstCross = true;
//    int fmCrossCounter = 0;

//    for(int i = 1024; i < temp.size(); i++) {
//        float f = temp[i];

//        if(f > stats.fmPeakPlus) stats.fmPeakPlus = f;
//        if(f < stats.fmPeakMinus) stats.fmPeakMinus = f;
//        stats.fmRMS += (f*f);

//        if(f > 0.0 && fmLastVal < 0.0) {
//            double crossLerp = double(i-1) + ((0.0 - fmLastVal) / (f - fmLastVal));
//            if(fmFirstCross) {
//                fmFirstCrossing = crossLerp;
//                fmFirstCross = false;
//            } else {
//                fmLastCrossing = crossLerp;
//                fmCrossCounter++;
//            }
//        }
//        fmLastVal = f;
//    }

//    stats.fmRMS = sqrt(stats.fmRMS / temp.size() - 1024);
//    if(fmCrossCounter == 0) {
//        stats.fmAudioFreq = 0.0;
//    } else {
//        stats.fmAudioFreq = (fmLastCrossing - fmFirstCrossing) / fmCrossCounter;
//        stats.fmAudioFreq = sweep.settings.SampleRate() / stats.fmAudioFreq;
//    }

//    // AM
//    stats.amPeakPlus = std::numeric_limits<double>::lowest();
//    stats.amPeakMinus = std::numeric_limits<double>::max();
//    stats.amRMS = 0.0;
//    double invAvg = 0.0;
//    float amLastVal = 1.0;
//    double amLastCrossing, amFirstCrossing = 0.0;
//    bool amFirstCross = true;
//    int amCrossCounter = 0;

//    for(int i = 0; i < am.size(); i++) {
//        float v = sqrt(am[i] * 50000.0);
//        temp[i] = v;
//        invAvg += v;
//    }

//    invAvg = (am.size() / invAvg);

//    // Normalize between [-1.0, 1.0]
//    for(int i = 0; i < temp.size(); i++) {
//        float n = (temp[i] * invAvg) - 1.0; // normalized value

//        if(n < stats.amPeakMinus) stats.amPeakMinus = n;
//        if(n > stats.amPeakPlus) stats.amPeakPlus = n;
//        stats.amRMS += (n*n);

//        if(n > 0.0 && amLastVal < 0.0) {
//            double crossLerp = double(i - 1) + ((-amLastVal) / (n - amLastVal));
//            if(amFirstCross) {
//                amFirstCrossing = crossLerp;
//                amFirstCross = false;
//            } else {
//                amLastCrossing = crossLerp;
//                amCrossCounter++;
//            }
//        }
//        amLastVal = n;
//    }

//    stats.amRMS = sqrt(stats.amRMS / temp.size());
//    if(amCrossCounter == 0) {
//        stats.amAudioFreq = 0.0;
//    } else {
//        stats.amAudioFreq = (amLastCrossing - amFirstCrossing) / amCrossCounter;
//        stats.amAudioFreq = sweep.settings.SampleRate() / stats.amAudioFreq;
//    }

//    return stats;
//}

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
    update();

    QGLWidget::mousePressEvent(e);
}

void DemodSweepPlot::paintEvent(QPaintEvent *)
{
    makeCurrent();

    if(GetSession()->demod_settings->MAEnabled()) {
        grat_sz = QPoint(size().width() - 380, size().height() - 100);
    } else {
        grat_sz = QPoint(size().width() - 80, size().height() - 100);
    }

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
    const DemodSettings *ds = GetSession()->demod_settings;
    const IQSweep &iq = GetSession()->iq_capture;
    if(iq.iq.size() <= 1) return;

    //waveform.clear();
    trace.clear();

    double ref, botRef;

    if(demodType == DemodTypeAM) {
//        for(int i = 0; i < iq.iq.size(); i++) {
//            waveform.push_back(iq.iq[i].re * iq.iq[i].re +
//                               iq.iq[i].im * iq.iq[i].im);
//        }
        if(ds->InputPower().IsLogScale()) {
            ref = ds->InputPower().ConvertToUnits(DBM);
            botRef = ref - 100.0;
//            for(int i = 0; i < waveform.size(); i++) {
//                waveform[i] = 10.0 * log10(waveform[i]);
//            }
            for(int i = 0; i < iq.amWaveform.size(); i++) {
                trace.push_back(i);
                trace.push_back(10.0 * log10(iq.amWaveform[i]));
            }
        } else {
            ref = ds->InputPower();
            botRef = 0.0;
//            for(int i = 0; i < waveform.size(); i++) {
//                waveform[i] = sqrt(waveform[i] * 50000.0);
//            }
            for(int i = 0; i < iq.amWaveform.size(); i++) {
                trace.push_back(i);
                trace.push_back(sqrt((double)iq.amWaveform[i] * 50000.0));
            }
        }

    } else if(demodType == DemodTypeFM) {
        //double sr = 40.0e6 / (0x1 << ds->DecimationFactor());
        //demod_fm(iq.iq, waveform, sr / 2.0);
        ref = 40.0e6 / (0x1 << ds->DecimationFactor()) / 2.0;
        botRef = -ref;

        //        FirFilter fir(0.03, 1024);
        //        std::vector<float> temp;
        //        temp.resize(waveform.size());
        //        fir.Filter(&waveform[0], &temp[0], waveform.size());
        //        waveform.clear();
        //        for(int i = 512; i < temp.size(); i++) {
        //            waveform.push_back(temp[i]);
        //        }

        for(int i = 0; i < iq.fmWaveform.size(); i++) {
            trace.push_back(i);
            trace.push_back(iq.fmWaveform[i]);
        }

    } else if(demodType == DemodTypePM) {
        //        for(int i = 0; i < iq.iq.size(); i++) {
        //            waveform.push_back(atan2(iq.iq[i].im, iq.iq[i].re));
        //        }
        ref = BB_PI;
        botRef = -BB_PI;

        for(int i = 0; i < iq.pmWaveform.size(); i++) {
            trace.push_back(i);
            trace.push_back(iq.pmWaveform[i]);
        }
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
    str.sprintf("%d pts", GetSession()->iq_capture.iq.size());
    DrawString(str, textFont, grat_ll.x() + grat_sz.x() - 5, grat_ll.y() - 22, RIGHT_ALIGNED);

    if(demodType == DemodTypeAM) {
        DrawString("Ref " + ds->InputPower().GetString(), textFont,
                   QPoint(grat_ul.x() + 5, grat_ul.y() + 22), LEFT_ALIGNED);
        double botVal, step;
        if(ds->InputPower().IsLogScale()) {
            botVal = ds->InputPower() - 100.0;
            step = 10.0;
        } else {
            botVal = 0;
            step = ds->InputPower() / 10.0;
        }
        for(int i = 0; i <= 8; i += 2) {
            int x_pos = grat_ul.x() - 2, y_pos = (grat_sz.y() / 10) * i + grat_ll.y() - 5;
            str.sprintf("%.2f", botVal + step * i);
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

    // Uncal text strings
    bool uncal = false;
    int uncal_x = grat_ul.x() + 5, uncal_y = grat_ul.y() - 22;
    glColor3f(1.0, 0.0, 0.0);
    if(!GetSession()->device->IsPowered()) {
        uncal = true;
        DrawString("Low Voltage", textFont, uncal_x, uncal_y, LEFT_ALIGNED);
        uncal_y -= 16;
    }
    if(GetSession()->device->ADCOverflow()) {
        uncal = true;
        DrawString("IF Overload", textFont, uncal_x, uncal_y, LEFT_ALIGNED);
        uncal_y -= 16;
    }
    if(GetSession()->device->NeedsTempCal()) {
        uncal = true;
        DrawString("Device Temp", textFont, uncal_x, uncal_y, LEFT_ALIGNED);
        uncal_y -= 16;
    }
    if(uncal) {
        DrawString("Uncal", textFont, grat_ul.x() - 5, grat_ul.y() - 22, RIGHT_ALIGNED);
    }

    if(GetSession()->demod_settings->MAEnabled()) {
        glQColor(GetSession()->colors.text);
        DrawMeasuringReceiverStats();
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
    QString str, delStr;
    double binSize = 1.0 / (40.0e6 / (0x1 << ds->DecimationFactor()));

    int index = (trace.size()/2) * markerPos.x();
    markerPos.setX((double)index / (trace.size() / 2));
    str = QVariant(index * binSize * 1000.0).toString() + " ms : ";
    index = index * 2 + 1;

    // Clamp to size, no out of bound indexing please
    if(index < 0) index = 1;
    if(index > trace.size()) index = trace.size();

    markerVal = trace[index];

    if(demodType == DemodTypeAM) {
        Amplitude markerAmp;
        if(ds->InputPower().IsLogScale()) {
            markerAmp = Amplitude(markerVal).ConvertToUnits(ds->InputPower().Units());
            float botRef = ds->InputPower() - 100.0; // Always div == 10
            markerPos.setY((markerAmp - botRef) / 100.0); // Always div == 10
        } else {
            markerAmp = Amplitude(markerVal, MV);
            float botRef = 0.0;
            markerPos.setY(markerAmp / ds->InputPower());
        }
        str += markerAmp.GetString();
    } else if(demodType == DemodTypeFM) {
        float botRef = -40.0e6 / (0x1 << ds->DecimationFactor()) / 2.0;
        markerPos.setY((markerVal - botRef) / fabs(2.0 * botRef));
        str += Frequency(markerVal).GetFreqString();
    } else if(demodType == DemodTypePM) {
        float botRef = -BB_PI;
        markerPos.setY((markerVal - botRef) / BB_TWO_PI);
        str += QVariant(markerVal).toString();
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
        float diff = markerVal - deltaVal;
        delStr = "Delta : ";
        delStr += QVariant(diff).toString() + ((ds->InputPower().IsLogScale()) ?
                                                   " dB" : " mV");
    }

    DrawString(str, textFont, grat_sz.x() - 5, grat_sz.y() - 22, RIGHT_ALIGNED);
    DrawString(delStr, textFont, grat_sz.x() - 5, grat_sz.y() - 42, RIGHT_ALIGNED);

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

void DemodSweepPlot::DrawDeltaMarker(int x, int y, int num)
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
    DrawString(str, divFont,
               QPoint(x, y+11), CENTER_ALIGNED);
}

void DemodSweepPlot::DrawMeasuringReceiverStats()
{
    ReceiverStats stats = GetSession()->iq_capture.stats;

    QPoint pos(grat_ul.x() + grat_sz.x() + 10, grat_ul.y() - 20);
    QString str;

    DrawString("AM/FM Modulation Analysis", textFont, pos, LEFT_ALIGNED);
    pos += QPoint(0, -20);
    DrawString("RF Center " + Frequency(stats.rfCenter).GetFreqString(), divFont, pos, LEFT_ALIGNED);
    pos += QPoint(0, -20);
    DrawString("FM RMS " + Frequency(stats.fmRMS).GetFreqString(), divFont, pos, LEFT_ALIGNED);
    pos += QPoint(0, -20);
    DrawString("FM Peak+ " + Frequency(stats.fmPeakPlus).GetFreqString(), divFont, pos, LEFT_ALIGNED);
    pos += QPoint(0, -20);
    DrawString("FM Peak- " + Frequency(stats.fmPeakMinus).GetFreqString(), divFont, pos, LEFT_ALIGNED);
    pos += QPoint(0, -20);
    DrawString("FM Audio Freq " + Frequency(stats.fmAudioFreq).GetFreqString(), divFont, pos, LEFT_ALIGNED);
    pos += QPoint(0, -20);

    str.sprintf("AM RMS %.3lf%%", stats.amRMS * 100.0);
    DrawString(str, divFont, pos, LEFT_ALIGNED);
    pos += QPoint(0, -20);
    str.sprintf("AM Peak+ %.3lf%%", stats.amPeakPlus * 100.0);
    DrawString(str, divFont, pos, LEFT_ALIGNED);
    pos += QPoint(0, -20);
    str.sprintf("AM Peak- %.3lf%%", stats.amPeakMinus * 100.0);
    DrawString(str, divFont, pos, LEFT_ALIGNED);
    pos += QPoint(0, -20);
    DrawString("AM Audio Freq " + Frequency(stats.amAudioFreq).GetFreqString(), divFont, pos, LEFT_ALIGNED);
    pos+= QPoint(0, -20);
    str.sprintf("SINAD %.2f dB", stats.SINAD);
    DrawString(str, divFont, pos, LEFT_ALIGNED);
    pos+= QPoint(0, -20);
    str.sprintf("THD %.2f %%", stats.THD);
    DrawString(str, divFont, pos, LEFT_ALIGNED);
}
