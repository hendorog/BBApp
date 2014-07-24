#include "demod_settings.h"

struct iq_auto_bandwidth_entry {
    Frequency bandwidth;
    int decimationRate;
};

static iq_auto_bandwidth_entry iq_auto_bandwidth_lut[] = {
    { 27.0e6,  0 },
    { 10.0e6,  1 },
    { 5.0e6,   2 },
    { 2.0e6,   3 },
    { 1.0e6,   4 },
    { 500.0e3, 5 },
    { 200.0e3, 6 },
    { 100.0e3, 7 }
};
static int iq_auto_bandwidth_lut_sz =
        sizeof(iq_auto_bandwidth_lut) /
        sizeof(iq_auto_bandwidth_entry);

void get_next_iq_bandwidth(Frequency current,
                           Frequency &next,
                           int &decimationRate)
{
    int lutIx = 0;
    while(current < iq_auto_bandwidth_lut[lutIx].bandwidth &&
          lutIx < iq_auto_bandwidth_lut_sz)
        lutIx++;

}

// Index is the decimation index
static double bin_size_table[] = {
    1.0 / 40.0e6,
    1.0 / 20.0e6,
    1.0 / 10.0e6,
    1.0 / 5.0e6,
    1.0 / 2.5e6,
    1.0 / 1.25e6,
    1.0 / 0.625e6,
    1.0 / 0.3125e6,
    1.0 / 0.15625e6
};
static int bin_size_table_sz =
        sizeof(bin_size_table) / sizeof(double);

static double max_bw_table[] = {
    27.0e6,
    17.8e6,
    8.0e6,
    3.75e6,
    2.0e6,
    1.0e6,
    0.5e6,
    0.25e6
};
static int max_bandwidth_sz =
        sizeof(max_bw_table) / sizeof(double);

DemodSettings::DemodSettings()
{
    LoadDefaults();
}

DemodSettings::DemodSettings(const DemodSettings &other)
{
    *this = other;
}

DemodSettings& DemodSettings::operator=(const DemodSettings &other)
{
    inputPower = other.InputPower();
    centerFreq = other.CenterFreq();
    gain = other.Gain();
    atten = other.Atten();
    decimationFactor = other.DecimationFactor();
    bandwidth = other.Bandwidth();
    autoBandwidth = other.AutoBandwidth();
    sweepTime = other.SweepTime();

    trigType = other.TrigType();
    trigEdge = other.TrigEdge();
    trigAmplitude = other.TrigAmplitude();

    mrEnabled = other.MREnabled();

    return *this;
}

bool DemodSettings::operator==(const DemodSettings &other) const
{
    if(inputPower != other.InputPower()) return false;
    if(centerFreq != other.CenterFreq()) return false;
    if(gain != other.Gain()) return false;
    if(atten != other.Atten()) return false;
    if(decimationFactor != other.DecimationFactor()) return false;
    if(bandwidth != other.Bandwidth()) return false;
    if(autoBandwidth != other.AutoBandwidth()) return false;
    if(sweepTime != other.SweepTime()) return false;

    if(trigType != other.TrigType()) return false;
    if(trigEdge != other.TrigEdge()) return false;
    if(trigAmplitude != other.TrigAmplitude()) return false;

    if(mrEnabled != other.MREnabled()) return false;

    return true;
}

bool DemodSettings::operator!=(const DemodSettings &other) const
{
    return !(*this == other);
}

void DemodSettings::LoadDefaults()
{
    inputPower = 0.0;
    centerFreq = 1.0e9;
    gain = 0; // Index, 0 == auto
    atten = 0; // Index, 0 == auto
    decimationFactor = 6;
    //bandwidth = iq_auto_bandwidth_lut[decimationFactor].bandwidth;
    bandwidth = max_bw_table[decimationFactor];
    autoBandwidth = true;
    sweepTime = 1.0e-3;

    trigType = TriggerTypeNone;
    trigEdge = TriggerEdgeRising;
    trigAmplitude = 0.0;

    mrEnabled = false;
}

bool DemodSettings::Load(QSettings &s)
{
    inputPower = s.value("Demod/InputPower", InputPower().Val()).toDouble();
    centerFreq = s.value("Demod/CenterFrequency", CenterFreq().Val()).toDouble();
    gain = s.value("Demod/Gain", Gain()).toInt();
    atten = s.value("Demod/Atten", Atten()).toInt();
    decimationFactor = s.value("Demod/Decimation", DecimationFactor()).toInt();
    bandwidth = s.value("Demod/Bandwidth", Bandwidth().Val()).toDouble();
    autoBandwidth = s.value("Demod/AutoBandwidth", AutoBandwidth()).toBool();
    sweepTime = s.value("Demod/SweepTime", SweepTime().Val()).toDouble();

    trigType = (TriggerType)s.value("Demod/TriggerType", TrigType()).toInt();
    trigEdge = (TriggerEdge)s.value("Demod/TriggerEdge", TrigEdge()).toInt();
    trigAmplitude = s.value("Demod/TriggerAmplitude",
                            TrigAmplitude().Val()).toDouble();

    emit updated(this);
    return true;
}

