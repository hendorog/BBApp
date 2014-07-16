#ifndef TRACE_H
#define TRACE_H

#include "sweep_settings.h"
#include "marker.h"
#include "lib/macros.h"

#include <QColor>

enum TraceType {
    OFF         = 0,
    NORMAL      = 1,
    MAX_HOLD    = 2,
    MIN_HOLD    = 3,
    MIN_AND_MAX = 4,
    AVERAGE_10  = 5,
    AVERAGE_100 = 6
};

// N/A for now
struct PeakInfo {
    PeakInfo(double f, double a)
        : freq(f), amp(a) {}

    Frequency freq;
    Amplitude amp;
};

struct OccupiedBandwidthInfo {
    OccupiedBandwidthInfo()
    {
        enabled = false;
        percentPower = 99.0;
        lix = 0;
        rix = 0;
        traceLen = 0;
        leftMarker.SetActive(true);
        rightMarker.SetActive(true);
    }

    bool enabled;
    double percentPower;
    // Left/Right Index
    Marker leftMarker, rightMarker;
    int lix, rix;
    int traceLen;
    Frequency bandwidth;
    Amplitude totalPower;
};

class Trace {
public:
    Trace(bool active = false, int size = 0);
    ~Trace();

    // Use instead of the operator= overload, more descriptive
    void Copy(const Trace &other);

    void Destroy(void);
    void Clear(void); // Sweep size = 0

    bool Active() const { return _active; }
    //void Activate(bool active) { _active = active; }
    void Disable() { _type = OFF; _active = false; }
    void SetColor(QColor newColor) { color = newColor; }
    void SetSize(int newSize) { Alloc(newSize); }
    void SetFreq(double bin, double start) {
        _binSize = bin;
        _start = start;
    }
    void SetTime(qint64 time) { msFromEpoch = time; }

    bool IsUpdating() const { return _update; }
    void SetUpdate(bool update) { _update = update; }
    QColor Color() const { return color; }

    void SetType(TraceType type);
    TraceType GetType() const { return _type; }

    int Length(void) const { return _size; }

    void SetSettings(const SweepSettings &other) { settings = other; }
    const SweepSettings* GetSettings() const { return &settings; }
    double StartFreq() const { return _start; }
    double StopFreq() const { return _start + _binSize * _size; }
    double BinSize() const { return _binSize; }
    void GetSignalPeak(double *freq, double *amp) const;
    double GetMean() const;
    double GetVariance() const;
    double GetVarianceFromMean(const double mean) const;
    double GetStandardDeviation() const;
    void GetPeakList(std::vector<int> &peak_index_list) const;
    float* Min() const { return _minBuf; }
    float* Max() const { return _maxBuf; }
    qint64 Time() const { return msFromEpoch; }

    void Update(const Trace &other);
    // Export to path, with a given bin size spacing
    // Spacing accomplished via lerping
    bool Export(const QString &path) const;
    // Get the power of the band from [start,stop]
    // Sum up power units, div by
    bool GetChannelPower(double ch_start, double ch_stop, double *power) const;
    // Apply a flat offset, either in linear or logarithmic scale
    void ApplyOffset(double dB);

    void GetOccupiedBandwidth(OccupiedBandwidthInfo &info) const;

private:
    void Alloc(int newSize);  // Allocate both buffers length n

    SweepSettings settings;

    QColor color;
    bool _active;
    bool _update;
    TraceType _type;

    int _size;

    double _binSize; // Either Hz value, or ms in zero-span
    double _start; // Real start freq

    float *_minBuf;
    float *_maxBuf;

    qint64 msFromEpoch;

private:
    DISALLOW_COPY_AND_ASSIGN(Trace)
};

class ChannelPower {
public:
    ChannelPower();
    ~ChannelPower();

    void Configure(bool ch_enable, double ch_width, double ch_spacing);
    void Update(const Trace *trace);

    bool IsEnabled() const { return enabled; }

    bool IsChannelInView(int channel) const;
    double GetChannelStart(int channel) const;
    double GetChannelStop(int channel) const;
    double GetChannelPower(int channel) const;

private:
    bool enabled;
    double width;
    double spacing;
    bool in_view[3];
    double ch_start[3], ch_stop[3], ch_power[3];
};

#endif // TRACE_H
