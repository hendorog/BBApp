#include "device_sa.h"
#include "mainwindow.h"

#include <QElapsedTimer>

#define STATUS_CHECK(status) \
    if(status < saNoError) { \
    return false; \
    }

DeviceSA::DeviceSA(const Preferences *preferences) :
    Device(preferences)
{
    id = -1;
    open = false;
    serial_number = 0;
    deviceType = saDeviceTypeNone;
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

    lastStatus = saOpenDevice(&id);
    if(lastStatus != saNoError) {
        return false;
    }

    saGetSerialNumber(id, &serial_number);
    serial_string.sprintf("%d", serial_number);
    // Get Firmware version
    char fs[16];
    saGetFirmwareString(id, fs);
    firmware_string = fs;

    saGetDeviceType(id, &deviceType);

    if(deviceType == saDeviceTypeSA44) {
        device_type = DeviceTypeSA44A;
    } else if(deviceType == saDeviceTypeSA44B) {
        device_type = DeviceTypeSA44B;
    } else if(deviceType == saDeviceTypeSA124A || deviceType == saDeviceTypeSA124B) {
        device_type = DeviceTypeSA124;
    }

    saQueryTemperature(id, &current_temp);
    QString diagnostics;
    diagnostics.sprintf("%.2f C", CurrentTemp());
    MainWindow::GetStatusBar()->SetDiagnostics(diagnostics);

    open = true;
    return true;
}

bool DeviceSA::OpenDeviceWithSerial(int serialToOpen)
{
    if(open) {
        return true;
    }

    lastStatus = saOpenDeviceBySerialNumber(&id, serialToOpen);
    if(lastStatus != saNoError) {
        return false;
    }

    saGetSerialNumber(id, &serial_number);
    serial_string.sprintf("%d", serial_number);
    // Get Firmware version
    char fs[16];
    saGetFirmwareString(id, fs);
    firmware_string = fs;
    firmware_string += "  ";

    saGetDeviceType(id, &deviceType);

    if(deviceType == saDeviceTypeSA44) {
        device_type = DeviceTypeSA44A;
    } else if(deviceType == saDeviceTypeSA44B) {
        device_type = DeviceTypeSA44B;
    } else if(deviceType == saDeviceTypeSA124A || deviceType == saDeviceTypeSA124B) {
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

bool DeviceSA::CloseDevice()
{
    saCloseDevice(id);

    id = -1;
    open = false;
    device_type = DeviceTypeSA44B;
    serial_number = 0;

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

    saAbort(id);
    saPreset(id);

    return true;
}

bool DeviceSA::Reconfigure(const SweepSettings *s, Trace *t)
{   
    Abort();
    tgCalState = tgCalStateUncalibrated;

    // Update temperature between configurations
    saQueryTemperature(id, &current_temp);
    QString diagnostics;
    if(deviceType != saDeviceTypeSA44) {
        diagnostics.sprintf("%.2f C", CurrentTemp());
    }
    MainWindow::GetStatusBar()->SetDiagnostics(diagnostics);

    int atten = (s->Atten() == 0) ? SA_AUTO_ATTEN : s->Atten() - 1;
    int gain = (s->Gain() == 0) ? SA_AUTO_GAIN : s->Gain() - 1;
    int preamp = (s->Preamp() == 0) ? SA_PREAMP_AUTO : s->Preamp() - 1;
    int scale = (s->RefLevel().IsLogScale() ? SA_LOG_SCALE : SA_LIN_SCALE);

    saConfigCenterSpan(id, s->Center(), s->Span());
    saConfigAcquisition(id, s->Detector(), scale);

    saConfigLevel(id, s->RefLevel());
    saConfigGainAtten(id, atten, gain, s->Preamp());

    if(atten == SA_AUTO_ATTEN || gain == SA_AUTO_GAIN) {
        saConfigLevel(id, s->RefLevel().ConvertToUnits(AmpUnits::DBM));
    } else {
        saConfigGainAtten(id, s->Atten(), s->Gain(), s->Preamp());
    }

    saConfigSweepCoupling(id, s->RBW(), s->VBW(), s->Rejection());
    saConfigProcUnits(id, s->ProcessingUnits());

    int init_mode = SA_SWEEPING;
    if(s->Mode() == BB_REAL_TIME) init_mode = SA_REAL_TIME;
    if(s->Mode() == MODE_NETWORK_ANALYZER) {
        saConfigTgSweep(id, s->tgSweepSize, s->tgHighRangeSweep, s->tgPassiveDevice);
        init_mode = SA_TG_SWEEP;
    }

    saInitiate(id, init_mode, 0);

    int traceLength = 0;
    double startFreq = 0.0, binSize = 0.0;
    saQuerySweepInfo(id, &traceLength, &startFreq, &binSize);

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

    status = saGetPartialSweep_32f(id, t->Min(), t->Max(), &startIx, &stopIx);

    t->SetUpdateRange(startIx, stopIx);

    if(s->Mode() == MODE_NETWORK_ANALYZER && tgCalState == tgCalStatePending) {
        if(stopIx >= t->Length()) {
            tgCalState = tgCalStateCalibrated;
        }
    }

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
    saStatus status = saGetIQ_32f(id, (float*)(&iqc->capture[0]));

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

const char* DeviceSA::GetLastStatusString() const
{
    return saGetErrorString(lastStatus);
}

QString DeviceSA::GetDeviceString() const
{
    if(deviceType == saDeviceTypeSA44) return "SA44";
    if(deviceType == saDeviceTypeSA44B) return "SA44B";
    if(deviceType == saDeviceTypeSA124A) return "SA124A";
    if(deviceType == saDeviceTypeSA124B) return "SA124B";

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

void DeviceSA::TgStoreThrough()
{
    saStoreTgThru(id, TG_THRU_0DB);
    tgCalState = tgCalStatePending;
}

void DeviceSA::TgStoreThroughPad()
{
    saStoreTgThru(id, TG_THRU_20DB);
}
