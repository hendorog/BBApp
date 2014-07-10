#ifndef DEMOD_CENTRAL_H
#define DEMOD_CENTRAL_H

#include <QToolBar>
#include <QBoxLayout>
#include <QMdiArea>
#include <QMdiSubWindow>

#include "lib/bb_lib.h"
#include "model/session.h"

#include "central_stack.h"

class MdiArea : public QMdiArea {
    Q_OBJECT

public:
    MdiArea(QWidget *parent = 0) : QMdiArea(parent) {}
    ~MdiArea() {}

    void retile() {
        auto list = subWindowList();

        int viewHeight = height() / list.size();
        int viewCount = 0;

        for(auto view : list) {
            view->move(0, viewCount * viewHeight);
            view->resize(width(), viewHeight);
            viewCount++;
        }
    }

private:
    DISALLOW_COPY_AND_ASSIGN(MdiArea)
};

class DemodCentral : public CentralWidget {
    Q_OBJECT

    static const int TOOLBAR_HEIGHT = 30;

public:
    DemodCentral(Session *sPtr, QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~DemodCentral();

    void StartStreaming();
    void StopStreaming();
    void ResetView();
    void GetViewImage(QImage &image);
    Frequency GetCurrentCenterFreq() const {
        return 1.0e6;
    }

protected:
    void resizeEvent(QResizeEvent *);

private:
    void Reconfigure(DemodSettings *ds, IQCapture *iqc);
    void GetCapture(const DemodSettings *ds, IQCapture &iqc,
                    IQSweep &iq, Device *device);
    void StreamThread();
    void UpdateView();

    Session *sessionPtr; // Copy, does not own

    QToolBar *toolBar;
    MdiArea *demodArea;

    DemodSettings lastConfig;
    std::atomic<int> captureCount;
    std::thread threadHandle;
    bool streaming;
    bool reconfigure;

public slots:
    void changeMode(int newState);
    void updateSettings(const DemodSettings *ds);

private slots:
    void singlePressed();
    void autoPressed();

signals:
    void updateView();
    void presetDevice();

private:
    DISALLOW_COPY_AND_ASSIGN(DemodCentral)
};

#endif // DEMOD_CENTRAL_H
