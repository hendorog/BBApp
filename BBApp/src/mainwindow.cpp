#include "mainwindow.h"
#include "views/sweep_central.h"
#include "widgets/audio_dialog.h"
#include "widgets/progress_dialog.h"
#include "widgets/preferences_dialog.h"

#include <QFile>
#include <QSplitter>
#include <QTreeView>
#include <QMenuBar>
#include <QInputDialog>
#include <QtPrintSupport>
#include <QMessageBox>

BBStatusBar *MainWindow::status_bar;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      progressDialog(this),
      saveLayoutOnClose(true)
{
    setWindowTitle(tr("Broadband Spectrum Analyzer"));
    move(200, 0);
    resize(1820, 1080);

    session = new Session();

    // Side widgets have priority over top/bottom widgets
    this->setDockNestingEnabled(true);

    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);

    // Tab positions on the outside
    setTabPosition(Qt::RightDockWidgetArea, QTabWidget::East);
    setTabPosition(Qt::LeftDockWidgetArea, QTabWidget::West);

    sweep_panel = new SweepPanel(tr("Sweep Settings"), this,
                                 session->sweep_settings);
    sweep_panel->setObjectName("SweepSettingsPanel");
    connect(sweep_panel, SIGNAL(zeroSpanPressed()), this, SLOT(zeroSpanPressed()));
    measure_panel = new MeasurePanel(tr("Measurements"), this,
                                     session->trace_manager, session->sweep_settings);
    measure_panel->setObjectName("TraceMarkerPanel");
    demodPanel = new DemodPanel(tr("Demod Settings"), this, session->demod_settings);
    demodPanel->setObjectName("DemodSettingsPanel");

    addDockWidget(Qt::RightDockWidgetArea, sweep_panel);
    addDockWidget(Qt::LeftDockWidgetArea, measure_panel);
    addDockWidget(Qt::RightDockWidgetArea, demodPanel);

    status_bar = new BBStatusBar();
    setStatusBar(status_bar);

    InitMenuBar();

    centralStack = new CentralStack(this);
    setCentralWidget(centralStack);

    sweepCentral = new SweepCentral(session);
    //addToolBar(sweepCentral->GetToolBar());
    //sweepCentral->GetToolBar()->hide();
    connect(sweepCentral, SIGNAL(presetDevice()), this, SLOT(presetDevice()));
    centralStack->AddWidget(sweepCentral);

    demodCentral = new DemodCentral(session);
    //addToolBar(demodCentral->GetToolBar());
    //demodCentral->GetToolBar()->hide();
    centralStack->AddWidget(demodCentral);

    RestoreState();

    //sweep_panel->show();
    //measure_panel->show();
    //demodPanel->hide();
    ChangeMode(MODE_SWEEPING);

    connect(session->device, SIGNAL(connectionIssues()), this, SLOT(forceDisconnectDevice()));

    connectDevice();
}

MainWindow::~MainWindow()
{
    disconnectDevice();

    delete centralStack;
    delete session;

    delete sweep_panel;
    delete measure_panel;
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    SaveState();
    QMainWindow::closeEvent(e);
}

