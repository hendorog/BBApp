#ifndef TG_PANEL_H
#define TG_PANEL_H

#include "dock_panel.h"
#include "entry_widgets.h"
#include "lib/bb_lib.h"

class Session;

class TgCtrlPanel : public DockPanel {
    Q_OBJECT

public:
    TgCtrlPanel(const QString &title, QWidget *parent, Session *session);
    ~TgCtrlPanel();

private:
    DockPage *main_page;
    FrequencyEntry *center, *freqStepSize;
    NumericEntry *amp, *ampStepSize;
    Session *session_ptr; // Does not own

private slots:
    void changeFrequency(Frequency f);
    void stepFrequencyDown();
    void stepFrequencyUp();
    void changeAmplitude(double a);
    void stepAmplitudeDown();
    void stepAmplitudeUp();

private:
    Q_DISABLE_COPY(TgCtrlPanel)
};

#endif // TG_PANEL_H
