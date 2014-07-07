#include "demod_central.h"

#include "demod_iq_time_plot.h"
#include "demod_spectrum_plot.h"

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

    demodArea = new QMdiArea(this);
    demodArea->move(0, TOOLBAR_HEIGHT);

    DemodSpectrumPlot *freqPlt = new DemodSpectrumPlot(sPtr);
    freqPlt->setWindowTitle(tr("Spectrum Plot"));
    demodArea->addSubWindow(freqPlt);
    demodArea->tileSubWindows();
    connect(this, SIGNAL(updateView()), freqPlt, SLOT(update()));

    plot = new DemodIQTimePlot(sPtr);
    plot->setWindowTitle(tr("IQ Plot"));
    demodArea->addSubWindow(plot);
    demodArea->tileSubWindows();
    connect(this, SIGNAL(updateView()), plot, SLOT(update()));

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
    demodArea->tileSubWindows();
}

void DemodCentral::changeMode(int newState)
{
    StopStreaming();
    captureCount = -1;

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
    qDebug() << sweepLen << "\n";
    qDebug() << iqc->desc.decimation << "\n";

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
