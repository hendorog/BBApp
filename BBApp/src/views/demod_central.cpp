#include "demod_central.h"

#include "demod_iq_time_plot.h"
#include "demod_spectrum_plot.h"
#include "demod_sweep_plot.h"

DemodCentral::DemodCentral(Session *sPtr, QWidget *parent, Qt::WindowFlags f) :
    CentralWidget(parent, f),
    sessionPtr(sPtr),
    reconfigure(false)
{
    toolBar = new QToolBar(this);
    toolBar->move(0, 0);
    toolBar->layout()->setContentsMargins(0, 0, 0, 0);
    toolBar->layout()->setSpacing(0);
    toolBar->addWidget(new FixedSpacer(QSize(10, TOOLBAR_HEIGHT)));

    demodArea = new MdiArea(this);
    demodArea->move(0, TOOLBAR_HEIGHT);

    DemodSweepPlot *sweepPlot = new DemodSweepPlot(sPtr);
    sweepPlot->setWindowTitle("Demod Plot");
    demodArea->addSubWindow(sweepPlot);
    connect(this, SIGNAL(updateView()), sweepPlot, SLOT(update()));

    DemodSpectrumPlot *freqPlot = new DemodSpectrumPlot(sPtr);
    freqPlot->setWindowTitle(tr("Spectrum Plot"));
    demodArea->addSubWindow(freqPlot);
    connect(this, SIGNAL(updateView()), freqPlot, SLOT(update()));

    DemodIQTimePlot *iqPlot = new DemodIQTimePlot(sPtr);
    iqPlot->setWindowTitle(tr("IQ Plot"));
    demodArea->addSubWindow(iqPlot);
    connect(this, SIGNAL(updateView()), iqPlot, SLOT(update()));

    connect(sessionPtr->demod_settings, SIGNAL(updated(const DemodSettings*)),
            this, SLOT(updateSettings(const DemodSettings*)));
}

DemodCentral::~DemodCentral()
{
    StopStreaming();
}

void DemodCentral::StartStreaming()
{
    streaming = true;
    threadHandle = std::thread(&DemodCentral::StreamThread, this);
}

void DemodCentral::StopStreaming()
{
    streaming = false;
    if(threadHandle.joinable()) {
        threadHandle.join();
    }
}

void DemodCentral::ResetView()
{

}

void DemodCentral::GetViewImage(QImage &image)
{

}

void DemodCentral::resizeEvent(QResizeEvent *)
{
    toolBar->resize(width(), TOOLBAR_HEIGHT);
    demodArea->resize(width(), height() - TOOLBAR_HEIGHT);
    //demodArea->tileSubWindows();
    demodArea->retile();
}

void DemodCentral::changeMode(int newState)
{
    StopStreaming();
    captureCount = -1;

    sessionPtr->sweep_settings->setMode((OperationalMode)newState);

    if(newState == MODE_ZERO_SPAN) {
        StartStreaming();
    }
}

void DemodCentral::Reconfigure(DemodSettings *ds, IQCapture *iqc)
{
    if(!sessionPtr->device->Reconfigure(ds, &iqc->desc)) {
        *ds = lastConfig;
    } else {
        lastConfig = *ds;
    }

    // Resize single capture size and full sweep
    iqc->capture.resize(iqc->desc.returnLen);

    int sweepLen = ds->SweepTime().Val() / iqc->desc.timeDelta;

    sessionPtr->iq_capture.len = sweepLen;
    sessionPtr->iq_capture.sweep.resize(sweepLen);

    reconfigure = false;
}

void DemodCentral::StreamThread()
{
    IQCapture iqc;
    IQSweep &iqs = sessionPtr->iq_capture;

    Reconfigure(sessionPtr->demod_settings, &iqc);

    while(streaming) {

        if(captureCount) {
            if(reconfigure) Reconfigure(sessionPtr->demod_settings, &iqc);
            qint64 start = bb_lib::get_ms_since_epoch();

            int retrieved = 0, toRetrieve = iqs.len;

            while(toRetrieve > 0) {
                sessionPtr->device->GetIQ(&iqc);
                int toCopy = bb_lib::min2(toRetrieve, iqc.desc.returnLen);
                simdCopy_32fc(&iqc.capture[0], &iqs.sweep[retrieved], toCopy);
                toRetrieve -= toCopy;
                retrieved += toCopy;
            }

            UpdateView();

            // Force 30 fps update rate
            qint64 elapsed = bb_lib::get_ms_since_epoch() - start;
            if(elapsed < 64) Sleep(64 - elapsed);
            if(captureCount > 0) {
                captureCount--;
            }
        } else {
            Sleep(64);
        }
    }
}

void DemodCentral::updateSettings(const DemodSettings *ds)
{
    reconfigure = true;
}

void DemodCentral::UpdateView()
{
    emit updateView();
}
