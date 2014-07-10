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

    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    toolBar->addWidget(spacer);

    QPushButton *singleSweep, *autoSweep, *presetBtn;

    singleSweep = new QPushButton("Single", toolBar);
    singleSweep->setObjectName("BBPushButton");
    singleSweep->setFixedSize(120, TOOLBAR_HEIGHT - 4);
    toolBar->addWidget(singleSweep);
    connect(singleSweep, SIGNAL(clicked()), this, SLOT(singlePressed()));

    autoSweep = new QPushButton("Auto", toolBar);
    autoSweep->setObjectName("BBPushButton");
    autoSweep->setFixedSize(120, TOOLBAR_HEIGHT - 4);
    toolBar->addWidget(autoSweep);
    connect(autoSweep, SIGNAL(clicked()), this, SLOT(autoPressed()));

    presetBtn = new QPushButton("Preset", toolBar);
    presetBtn->setObjectName("BBPresetButton");
    presetBtn->setFixedSize(120, TOOLBAR_HEIGHT - 4);
    toolBar->addWidget(presetBtn);
    connect(presetBtn, SIGNAL(clicked()), this, SIGNAL(presetDevice()));

    demodArea = new MdiArea(this);
    demodArea->move(0, TOOLBAR_HEIGHT);

    DemodSweepArea *sweepPlot = new DemodSweepArea(sPtr);
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

void DemodCentral::GetCapture(const DemodSettings *ds, IQCapture &iqc, IQSweep &iqs, Device *device)
{
    int retrieved = 0, toRetrieve = iqs.len;
    int firstIx = 0;
    iqs.triggered = false;

    if(ds->TrigType() == TriggerTypeVideo) {
        double trigVal = ds->TrigAmplitude().ConvertToUnits(DBM);
        trigVal = pow(10.0, (trigVal/10.0));
        int maxTimeForTrig = 0;
        while(maxTimeForTrig < 7) {
            device->GetIQ(&iqc);
            if(ds->TrigEdge() == TriggerEdgeRising) {
                firstIx = find_rising_trigger(&iqc.capture[0], trigVal, iqc.capture.size());
            } else {
                firstIx = find_falling_trigger(&iqc.capture[0], trigVal, iqc.capture.size());
            }
            if(firstIx > 0) {
                iqs.triggered = true;
                break;
            }
            firstIx = 0;
            maxTimeForTrig++;
        }
    } else if(ds->TrigType() == TriggerTypeExternal) {

    } else {
        device->GetIQ(&iqc);
        iqs.triggered = true;
    }

    while(toRetrieve > 0) {
        int toCopy = bb_lib::min2(toRetrieve, iqc.desc.returnLen - firstIx);
        simdCopy_32fc(&iqc.capture[firstIx], &iqs.sweep[retrieved], toCopy);
        firstIx = 0;
        toRetrieve -= toCopy;
        retrieved += toCopy;
        if(toRetrieve > 0) {
            device->GetIQ(&iqc);
        }
    }
}

void DemodCentral::StreamThread()
{
    IQCapture iqc;
    IQSweep &iqs = sessionPtr->iq_capture;

    Reconfigure(sessionPtr->demod_settings, &iqc);

    while(streaming) {

        if(captureCount) {
            if(reconfigure) {
                Reconfigure(sessionPtr->demod_settings, &iqc);
            }
            qint64 start = bb_lib::get_ms_since_epoch();

            GetCapture(sessionPtr->demod_settings, iqc, iqs, sessionPtr->device);

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

void DemodCentral::singlePressed()
{
    if(captureCount < 0) {
        captureCount = 1;
    } else {
        captureCount++;
    }
}

void DemodCentral::autoPressed()
{
    captureCount = -1;
}

void DemodCentral::UpdateView()
{
    emit updateView();
}
