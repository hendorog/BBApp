#include "demod_central.h"

#include "demod_iq_time_plot.h"

DemodCentral::DemodCentral(Session *sPtr, QWidget *parent, Qt::WindowFlags f) :
    CentralWidget(parent, f),
    sessionPtr(sPtr)
{
    toolBar = new QToolBar(this);
    toolBar->move(0, 0);
    toolBar->layout()->setContentsMargins(0, 0, 0, 0);
    toolBar->layout()->setSpacing(0);
    toolBar->addWidget(new FixedSpacer(QSize(10, TOOLBAR_HEIGHT)));

    demodArea = new QMdiArea(this);
    demodArea->move(0, TOOLBAR_HEIGHT);

    plot = new DemodIQTimePlot(sPtr);
    demodArea->addSubWindow(plot);
    demodArea->tileSubWindows();
    connect(this, SIGNAL(updateView()), plot, SLOT(update()));

    plot = new DemodIQTimePlot(sPtr);
    demodArea->addSubWindow(plot);
    demodArea->tileSubWindows();
    connect(this, SIGNAL(updateView()), plot, SLOT(update()));
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

    if(newState == BB_SWEEPING) {
        StartStreaming();
    }
}

void DemodCentral::StreamThread()
{
    sessionPtr->device->Reconfigure(sessionPtr->demod_settings,
                                    sessionPtr->iq_capture);
    sessionPtr->iq_capture->capture = new complex_f[1 << 20];

    while(streaming) {

        if(captureCount) {
            sessionPtr->device->GetIQ(sessionPtr->demod_settings,
                                      sessionPtr->iq_capture);

            UpdateView();
            Sleep(100);

            if(captureCount > 0) {
                captureCount--;
            }
        } else {
            Sleep(100);
        }
    }
}

void DemodCentral::UpdateView()
{
    emit updateView();
}
