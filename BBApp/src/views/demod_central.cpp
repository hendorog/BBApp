#include "demod_central.h"

DemodCentral::DemodCentral(Session *sPtr, QWidget *parent, Qt::WindowFlags f) :
    CentralWidget(parent, f)
{
    toolBar = new QToolBar(this);
    toolBar->move(0, 0);
    toolBar->layout()->setContentsMargins(0, 0, 0, 0);
    toolBar->layout()->setSpacing(0);
    toolBar->addWidget(new FixedSpacer(QSize(10, TOOLBAR_HEIGHT)));

    demodArea = new QMdiArea(this);
    demodArea->move(0, TOOLBAR_HEIGHT);

    for(int i = 0; i < 6; i++) {
        QWidget *w = new QWidget();
        w->resize(200, 200);
        QMdiSubWindow *sw = demodArea->addSubWindow(w);
        demodArea->tileSubWindows();
    }
}

DemodCentral::~DemodCentral()
{

}

void DemodCentral::StartStreaming()
{

}

void DemodCentral::StopStreaming()
{

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
}

void DemodCentral::changeMode(int newState)
{

}

