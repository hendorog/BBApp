#include "phase_noise_central.h"
#include "model/session.h"

PhaseNoiseCentral::PhaseNoiseCentral(Session *sPtr,
                                     QToolBar *mainToolBar,
                                     QWidget *parent,
                                     Qt::WindowFlags f) :
    CentralWidget(parent, f),
    session_ptr(sPtr)
{
    sweeping = false;
    reconfigure = true;

    startDecade = tempStartDecade = 2;
    stopDecade = tempStopDecade = 4;

    plot = new PhaseNoisePlot(session_ptr, this);
    plot->move(0, 0);
    connect(this, SIGNAL(updateView()), plot, SLOT(update()));

    tools.push_back(mainToolBar->addWidget(new FixedSpacer(QSize(10, 30))));

    Label *sweepLabel = new Label("Sweep");
    sweepLabel->resize(100, 25);
    tools.push_back(mainToolBar->addWidget(sweepLabel));

    tools.push_back(mainToolBar->addWidget(new FixedSpacer(QSize(10, 30))));

    startDecadeEntry = new ComboBox();
    startDecadeEntry->setFixedSize(150, 25);
    QStringList startList;
    startList << "10Hz" << "100Hz" << "1kHz" << "10kHz";
    startDecadeEntry->insertItems(0, startList);
    tools.push_back(mainToolBar->addWidget(startDecadeEntry));

    tools.push_back(mainToolBar->addWidget(new FixedSpacer(QSize(10, 30))));

    Label *startDecadeLabel = new Label("to");
    startDecadeLabel->resize(40, 25);
    tools.push_back(mainToolBar->addWidget(startDecadeLabel));

    tools.push_back(mainToolBar->addWidget(new FixedSpacer(QSize(10, 30))));

    stopDecadeEntry = new ComboBox();
    stopDecadeEntry->setFixedSize(150, 25);
    QStringList stopList;
    stopList << "1kHz" << "10kHz" << "100kHz" << "1MHz";
    stopDecadeEntry->insertItems(0, stopList);
    tools.push_back(mainToolBar->addWidget(stopDecadeEntry));

    connect(session_ptr->sweep_settings, SIGNAL(updated(const SweepSettings*)),
            this, SLOT(settingsChanged(const SweepSettings*)));

    startDecadeEntry->setCurrentIndex(startDecade);
    stopDecadeEntry->setCurrentIndex(stopDecade - 2);

    connect(startDecadeEntry, SIGNAL(activated(int)),
            this, SLOT(startDecadeChanged(int)));
    connect(stopDecadeEntry, SIGNAL(activated(int)),
            this, SLOT(stopDecadeChanged(int)));
}

PhaseNoiseCentral::~PhaseNoiseCentral()
{
    StopStreaming();
}

void PhaseNoiseCentral::GetViewImage(QImage &image)
{
    image = plot->grabFrameBuffer();
}

void PhaseNoiseCentral::StartStreaming()
{
    sweeping = true;
    sweepThreadHandle = std::thread(&PhaseNoiseCentral::SweepThread, this);
}

void PhaseNoiseCentral::StopStreaming()
{
    if(sweepThreadHandle.joinable()) {
        sweeping = false;
        sweepThreadHandle.join();
    }
}

void PhaseNoiseCentral::ResetView()
{

}

Frequency PhaseNoiseCentral::GetCurrentCenterFreq() const
{
    return session_ptr->sweep_settings->Center();
}

void PhaseNoiseCentral::resizeEvent(QResizeEvent *)
{
    plot->resize(width(), height());
}

void PhaseNoiseCentral::changeMode(int newState)
{
    StopStreaming();

    session_ptr->sweep_settings->setMode((OperationalMode)newState);

    if(newState == MODE_PHASE_NOISE) {
        StartStreaming();
    }
}

void PhaseNoiseCentral::Reconfigure(SweepSettings &ss)
{
    startDecade = tempStartDecade;
    stopDecade = tempStopDecade;
    plot->SetDecades(startDecade, stopDecade);
    ss = *session_ptr->sweep_settings;
    reconfigure = false;
}

