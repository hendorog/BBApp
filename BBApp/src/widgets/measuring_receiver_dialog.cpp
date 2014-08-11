#include "measuring_receiver_dialog.h"

#include <QRadioButton>
#include <QButtonGroup>

static const int WIDGET_HEIGHT = 25;

MeasuringReceiver::MeasuringReceiver(Device *devicePtr,
                                     double initialCenter,
                                     QWidget *parent) :
    QDialog(parent),
    device(devicePtr),
    running(true)
{
    setWindowTitle("Measuring Receiever");
    setObjectName("SH_Page");
    setFixedWidth(400);

    QPoint pos(0, 0); // For positioning widgets

    Label *title = new Label("Synchronous Level Detector", this);
    title->move(QPoint(5, pos.y()));
    title->resize(width(), WIDGET_HEIGHT);
    pos += QPoint(0, WIDGET_HEIGHT);
    freqEntry = new FrequencyEntry("Center Freq", initialCenter, this);
    freqEntry->move(pos);
    freqEntry->resize(width(), WIDGET_HEIGHT);
    pos += QPoint(0, WIDGET_HEIGHT);

    Label *ampRangeLabel = new Label("Amplitude Range", this);
    ampRangeLabel->move(QPoint(5, pos.y()));
    ampRangeLabel->resize(width(), WIDGET_HEIGHT);
    pos += QPoint(0, WIDGET_HEIGHT);

    ampGroup = new QButtonGroup(this);
    QRadioButton *highAmp = new QRadioButton("High Power Range", this),
            *midAmp = new QRadioButton("Mid Power Range", this),
            *lowAmp = new QRadioButton("Low Power Range", this);

    highAmp->move(pos);
    highAmp->resize(width()/2, WIDGET_HEIGHT);
    highAmp->setObjectName("SHPrefRadioButton");
    highAmp->setChecked(true);
    ampGroup->addButton(highAmp, 0);

    highLabel = new Label("HighLabel", this);
    highLabel->move(QPoint(width()/2, pos.y()));
    highLabel->resize(width()/2, WIDGET_HEIGHT);
    pos += QPoint(0, WIDGET_HEIGHT);

    midAmp->move(pos);
    midAmp->resize(width()/2, WIDGET_HEIGHT);
    midAmp->setObjectName("SHPrefRadioButton");
    ampGroup->addButton(midAmp, 1);

    midLabel = new Label("MidLabel", this);
    midLabel->move(QPoint(width()/2, pos.y()));
    midLabel->resize(width()/2, WIDGET_HEIGHT);
    pos += QPoint(0, WIDGET_HEIGHT);

    lowAmp->move(pos);
    lowAmp->resize(width()/2, WIDGET_HEIGHT);
    lowAmp->setObjectName("SHPrefRadioButton");
    ampGroup->addButton(lowAmp, 2);

    lowLabel = new Label("LowLabel", this);
    lowLabel->move(QPoint(width()/2, pos.y()));
    lowLabel->resize(width()/2, WIDGET_HEIGHT);
    pos += QPoint(0, WIDGET_HEIGHT);

    Label *centerLabel = new Label("RF Sync Center", this);
    centerLabel->move(QPoint(5, pos.y()));
    centerLabel->resize(width()/2, WIDGET_HEIGHT);
    centerReadout = new Label("915.11002 MHz", this);
    centerReadout->move(QPoint(width()/2 + 5, pos.y()));
    centerReadout->resize(width()/2, WIDGET_HEIGHT);
    pos += QPoint(0, WIDGET_HEIGHT);

    Label *powerLabel = new Label("RF Power", this);
    powerLabel->move(QPoint(5, pos.y()));
    powerLabel->resize(width()/2, WIDGET_HEIGHT);
    powerReadout = new Label("-32.22 dBm", this);
    powerReadout->move(QPoint(width()/2 + 5, pos.y()));
    powerReadout->resize(width()/2, WIDGET_HEIGHT);
    pos += QPoint(0, WIDGET_HEIGHT);

    Label *relativeLabel = new Label("Relative Power", this);
    relativeLabel->move(QPoint(5, pos.y()));
    relativeLabel->resize(width()/2, WIDGET_HEIGHT);
    relativeReadout = new Label("-5.002 dB", this);
    relativeReadout->move(QPoint(width()/2 + 5, pos.y()));
    relativeReadout->resize(width()/2, WIDGET_HEIGHT);
    pos += QPoint(0, WIDGET_HEIGHT);

    Label *averageLabel = new Label("Averaged Power", this);
    averageLabel->move(QPoint(5, pos.y()));
    averageLabel->resize(width()/2, WIDGET_HEIGHT);
    averageReadout = new Label("-4.998 dB", this);
    averageReadout->move(QPoint(width()/2 + 5, pos.y()));
    averageReadout->resize(width()/2, WIDGET_HEIGHT);
    pos += QPoint(0, WIDGET_HEIGHT*1.5);

    PushButton *sync = new PushButton("Sync", this);
    sync->move(QPoint(5, pos.y()));
    sync->resize(width()/2 - 10, WIDGET_HEIGHT);
    //pos += QPoint(0, WIDGET_HEIGHT);
    connect(sync, SIGNAL(clicked()), this, SLOT(syncPressed()));

    PushButton *done = new PushButton("Done", this);
    done->move(QPoint(width()/2 + 5, pos.y()));
    done->resize(width()/2 - 10, WIDGET_HEIGHT);
    pos += QPoint(0, WIDGET_HEIGHT*2);
    connect(done, SIGNAL(clicked()), this, SLOT(accept()));

    setFixedHeight(pos.y());
    setSizeGripEnabled(false);

    threadHandle = std::thread(&MeasuringReceiver::ProcessThread, this);
}

