#include "device_sa.h"
#include "mainwindow.h"
#include "lib/sa_api.h"

#include <QElapsedTimer>

DeviceSA::DeviceSA(const Preferences *preferences) :
    Device(preferences)
{
    id = -1;
    open = false;
    serial_number = 0;
}

DeviceSA::~DeviceSA()
{
    CloseDevice();
}

bool DeviceSA::OpenDevice()
{
    if(open) {
        return true;
    }

    saStatus status = saOpenDevice(&id);
    if(status != saNoError) {
        return false;
    }

    saGetSerialNumber(id, &serial_number);
    serial_string.sprintf("%d", serial_number);
    // Get Firmware version
    char fs[16];
    saGetFirmwareString(id, fs);
    firmware_string = fs;

    saGetDeviceType(id, &saDeviceType);

    if(saDeviceType == SA_DEVICE_SA44 || saDeviceType == SA_DEVICE_SA44B) {
        device_type = DeviceTypeSA44;
    } else if(saDeviceType == SA_DEVICE_SA124A || saDeviceType == SA_DEVICE_SA124B) {
        device_type = DeviceTypeSA124;
    }

    saQueryTemperature(id, &current_temp);
    QString diagnostics;
    diagnostics.sprintf("%.2f C", CurrentTemp());
    MainWindow::GetStatusBar()->SetDiagnostics(diagnostics);

    open = true;
    return true;
}

bool DeviceSA::AttachTg()
{
    saStatus status = saAttachTg(id);
    if(status != saNoError) {
        return false;
    }

    return true;
}

bool DeviceSA::IsTgAttached()
{

    return false;
}

void DeviceSA::TgStoreThrough()
{
    saStoreTgThru(id, TG_THRU_0DB);
}

void DeviceSA::TgStoreThroughPad()
{
    saStoreTgThru(id, TG_THRU_20DB);
}

bool DeviceSA::CloseDevice()
{
    saCloseDevice(id);
    return true;
}

bool DeviceSA::Abort()
{
    saAbort(id);
    return true;
}

bool DeviceSA::Preset()
{
    if(!open) {
        //lastStatus = saDeviceNotOpenErr;
        return false;
    }

    return true;
}

bool DeviceSA::Reconfigure(const SweepSettings *s, Trace *t)
{   
    int atten = (s->Atten() == 0) ? SA_AUTO_ATTEN : s->Atten() - 1;
    int gain = (s->Gain() == 0) ? SA_AUTO_GAIN : s->Gain() - 1;
    int preamp = (s->Preamp() == 0) ? SA_PREAMP_AUTO : s->Preamp() - 1;

    saConfigCenterSpan(id, s->Center(), s->Span());
    saConfigAcquisition(id, SA_MIN_MAX, SA_LOG_SCALE, 0.1);

    if(s->Atten() == 0 || s->Gain() == 0 || s->Preamp() == 0) {
        saConfigLevel(id, s->RefLevel());
    } else {
        saConfigGainAtten(id, s->Atten(), s->Gain(), s->Preamp());
    }

    saConfigSweepCoupling(id, s->RBW(), s->VBW(), s->Rejection());
    saConfigProcUnits(id, SA_LOG_UNITS);

    int init_mode = SA_SWEEPING;
    if(s->Mode() == BB_REAL_TIME) init_mode = SA_REAL_TIME;
    if(s->Mode() == MODE_NETWORK_ANALYZER) {
        //saConfigTG(id, (tgStepSize)s->tgStepSizeIx, s->tgPassiveDevice);
        saConfigTgSweep(id, s->tgSweepSize, s->tgHighRangeSweep, s->tgPassiveDevice);
        init_mode = SA_TG_SWEEP;
    }

    saInitiate(id, init_mode, 0);

    int traceLength = 0;
    double startFreq = 0.0, binSize = 0.0;
    saQueryTraceInfo(id, &traceLength, &startFreq, &binSize);

    t->SetSettings(*s);
    t->SetSize(traceLength);
    t->SetFreq(binSize, startFreq);

    return true;
}

