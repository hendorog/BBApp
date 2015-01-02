#ifndef DEVICE_SA_H
#define DEVICE_SA_H

#include "device.h"
#include "lib/sa_api.h"

class Preferences;

class DeviceSA : public Device {
public:
    DeviceSA(const Preferences *preferences);
    virtual ~DeviceSA();

    virtual bool OpenDevice();
    virtual bool OpenDeviceWithSerial(int serialToOpen);
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

    virtual const char* GetLastStatusString() const;

    virtual QString GetDeviceString() const;
    virtual void UpdateDiagnostics();

    virtual bool IsPowered() const;
    virtual bool NeedsTempCal() const;

    virtual bool IsCompatibleWithTg() const { return true; }
    virtual bool AttachTg();
    virtual bool IsTgAttached();
    virtual bool SetTg(Frequency freq, double amp);

    virtual void TgStoreThrough();
    virtual void TgStoreThroughPad();

    virtual int MsPerIQCapture() const { return 34; }

private:
    saDeviceType deviceType;
    saStatus lastStatus;

private:
    DISALLOW_COPY_AND_ASSIGN(DeviceSA)
};

#endif // DEVICE_SA_H