bool DemodSettings::Save(QSettings &s) const
{
    s.setValue("Demod/InputPower", InputPower().Val());
    s.setValue("Demod/CenterFrequency", CenterFreq().Val());
    s.setValue("Demod/Gain", Gain());
    s.setValue("Demod/Atten", Atten());
    s.setValue("Demod/Decimation", DecimationFactor());
    s.setValue("Demod/Bandwidth",Bandwidth().Val());
    s.setValue("Demod/AutoBandwidth", AutoBandwidth());
    s.setValue("Demod/SweepTime", SweepTime().Val());

    s.setValue("Demod/TriggerType", TrigType());
    s.setValue("Demod/TriggerEdge", TrigEdge());
    s.setValue("Demod/TriggerAmplitude", TrigAmplitude().Val());

    return true;
}

void DemodSettings::setInputPower(Amplitude a)
{
    a.Clamp(Amplitude(-100, DBM), Amplitude(20.0, DBM));
    inputPower = a;
    emit updated(this);
}

void DemodSettings::setCenterFreq(Frequency f)
{
    if(f > Frequency(6.0e9)) {
        f = Frequency(6.0e9);
    }

    if(f < Frequency(20.0e6)) {
        f = Frequency(20.0e6);
    }

    centerFreq = f;
    emit updated(this);
}

void DemodSettings::setGain(int g)
{
    if(g < 0) g = 0;
    if(g > 4) g = 4;

    gain = g;
    emit updated(this);
}

void DemodSettings::setAtten(int a)
{
    if(a < 0) a = 0;
    if(a > 4) a = 4;

    atten = a;
    emit updated(this);
}

void DemodSettings::setDecimation(int d)
{
    if(d < 0) d = 0;
    if(d > 7) d = 7;

    decimationFactor = d;

    UpdateAutoBandwidths();
    ClampSweepTime();

    emit updated(this);
}

void DemodSettings::setBandwidth(Frequency bw)
{
    autoBandwidth = false;
    double maxBandwidth = max_bw_table[decimationFactor];

    if(bw < 100.0e3) {
        bandwidth = 100.0e3;
    } else if(bw > maxBandwidth) {
        bandwidth = maxBandwidth;
    } else {
        bandwidth = bw;
    }

    UpdateAutoBandwidths();
    emit updated(this);
}

void DemodSettings::setAutoBandwidth(bool setAuto)
{
    autoBandwidth = setAuto;
    UpdateAutoBandwidths();
    emit updated(this);
}

// Clamp sweep time to represent a maximum sweep length
void DemodSettings::setSweepTime(Time t)
{
    sweepTime = t;
    ClampSweepTime();

    emit updated(this);
}

void DemodSettings::setTrigType(int tt)
{
    trigType = (TriggerType)tt;
    emit updated(this);
}

void DemodSettings::setTrigEdge(int te)
{
    trigEdge = (TriggerEdge)te;
    emit updated(this);
}

void DemodSettings::setTrigAmplitude(Amplitude ta)
{
    trigAmplitude = ta;
    emit updated(this);
}

void DemodSettings::setMREnabled(bool enabled)
{
    mrEnabled = enabled;

    if(mrEnabled) {
        SetMRConfiguration();
    }

    emit updated(this);
}

void DemodSettings::SetMRConfiguration()
{
    // Max decimation
    decimationFactor = 7;
    // Clamp down bandwidth
    double maxBandwidth = max_bw_table[decimationFactor];
    if(bandwidth > maxBandwidth) {
        bandwidth = maxBandwidth;
    }
    // Max possible sweep time
    sweepTime = bin_size_table[decimationFactor] * MAX_IQ_SWEEP_LEN;
}

void DemodSettings::ClampSweepTime()
{
    // Clamp sweep time to a min/max sweepCount
    double binSize = bin_size_table[decimationFactor];
    int sweepLen = sweepTime.Val() / binSize;

    if(sweepLen < MIN_IQ_SWEEP_LEN) {
        sweepTime = binSize * MIN_IQ_SWEEP_LEN;
    } else if(sweepLen > MAX_IQ_SWEEP_LEN) {
        sweepTime = binSize * MAX_IQ_SWEEP_LEN;
    }
}

void DemodSettings::UpdateAutoBandwidths()
{
    if(autoBandwidth) {
        bandwidth = max_bw_table[decimationFactor];
    }
}
