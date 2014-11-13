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

private:
    Session *session_ptr;

private:
   Q_DISABLE_COPY(HarmonicsCentral)
};

#endif // HARMONICS_CENTRAL_H
