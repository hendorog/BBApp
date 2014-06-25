#ifndef DEMOD_SPECTRUM_H
#define DEMOD_SPECTRUM_H

#include "gl_sub_view.h"

class DemodSpectrumPlot : public GLSubView {
    Q_OBJECT

public:
    DemodSpectrumPlot(Session *sPtr, QWidget *parent = 0);
    ~DemodSpectrumPlot();

protected:
    void resizeEvent(QResizeEvent *);

private:
    std::unique_ptr<FFT> fft;

    QPoint grat_sz, grat_ul, grat_ll;
    GLVector grat, gratBorder;
    GLuint gratVBO, gratBorderVBO;

    GLVector spectrum;
    GLuint traceVBO;

private:
    DISALLOW_COPY_AND_ASSIGN(DemodSpectrumPlot)
};

#endif // DEMOD_SPECTRUM_H
