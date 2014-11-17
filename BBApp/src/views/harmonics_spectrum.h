#ifndef HARMONICS_SPECTRUM_H
#define HARMONICS_SPECTRUM_H

#include "gl_sub_view.h"

class HarmonicsSpectrumPlot : public GLSubView {
    Q_OBJECT

public:
    HarmonicsSpectrumPlot(Session *sPtr, QWidget *parent = 0);
    ~HarmonicsSpectrumPlot();

protected:
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);

private:

private:
    Q_DISABLE_COPY(HarmonicsSpectrumPlot)
};

#endif // HARMONICS_SPECTRUM_H
