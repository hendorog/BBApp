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
    saGetDeviceType(id, &device_type);

    saQueryTemperature(id, &current_temp);
    QString diagnostics;
    diagnostics.sprintf("%.2f C", CurrentTemp());
    MainWindow::GetStatusBar()->SetDiagnostics(diagnostics);

    open = true;
    return true;
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
    return true;
}

bool DeviceSA::Reconfigure(const SweepSettings *s, Trace *t)
{
    saConfigCenterSpan(id, s->Center(), s->Span());
    saConfigAcquisition(id, SA_MIN_MAX, SA_LOG_SCALE, 0.1);
    saConfigLevel(id, s->RefLevel(), SA_AUTO_ATTEN, SA_AUTO_GAIN, false);
    saConfigSweepCoupling(id, s->RBW(), s->VBW(), s->Rejection());
    saConfigProcUnits(id, SA_LOG_UNITS);

    int init_mode = SA_SWEEPING;
    if(s->Mode() == BB_REAL_TIME) init_mode = SA_REAL_TIME;

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
    adc_overflow = false;

    return true;
}

// Stream
bool DeviceSA::Reconfigure(const DemodSettings *s, IQDescriptor *iqc)
{
    saAbort(id);

    int atten = (s->Atten() == 0) ? SA_AUTO_ATTEN : s->Atten() - 1;
    int gain = (s->Gain() == 0) ? SA_AUTO_GAIN : s->Gain() - 1;
    saConfigCenterSpan(id, s->CenterFreq(), 250.0e3);
    saConfigLevel(id, s->InputPower(), atten, gain, false);
    saConfigIQ(id, 0x1 << s->DecimationFactor(), s->Bandwidth());
    saInitiate(id, SA_IQ, 0);

    saQueryStreamInfo(id, &iqc->returnLen, &iqc->bandwidth, &iqc->sampleRate);
    iqc->timeDelta = 1.0 / iqc->sampleRate;
    iqc->decimation = 1;

    return true;
}

bool DeviceSA::GetIQ(IQCapture *iqc)
{
    float *re = new float[iqc->capture.size()], *im = new float[iqc->capture.size()];
    saFetchData(id, re, im);

    for(int i = 0; i < iqc->capture.size(); i++) {
        iqc->capture[i].re = re[i];
        iqc->capture[i].im = im[i];
    }

    delete [] re;
    delete [] im;
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

QString DeviceSA::GetDeviceString() const
{
    return "SA44B";
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