void PhaseNoiseCentral::SweepThread()
{
    Trace f; // Full sweep, combined partial sweeps
    Trace p; // Partial 1 decade sweep
    SweepSettings ss; // Full sweep settings
    SweepSettings ps; // Partial sweep settings
    Frequency center;

    double pTempMin[1000], pTempMax[1000];
    double pDataMin[1000], pDataMax[1000];

    while(sweeping) {
        if(reconfigure) {
            Reconfigure(ss);
            //center = hss.Center();
        }

        plot->trace.SetSize(100 * (stopDecade - startDecade));

        // Loop to generate one phase noise sweep
        for(int decade = startDecade + 1; decade < stopDecade + 1; decade++) {

            switch(decade) {
            case 0:
                // No longer doing 1Hz to 10Hz
                Q_ASSERT(false);
                break;
            case 1: // 10 Hz - 100 Hz, 1.6 Hz RBW, .46 Hz per bin
                ps.setRBW(1.6);
                ps.setVBW(1.6);
                ps.setStart(ss.Center() + 10.0);
                ps.setStop(ss.Center() + 100.0);
                break;
            case 2: // 100 Hz - 1 KHz, 12.5 Hz RBW 3.7 Hz per bin
                ps.setRBW(12.5);
                ps.setVBW(12.5);
                ps.setStart(ss.Center() + 100.0);
                ps.setStop(ss.Center() + 1000.0);
                break;
            case 3: // 1 KHz - 10 KHz, 100 Hz RBW
                ps.setStart(ss.Center() + 900.0);
                ps.setStop(ss.Center() + 12.0e3);
                ps.setRBW(100.0);
                ps.setVBW(100.0);
                break;
            case 4: // 10 KHz - 100 KHz, 1.6 KHz RBW
                ps.setStart(ss.Center() + 8.0e3);
                ps.setStop(ss.Center() + 150.0e3);
                ps.setRBW(1.6e3);
                ps.setVBW(1.6e3);
                break;
            default: // 100 KHz - 10 MHz, 12.5 KHz RBW
                ps.setRBW(12.5e3);
                ps.setVBW(12.5e3);
                ps.setStart(ss.Center() + 100000.0);
                ps.setStop(ss.Center() + 10000000.0);
                break;
            }

            session_ptr->device->Reconfigure(&ps, &p);

            if(decade < 5) {
                session_ptr->device->GetSweep(&ps, &p);
            } else {
                // Sweep 100k - 1M with slow sweep

            }

            // Copy sweep to temp arrays
            for(int i = 0; i < p.Length(); i++) {
                pTempMin[i] = p.Min()[i];
                pTempMax[i] = p.Max()[i];
            }

            double HzPerBin = p.BinSize();
            double HzRBW = 10.0 * log10(ps.RBW());
            int ptsPerDecade = 100;
            //double recipPtsPerDecade = 1.0 / ptsPerDecade;

            double startFreq = ss.Center() + pow(10.0, decade);
            double stopFreq = ss.Center() + pow(10.0, decade + 1);
            double span = stopFreq - startFreq;

            for(int i = 0; i < ptsPerDecade; i++) {
                //int centerBin = p.Length() / 2; // 55 for 200 KHz span

//                double startFracFreq = startFreq + log10((9.0/100.0)*i + 1) * span;
//                double stopFracFreq = startFreq + log10((9.0/100.0)*(i+1) + 1) * span;
                //double startFrac = (pow(10.0, double(i)/100.0) - 1.0) / 9.0;
                double startFrac = pow(10.0, decade + double(i) / 101.0);
                //double stopFrac = (pow(10.0, double(i+1)/100.0) - 1.0) / 9.0;
                double stopFrac = pow(10.0, decade + double(i+1) / 101.0);
                double startFracFreq = startFreq + startFrac * span;
                startFracFreq = ss.Center() + startFrac;
                double stopFracFreq = startFreq + stopFrac * span;
                stopFracFreq = ss.Center() + stopFrac;

                int startBin = floor((startFracFreq - p.StartFreq()) / HzPerBin + 0.5);
                int stopBin = floor((stopFracFreq - p.StartFreq()) / HzPerBin + 0.5);

//                double mySpan = pow(10.0, decade + 1);
//                double freqOfsStart = pow(10.0, decade + i * recipPtsPerDecade) - mySpan * 0.5;
//                double freqOfsStop  = pow(10.0, decade + i * recipPtsPerDecade + recipPtsPerDecade) - mySpan * 0.5;

//                int startBin = floor(0.5 + freqOfsStart / HzPerBin) + centerBin;
//                int stopBin = floor(0.5 + freqOfsStop / HzPerBin) + centerBin;

                double thisMin = 0.0;
                double thisMax = 0.0;

                // Now average all valid bins together to get phase noise
                for(int j = startBin; j <= stopBin; j++) {
                    thisMin += p.Min()[j]; //pTempMin[j];
                    thisMax += p.Max()[j]; //pTempMax[j];
                }

                // Convert to dBm/Hz
                thisMin = (thisMin / (stopBin-startBin+1)) - HzRBW - ps.RefLevel().Val();
                thisMax = (thisMax / (stopBin-startBin+1)) - HzRBW - ps.RefLevel().Val();

                int thisIndex = i;// + (decade - startDecade) * ptsPerDecade;
                pDataMin[thisIndex] = thisMin;
                pDataMax[thisIndex] = thisMax;

                // Make sure we fill in any gaps. Overkill probably. Gets written over if not needed
                if(thisIndex < (ptsPerDecade/2)) {
                    pDataMin[thisIndex+1] = thisMin;
                    pDataMin[thisIndex+2] = thisMin;
                    pDataMax[thisIndex+1] = thisMax;
                    pDataMax[thisIndex+2] = thisMax;
                }

                // Make sure we fill in any gaps.  Overkill probably
                if(thisIndex < (ptsPerDecade/4)) {
                    pDataMin[thisIndex+3] = thisMin;
                    pDataMin[thisIndex+4] = thisMin;
                    pDataMin[thisIndex+5] = thisMin;
                    pDataMax[thisIndex+3] = thisMax;
                    pDataMax[thisIndex+4] = thisMax;
                    pDataMax[thisIndex+5] = thisMax;
                }
            }

            // Copy decade sweep into trace class
            int decadeStart = (decade - 1 - startDecade) * 100;
            for(int i = 0; i < ptsPerDecade; i++) {
                plot->trace.Min()[decadeStart + i] = pDataMin[i];
                plot->trace.Max()[decadeStart + i] = pDataMax[i];
            }


        }

        emit updateView();
    }
}

void PhaseNoiseCentral::settingsChanged(const SweepSettings *ss)
{
    reconfigure = true;
}

void PhaseNoiseCentral::startDecadeChanged(int newStart)
{
    if(newStart >= stopDecade) {
        newStart = stopDecade - 1;
        startDecadeEntry->setCurrentIndex(newStart);
    }

    //startDecade = newStart;
    tempStartDecade = newStart;
    reconfigure = true;
}

void PhaseNoiseCentral::stopDecadeChanged(int newStop)
{
    newStop += 2;
    if(newStop <= startDecade) {
        newStop = startDecade + 1;
        stopDecadeEntry->setCurrentIndex(newStop - 2);
    }

    //stopDecade = newStop;
    tempStopDecade = newStop;
    reconfigure = true;
}
