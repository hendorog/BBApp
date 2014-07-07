#ifndef DEVICE_BB60A_H
#define DEVICE_BB60A_H

#include "device.h"

class Preferences;

class DeviceBB60A : public Device {
public:
    DeviceBB60A(const Preferences *preferences);
    virtual ~DeviceBB60A();

    bool OpenDevice();
    bool CloseDevice();
    bool Abort();
    bool Preset();
    // Sweep
    bool Reconfigure(const SweepSettings *s, Trace *t);
    bool GetSweep(const SweepSettings *s, Trace *t);
    // Stream
    bool Reconfigure(const DemodSettings *s, IQDescriptor *iqc);
    bool GetIQ(IQCapture *iqc);

    bool IsPowered() const;

private:


private:
    DISALLOW_COPY_AND_ASSIGN(DeviceBB60A)
};

#endif // DEVICE_BB60A_H
