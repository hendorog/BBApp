#include "demod_spectrum.h"

DemodSpectrumPlot::DemodSpectrumPlot(Session *sPtr, QWidget *parent) :
    GLSubView(sPtr, parent)
{
    fft = std::unique_ptr<FFT>(new FFT(1024, false));
}

DemodSpectrumPlot::~DemodSpectrumPlot()
{

}


