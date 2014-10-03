#include "device_sa.h"
#include "mainwindow.h"

#include "lib/sa_api.h"

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
    saConfigLevel(id, 0.0, SA_AUTO_ATTEN, SA_AUTO_GAIN, false);
    saConfigSweepCoupling(id, s->RBW(), s->VBW(), s->Rejection());
    //saConfigProcUnits(id, SA_LOG_UNITS);

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
    double *min = new double[t->Length()];
    double *max = new double[t->Length()];
    status = saFetchPartialData_64f(id, min, max, &startIx, &stopIx);

    adc_overflow = false;

    for(int i = 0; i < t->Length(); i++) {
        t->Min()[i] = min[i];
        t->Max()[i] = max[i];
    }

    delete [] min;
    delete [] max;

    return true;
}

// Stream
bool DeviceSA::Reconfigure(const DemodSettings *s, IQDescriptor *iqc)
{
    return true;
}

bool DeviceSA::GetIQ(IQCapture *iqc)
{
    return true;
}

bool DeviceSA::GetIQFlush(IQCapture *iqc, bool sync)
{
    return true;
}

bool DeviceSA::ConfigureForTRFL(double center, int atten, int gain, IQDescriptor &desc)
{
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