void MainWindow::InitMenuBar()
{
    main_menu = new QMenuBar();

    // File Menu
    file_menu = main_menu->addMenu(tr("File"));
    file_menu->addAction(tr("New"));
    file_menu->addSeparator();
    file_menu->addAction(tr("Print"), this, SLOT(printView()));
    file_menu->addAction(tr("Save as Image"), this, SLOT(saveAsImage()));
    file_menu->addSeparator();
    QMenu *import_menu = file_menu->addMenu(tr("Import"));
    QMenu *import_path_loss = import_menu->addMenu(tr("Path-Loss"));
    import_path_loss->addAction(tr("Import Path-Loss Table"), session->trace_manager, SLOT(importPathLoss()));
    import_path_loss->addAction(tr("Clear Path-Loss Table"), session->trace_manager, SLOT(clearPathLoss()));
    QMenu *import_limits = import_menu->addMenu(tr("Limit-Lines"));
    import_limits->addAction(tr("Import Limit-Lines"), session->trace_manager, SLOT(importLimitLines()));
    import_limits->addAction(tr("Clear Limit-Lines"), session->trace_manager, SLOT(clearLimitLines()));
    file_menu->addSeparator();
    file_menu->addAction(tr("Connect Device"), this, SLOT(connectDevice()));
    file_menu->addAction(tr("Disconnect Device"), this, SLOT(disconnectDevice()));
    connect(file_menu, SIGNAL(aboutToShow()),
            this, SLOT(aboutToShowFileMenu()));
    file_menu->addSeparator();
    file_menu->addAction(tr("Exit"), this, SLOT(close()));

    // Edit Menu
    QMenu *edit_menu = main_menu->addMenu(tr("Edit"));
    edit_menu->addAction(tr("Restore Default Layout"), this, SLOT(restoreDefaultLayout()));
    edit_menu->addSeparator();
    edit_menu->addAction(tr("Title"), this, SLOT(setTitle()));
    edit_menu->addAction(tr("Clear Title"), this, SLOT(clearTitle()));
    QMenu *view_color_menu = edit_menu->addMenu(tr("Colors"));
    view_color_menu->addAction(tr("Load Default Colors"), this, SLOT(loadDefaultColors()));
    view_color_menu->addAction(tr("Load Printer Friendly Colors"),
                               this, SLOT(loadPrinterFriendlyColors()));
    view_color_menu->addAction(tr("Save as Default"), this, SLOT(saveAsDefaultColorScheme()));
    edit_menu->addSeparator();

    QMenu *style_menu = edit_menu->addMenu("Program Style");
    style_menu->addAction(tr("Light"), this, SLOT(loadStyleLight()));
    style_menu->addAction(tr("Dark"), this, SLOT(loadStyleDark()));

    edit_menu->addSeparator();
    edit_menu->addAction(tr("Preferences"), this, SLOT(showPreferencesDialog()));

    // Preset Menu
    preset_menu = main_menu->addMenu(tr("Presets"));

    preset_menu->addAction(tr("Load Default Settings"), this, SLOT(loadDefaultSettings()));
    preset_menu->addSeparator();
    preset_load = preset_menu->addMenu(tr("Load"));
    preset_save = preset_menu->addMenu(tr("Save"));
    preset_menu->addSeparator();
    preset_name = preset_menu->addMenu(tr("Rename"));

    for(int i = 0; i < PRESET_COUNT; i++) {
        QAction *load_action = preset_load->addAction(tr(""));
        QAction *save_action = preset_save->addAction(tr(""));
        QAction *name_action = preset_name->addAction(tr(""));

        load_action->setData(int(i));
        save_action->setData(int(i));
        name_action->setData(int(i));

        QString numeric;
        numeric.sprintf("%d", i+1);
        load_action->setShortcut(QKeySequence("Ctrl+" + numeric));
    }

    connect(preset_load, SIGNAL(triggered(QAction*)),
            this, SLOT(loadPreset(QAction*)));
    connect(preset_save, SIGNAL(triggered(QAction*)),
            this, SLOT(savePreset(QAction*)));
    connect(preset_name, SIGNAL(triggered(QAction*)),
            this, SLOT(renamePreset(QAction*)));
    connect(preset_menu, SIGNAL(aboutToShow()),
            this, SLOT(loadPresetNames()));

    settings_menu = main_menu->addMenu(tr("Settings"));

    timebase_menu = settings_menu->addMenu(tr("Reference"));
    QAction *timebase_action;
    timebase_action = timebase_menu->addAction(tr("Internal"));
    timebase_action->setData(TIMEBASE_INTERNAL);
    timebase_action->setCheckable(true);
    timebase_action = timebase_menu->addAction(tr("External Sin Wave"));
    timebase_action->setData(TIMEBASE_EXT_AC);
    timebase_action->setCheckable(true);
    timebase_action = timebase_menu->addAction(tr("External CMOS-TTL"));
    timebase_action->setData(TIMEBASE_EXT_DC);
    timebase_action->setCheckable(true);
    connect(timebase_menu, SIGNAL(aboutToShow()), this, SLOT(aboutToShowTimebaseMenu()));
    connect(timebase_menu, SIGNAL(triggered(QAction*)), this, SLOT(timebaseChanged(QAction*)));

    QAction *sr_action = settings_menu->addAction(tr("Spur Reject"));
    sr_action->setCheckable(true);
    connect(sr_action, SIGNAL(triggered(bool)), session->sweep_settings, SLOT(setRejection(bool)));
    connect(settings_menu, SIGNAL(aboutToShow()), this, SLOT(aboutToShowSettingsMenu()));

    // Mode Select Menu
    mode_menu = main_menu->addMenu(tr("Analysis Mode"));

    QActionGroup *mode_action_group = new QActionGroup(mode_menu);
    QAction *mode_action;
    mode_action = mode_menu->addAction(tr("Idle"));
    mode_action->setData(MODE_IDLE);
    mode_action->setCheckable(true);
    mode_action_group->addAction(mode_action);

    mode_action = mode_menu->addAction(tr("Sweep"));
    mode_action->setData(MODE_SWEEPING);
    mode_action->setCheckable(true);
    mode_action->setChecked(true);
    mode_action_group->addAction(mode_action);

    mode_action = mode_menu->addAction(tr("Real-Time"));
    mode_action->setData(MODE_REAL_TIME);
    mode_action->setCheckable(true);
    mode_action_group->addAction(mode_action);

    mode_action = mode_menu->addAction(tr("Zero-Span"));
    mode_action->setData(MODE_ZERO_SPAN);
    mode_action->setCheckable(true);
    mode_action_group->addAction(mode_action);

    connect(mode_action_group, SIGNAL(triggered(QAction*)),
            this, SLOT(modeChanged(QAction*)));
    connect(mode_menu, SIGNAL(aboutToShow()),
            this, SLOT(aboutToShowModeMenu()));

    utilities_menu = main_menu->addMenu(tr("Utilities"));
    utilities_menu->addAction(tr("Audio Player"), this, SLOT(startAudioPlayer()));
    connect(utilities_menu, SIGNAL(aboutToShow()), this, SLOT(aboutToShowUtilitiesMenu()));

    help_menu = main_menu->addMenu(tr("Help"));
    QAction *help_action = help_menu->addAction(tr("About"));
    connect(help_action, SIGNAL(triggered()),
            this, SLOT(showAboutBox()));

    setMenuBar(main_menu);
}

