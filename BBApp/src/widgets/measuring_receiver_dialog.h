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
    MeasuringReceiver(Device *devicePtr,
                      double initialCenter,
                      QWidget *parent = 0);
    ~MeasuringReceiver();

public slots:
    //virtual void accept();

private slots:
    void syncPressed();

protected:

private:
    void ProcessThread();

    FrequencyEntry *freqEntry;
    Label *highLabel, *midLabel, *lowLabel;
    QButtonGroup *ampGroup;
    Label *centerReadout, *powerReadout;
    Label *relativeReadout, *averageReadout;

    std::thread threadHandle;
    std::atomic<bool> running;
    Device *device;
};

#endif // MEASURING_RECEIVER_DIALOG_H
