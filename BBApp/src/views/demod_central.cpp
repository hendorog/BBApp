#include "demod_central.h"

#include "demod_iq_time_plot.h"
#include "demod_spectrum_plot.h"
#include "demod_sweep_plot.h"

#include <QXmlStreamWriter>

DemodCentral::DemodCentral(Session *sPtr, QWidget *parent, Qt::WindowFlags f) :
    CentralWidget(parent, f),
    sessionPtr(sPtr),
    reconfigure(false),
    recordBinary(true),
    recordDialog(this)
{
    currentRecordDir = bb_lib::get_my_documents_path();
    recordLength = 1.0;

    recordDialog.setModal(true);
    recordDialog.setText("Recording may take many seconds. Please wait.");
    recordDialog.setStandardButtons(QMessageBox::NoButton);
    recordDialog.setDefaultButton(QMessageBox::NoButton);

    toolBar = new QToolBar();
    toolBar->setObjectName("DemodToolBar");
    toolBar->setMovable(false);
    toolBar->setFloatable(false);
    toolBar->layout()->setContentsMargins(0, 0, 0, 0);
    toolBar->layout()->setSpacing(0);

    ComboBox *demodSelect = new ComboBox();
    QStringList comboString;
    comboString << "AM Demod" << "FM Demod" << "PM Demod";
    demodSelect->insertItems(0, comboString);
    demodSelect->setFixedSize(200, 30-4);

    QPushButton *markerOff, *markerDelta;
    markerOff = new QPushButton("Marker Off");
    markerOff->setObjectName("BBPushButton");
    markerOff->setFixedSize(120, 30-4);

    markerDelta = new QPushButton("Marker Delta");
    markerDelta->setObjectName("BBPushButton");
    markerDelta->setFixedSize(120, 30-4);

    toolBar->addWidget(new FixedSpacer(QSize(10, 30)));
    toolBar->addWidget(demodSelect);
    toolBar->addWidget(new FixedSpacer(QSize(10, 30)));
    toolBar->addSeparator();
    toolBar->addWidget(new FixedSpacer(QSize(10, 30)));
    toolBar->addWidget(markerOff);
    toolBar->addWidget(markerDelta);

    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    toolBar->addWidget(spacer);

    QPushButton *singleSweep, *autoSweep, *presetBtn;

    singleSweep = new PushButton("Single", toolBar);
    singleSweep->setFixedSize(120, TOOLBAR_HEIGHT - 4);
    connect(singleSweep, SIGNAL(clicked()), this, SLOT(singlePressed()));

    autoSweep = new PushButton("Auto", toolBar);
    autoSweep->setFixedSize(120, TOOLBAR_HEIGHT - 4);
    connect(autoSweep, SIGNAL(clicked()), this, SLOT(autoPressed()));

    presetBtn = new QPushButton("Preset", toolBar);
    presetBtn->setObjectName("BBPresetButton");
    presetBtn->setFixedSize(120, TOOLBAR_HEIGHT - 4);
    connect(presetBtn, SIGNAL(clicked()), this, SIGNAL(presetDevice()));

    toolBar->addWidget(singleSweep);
    toolBar->addWidget(autoSweep);
    toolBar->addWidget(new FixedSpacer(QSize(10, 30)));
    toolBar->addSeparator();
    toolBar->addWidget(new FixedSpacer(QSize(10, 30)));
    toolBar->addWidget(presetBtn);
    toolBar->addWidget(new FixedSpacer(QSize(10, 30)));

    recordToolBar = new QToolBar(this);
    recordToolBar->setMovable(false);
    recordToolBar->setFloatable(false);
    recordToolBar->layout()->setContentsMargins(0, 0, 0, 0);
    recordToolBar->layout()->setSpacing(0);

    Label *recordDirLabel = new Label("Record Directory");
    recordDirLabel->setFixedSize(100, 30);
    recordDirLabel->setAlignment(Qt::AlignCenter);
    PushButton *browseDirButton = new PushButton("Browse");
    browseDirButton->setFixedSize(80, 26);
    currentRecordDirLabel = new Label(currentRecordDir);
    currentRecordDirLabel->setAlignment(Qt::AlignCenter);
    currentRecordDirLabel->setFixedHeight(30);
    currentRecordDirLabel->setFixedWidth(
                currentRecordDirLabel->fontMetrics().width(currentRecordDirLabel->text()) + 40);
    Label *recordLenLabel = new Label("Record Length(ms)");
    recordLenLabel->setFixedSize(120, 30);
    recordLenLabel->setAlignment(Qt::AlignCenter);
    recordLenEntry = new LineEntry(VALUE_ENTRY);
    recordLenEntry->SetValue(recordLength);
    recordLenEntry->setFixedSize(80, 26);
    Label *recordSaveAs = new Label("Save as");
    recordSaveAs->setFixedSize(60, 30);
    recordSaveAs->setAlignment(Qt::AlignCenter);
    ComboBox *saveAsSelect = new ComboBox();
    QStringList saveAsComboString;
    saveAsComboString << "Binary" << "Text (csv)";
    saveAsSelect->insertItems(0, saveAsComboString);
    saveAsSelect->setFixedSize(80, 26);
    connect(saveAsSelect, SIGNAL(activated(int)), this, SLOT(saveAsType(int)));
    PushButton *recordButton = new PushButton("Record");
    recordButton->setFixedSize(120, 26);

    recordToolBar->addWidget(new FixedSpacer(QSize(10, 30)));
    recordToolBar->addWidget(recordDirLabel);
    recordToolBar->addWidget(browseDirButton);
    recordToolBar->addWidget(currentRecordDirLabel);
    recordToolBar->addSeparator();
    recordToolBar->addWidget(recordLenLabel);
    recordToolBar->addWidget(recordLenEntry);
    recordToolBar->addWidget(new FixedSpacer(QSize(10, 30)));
    recordToolBar->addSeparator();
    recordToolBar->addWidget(new FixedSpacer(QSize(10, 30)));
    recordToolBar->addWidget(recordSaveAs);
    recordToolBar->addWidget(saveAsSelect);
    recordToolBar->addWidget(new FixedSpacer(QSize(10, 30)));
    recordToolBar->addSeparator();
    recordToolBar->addWidget(new FixedSpacer(QSize(10, 30)));
    recordToolBar->addWidget(recordButton);

    connect(browseDirButton, SIGNAL(clicked()), this, SLOT(changeRecordDirectory()));
    connect(recordLenEntry, SIGNAL(entryUpdated()), this, SLOT(recordLengthChanged()));
    connect(recordButton, SIGNAL(clicked()), this, SLOT(recordPressed()));

    spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    recordToolBar->addWidget(spacer);

    demodArea = new MdiArea(this);
    demodArea->move(0, 0);

    DemodSweepPlot *sweepPlot = new DemodSweepPlot(sPtr);
    demodArea->addSubWindow(sweepPlot);

    connect(demodSelect, SIGNAL(activated(int)), sweepPlot, SLOT(changeDemod(int)));
    connect(markerOff, SIGNAL(clicked()), sweepPlot, SLOT(disableMarker()));
    connect(markerDelta, SIGNAL(clicked()), sweepPlot, SLOT(toggleDelta()));

    DemodSpectrumPlot *freqPlot = new DemodSpectrumPlot(sPtr);
    freqPlot->setWindowTitle(tr("Spectrum Plot"));
    demodArea->addSubWindow(freqPlot);

    DemodIQTimePlot *iqPlot = new DemodIQTimePlot(sPtr);
    iqPlot->setWindowTitle(tr("IQ Plot"));
    demodArea->addSubWindow(iqPlot);

    connect(this, SIGNAL(updateViews()), demodArea, SLOT(updateViews()));
    connect(this, SIGNAL(showRecordingDialog(bool)), this, SLOT(updateRecordingDialog(bool)));

    for(QMdiSubWindow *window : demodArea->subWindowList()) {
        window->setWindowFlags(Qt::FramelessWindowHint);
    }

    connect(sessionPtr->demod_settings, SIGNAL(updated(const DemodSettings*)),
            this, SLOT(updateSettings(const DemodSettings*)));
}

