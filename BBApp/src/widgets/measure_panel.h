#ifndef MEASURE_PANEL_H
#define MEASURE_PANEL_H

#include "dock_panel.h"
#include "entry_widgets.h"

class TraceManager;

class MeasurePanel : public DockPanel {
    Q_OBJECT

public:
    MeasurePanel(const QString &title,
                 QWidget *parent,
                 TraceManager *trace_manager);
    ~MeasurePanel();

private:
    // Trace Widgets
    ComboEntry *trace_select;
    ComboEntry *trace_type;
    ColorEntry *trace_color;
    CheckBoxEntry *trace_updating;
    DualButtonEntry *export_clear;

    // Marker Widgets
    ComboEntry *marker_select;
    ComboEntry *on_trace_select;
    CheckBoxEntry *marker_update;
    CheckBoxEntry *marker_active;
    DualButtonEntry *peak_delta;
    DualButtonEntry *to_center_ref;
    DualButtonEntry *peak_left_right;

    // Offsets
    NumericEntry *ref_offset;

    // Channel Power
    FrequencyEntry *channel_width;
    FrequencyEntry *channel_spacing;
    CheckBoxEntry *channel_power_enabled;

    // Copy of the pointer, does not own
    TraceManager *trace_manager_ptr;

public slots:
    void updateTraceView();
    void updateTraceView(int);
    void updateMarkerView();
    void updateMarkerView(int);

private slots:
    void channelPowerUpdated();

signals:

private:
    DISALLOW_COPY_AND_ASSIGN(MeasurePanel)
};

#endif // MEASURE_PANEL_H