bool DeviceSA::GetSweep(const SweepSettings *s, Trace *t)
{
    saStatus status = saNoError;

//    UpdateDiagnostics();

//    if(update_diagnostics_string) {
//        QString diagnostics;
//        //diagnostics.sprintf()
//    }

    int startIx, stopIx;

    status = saFetchPartialData(id, t->Min(), t->Max(), &startIx, &stopIx);

    t->SetUpdateRange(startIx, stopIx);

    adc_overflow = (status == saCompressionWarning);

    return true;
}

// Stream
bool DeviceSA::Reconfigure(const DemodSettings *s, IQDescriptor *iqc)
{
    saAbort(id);

    int atten = (s->Atten() == 0) ? SA_AUTO_ATTEN : s->Atten() - 1;
    int gain = (s->Gain() == 0) ? SA_AUTO_GAIN : s->Gain() - 1;
    saConfigCenterSpan(id, s->CenterFreq(), 250.0e3);

    if(s->Atten() == 0 || s->Gain() == 0 || s->Preamp() == 0) {
        saConfigLevel(id, s->InputPower());
    } else {
        saConfigGainAtten(id, s->Atten(), s->Gain(), s->Preamp());
    }

    saConfigIQ(id, 0x1 << s->DecimationFactor(), s->Bandwidth());
    saInitiate(id, SA_IQ, 0);

    saQueryStreamInfo(id, &iqc->returnLen, &iqc->bandwidth, &iqc->sampleRate);
    iqc->timeDelta = 1.0 / iqc->sampleRate;
    iqc->decimation = 1;

    return true;
}

bool DeviceSA::GetIQ(IQCapture *iqc)
{
    saStatus status = saGetIQ(id, (float*)(&iqc->capture[0]));

    adc_overflow = (status == saCompressionWarning);

    return true;
}

bool DeviceSA::GetIQFlush(IQCapture *iqc, bool sync)
{
    //GetIQ(iqc);
    int rs;
    if(!sync) {
        return GetIQ(iqc);
    } else {
        QElapsedTimer timer;
        do {
            timer.start();
            if(!GetIQ(iqc)) {
                return false;
            }
            rs = timer.restart();
            //printf("%d\n", rs);
        } while(rs < 2);
        //} while(timer.restart() < 2);
    }
    return true;
}

bool DeviceSA::ConfigureForTRFL(double center,
                                int atten,
                                int gain,
                                IQDescriptor &desc)
{
    saAbort(id);

//    int atten = (atten == 0) ? SA_AUTO_ATTEN : atten - 1;
//    int gain = (gain == 0) ? SA_AUTO_GAIN : gain - 1;
//    saConfigCenterSpan(id, center, 250.0e3);
//    saConfigLevel(id, s->InputPower(), atten, gain, false);
//    saConfigIQ(id, 1, 250.0e3);
//    saInitiate(id, SA_IQ, 0);

//    saQueryStreamInfo(id, &desc->returnLen, &desc->bandwidth, &desc->sampleRate);
//    desc->timeDelta = 1.0 / desc->sampleRate;
//    desc->decimation = 1;

    return true;
}

bool DeviceSA::ConfigureAudio(const AudioSettings &as)
{
    /*lastStatus = */saConfigAudio(
                id,
                as.AudioMode(),
                as.CenterFreq(),
                as.IFBandwidth(),
                as.LowPassFreq(),
                as.HighPassFreq(),
                as.FMDeemphasis());

    /*lastStatus = */saInitiate(id, SA_AUDIO, 0);

    return true;
}

bool DeviceSA::GetAudio(float *audio)
{
    /*lastStatus = */ saGetAudio(id, audio);

    return true;
}

QString DeviceSA::GetDeviceString() const
{
    if(saDeviceType == SA_DEVICE_SA44) return "SA44";
    if(saDeviceType == SA_DEVICE_SA44B) return "SA44B";
    if(saDeviceType == SA_DEVICE_SA124A) return "SA124A";
    if(saDeviceType == SA_DEVICE_SA124B) return "SA124B";

    return "No Device Open";
}

void DeviceSA::UpdateDiagnostics()
{

}

bool DeviceSA::IsPowered() const
{
    return true;
}

bool DeviceSA::NeedsTempCal() const
{
    return false;
}

bool DeviceSA::SetTg(Frequency freq, double amp)
{
    saStatus stat = saSetTg(id, freq, amp);
    if(stat != saNoError) {
        return false;
    }

    return true;
}
