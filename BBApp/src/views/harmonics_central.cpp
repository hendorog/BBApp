#include "harmonics_central.h"

HarmonicsCentral::HarmonicsCentral(Session *sPtr, QWidget *parent, Qt::WindowFlags f)
    : CentralWidget(parent, f),
      session_ptr(sPtr)
{
    toolBar = new QToolBar();

}

HarmonicsCentral::~HarmonicsCentral()
{

}
