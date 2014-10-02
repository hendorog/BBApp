#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>

#include "lib/bb_lib.h"
#include "lib/bb_api.h"
#include "lib/macros.h"

#include "model/sweep_settings.h"
#include "model/demod_settings.h"
#include "model/trace.h"

const int TIMEBASE_INTERNAL = 0;
const int TIMEBASE_EXT_AC = 1;
const int TIMEBASE_EXT_DC = 2;

class Preferences;

class Device : public QObject {
    Q_OBJECT

protected:
    const Preferences *prefs;

public:
    Device(const Preferences *preferences) : prefs(preferences)
    {
        current_temp = 0.0;
        voltage = 0.0;
        current = 0.0;
    }
    virtual ~Device() = 0;

    virtual bool OpenDevice() = 0;
    virtual bool CloseDevice() = 0;
    virtual bool Abort() = 0;
    virtual bool Preset() = 0;
    virtual bool Reconfigure(const SweepSettings *s, Trace *t) = 0;
    // Returns false on an unfixable error
    virtual bool GetSweep(const SweepSettings *s, Trace *t) = 0;
    virtual bool Reconfigure(const DemodSettings *s, IQDescriptor *capture) = 0;
    virtual bool GetIQ(IQCapture *iqc) = 0;
    virtual bool GetIQFlush(IQCapture *iqc, bool flush) = 0;
    virtual bool ConfigureForTRFL(double center, int atten, int gain, IQDescriptor &desc) = 0;

    bool IsOpen() const { return open; }
    int Handle() const { return id; }

    virtual QString GetDeviceString() const = 0;
    virtual void UpdateDiagnostics() = 0;

    const char* GetLastStatusString() const { return bbGetErrorString(lastStatus); }
    bbStatus GetLastStatus() const { return lastStatus; }
    const char* GetStatusString(bbStatus status) const { return bbGetErrorString(status); }

    int DeviceType() const { return device_type; }
    int SerialNumber() const { return serial_number; }
    QString SerialString() const { return serial_string; }
    QString FirmwareString() const { return firmware_string; }

    float LastConfiguredTemp() const { return last_temp; }
    float CurrentTemp() const { return current_temp; }
    float Voltage() const { return voltage; }
    virtual bool IsPowered() const = 0;
    bool ADCOverflow() const { return adc_overflow; }

    int TimebaseReference() const { return timebase_reference; }
    bool NeedsTempCal() const { return fabs(last_temp - current_temp) > 2; }

protected:
    bool open;
    int id;
    bbStatus lastStatus; // Each devices enum

    float last_temp; // Temp of last configured state
    float current_temp; // Last retrieved temp
    float voltage;
    float current;
    //
    bool update_diagnostics_string;

    int device_type;
    int serial_number;
    QString serial_string;
    QString firmware_string;

    int timebase_reference; // Internal/Ext(AC/DC)
    bool reconfigure_on_next; // set true to reconfigure on next sweep

    bool adc_overflow;

public slots:
    void setTimebase(int new_val) {
        timebase_reference = new_val;
        reconfigure_on_next = true;
    }

signals:
    void connectionIssues();

private:
    DISALLOW_COPY_AND_ASSIGN(Device)
};

inline Device::~Device() {}

#endif // DEVICE_H
