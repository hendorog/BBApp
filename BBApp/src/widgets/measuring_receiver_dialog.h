#ifndef MEASURING_RECEIVER_DIALOG_H
#define MEASURING_RECEIVER_DIALOG_H

#include <thread>
#include <atomic>

#include <QDialog>
#include <QButtonGroup>

#include "../model/device.h"
#include "../widgets/entry_widgets.h"

class MeasuringReceiver : public QDialog {
    Q_OBJECT

    static const int rangeLevelNone = -1;
    enum RangeLevel {
        rangeLevelHigh = 0,
        rangeLevelMid = 1,
        rangeLevelLow = 2
    };
    enum RangeColor {
        rangeColorBlack = 0,
        rangeColorRed,
        rangeColorOrange,
        rangeColorGreen
    };

public:
    MeasuringReceiver(Device *devicePtr,
                      double initialCenter,
                      QWidget *parent = 0);
    ~MeasuringReceiver();

public slots:

private slots:
    void syncPressed() { reinitialize = true; }
    void triggerReinitialize() { reinitialize = true; }
    void triggerRecalibration() { recalibrate = true; }
    // Set all our output RF measurement labels
    void setLabels(const QString &centerStr, const QString &powerStr,
                   const QString &relativeStr, const QString &averageStr)
    {
        centerReadout->setText(centerStr);
        powerReadout->setText(powerStr);
        relativeReadout->setText(relativeStr);
        averageReadout->setText(averageStr);
    }
    // Set a single range text label, clears all others
    void setRangeLevelText(int pl, const QString &text, int color)
    {
        highLabel->setText("");
        midLabel->setText("");
        lowLabel->setText("");

        QString lblText;

        switch(color) {
        case rangeColorRed: lblText = "<font color=red>" + text + "</font>"; break;
        case rangeColorOrange: lblText = "<font color=orange>" + text + "</font>"; break;
        case rangeColorGreen: lblText = "<font color=green>" + text + "</font>"; break;
        default: lblText = "<font color=black>" + text + "</font>"; break;
        }

        switch(pl) {
        case rangeLevelHigh: highLabel->setText(lblText); break;
        case rangeLevelMid: midLabel->setText(lblText); break;
        case rangeLevelLow: lowLabel->setText(lblText); break;
        }
    }
    // Set the current range text label
    void setRangeLevelText(const QString &text, int color) {
        setRangeLevelText(GetCurrentRange(), text, color);
    }
    // Enable a single range button or disable with rangeLevelNone
    void setRangeEnabled(int range) {
        ampGroup->button(rangeLevelHigh)->setEnabled(range == rangeLevelHigh);
        ampGroup->button(rangeLevelMid)->setEnabled(range == rangeLevelMid);
        ampGroup->button(rangeLevelLow)->setEnabled(range == rangeLevelLow);
    }

    void setEntryEnabled(bool enabled) {
        freqEntry->setEnabled(enabled);
    }

signals:
    void updateLabels(const QString &, const QString &,
                      const QString &, const QString &);
    void updateRangeLevelText(int, const QString &, int);
    void updateRangeLevelText(const QString &, int);
    void updateRangeEnabled(int);
    void updateEntryEnabled(bool);

protected:

private:
    void Recalibrate(double &centerOut, double &powerOut, IQCapture &iqc);
    void ProcessThread();
    RangeLevel GetCurrentRange() const {
        return static_cast<RangeLevel>(ampGroup->checkedId());
    }

    FrequencyEntry *freqEntry;
    Label *highLabel, *midLabel, *lowLabel;
    QButtonGroup *ampGroup;
    Label *centerReadout, *powerReadout;
    Label *relativeReadout, *averageReadout;

    std::thread threadHandle;
    std::atomic<bool> running, reinitialize, recalibrate;
    Device *device;
};

#endif // MEASURING_RECEIVER_DIALOG_H
