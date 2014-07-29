#ifndef DEMOD_PANEL_H
#define DEMOD_PANEL_H

#include "dock_panel.h"
#include "entry_widgets.h"

#include "model/demod_settings.h"

class DemodPanel : public DockPanel {
    Q_OBJECT

public:
    DemodPanel(const QString &title,
               QWidget *parent,
               const DemodSettings *settings);
    ~DemodPanel();

private:
    AmpEntry *inputPowerEntry;
    FrequencyEntry *centerEntry;
    ComboEntry *gainEntry;
    ComboEntry *attenEntry;
    ComboEntry *decimationEntry;
    FrequencyEntry *bandwidthEntry;
    CheckBoxEntry *autoBandwidthEntry;
    TimeEntry *sweepTimeEntry;

    ComboEntry *triggerTypeEntry;
    ComboEntry *triggerEdgeEntry;
    AmpEntry *triggerAmplitudeEntry; // Only for video triggers

    CheckBoxEntry *maEnabledEntry;
    FrequencyEntry *maLowPass;

public slots:
    void updatePanel(const DemodSettings *ds);

private:
    DISALLOW_COPY_AND_ASSIGN(DemodPanel)
};

#endif // DEMOD_PANEL_H