/*
 * Save the application state
 * Called in MainWindow::closeEvent()
 */
void MainWindow::SaveState()
{
    if(!saveLayoutOnClose) {
        return;
    }

    // Use .ini files in AppData folder to save settings
    //  instead of the registry, user scope
    QSettings settings(QSettings::IniFormat,
                       QSettings::UserScope,
                       "SignalHound",
                       "Layout");

    settings.setValue("MainWindow/State", saveState());
    settings.setValue("MainWindow/Geometry", geometry());
    settings.setValue("MainWindow/Maximized", isMaximized());

    sweep_panel->SaveState(settings);
    measure_panel->SaveState(settings);
    demodPanel->SaveState(settings);
}

/*
 * Restore previously saved mainwindow state
 * Do nothing if the save .ini file doesn't exist
 */
void MainWindow::RestoreState()
{   
    // Use .ini files in AppData folder to save settings
    //  instead of the registry, user scope
    QSettings settings(QSettings::IniFormat,
                       QSettings::UserScope,
                       "SignalHound",
                       "Layout");

    QVariant value;

    // Get widget positions
    value = settings.value("MainWindow/State");
    if(value.isValid()) {
        restoreState(value.toByteArray());
    }

    // Get window geometry
    value = settings.value("MainWindow/Geometry");
    if(value.isValid()) {
        setGeometry(value.toRect());
    }

    // Reset maximized
    value = settings.value("MainWindow/Maximized");
    if(value.isValid()) {
        if(value.toBool()) {
            showMaximized();
        }
    }

    sweep_panel->RestoreState(settings);
    measure_panel->RestoreState(settings);
    demodPanel->RestoreState(settings);
}

void MainWindow::restoreDefaultLayout()
{
    // Get the file name of the layout .inf file
    QString fileName;
    {
        // QSettings must be destroyed before attempting to open
        //   QFile?
        QSettings settings(QSettings::IniFormat,
                           QSettings::UserScope,
                           "SignalHound",
                           "Layout");
        fileName = settings.fileName();
    }

    QFile file(fileName);
    if(file.exists()) {
        file.remove();
    }

    // Doesn't technically matter if the file existed to delete or not
    // Tell user to restart program
    saveLayoutOnClose = false;
    QMessageBox::information(this, "Restart Application",
                             "The default application layout will be restored "
                             "the next time the application is started.");
}

