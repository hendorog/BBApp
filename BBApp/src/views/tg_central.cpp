#include "tg_central.h"
#include "model/session.h"

TGCentral::TGCentral(Session *sPtr,
                     QToolBar *mainToolBar,
                     QWidget *parent,
                     Qt::WindowFlags f) :
    CentralWidget(parent, f),
    session_ptr(sPtr)
{
    sweepCount = -1;

    plot = new TGPlot(session_ptr, this);
    plot->move(0, 0);
    connect(this, SIGNAL(updateView()), plot, SLOT(update()));

    connect(sPtr->sweep_settings, SIGNAL(updated(const SweepSettings*)),
            this, SLOT(settingsChanged(const SweepSettings*)));
    connect(sPtr->trace_manager, SIGNAL(updated()),
            plot, SLOT(update()));
}

TGCentral::~TGCentral()
{
    StopStreaming();
}

void TGCentral::GetViewImage(QImage &image)
{

}

void TGCentral::StartStreaming()
{
    sweeping = true;
    sweepThreadHandle = std::thread(&TGCentral::SweepThread, this);
}

void TGCentral::StopStreaming()
{
    sweeping = false;
    if(sweepThreadHandle.joinable()) {
        sweepThreadHandle.join();
    }
}

void TGCentral::ResetView()
{

}

Frequency TGCentral::GetCurrentCenterFreq() const
{
    return session_ptr->sweep_settings->Center();
}

void TGCentral::resizeEvent(QResizeEvent *)
{
    plot->resize(width(), height());
}

void TGCentral::changeMode(int newState)
{
    StopStreaming();

    session_ptr->sweep_settings->setMode(MODE_NETWORK_ANALYZER);

    StartStreaming();
}

void TGCentral::singlePressed()
{
    if(sweepCount <= 0) {
        sweepCount = 1;
    }
}

void TGCentral::continuousPressed()
{
    sweepCount = 1;
}

void TGCentral::SweepThread()
{
    Trace trace;
    reconfigure = true;

    while(sweeping) {
        if(reconfigure) {
            session_ptr->device->Reconfigure(session_ptr->sweep_settings, &trace);
            reconfigure = false;
        }

        if(sweepCount) {
            session_ptr->device->GetSweep(session_ptr->sweep_settings, &trace);
            session_ptr->trace_manager->UpdateTraces(&trace);
            emit updateView();

            if(sweepCount > 0 && trace.IsFullSweep()) {
                sweepCount--;
            }
        }
    }
}

