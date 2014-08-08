#include "measuring_receiver_dialog.h"

#include <QRadioButton>
#include <QButtonGroup>

static const int WIDGET_HEIGHT = 25;

MeasuringReceiver::MeasuringReceiver(const Device *devicePtr,
                                     double initialCenter,
                                     QWidget *parent) :
    QDialog(parent),
    device(devicePtr)
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

    QButtonGroup *ampGroup = new QButtonGroup(this);
    QRadioButton *highAmp = new QRadioButton("High Power Range", this),
            *midAmp = new QRadioButton("Mid Power Range", this),
            *lowAmp = new QRadioButton("Low Power Range", this);

    highAmp->move(pos);
    highAmp->resize(width(), WIDGET_HEIGHT);
    highAmp->setObjectName("SHPrefRadioButton");
    pos += QPoint(0, WIDGET_HEIGHT);
    midAmp->move(pos);
    midAmp->resize(width(), WIDGET_HEIGHT);
    midAmp->setObjectName("SHPrefRadioButton");
    pos += QPoint(0, WIDGET_HEIGHT);
    lowAmp->move(pos);
    lowAmp->resize(width(), WIDGET_HEIGHT);
    lowAmp->setObjectName("SHPrefRadioButton");
    pos += QPoint(0, WIDGET_HEIGHT);

    Label *centerLabel = new Label("RF Sync Center", this);
    centerLabel->move(QPoint(5, pos.y()));
    centerLabel->resize(width()/2, WIDGET_HEIGHT);
    Label *centerReadout = new Label("915.11002 MHz", this);
    centerReadout->move(QPoint(width()/2 + 5, pos.y()));
    centerReadout->resize(width()/2, WIDGET_HEIGHT);
    pos += QPoint(0, WIDGET_HEIGHT);

    Label *powerLabel = new Label("RF Power", this);
    powerLabel->move(QPoint(5, pos.y()));
    powerLabel->resize(width()/2, WIDGET_HEIGHT);
    Label *powerReadout = new Label("-32.22 dBm", this);
    powerReadout->move(QPoint(width()/2 + 5, pos.y()));
    powerReadout->resize(width()/2, WIDGET_HEIGHT);
    pos += QPoint(0, WIDGET_HEIGHT);

    Label *relativeLabel = new Label("Relative Power", this);
    relativeLabel->move(QPoint(5, pos.y()));
    relativeLabel->resize(width()/2, WIDGET_HEIGHT);
    Label *relativeReadout = new Label("-5.002 dB", this);
    relativeReadout->move(QPoint(width()/2 + 5, pos.y()));
    relativeReadout->resize(width()/2, WIDGET_HEIGHT);
    pos += QPoint(0, WIDGET_HEIGHT);

    Label *averageLabel = new Label("Averaged Power", this);
    averageLabel->move(QPoint(5, pos.y()));
    averageLabel->resize(width()/2, WIDGET_HEIGHT);
    Label *averageReadout = new Label("-4.998 dB", this);
    averageReadout->move(QPoint(width()/2 + 5, pos.y()));
    averageReadout->resize(width()/2, WIDGET_HEIGHT);
    pos += QPoint(0, WIDGET_HEIGHT*1.5);

    PushButton *sync = new PushButton("Sync", this);
    sync->move(QPoint(5, pos.y()));
    sync->resize(width()/2 - 10, WIDGET_HEIGHT);
    //pos += QPoint(0, WIDGET_HEIGHT);

    PushButton *done = new PushButton("Done", this);
    done->move(QPoint(width()/2 + 5, pos.y()));
    done->resize(width()/2 - 10, WIDGET_HEIGHT);
    pos += QPoint(0, WIDGET_HEIGHT*2);

    setFixedHeight(pos.y());
    setSizeGripEnabled(false);
}

MeasuringReceiver::~MeasuringReceiver()
{

}

