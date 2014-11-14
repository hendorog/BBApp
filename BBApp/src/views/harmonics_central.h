#ifndef HARMONICS_CENTRAL_H
#define HARMONICS_CENTRAL_H

#include <thread>
#include <atomic>

#include "central_stack.h"

class HarmonicsCentral : public CentralWidget {
    Q_OBJECT

public:
    HarmonicsCentral(Session *sPtr, QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~HarmonicsCentral();

    virtual void GetViewImage(QImage &image);
    virtual void StartStreaming();
    virtual void StopStreaming();
    virtual void ResetView();

    virtual Frequency GetCurrentCenterFreq() const;

public slots:
    virtual void changeMode(int newState);

private:
    void Reconfigure();
    void SweepThread();

    std::thread sweepThreadHandle;
    bool streaming;

    Session *session_ptr;

private:
   Q_DISABLE_COPY(HarmonicsCentral)
};

#endif // HARMONICS_CENTRAL_H
