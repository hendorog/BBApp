#ifndef DEMOD_SETTINGS_H
#define DEMOD_SETTINGS_H

#include "lib/bb_lib.h"

#include <QSettings>

void get_next_iq_bandwidth(Frequency current, Frequency &next, int &decimationRate);
void get_prev_iq_bandwidth(Frequency current, Frequency &prev, int &decimationRate);
void get_clamped_iq_bandwidth(Frequency &bw, int downsampleFactor);

enum TriggerType {
    TriggerTypeNone = 0,
    TriggerTypeVideo = 1,
    TriggerTypeExternal = 2
};

enum TriggerEdge {
    TriggerEdgeRising = 0,
    TriggerEdgeFalling = 1
};

class DemodSettings : public QObject {
    Q_OBJECT

public:
    DemodSettings();
    DemodSettings(const DemodSettings &other);
    ~DemodSettings() {}

    DemodSettings& operator=(const DemodSettings &other);
    bool operator==(const DemodSettings &other) const;
    bool operator!=(const DemodSettings &other) const;

    void LoadDefaults();
    bool Load(QSettings &s);
    bool Save(QSettings &s) const;

    Amplitude InputPower() const { return inputPower; }
    Frequency CenterFreq() const { return centerFreq; }
    int Gain() const { return gain; }
    int Atten() const { return atten; }
    int DecimationFactor() const { return decimationFactor; }
    Frequency Bandwidth() const { return bandwidth; }
    bool AutoBandwidth() const { return autoBandwidth; }
    Time SweepTime() const { return sweepTime; }

    TriggerType TrigType() const { return trigType; }
    TriggerEdge TrigEdge() const { return trigEdge; }
    Amplitude TrigAmplitude() const { return trigAmplitude; }

    bool MREnabled() const { return mrEnabled; }

private:
    // Call before updating, configures an appropriate sweep time value
    void ClampSweepTime();
    void UpdateAutoBandwidths();
    void SetMRConfiguration();

    Amplitude inputPower;
    Frequency centerFreq;
    int gain; // Index, 0 == auto
    int atten; // Index, 0 == auto
    int decimationFactor;
    Frequency bandwidth;
    bool autoBandwidth;
    Time sweepTime;

    TriggerType trigType;
    TriggerEdge trigEdge;
    Amplitude trigAmplitude;

    bool mrEnabled;

public slots:
    void setInputPower(Amplitude);
    void setCenterFreq(Frequency);
    void setGain(int);
    void setAtten(int);
    void setDecimation(int);
    void setBandwidth(Frequency);
    void setAutoBandwidth(bool);
    void setSweepTime(Time);

    void setTrigType(int);
    void setTrigEdge(int);
    void setTrigAmplitude(Amplitude);

    void setMREnabled(bool);

signals:
    void updated(const DemodSettings*);
};

// Descriptor for the device IQ stream
struct IQDescriptor {
    IQDescriptor() {
        sampleRate = 0;
        decimation = 0;
        timeDelta = 0.0;
        returnLen = 0;
        bandwidth = 0.0;
    }

    int sampleRate;
    int decimation;
    double timeDelta; // Delta 'time' per sample in seconds?
    int returnLen;
    double bandwidth;
};

// Represents a single
struct IQCapture {
    IQCapture()
    {
        simdZero_32s(triggers, 70);
    }
    ~IQCapture() {}

    IQDescriptor desc;
    std::vector<complex_f> capture;
    int triggers[70];
};

// One full sweep
typedef struct IQSweep {
    DemodSettings settings;
    std::vector<complex_f> iq;
    std::vector<float> waveform;
    bool triggered;
} IQSweep;

#endif // DEMOD_SETTINGS_H
