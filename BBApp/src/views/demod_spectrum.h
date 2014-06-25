#ifndef DEMOD_SPECTRUM_H
#define DEMOD_SPECTRUM_H

#include "gl_sub_view.h"
#include "kiss_fft/kissfft.hh"

class DemodSpectrumPlot : public GLSubView {
    Q_OBJECT

public:
    DemodSpectrumPlot(Session *sPtr, QWidget *parent = 0);
    ~DemodSpectrumPlot();

private:
    typedef std::complex<float> kiss_cplx;
    typedef kissfft<float> FFT;
    std::unique_ptr<FFT> fft;

private:
    DISALLOW_COPY_AND_ASSIGN(DemodSpectrumPlot)
};

#endif // DEMOD_SPECTRUM_H
