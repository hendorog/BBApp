#ifndef __SA_API_H__
#define __SA_API_H__

#ifdef SA_EXPORTS
#define SA_API __declspec(dllexport)
#else
#define SA_API __declspec(dllimport)
#endif

#define SA_MAX_DEVICES 8

#define SA_DEVICE_SA44B 0x0
#define SA_DEVICE_SA124B 0x1

// Limits
#define SA_MIN_FREQ (0.01)
#define SA_MAX_FREQ (4.4e9)
#define SA_MIN_SPAN (100.0)
#define SA_MAX_REF (20) // dBm
#define SA_MAX_ATTEN (3)
#define SA_MAX_GAIN (2)
#define SA_MIN_RBW (0.1)
#define SA_MAX_RBW (5.0e6)
#define SA_MIN_RT_RBW (100.0)
#define SA_MAX_RT_RBW (10000.0)
#define SA_MAX_SWEEP_TIME (30000) // ms
#define SA_MIN_IQ_BANDWIDTH (100.0)
#define SA_MAX_IQ_DECIMATION (128)

// Modes
#define SA_IDLE     -1
#define SA_SWEEPING  0x0
#define SA_REAL_TIME 0x1
#define SA_IQ        0x2

// Detectors
#define SA_MIN_MAX 0x0
#define SA_AVERAGE 0x1

// Scales
#define SA_LOG_SCALE      0x0
#define SA_LOG_FULL_SCALE 0x1
#define SA_LIN_SCALE      0x2
#define SA_LIN_FULL_SCALE 0x3

// Levels
#define SA_AUTO_ATTEN -1
#define SA_AUTO_GAIN  -1

// Video Processing Units
#define SA_POWER_UNITS 0x0
#define SA_LOG_UNITS   0x1
#define SA_VOLT_UNITS  0x2
#define SA_BYPASS      0x3

// Return values
enum saStatus {
    // Impossible
    saUnknownErr = -666,

    // Setting specific error codes
    saReferenceLevelErr = -100,
    saFrequencyRangeErr = -99,
    saAttenuationErr = -98,
    saGainErr = -97,
    saVideoBWErr = -96,
    saInvalidDetectorErr = -95,
    saInvalidScaleErr = -94,
    saInvalidProcUnitsErr = -93,
    saSweepTimeTooLargeErr = -92,
    saBandwidthErr = -91,
    saRealTimeBandwidthErr = -90,

    // Data errors
    saInternetErr = -11,
    saUSBCommErr = -10,

    // General configuration errors
    saDeviceNotIdleErr = -9,
    saDeviceNotFoundErr = -8,
    saInvalidModeErr = -7,
    saNotConfiguredErr = -6,
    saTooManyDevicesErr = -5,
    saInvalidParameterErr = -4,
    saDeviceNotOpenErr = -3,
    saInvalidDeviceErr = -2,
    saNullPtrErr = -1,

    // No Error
    saNoError = 0,

    // Warnings
    saNoCorrections = 1,
    saCompressionWarning = 2
};

#ifdef __cplusplus
extern "C" {
#endif

SA_API saStatus saOpenDevice(int *device);
SA_API saStatus saCloseDevice(int device);
SA_API saStatus saGetSerialNumber(int device, int *serial);
SA_API saStatus saGetFirmwareString(int device, char firmwareString[16]);
SA_API saStatus saGetDeviceType(int device, int *device_type);
SA_API saStatus saConfigAcquisition(int device, int detector, int scale, int sweepTime);
SA_API saStatus saConfigCenterSpan(int device, double center, double span);
SA_API saStatus saConfigLevel(int device, double ref, int atten, int gain, bool preAmp);
SA_API saStatus saConfigSweepCoupling(int device, double rbw, double vbw, bool reject);
SA_API saStatus saConfigProcUnits(int device, int units);
SA_API saStatus saConfigIQ(int device, int decimation, double bandwidth);

SA_API saStatus saInitiate(int device, int mode, int flag);
SA_API saStatus saAbort(int device);

SA_API saStatus saQuerySweepTime(int device, int *time);
SA_API saStatus saQueryTraceInfo(int device, int *traceLength, double *startFreq, double *binSize);
SA_API saStatus saQueryStreamInfo(int device, int *returnLen, double *bandwidth, double *samplesPerSecond);
SA_API saStatus saFetchData(int device, float *a, float *b);
SA_API saStatus saFetchData_64f(int device, double *a, double *b);
SA_API saStatus saFetchPartialData(int device, float *a, float *b, int *start, int *stop);
SA_API saStatus saFetchPartialData_64f(int device, double *a, double *b, int *start, int *stop);
SA_API saStatus saQueryTemperature(int device, float *temp);
SA_API saStatus saQueryDiagnostics(int device, float *voltage, float *current);

SA_API const char* saGetErrorString(saStatus code);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SA_API_H
