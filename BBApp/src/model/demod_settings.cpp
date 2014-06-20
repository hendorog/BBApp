#include "demod_settings.h"

struct iq_auto_bandwidth_entry {
    Frequency bandwidth;
    int decimationRate;
};

static iq_auto_bandwidth_entry iq_auto_bandwidth_lut[] = {
    { 100.0e3, 7 },
    { 200.0e3, 6 },
    { 500.0e3, 5 },
    { 1.0e6,   4 },
    { 2.0e6,   3 },
    { 5.0e6,   2 },
    { 10.0e6,  1 },
    { 27.0e6,  0 }
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
    vbw = other.VBW();
    sweepTime = other.SweepTime();

    trigType = other.TrigType();
    trigEdge = other.TrigEdge();
    trigAmplitude = other.TrigAmplitude();

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
    if(vbw != other.VBW()) return false;
    if(sweepTime != other.SweepTime()) return false;

    if(trigType != other.TrigType()) return false;
    if(trigEdge != other.TrigEdge()) return false;
    if(trigAmplitude != other.TrigAmplitude()) return false;

    return true;
}

bool DemodSettings::operator!=(const DemodSettings &other) const
{
    return !(*this == other);
}

void DemodSettings::LoadDefaults()
{

}

bool DemodSettings::Load(QSettings &s)
{
    inputPower = s.value("Demod/InputPower", InputPower().Val()).toDouble();
    centerFreq = s.value("Demod/CenterFreq", CenterFreq().Val()).toDouble();
    gain = s.value("Demod/Gain", Gain()).toInt();
    atten = s.value("Demod/Atten", Atten()).toInt();
    decimationFactor = s.value("Demod/Decimation", DecimationFactor()).toInt();
    bandwidth = s.value("Demod/Bandwidth", Bandwidth().Val()).toDouble();
    vbw = s.value("Demod/VBW", VBW().Val()).toDouble();
    sweepTime = s.value("Demod/SweepTime", SweepTime().Val()).toDouble();

    trigType = (TriggerType)s.value("Demod/TriggerType", TrigType()).toInt();
    trigEdge = (TriggerEdge)s.value("Demod/TriggerEdge", TrigEdge()).toInt();
    trigAmplitude = s.value("Demod/TriggerAmplitude",
                            TrigAmplitude().Val()).toDouble();

    return true;
}

bool DemodSettings::Save(QSettings &s) const
{

    return true;
}
