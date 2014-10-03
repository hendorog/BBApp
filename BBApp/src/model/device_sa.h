#ifndef DEVICE_SA_H
#define DEVICE_SA_H

#include "device.h"

class Preferences;

class DeviceSA : public Device {
public:
    DeviceSA(const Preferences *preferences);
    virtual ~DeviceSA();

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
    bool GetIQFlush(IQCapture *iqc, bool sync);
    bool ConfigureForTRFL(double center, int atten, int gain, IQDescriptor &desc);

    QString GetDeviceString() const;
    void UpdateDiagnostics();

    bool IsPowered() const;
    bool NeedsTempCal() const;

private:
    DISALLOW_COPY_AND_ASSIGN(DeviceSA)
};

#endif // DEVICE_SA_H
