#ifndef SWEEP_PANEL_H
#define SWEEP_PANEL_H

#include "dock_panel.h"
#include "entry_widgets.h"

class SweepSettings;

class SweepPanel : public DockPanel {
    Q_OBJECT

public:
    SweepPanel(const QString &title,
               QWidget *parent,
               const SweepSettings *settings);
    ~SweepPanel();

private:
    FreqShiftEntry *center;
    FreqShiftEntry *span;
    FrequencyEntry *start;
    FrequencyEntry *stop;
    FrequencyEntry *step;
    DualButtonEntry *full_zero_span;

    AmplitudeEntry *ref;
    NumericEntry *div;
    ComboEntry *gain;
    ComboEntry *atten;

    CheckBoxEntry *native_rbw;
    FreqShiftEntry *rbw;
    FreqShiftEntry *vbw;
    DualCheckBox *auto_bw;

    ComboEntry *video_units;
    ComboEntry *detector;
    TimeEntry *sweep_time;

public slots:
    void updatePanel(const SweepSettings *settings);

signals:


private:
    DISALLOW_COPY_AND_ASSIGN(SweepPanel)
};

#endif // SWEEP_PANEL_H
