#ifndef __SA_API_H__
#define __SA_API_H__

#ifdef SA_EXPORTS
#define SA_API __declspec(dllexport)
#else
#define SA_API __declspec(dllimport)
#endif

#define SA_MAX_DEVICES 8

enum saDeviceType {
    saDeviceTypeNone = 0,
    saDeviceTypeSA44 = 1,
    saDeviceTypeSA44B = 2,
    saDeviceTypeSA124A = 3,
    saDeviceTypeSA124B = 4
};

// Limits
#define SA44_MIN_FREQ (1.0)
#define SA124_MIN_FREQ (100.0e3)
#define SA44_MAX_FREQ (4.4e9)
#define SA124_MAX_FREQ (13.0e9)
#define SA_MIN_SPAN (1.0)
#define SA_MAX_REF (20) // dBm
#define SA_MAX_ATTEN (3)
#define SA_MAX_GAIN (2)
#define SA_MIN_RBW (0.1)
#define SA_MAX_RBW (6.0e6)
#define SA_MIN_RT_RBW (100.0)
#define SA_MAX_RT_RBW (10000.0)
#define SA_MIN_IQ_BANDWIDTH (100.0)
#define SA_MAX_IQ_DECIMATION (128)

#define SA_IQ_SAMPLE_RATE (486111.111)

// Modes
#define SA_IDLE      (-1)
#define SA_SWEEPING  (0x0)
#define SA_REAL_TIME (0x1)
#define SA_IQ        (0x2)
#define SA_AUDIO     (0x3)
#define SA_TG_SWEEP  (0x4)

// Detectors
#define SA_MIN_MAX (0x0)
#define SA_AVERAGE (0x1)

// Scales
#define SA_LOG_SCALE      (0x0)
#define SA_LIN_SCALE      (0x1)
#define SA_LOG_FULL_SCALE (0x2) // N/A
#define SA_LIN_FULL_SCALE (0x3) // N/A

// Levels
#define SA_AUTO_ATTEN (-1)
#define SA_AUTO_GAIN  (-1)

// Video Processing Units
#define SA_LOG_UNITS   (0x0)
#define SA_VOLT_UNITS  (0x1)
#define SA_POWER_UNITS (0x2)
#define SA_BYPASS      (0x3)

#define SA_AUDIO_AM  (0x0)
#define SA_AUDIO_FM  (0x1)
#define SA_AUDIO_USB (0x2)
#define SA_AUDIO_LSB (0x3)
#define SA_AUDIO_CW  (0x4)

// TG Notify Flags
#define TG_THRU_0DB  (0x1)
#define TG_THRU_20DB  (0x2)

struct saSelfTestResults {
    // Pass/Fail
    bool highBandMixer, lowBandMixer;
    bool attenuator, secondIF, preamplifier;
    // Readings
    double highBandMixerValue, lowBandMixerValue;
    double attenuatorValue, secondIFValue, preamplifierValue;
};

// Return values
enum saStatus {
    saUnknownErr = -666,

    // Setting specific error codes
    saFrequencyRangeErr = -99,
    saInvalidDetectorErr = -95,
    saInvalidScaleErr = -94,
    saBandwidthErr = -91,
    saExternalReferenceNotFound = -89,

    // Device-specific errors
    saOvenColdErr = -20,

    // Data errors
    saInternetErr = -12,
    saUSBCommErr = -11,

    // General configuration errors
    saTrackingGeneratorNotFound = -10,
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
    saCompressionWarning = 2,
    saParameterClamped = 3,
    saBandwidthClamped = 4,
};

#ifdef __cplusplus
extern "C" {
#endif

SA_API saStatus saGetSerialNumberList(int serialNumbers[8], int *deviceCount);
SA_API saStatus saOpenDeviceBySerialNumber(int *device, int serialNumber);
SA_API saStatus saOpenDevice(int *device);
SA_API saStatus saCloseDevice(int device);
SA_API saStatus saPreset(int device);

SA_API saStatus saGetSerialNumber(int device, int *serial);
SA_API saStatus saGetFirmwareString(int device, char firmwareString[16]);
SA_API saStatus saGetDeviceType(int device, saDeviceType *device_type);
SA_API saStatus saConfigAcquisition(int device, int detector, int scale);
SA_API saStatus saConfigCenterSpan(int device, double center, double span);
SA_API saStatus saConfigLevel(int device, double ref);
SA_API saStatus saConfigGainAtten(int device, int atten, int gain, bool preAmp);
SA_API saStatus saConfigSweepCoupling(int device, double rbw, double vbw, bool reject);
SA_API saStatus saConfigProcUnits(int device, int units);
SA_API saStatus saConfigIQ(int device, int decimation, double bandwidth);
SA_API saStatus saConfigAudio(int device, int audioType, double centerFreq,
                              double bandwidth, double audioLowPassFreq,
                              double audioHighPassFreq, double fmDeemphasis);
SA_API saStatus saEnableExternalReference(int device);

SA_API saStatus saInitiate(int device, int mode, int flag);
SA_API saStatus saAbort(int device);

SA_API saStatus saQuerySweepInfo(int device, int *sweepLength, double *startFreq, double *binSize);
SA_API saStatus saQueryStreamInfo(int device, int *returnLen, double *bandwidth, double *samplesPerSecond);
SA_API saStatus saGetSweep_32f(int device, float *min, float *max);
SA_API saStatus saGetSweep_64f(int device, double *min, double *max);
SA_API saStatus saGetPartialSweep_32f(int device, float *min, float *max, int *start, int *stop);
SA_API saStatus saGetPartialSweep_64f(int device, double *min, double *max, int *start, int *stop);
SA_API saStatus saGetIQ_32f(int device, float *iq);
SA_API saStatus saGetIQ_64f(int device, double *iq);
SA_API saStatus saGetAudio(int device, float *audio);
SA_API saStatus saQueryTemperature(int device, float *temp);
SA_API saStatus saQueryDiagnostics(int device, float *voltage);

SA_API saStatus saAttachTg(int device);
SA_API saStatus saIsTgAttached(int device, bool *attached);
SA_API saStatus saConfigTgSweep(int device, int sweepSize, bool highDynamicRange, bool passiveDevice);
SA_API saStatus saStoreTgThru(int device, int flag);
SA_API saStatus saSetTg(int device, double frequency, double amplitude);

SA_API saStatus saConfigIFOutput(int device, double inputFreq, double outputFreq,
                                 int inputAtten, int outputGain);
SA_API saStatus saSelfTest(int device, saSelfTestResults *results);

SA_API const char* saGetAPIVersion();
SA_API const char* saGetErrorString(saStatus code);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SA_API_H
