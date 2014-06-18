#ifndef DEMOD_CENTRAL_H
#define DEMOD_CENTRAL_H

#include <QToolBar>
#include <QBoxLayout>
#include <QMdiArea>

#include "lib/bb_lib.h"
#include "model/session.h"
#include "views/central_stack.h"

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
    Session *sessionPtr; // Copy, does not own

    QToolBar *toolBar;
    QMdiArea *demodArea;

public slots:
    void changeMode(int newState);

private slots:

signals:

private:
    DISALLOW_COPY_AND_ASSIGN(DemodCentral)
};

#endif // DEMOD_CENTRAL_H
