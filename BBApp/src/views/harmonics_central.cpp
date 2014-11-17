#include "harmonics_central.h"
#include "model/session.h"

HarmonicsCentral::HarmonicsCentral(Session *sPtr,
                                   QToolBar *toolBar,
                                   QWidget *parent,
                                   Qt::WindowFlags f)
    : CentralWidget(parent, f),
      session_ptr(sPtr)
{
    sweeping = false;

    plot = new HarmonicsSpectrumPlot(session_ptr, this);
    plot->move(0, 0);
    connect(this, SIGNAL(updateView()), plot, SLOT(update()));
}

HarmonicsCentral::~HarmonicsCentral()
{
    StopStreaming();
}

void HarmonicsCentral::GetViewImage(QImage &image)
{
    image = plot->grabFrameBuffer();
}

void HarmonicsCentral::StartStreaming()
{
    sweeping = true;
    sweepThreadHandle = std::thread(&HarmonicsCentral::SweepThread, this);
}

void HarmonicsCentral::StopStreaming()
{
    if(sweepThreadHandle.joinable()) {
        sweeping = false;
        sweepThreadHandle.join();
    }
}

void HarmonicsCentral::ResetView()
{

}

Frequency HarmonicsCentral::GetCurrentCenterFreq() const
{
    return session_ptr->sweep_settings->Center();
}

void HarmonicsCentral::resizeEvent(QResizeEvent *)
{
    plot->resize(width(), height());
}

void HarmonicsCentral::changeMode(int newState)
{
    StopStreaming();

    StartStreaming();
}

void HarmonicsCentral::Reconfigure()
{

}

void HarmonicsCentral::SweepThread()
{
    // Special harmonic sweep settings and trace
    SweepSettings hss = *session_ptr->sweep_settings;
    Trace ht;
    Frequency center = hss.Center();

    while(sweeping) {
        for(int i = 0; i < 5; i++) {
            Frequency hCenter = center * (i+1);
            if(hCenter > device_traits::max_frequency()) {
                continue;
            }

            hss.setCenter(hCenter);
            hss.setSpan(200.0e3);
            hss.setAutoVbw(true);
            hss.setAutoRbw(true);

            session_ptr->device->Reconfigure(&hss, &ht);
            session_ptr->device->GetSweep(&hss, &ht);
        }

        emit updateView();
    }
}
