#include "harmonics_central.h"
#include "model/session.h"

HarmonicsCentral::HarmonicsCentral(Session *sPtr, QWidget *parent, Qt::WindowFlags f)
    : CentralWidget(parent, f),
      session_ptr(sPtr)
{
    toolBar = new QToolBar();


}

HarmonicsCentral::~HarmonicsCentral()
{
    StopStreaming();
}

void HarmonicsCentral::GetViewImage(QImage &image)
{

}

void HarmonicsCentral::StartStreaming()
{

}

void HarmonicsCentral::StopStreaming()
{

}

void HarmonicsCentral::ResetView()
{

}

Frequency HarmonicsCentral::GetCurrentCenterFreq() const
{
    return session_ptr->sweep_settings->Center();
}

void HarmonicsCentral::changeMode(int newState)
{

}

void HarmonicsCentral::Reconfigure()
{

}

void HarmonicsCentral::SweepThread()
{

}