// Hide Connect/Disconnect Device Options
void MainWindow::aboutToShowFileMenu()
{
    QList<QAction*> a_list = file_menu->actions();
    if(a_list.length() <= 0) return;

    for(QAction *a : a_list) {
        if(a->text() == tr("Connect Device")) {
            a->setDisabled(session->device->IsOpen());
        }
        if(a->text() == tr("Disconnect Device")) {
            a->setDisabled(!session->device->IsOpen());
        }
    }
}

void MainWindow::aboutToShowSettingsMenu()
{
    for(QAction *a : settings_menu->actions()) {
        if(a->text() == tr("Spur Reject")) {
            a->setChecked(session->sweep_settings->Rejection());
        }
    }
}

void MainWindow::aboutToShowModeMenu()
{
    int current_mode = session->sweep_settings->Mode();

    QList<QAction*> a_list = mode_menu->actions();
    for(QAction *a : a_list) {
        a->setEnabled(session->device->IsOpen());

        if(a->data() == current_mode) {
            a->setChecked(true);
        }
    }
}

// Disable utilities that require a device to be open
void MainWindow::aboutToShowUtilitiesMenu()
{
    for(QAction *a : utilities_menu->actions()) {
        a->setEnabled(session->device->IsOpen());
    }
}

void MainWindow::OpenDeviceInThread()
{
    progressDialog.makeVisible("Opening Device\n"
                               "Estimated 3 seconds");

    session->device->OpenDevice();

    QMetaObject::invokeMethod(this, "deviceConnected",
                              Q_ARG(bool, session->device->IsOpen()));

    progressDialog.makeDisappear();

    return;
}

void MainWindow::PresetDeviceInThread()
{
    progressDialog.makeVisible("Preset Device\n"
                               "Estimated 3 seconds");

    // Stop all operation
    // Preset -> Close -> Wait -> Open
    //central_widget->changeMode(BB_IDLE);
    centralStack->CurrentWidget()->changeMode(BB_IDLE);
    session->device->Preset();
    session->device->CloseDevice();

    SleepEvent preset_wait;
    preset_wait.Sleep(3000);

    progressDialog.makeDisappear();

    session->LoadDefaults();
    OpenDeviceInThread();
    return;
}

/*
 * File Menu Connect Device
 */
void MainWindow::connectDevice()
{
    if(device_thread.joinable()) {
        device_thread.join();
    }

    device_thread = std::thread(&MainWindow::OpenDeviceInThread, this);
}

/*
 * File Menu Disconnect Device
 * Can also be called from the device recieving a disconnect error
 *   during normal operation
 */
void MainWindow::disconnectDevice()
{
    // Stop any sweeping
    centralStack->CurrentWidget()->changeMode(BB_IDLE);
    centralStack->CurrentWidget()->ResetView();

    session->LoadDefaults();
    session->trace_manager->Reset();

    session->device->CloseDevice();

    status_bar->SetMessage("");
    status_bar->SetDeviceType("No Device Connected");
    status_bar->SetDiagnostics("");
    status_bar->UpdateDeviceInfo("");
}

/*
 * Call disconnect AND provide user warning
 */
void MainWindow::forceDisconnectDevice()
{
    disconnectDevice();
    QMessageBox::warning(0, "Connectivity Issues", "Device Connection Issues Detected, Ensure the"
                         " device is connected before opening again via the File Menu");
}

/*
 * Preset Button Disconnect Device
 */
void MainWindow::presetDevice()
{
    if(device_thread.joinable()) {
        device_thread.join();
    }

    device_thread = std::thread(&MainWindow::PresetDeviceInThread, this);
}

void MainWindow::deviceConnected(bool success)
{
    QString device_string;

    if(success) {
        device_string.sprintf("Serial - %d    Firmware Ver. - %d",
                              session->device->SerialNumber(),
                              session->device->FirmwareVer());

        status_bar->SetDeviceType(session->device->GetDeviceString());
        status_bar->UpdateDeviceInfo(device_string);

        ChangeMode(MODE_SWEEPING);
        centralStack->CurrentWidget()->changeMode(BB_SWEEPING);

        if(session->device->DeviceType() == BB_DEVICE_BB60A) {
            SweepSettings::maxRealTimeSpan = BB60A_MAX_RT_SPAN;
        } else {
            SweepSettings::maxRealTimeSpan = BB60C_MAX_RT_SPAN;
        }
    } else {
        QMessageBox::information(this, tr("Connection Status"),
                                 tr("No device found. Use the file menu to"
                                    " open a device once connected."));
        status_bar->SetDeviceType("No Device Connected");
    }

    if(device_thread.joinable()) device_thread.join();
}

