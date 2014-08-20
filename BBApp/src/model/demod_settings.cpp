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

    maEnabled = other.MAEnabled();
    maLowPass = other.MALowPass();

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

    if(maEnabled != other.MAEnabled()) return false;
    if(maLowPass != other.MALowPass()) return false;

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

    maEnabled = false;
    maLowPass = 10.0e3;
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

    maEnabled = s.value("Demod/MAEnabled", false).toBool();
    maLowPass = s.value("Demod/MALowPass", 10.0e3).toDouble();

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

    s.setValue("Demod/MAEnabled", MAEnabled());
    s.setValue("Demod/MALowPass", MALowPass().Val());

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

void DemodSettings::setMAEnabled(bool enabled)
{
    maEnabled = enabled;

    if(maEnabled) {
        SetMRConfiguration();
    }

    emit updated(this);
}

void DemodSettings::setMALowPass(Frequency f)
{
    f.Clamp(100.0, 20.0e3);

    maLowPass = f;

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

void IQSweep::Demod()
{
    Q_ASSERT(iq.size() > 0);
    if(iq.size() <= 0) {
        return;
    }

    amWaveform.clear();
    fmWaveform.clear();
    pmWaveform.clear();

    //for(complex_f cplx : iq) {
    for(auto i = 0; i < sweepLen; i++) {
        amWaveform.push_back(iq[i].re * iq[i].re + iq[i].im * iq[i].im);
        pmWaveform.push_back(atan2(iq[i].im, iq[i].re));
    }

    double phaseToFreq = settings.SampleRate() / BB_TWO_PI;
    double lastPhase = pmWaveform[0];

    for(float phase : pmWaveform) {
        double delPhase = phase - lastPhase;
        lastPhase = phase;

        if(delPhase > BB_PI)
            delPhase -= BB_TWO_PI;
        else if(delPhase < (-BB_PI))
            delPhase += BB_TWO_PI;

        fmWaveform.push_back(delPhase * phaseToFreq);
    }
}

void IQSweep::CalculateReceiverStats()
{
    const std::vector<float> &am = amWaveform;
    const std::vector<float> &fm = fmWaveform;

    // Temp buffers, storing offset removed modulations
    std::vector<float> temp, temp2;
    std::vector<double> audioRate; // Downsampled audio

    temp.resize(fm.size());
    temp2.resize(fm.size());

    // Low pass filter
    FirFilter fir(settings.MALowPass() / settings.SampleRate(), 1024); // Filters AM and FM

    // Calculate RF Center based on average of FM frequencies
    stats.rfCenter = 0.0;
    for(int i = 2048; i < fm.size(); i++) {
        stats.rfCenter += fm[i];
    }
    double fmAvg = stats.rfCenter / (temp.size() - 2048);
//    qDebug() << "FM Avg " << fmAvg;
    stats.rfCenter = settings.CenterFreq() + fmAvg;

    // Remove DC offset
    for(int i = 0; i < fm.size(); i++) {
        temp[i] = fm[i] - fmAvg;
    }

    fir.Filter(&temp[0], &temp2[0], fm.size());
    fir.Reset();

    stats.fmAudioFreq = getAudioFreq(temp2, settings.SampleRate(), 1024);

    // FM RMS
    stats.fmPeakPlus = std::numeric_limits<double>::lowest();
    stats.fmPeakMinus = std::numeric_limits<double>::max();
    stats.fmRMS = 0.0;
    for(int i = 1024; i < temp2.size(); i++) {
        if(temp2[i] > stats.fmPeakPlus) stats.fmPeakPlus = temp2[i];
        if(temp2[i] < stats.fmPeakMinus) stats.fmPeakMinus = temp2[i];
        stats.fmRMS += temp2[i] * temp2[i];
    }
    stats.fmRMS = sqrt(stats.fmRMS / (temp2.size() - 1024));

    downsample(temp2, audioRate, 8);
    stats.fmSINAD = 10.0 * log10(CalculateSINAD(audioRate, 39062.5, stats.fmAudioFreq));
    stats.fmTHD = CalculateTHD(audioRate, 39062.5, stats.fmAudioFreq);

    // AM
    double invAvg = 0.0;
    for(int i = 0; i < am.size(); i++) {
        float v = sqrt(am[i] * 50000.0);
        temp[i] = v;
        invAvg += v;
    }
    invAvg = am.size() / invAvg;

    // Normalize around zero
    for(int i = 0; i < temp.size(); i++) {
        temp2[i] = (temp[i] * invAvg) - 1.0;
    }

    fir.Filter(&temp2[0], &temp[0], temp2.size());

    stats.amAudioFreq = getAudioFreq(temp, settings.SampleRate(), 1024);
    stats.amPeakPlus = std::numeric_limits<double>::lowest();
    stats.amPeakMinus = std::numeric_limits<double>::max();
    stats.amRMS = 0.0;
    for(int i = 1024; i < temp.size(); i++) {
        if(temp[i] > stats.amPeakPlus) stats.amPeakPlus = temp[i];
        if(temp[i] < stats.amPeakMinus) stats.amPeakMinus = temp[i];
        stats.amRMS += (temp[i] * temp[i]);
    }
    stats.amRMS = sqrt(stats.amRMS / (temp.size() - 1024));

    audioRate.clear();
    downsample(temp, audioRate, 8);
    stats.amSINAD = 10.0 * log10(CalculateSINAD(audioRate, 39062.5, stats.amAudioFreq));
    stats.amTHD = CalculateTHD(audioRate, 39062.5, stats.amAudioFreq);
}

// Returns dB ratio of the average power of the waveform over the average
//   power of the waveform with a band reject filter on center freq
double CalculateSINAD(const std::vector<double> &waveform, double sampleRate, double centerFreq)
{
    auto len = waveform.size();
    std::vector<double> rejected;
    std::vector<double> filtered;
    filtered.resize(len);
    rejected.resize(len);

    iirHighPass(&waveform[0], &filtered[0], len);
    iirBandReject(&filtered[0], &rejected[0], centerFreq / sampleRate, 0.005, len);
    //iirBandPass(&waveform[0], &rejected[0], centerFreq / 312.5e3, 0.1, waveform.size());

//    qDebug() << averagePower(&filtered[len/2], len/2);
//    qDebug() << "Rejected " << averagePower(&rejected[len/2], len/2);
    return averagePower(&filtered[len/2], len/2) /
            averagePower(&rejected[len/2], len/2);
}


double CalculateTHD(const std::vector<double> &waveform, double sampleRate, double centerFreq)
{
    const int TOTAL_HARMONICS = 9;
    const double PASS_BAND = 50.0;

    auto len = waveform.size();
    std::vector<double> filtered, filtered2;
    filtered.resize(len);
    filtered2.resize(len);
    double Vharmonics[TOTAL_HARMONICS] = {0};

    for(int h = 0; h < TOTAL_HARMONICS; h++) {
        iirBandPass(&waveform[0], &filtered[0], ((h+1) * centerFreq) / sampleRate,
                PASS_BAND / sampleRate, len);
        iirBandPass(&filtered[0], &filtered2[0], ((h+1) * centerFreq) / sampleRate,
                PASS_BAND / sampleRate, len);
        iirBandPass(&filtered2[0], &filtered[0], ((h+1) * centerFreq) / sampleRate,
                PASS_BAND / sampleRate, len);

        for(int i = 2048; i < len; i++) {
            Vharmonics[h] += (filtered[i] * filtered[i]);
        }
        Vharmonics[h] = sqrt(Vharmonics[h] / (len - 2048));
    }

    double Vrms = 0.0;
    for(int h = 1; h < TOTAL_HARMONICS; h++) {
        Vrms += Vharmonics[h] * Vharmonics[h];
    }
    Vrms = sqrt(Vrms);

//    for(int i = 0; i < TOTAL_HARMONICS; i++) {
//        qDebug() << "V" << QVariant(i+1).toString() << " " << Vharmonics[i];
//    }

    return Vrms / Vharmonics[0];
}
