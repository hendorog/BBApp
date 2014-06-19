#ifndef DEMOD_SETTINGS_H
#define DEMOD_SETTINGS_H

#include "lib/bb_lib.h"

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

class TriggerSettings {
public:

private:
};

class DemodSettings {
public:
    DemodSettings();
    ~DemodSettings();

private:

    Amplitude inputPower;
    Frequency centerFreq;
    int gain;
    int atten;
    int decimationFactor;
    Frequency bandwidth;
    Frequency vbw;
    Time sweepTime;

    TriggerType triggerType;
    TriggerEdge triggerEdge;
    Amplitude triggerAmplitude;
};

struct IQDescriptor {
    IQDescriptor() {
        totalSamples = 0;
    }

    int sampleRate;
    int decimation;
    double timeDelta; // Delta 'time' per sample in seconds?
    int totalSamples;
    double bandwidth;
};

// Represents a single
struct IQCapture {
    IQCapture() : capture(nullptr) {}

    complex_f *capture;
    IQDescriptor desc;
};

#endif // DEMOD_SETTINGS_H