void MainWindow::printView()
{
    QPrinter printer;
    printer.setOrientation(QPrinter::Landscape);

    QPrintDialog *dialog = new QPrintDialog(&printer, this);
    if(dialog->exec() != QDialog::Accepted)
        return;

    QPainter painter(&printer);
    QRect rect = painter.viewport();

    QImage image;
    centralStack->CurrentWidget()->GetViewImage(image);

    QSize size = image.size();
    size.scale(rect.size(), Qt::KeepAspectRatio);
    painter.setViewport(rect.x(), rect.y(),
                        size.width()-5, size.height()-5);
    painter.setWindow(image.rect());
    painter.drawImage(2, 2, image);
}

void MainWindow::saveAsImage()
{
    QString file_name = QFileDialog::getSaveFileName(this,
                                                     tr("Image Save Name"),
                                                     bb_lib::get_my_documents_path(),
                                                     tr("Images (*.png)"));
    if(file_name.isNull()) return;

    QImage image;
    //central_widget->GetViewImage(image);
    centralStack->CurrentWidget()->GetViewImage(image);

    image.save(file_name);
}

void MainWindow::setTitle()
{
    bool ok;
    QString new_title =
            QInputDialog::getText(this,
                                  tr("Set Title"),
                                  tr("Enter Title (Between 3-63 characters)"),
                                  QLineEdit::Normal,
                                  QString(),
                                  &ok);

    if(ok) {
        session->SetTitle(new_title);
    }
}

void MainWindow::loadDefaultColors()
{
    session->colors.LoadDefaults();
}

void MainWindow::loadPrinterFriendlyColors()
{
    session->colors.LoadPrinterFriendly();
}

void MainWindow::saveAsDefaultColorScheme()
{

}

void MainWindow::loadDefaultSettings()
{
    centralStack->CurrentWidget()->StopStreaming();

    session->LoadDefaults();

    centralStack->CurrentWidget()->StartStreaming();
}

/*
 * Get user input for a new name
 * Replace the text in the .ini file
 */
void MainWindow::RenamePreset(int p)
{
    bool ok;
    QString newName =
            QInputDialog::getText(this,
                                  tr("Preset Renaming"),
                                  tr("New Name(Between 3-20 characters)"),
                                  QLineEdit::Normal,
                                  QString(),
                                  &ok);

    if(ok && (newName.length() > 2) && (newName.length() < 20)) {
        QSettings settings(QSettings::IniFormat,
                           QSettings::UserScope,
                           "SignalHound",
                           "PresetNames");

        QString key;
        key.sprintf("PresetNames/Preset%d", p + 1);
        settings.setValue(key, newName);
    }
}

/*
 * Slot for when the main preset file menu is about to
 *  be shown. All preset menu's share the same actions in
 *  which the text for those actions are updated here with the
 *  values found in the .ini file
 */
void MainWindow::loadPresetNames()
{
    QSettings settings(QSettings::IniFormat,
                       QSettings::UserScope,
                       "SignalHound",
                       "PresetNames");

    QString key;
    QVariant value;
    QList<QAction*> saveActions = preset_save->actions();
    QList<QAction*> loadAction = preset_load->actions();
    QList<QAction*> renameActions = preset_name->actions();

    for(int i = 0; i < PRESET_COUNT; i++) {
        key.sprintf("Preset%d", i + 1);
        value = settings.value("PresetNames/" + key);

        if(value.isValid()) {
            saveActions.at(i)->setText(value.toString());
            loadAction.at(i)->setText(value.toString());
            renameActions.at(i)->setText(value.toString());
        } else {
            saveActions.at(i)->setText(key);
            loadAction.at(i)->setText(key);
            renameActions.at(i)->setText(key);
        }
    }
}

