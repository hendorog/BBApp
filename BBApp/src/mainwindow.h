#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStatusBar>
#include <QAction>

#include "model/session.h"
#include "widgets/measure_panel.h"
#include "widgets/sweep_panel.h"
#include "widgets/status_bar.h"
#include "widgets/progress_dialog.h"

class SweepCentral;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    Session* GetSession() { return session; }
    static BBStatusBar* GetStatusBar() { return status_bar; }

protected:
    void closeEvent(QCloseEvent *);

private:
    void InitMenuBar();
    void SaveState();
    void RestoreState();

    void RenamePreset(int p);

    void OpenDeviceInThread();
    void PresetDeviceInThread();

    // The one and only session instance
    //   all other Session pointers are copies
    Session *session;

    QMenuBar *main_menu;
    QMenu *file_menu;
    QMenu *preset_menu;
    QMenu *preset_load;
    QMenu *preset_save;
    QMenu *preset_name;
    QMenu *mode_menu;
    QMenu *settings_menu, *timebase_menu;
    QMenu *utilities_menu;
    QMenu *help_menu;

    SweepPanel *sweep_panel;
    MeasurePanel *measure_panel;

    SweepCentral *central_widget;
    static BBStatusBar *status_bar;

    // Used for opening/closing BB60
    std::thread device_thread;
    ProgressDialog progressDialog;

public slots:
    void connectDevice();
    // Call direct when user closes device
    void disconnectDevice();
    // Call when device must be forced close
    void forceDisconnectDevice();
    void presetDevice();

private slots:
    void aboutToShowFileMenu();
    void aboutToShowSettingsMenu();
    void aboutToShowModeMenu();
    void aboutToShowUtilitiesMenu();

    void deviceConnected(bool);

    void printView();
    void saveAsImage();

    void setTitle();
    void clearTitle() { session->SetTitle(QString()); }
    void loadDefaultColors();
    void loadPrinterFriendlyColors();
    void saveAsDefaultColorScheme();
    void loadStyleLight() { session->prefs.SetProgramStyle(LIGHT_STYLE_SHEET); }
    void loadStyleDark() { session->prefs.SetProgramStyle(DARK_STYLE_SHEET); }

    void loadDefaultSettings();
    void loadPreset(QAction *a) { session->LoadPreset(a->data().toInt()); }
    void savePreset(QAction *a) { session->SavePreset(a->data().toInt()); }
    void renamePreset(QAction *a) { RenamePreset(a->data().toInt()); }
    void loadPresetNames();
    void modeChanged(QAction *a);
    void startAudioPlayer();
    void aboutToShowTimebaseMenu();
    void timebaseChanged(QAction *a) { session->device->setTimebase(a->data().toInt()); }
    void showPreferencesDialog();
    void showAboutBox();

signals:

private:
    DISALLOW_COPY_AND_ASSIGN(MainWindow)
};

#endif // MAINWINDOW_H