DemodCentral::~DemodCentral()
{
    StopStreaming();
}

void DemodCentral::StartStreaming()
{
    streaming = true;
    threadHandle = std::thread(&DemodCentral::StreamThread, this);
}

void DemodCentral::StopStreaming()
{
    streaming = false;
    if(threadHandle.joinable()) {
        threadHandle.join();
    }
}

void DemodCentral::ResetView()
{

}

// Paint into image via QPainter
void DemodCentral::GetViewImage(QImage &image)
{
    QImage temp(demodArea->size(), QImage::Format_RGB32);
    QList<QMdiSubWindow*> list = demodArea->subWindowList();

    QPainter painter;
    painter.begin(&temp);
    for(int i = 0; i < list.length(); i++) {
        QMdiSubWindow *window = list.at(i);
        QGLWidget *view = static_cast<QGLWidget*>(window->widget());
        painter.drawImage(window->x(), window->y(), view->grabFrameBuffer());
    }
    painter.end();

    image = temp;
}

void DemodCentral::resizeEvent(QResizeEvent *)
{
    demodArea->resize(width(), height() - 30);
    demodArea->retile();
    recordToolBar->resize(width(), 30);
    recordToolBar->move(0, height() - 30);
}

void DemodCentral::changeMode(int newState)
{
    StopStreaming();
    captureCount = -1;

    sessionPtr->sweep_settings->setMode((OperationalMode)newState);

    if(newState == MODE_ZERO_SPAN) {
        StartStreaming();
    }
}