MeasuringReceiver::~MeasuringReceiver()
{
    running = false;
    if(threadHandle.joinable()) {
        threadHandle.join();
    }

}

//void MeasuringReceiver::accept()
//{

//    QDialog::accept();
//}

void MeasuringReceiver::syncPressed()
{
    ampGroup->button(0)->setChecked(true);
}

static const int PROC_LEN = 1024;

void MeasuringReceiver::ProcessThread()
{
    bool firstPass = true;
    IQCapture capture;
    double center, power, relative = 100.0;
    double centerOut;
    std::vector<double> phase, fm;
    std::list<double> average;
    phase.resize(PROC_LEN);
    fm.resize(PROC_LEN);

    while(running) {
        if(firstPass) {
            center = freqEntry->GetFrequency().Val();
            power = 0.0;
            device->ConfigureForTRFL(center, BB_AUTO_ATTEN, BB_AUTO_GAIN, capture.desc);
            capture.capture.resize(capture.desc.returnLen);
        }

        device->GetIQFlush(&capture, true);
        device->GetIQFlush(&capture, true);
        device->GetIQFlush(&capture, true);

        if(firstPass) {
            for(int i = 0; i < PROC_LEN; i++) {
                phase[i] = atan2(capture.capture[i].im, capture.capture[i].re);
            }

            double phaseToFreq = (double)capture.desc.sampleRate / BB_TWO_PI;
            double lastPhase = phase[0];

            for(int i = 1; i < PROC_LEN; i++) {
                double delPhase = phase[i] - lastPhase;
                lastPhase = phase[i];

                if(delPhase > BB_PI)
                    delPhase -= BB_TWO_PI;
                else if(delPhase < (-BB_PI))
                    delPhase += BB_TWO_PI;

                fm[i] = delPhase * phaseToFreq;
            }

            centerOut = 0.0;
            for(int i = 1; i < PROC_LEN; i++) {
                centerOut += fm[i];
            }
            centerOut /= PROC_LEN;
            centerOut /= 312500.0;

            firstPass = false;
        }

        getPeakCorrelation(&capture.capture[0], PROC_LEN*4, centerOut, centerOut, power);

        QString readout;

        readout = Frequency(center + centerOut * 312.5e3).GetFreqString();
        centerReadout->setText(readout);
        readout.sprintf("%.2f dBm", 10.0 * log10(power));
        powerReadout->setText(readout);

        if(relative > 50.0) relative = 10.0 * log10(power);
        double diff = 10.0 * log10(power) - relative;
        average.push_front(diff);
        while(average.size() > 20) {
            average.pop_back();
        }
        double avgPower = 0.0;
        for(double d : average) avgPower += d;
        avgPower /= average.size();

        readout.sprintf("%.2f dB", relative);
        relativeReadout->setText(readout);
        readout.sprintf("%.2f dB", avgPower);
        averageReadout->setText(readout);

    }

    device->Abort();
}
