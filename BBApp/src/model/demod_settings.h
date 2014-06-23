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

class DemodSettings {
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
    Frequency VBW() const { return vbw; }
    Time SweepTime() const { return sweepTime; }

    TriggerType TrigType() const { return trigType; }
    TriggerEdge TrigEdge() const { return trigEdge; }
    Amplitude TrigAmplitude() const { return trigAmplitude; }

private:
    Amplitude inputPower;
    Frequency centerFreq;
    int gain;
    int atten;
    int decimationFactor;
    Frequency bandwidth;
    Frequency vbw;
    Time sweepTime;

    TriggerType trigType;
    TriggerEdge trigEdge;
    Amplitude trigAmplitude;
};

// Descriptor for the device IQ stream
struct IQDescriptor {
    IQDescriptor() {
        sampleRate = 0;
        decimation = 0;
        timeDelta = 0.0;
        totalSamples = 0;
        bandwidth = 0.0;
    }

    int sampleRate;
    int decimation;
    double timeDelta; // Delta 'time' per sample in seconds?
    int totalSamples;
    double bandwidth;
};

// Represents a single
struct IQCapture {
    IQCapture() :
        capture(nullptr)
    {
        simdZero_32s(triggers, 70);
    }
    ~IQCapture()
    {
        if(capture) delete [] capture;
    }

    IQDescriptor desc;
    complex_f *capture;
    int triggers[70];
};

// One full sweep
struct IQSweep {
    complex_f *sweep;
};

#endif // DEMOD_SETTINGS_H
