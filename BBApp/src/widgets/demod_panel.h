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
               const SweepSettings *settings);
    ~DemodPanel();

private:
    CheckBoxEntry triggerTypeEntry;
    CheckBoxEntry triggerEdgeEntry;
    AmplitudeEntry triggerAmplitudeEntry; // Only for video triggers

public slots:
    void updateTriggerSettings(TriggerSettings &ts);

signals:
    void triggerSettingsChanged(TriggerSettings &ts);

private:
    DISALLOW_COPY_AND_ASSIGN(DemodPanel)
};

#endif // DEMOD_PANEL_H
