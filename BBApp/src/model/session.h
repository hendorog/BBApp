#ifndef SESSION_H
#define SESSION_H

#include "device_bb60a.h"

#include "sweep_settings.h"
#include "trace_manager.h"
#include "audio_settings.h"
#include "color_prefs.h"
#include "preferences.h"

const int MAX_TITLE_LEN = 127;

class Session : public QObject {
    Q_OBJECT

public:
    Session();
    ~Session();

    void LoadDefaults();
    void LoadPreset(int p);
    void SavePreset(int p);

    static QString GetTitle() { return title; }
    static void SetTitle(const QString &new_title);
    static QString title;

    Device *device;
    SweepSettings *sweep_settings;
    TraceManager *trace_manager;
    AudioSettings *audio_settings;
    ColorPrefs colors;
    Preferences prefs;
};

#endif // SESSION_H