void MainWindow::loadPreset(QAction *a)
{
    centralStack->CurrentWidget()->StopStreaming();
    session->LoadPreset(a->data().toInt());

    int newMode = session->sweep_settings->Mode();

    ChangeMode((OperationalMode)newMode);
//    if(newMode == MODE_ZERO_SPAN) {
//        centralStack->setCurrentWidget(demodCentral);
//        sweep_panel->hide();
//        measure_panel->hide();
//        demodPanel->show();
//    } else {
//        demodPanel->hide();
//        sweep_panel->show();
//        measure_panel->show();
//        centralStack->setCurrentWidget(sweepCentral);
//    }

    centralStack->CurrentWidget()->changeMode(newMode);
}

void MainWindow::modeChanged(QAction *a)
{
    centralStack->CurrentWidget()->StopStreaming();

    int newMode = a->data().toInt();
    ChangeMode((OperationalMode)newMode);

//    if(newMode == MODE_ZERO_SPAN) {
//        centralStack->setCurrentWidget(demodCentral);
//        sweep_panel->hide();
//        measure_panel->hide();
//        demodPanel->show();
//    } else {
//        demodPanel->hide();
//        sweep_panel->show();
//        measure_panel->show();
//        centralStack->setCurrentWidget(sweepCentral);
//        addToolBar(centralStack->CurrentWidget()->GetToolBar());
//    }

    centralStack->CurrentWidget()->changeMode(newMode);
}

void MainWindow::ChangeMode(OperationalMode newMode)
{
    this->removeToolBar(centralStack->CurrentWidget()->GetToolBar());
    //centralStack->CurrentWidget()->GetToolBar()->hide();
    if(newMode == MODE_ZERO_SPAN) {
        centralStack->setCurrentWidget(demodCentral);
        sweep_panel->hide();
        measure_panel->hide();
        demodPanel->show();
    } else {
        centralStack->setCurrentWidget(sweepCentral);
        demodPanel->hide();
        sweep_panel->show();
        measure_panel->show();
    }
    this->addToolBar(centralStack->CurrentWidget()->GetToolBar());
    centralStack->CurrentWidget()->GetToolBar()->show();
}

// Function gets called when the zero-span button is pressed on the sweep panel
// Know for a fact that the device is coming from a sweep mode
void MainWindow::zeroSpanPressed()
{
    centralStack->CurrentWidget()->StopStreaming();

    // Set the zero-span center to the current sweep center
    Frequency currentCenter = session->sweep_settings->Center();
    session->demod_settings->setCenterFreq(currentCenter);

    ChangeMode(MODE_ZERO_SPAN);
//    // Hide sweep panels, show zero-span ones
//    centralStack->setCurrentWidget(demodCentral);
//    sweep_panel->hide();
//    measure_panel->hide();
//    demodPanel->show();

    centralStack->CurrentWidget()->changeMode(MODE_ZERO_SPAN);
}

void MainWindow::startAudioPlayer()
{
    int temp_mode = session->sweep_settings->Mode();

    centralStack->CurrentWidget()->changeMode(MODE_IDLE);
    // Start the Audio Dialog with the active center frequency
    session->audio_settings->setCenterFrequency(
                centralStack->CurrentWidget()->GetCurrentCenterFreq());

    AudioDialog *dlg = new AudioDialog(session->device, session->audio_settings);

    dlg->exec();
    *session->audio_settings = *dlg->Configuration();
    delete dlg;

    centralStack->CurrentWidget()->changeMode(temp_mode);
}

void MainWindow::aboutToShowTimebaseMenu()
{
    for(QAction *a : timebase_menu->actions()) {
        a->setChecked(a->data().toInt() == session->device->TimebaseReference());
    }
}

void MainWindow::showPreferencesDialog()
{
    PreferenceDialog prefDlg(session);
    prefDlg.exec();
}

/*
 * String for our "About" box
 */
QChar trademark_char(short(174));
QChar copyright_char(short(169));
const QString about_string =
        QWidget::tr("Signal Hound") + trademark_char + QWidget::tr("\n") +
        QWidget::tr("Copyright ") + copyright_char + QWidget::tr(" 2014\n");
const QString gui_version = QWidget::tr("Software Version 2.0.4\n");

void MainWindow::showAboutBox()
{
    QString api_string = tr("API Version ") + tr(bbGetAPIVersion());
    QMessageBox::about(this, tr("About"), about_string + gui_version + api_string);
}
