#ifndef MEASURING_RECEIVER_DIALOG_H
#define MEASURING_RECEIVER_DIALOG_H

#include <thread>
#include <atomic>

#include <QDialog>

#include "../model/device.h"
#include "../widgets/entry_widgets.h"

class MeasuringReceiver : public QDialog {
    Q_OBJECT

public:
    MeasuringReceiver(const Device *devicePtr,
                      double initialCenter,
                      QWidget *parent = 0);
    ~MeasuringReceiver();

protected:

private:
    void ProcessThread();

    FrequencyEntry *freqEntry;
    std::thread threadHandle;
    std::atomic<bool> running;
    const Device *device;
};

#endif // MEASURING_RECEIVER_DIALOG_H