void DemodCentral::Reconfigure(DemodSettings *ds, IQCapture *iqc, IQSweep &iqs)
{
    if(!sessionPtr->device->Reconfigure(ds, &iqs.descriptor)) {
        *ds = lastConfig;
    } else {
        lastConfig = *ds;
    }

    // Resize single capture size and full sweep
    iqc->capture.resize(iqs.descriptor.returnLen);

    int sweepLen = ds->SweepTime().Val() / iqs.descriptor.timeDelta;
    int fullLen = bb_lib::next_multiple_of(iqs.descriptor.returnLen,
                                           sweepLen + iqs.descriptor.returnLen);

    iqs.iq.resize(fullLen);
    iqs.sweepLen = sweepLen;
    iqs.settings = *ds;

    reconfigure = false;
}

void DemodCentral::GetCapture(const DemodSettings *ds, IQCapture &iqc, IQSweep &iqs, Device *device)
{
    int retrieved = 0, toRetrieve = iqs.sweepLen;
    int firstIx = 0; // Start index in first capture
    bool flush = iqs.triggered; // Clear the API buffer?
    iqs.triggered = false;

    if(ds->TrigType() == TriggerTypeNone) {
        device->GetIQFlush(&iqc, flush);
        iqs.triggered = true;
    } else {
        device->GetIQFlush(&iqc, flush); // Start with flush
        if(ds->TrigType() == TriggerTypeVideo) {
            double trigVal = ds->TrigAmplitude().ConvertToUnits(DBM);
            trigVal = pow(10.0, (trigVal/10.0));
            int maxTimeForTrig = 0;
            while(maxTimeForTrig++ < 7) {
                if(ds->TrigEdge() == TriggerEdgeRising) {
                    firstIx = find_rising_trigger(&iqc.capture[0], trigVal, iqc.capture.size());
                } else {
                    firstIx = find_falling_trigger(&iqc.capture[0], trigVal, iqc.capture.size());
                }
                if(firstIx >= 0) {
                    iqs.triggered = true;
                    break;
                }
                device->GetIQ(&iqc);
                firstIx = 0;
            }
            int i = 2;
        } else if(ds->TrigType() == TriggerTypeExternal) {
            int maxTimeForTrig = 0;
            while(maxTimeForTrig++ < 7) {
                firstIx = iqc.triggers[0];
                if(firstIx != 0) {
                    iqs.triggered = true;
                    firstIx /= ((0x1 << ds->DecimationFactor()) * 2);
                    break;
                }
                device->GetIQ(&iqc);
            }
        }
    }

    // Retrieve the rest of the capture after the trigger, or retrieve the
    //   entire capture if there is no trigger
    while(toRetrieve > 0) {
        int toCopy = bb_lib::min2(iqs.descriptor.returnLen,
                                  iqs.descriptor.returnLen - firstIx);
        simdCopy_32fc(&iqc.capture[firstIx], &iqs.iq[retrieved], toCopy);
        firstIx = 0;
        toRetrieve -= toCopy;
        retrieved += toCopy;
        if(toRetrieve > 0) {
            device->GetIQ(&iqc);
        }
    }

    // Store how many total samples we have access to
    iqs.dataLen = retrieved;
}

