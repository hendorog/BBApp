#ifndef SWEEP_CENTRAL_H
#define SWEEP_CENTRAL_H

#include <QWidget>
#include <thread>
#include <atomic>

#include "../model/session.h"
#include "../widgets/entry_widgets.h"

class QToolBar;
class PlaybackToolBar;
class QSplitter;
class TraceView;
class WaterfallView;

class SweepCentral : public QWidget {
    Q_OBJECT

    const static int TOOLBAR_H = 30;
public:
    SweepCentral(QWidget *parent = 0);
    ~SweepCentral();

    void GetViewImage(QImage &image);
    void StopStreaming();
    void StartStreaming();
    Frequency GetCurrentCenterFreq() const {
        return session_ptr->sweep_settings->Center(); }
    // Force view back to initial start-up values
    // No persistence, waterfall, etc.
    void ResetView();

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

    QToolBar *tool_bar;
    ComboBox *waterfall_combo;
    QCheckBox *persistence_check;
    QPushButton *persistence_clear;
    QPushButton *single_sweep, *continuous_sweep;
    QPushButton *preset_button;

    PlaybackToolBar *playback;

    // Copy of session
    Session *session_ptr;

    std::thread thread_handle;
    std::atomic<bool> sweeping;
    std::atomic<int> sweep_count;

signals:
    void updateView();

public slots:
    void changeMode(int new_state);
    void settingsChanged(const SweepSettings *ss);

private slots:
    void singleSweepPressed();
    void continuousSweepPressed();
    // Update the view behind the scenes
    void forceUpdateView();
    void playFromFile(bool play);
};

#endif // SWEEP_CENTRAL_H
