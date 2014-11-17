#ifndef HARMONICS_CENTRAL_H
#define HARMONICS_CENTRAL_H

#include <thread>
#include <atomic>

#include "central_stack.h"
#include "harmonics_spectrum.h"

class HarmonicsCentral : public CentralWidget {
    Q_OBJECT

public:
    HarmonicsCentral(Session *sPtr,
                     QToolBar *mainToolBar,
                     QWidget *parent = 0,
                     Qt::WindowFlags f = 0);
    ~HarmonicsCentral();

    virtual void GetViewImage(QImage &image);
    virtual void StartStreaming();
    virtual void StopStreaming();
    virtual void ResetView();

    virtual Frequency GetCurrentCenterFreq() const;

protected:
    void resizeEvent(QResizeEvent *);

public slots:
    virtual void changeMode(int newState);

private:
    void Reconfigure();
    void SweepThread();

    std::thread sweepThreadHandle;
    bool sweeping;

    Session *session_ptr;
    HarmonicsSpectrumPlot *plot;

signals:
    void updateView();

private:
    Q_DISABLE_COPY(HarmonicsCentral)
};

#endif // HARMONICS_CENTRAL_H
