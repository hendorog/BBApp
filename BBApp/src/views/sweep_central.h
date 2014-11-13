#ifndef SWEEP_CENTRAL_H
#define SWEEP_CENTRAL_H

#include <thread>
#include <atomic>

#include "views/central_stack.h"
#include "../model/session.h"
#include "../widgets/entry_widgets.h"

class QToolBar;
class PlaybackToolBar;
class QSplitter;
class TraceView;
class WaterfallView;

class SweepCentral : public CentralWidget {
    Q_OBJECT

    const static int TOOLBAR_H = 30;
public:
    SweepCentral(Session *sPtr, QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~SweepCentral();

    void StartStreaming();
    void StopStreaming();
    void ResetView();
    void GetViewImage(QImage &image);
    Frequency GetCurrentCenterFreq() const {
        return session_ptr->sweep_settings->Center(); }
    // Force view back to initial start-up values
    // No persistence, waterfall, etc.

protected:
    void resizeEvent(QResizeEvent *);
    void keyPressEvent(QKeyEvent *);

private:
    void Reconfigure();
    void SweepThread();
    void PlaybackThread();

    bool reconfigure;
    Trace trace;
    SweepSettings last_config; // Last known working settings

    TraceView *trace_view;

    ComboBox *waterfall_combo;
    QCheckBox *persistence_check;
    SHPushButton *persistence_clear;
    SHPushButton *single_sweep, *continuous_sweep;
    QPushButton *preset_button;

    PlaybackToolBar *playback;

    // Copy of session
    Session *session_ptr;

    std::thread thread_handle;
    std::atomic<bool> sweeping;
    std::atomic<int> sweep_count;

signals:
    void updateView();
    void presetDevice();

public slots:
    void changeMode(int newState);
    void settingsChanged(const SweepSettings *ss);

private slots:
    void singleSweepPressed();
    void continuousSweepPressed();
    // Update the view behind the scenes
    void forceUpdateView();
    void playFromFile(bool play);
};

#endif // SWEEP_CENTRAL_H
