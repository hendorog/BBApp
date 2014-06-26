#ifndef DEMOD_SPECTRUM_PLOT_H
#define DEMOD_SPECTRUM_PLOT_H

#include "gl_sub_view.h"

class DemodSpectrumPlot : public GLSubView {
    Q_OBJECT

public:
    DemodSpectrumPlot(Session *sPtr, QWidget *parent = 0);
    ~DemodSpectrumPlot();

protected:
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);

private:
    void DrawSpectrum();
    void DrawTrace(const GLVector &v);
    void DrawPlotText();

    std::unique_ptr<FFT> fft;
    std::vector<complex_f> postTransform;

    QPoint grat_sz, grat_ul, grat_ll;
    GLVector grat, gratBorder;
    GLuint gratVBO, gratBorderVBO;

    GLVector spectrum;
    GLuint traceVBO;
    GLFont textFont;

private:
    DISALLOW_COPY_AND_ASSIGN(DemodSpectrumPlot)
};

#endif // DEMOD_SPECTRUM_PLOT_H
