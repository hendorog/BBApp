#ifndef DEVICE_SA_H
#define DEVICE_SA_H

#include "device.h"

class Preferences;

class DeviceSA : public Device {
public:
    DeviceSA(const Preferences *preferences);
    virtual ~DeviceSA();

    virtual bool OpenDevice();
    virtual bool CloseDevice();
    virtual bool Abort();
    virtual bool Preset();
    // Sweep
    virtual bool Reconfigure(const SweepSettings *s, Trace *t);
    virtual bool GetSweep(const SweepSettings *s, Trace *t);
    // Stream
    virtual bool Reconfigure(const DemodSettings *s, IQDescriptor *iqc);
    virtual bool GetIQ(IQCapture *iqc);
    virtual bool GetIQFlush(IQCapture *iqc, bool sync);
    virtual bool ConfigureForTRFL(double center, int atten, int gain, IQDescriptor &desc);
    virtual bool ConfigureAudio(const AudioSettings &as);
    virtual bool GetAudio(float *audio);

    virtual QString GetDeviceString() const;
    virtual void UpdateDiagnostics();

    virtual bool IsPowered() const;
    virtual bool NeedsTempCal() const;

private:
    DISALLOW_COPY_AND_ASSIGN(DeviceSA)
};

#endif // DEVICE_SA_H