void DemodCentral::RecordIQCapture(const IQSweep &sweep, IQCapture &capture, Device *device)
{
    emit showRecordingDialog(true);

    QString fileName = bb_lib::get_iq_filename();
    QFile xmlFile(currentRecordDir + fileName + ".xml");
    QFile iqFile(currentRecordDir + fileName + (recordBinary ? ".bin" : ".csv"));

    if(!xmlFile.open(QIODevice::WriteOnly) || !iqFile.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Warning", "Unable to create recording files, recording cancelled.");
        recordNext = false;
        return;
    }

    int totalLen = (this->recordLength / 1000.0) / sweep.descriptor.timeDelta;
    int fromSweep = qMin(totalLen, sweep.dataLen);
    int toGet = totalLen - sweep.dataLen;
    int capturesLeft = (toGet <= 0) ? 0 : ((toGet / sweep.descriptor.returnLen) + 1);

    std::vector<complex_f> rest;
    rest.resize(capturesLeft * sweep.descriptor.returnLen);

    int fetches = 0, retrieved = toGet;
    while(retrieved > 0) {
        device->GetIQ(&capture);
        simdCopy_32fc(&capture.capture[0],
                &rest[fetches * sweep.descriptor.returnLen],
                sweep.descriptor.returnLen);
        fetches++;
        retrieved -= sweep.descriptor.returnLen;
    }

    QXmlStreamWriter xmlWriter;
    xmlWriter.setAutoFormatting(true);
    xmlWriter.setDevice(&xmlFile);
    xmlWriter.writeStartDocument();
    xmlWriter.writeStartElement("Stream Description");

    xmlWriter.writeStartElement("Sample Rate");
    xmlWriter.writeCharacters(QVariant(sweep.descriptor.sampleRate).toString());
    xmlWriter.writeEndElement();
    xmlWriter.writeStartElement("Date");
    xmlWriter.writeCharacters(QDateTime::currentDateTime().toString());
    xmlWriter.writeEndElement();
    xmlWriter.writeStartElement("Center Frequency");
    xmlWriter.writeCharacters(QVariant(sweep.settings.CenterFreq().Val()).toString());
    xmlWriter.writeEndElement();
    xmlWriter.writeStartElement("Samples");
    xmlWriter.writeCharacters(QVariant(totalLen).toString());
    xmlWriter.writeEndElement();
    xmlWriter.writeStartElement("Data Type");
    xmlWriter.writeCharacters("32-bit complex");
    xmlWriter.writeEndElement();
    xmlWriter.writeStartElement("Capture Duration");
    xmlWriter.writeCharacters(Time(recordLength / 1000.0).GetString());
    xmlWriter.writeEndElement();

    xmlWriter.writeEndElement(); // End "Stream Description"
    xmlWriter.writeEndDocument();

    if(recordBinary) {
        iqFile.write(reinterpret_cast<const char*>(&sweep.iq[0]),
                fromSweep * sizeof(complex_f));
        if(toGet > 0) {
            iqFile.write(reinterpret_cast<const char*>(&rest[0]),
                    toGet * sizeof(complex_f));
        }
    } else {
        QTextStream out(&iqFile);
        for(auto i = 0; i < fromSweep; i++) {
            out << sweep.iq[i].re << ", " << sweep.iq[i].im << "\r\n";
        }
        if(toGet > 0) {
            for(auto i = 0; i < toGet; i++) {
                out << rest[i].re << ", " << rest[i].im << "\r\n";
            }
        }
    }

    emit showRecordingDialog(false);

    recordNext = false;
}

void DemodCentral::StreamThread()
{
    IQCapture iqc;
    IQSweep sweep;

    Reconfigure(sessionPtr->demod_settings, &iqc, sweep);

    while(streaming) {
        if(captureCount) {
            if(reconfigure) {
                Reconfigure(sessionPtr->demod_settings, &iqc, sweep);
            }
            qint64 start = bb_lib::get_ms_since_epoch();

            GetCapture(sessionPtr->demod_settings, iqc, sweep, sessionPtr->device);
            if(recordNext && sweep.triggered) {
                RecordIQCapture(sweep, iqc, sessionPtr->device);
            }

            if(demodArea->viewLock.try_lock()) {
                sweep.Demod();
                if(sweep.settings.MAEnabled()) {
                    sweep.CalculateReceiverStats();
                }
                sessionPtr->iq_capture = sweep;
                UpdateView();
                demodArea->viewLock.unlock();
            }

            // Force 30 fps update rate
            qint64 elapsed = bb_lib::get_ms_since_epoch() - start;
            if(elapsed < MAX_ZERO_SPAN_UPDATE_RATE) {
                Sleep(MAX_ZERO_SPAN_UPDATE_RATE - elapsed);
            }
            if(captureCount > 0) {
                if(sweep.triggered) {
                    captureCount--;
                }
            }
        } else {
            Sleep(MAX_ZERO_SPAN_UPDATE_RATE);
        }
    }
}

void DemodCentral::updateSettings(const DemodSettings *ds)
{
    reconfigure = true;
}

void DemodCentral::singlePressed()
{
    if(captureCount < 0) {
        captureCount = 1;
    } else {
        captureCount++;
    }
}

void DemodCentral::autoPressed()
{
    captureCount = -1;
}

void DemodCentral::recordPressed()
{
    if(captureCount == 0) {
        captureCount = 1;
    }
    recordNext = true;
}

void DemodCentral::updateRecordingDialog(bool show)
{
    if(show) {
        recordDialog.show();
    } else {
        recordDialog.hide();
    }
}

void DemodCentral::changeRecordDirectory()
{
    QString dir = bb_lib::getUserDirectory(currentRecordDir);
    if(dir.isNull()) dir = currentRecordDir;
    int w = currentRecordDirLabel->fontMetrics().width(dir);
    currentRecordDirLabel->setFixedWidth(w + 10);
    currentRecordDirLabel->setText(dir);
    currentRecordDir = dir;
}

// Clamp and update the record time/length
void DemodCentral::recordLengthChanged()
{
    double val = recordLenEntry->GetValue();
    if(val < 1.0) val = 1.0;
    if(val > 1000.0) val = 1000.0;

    recordLength = val;
    recordLenEntry->SetValue(recordLength);
}

void DemodCentral::UpdateView()
{
    emit updateViews();
}
