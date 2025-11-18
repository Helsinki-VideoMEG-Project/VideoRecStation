/*
 * maindialog.cpp
 *
 * Author: Andrey Zhdanov
 * Copyright (C) 2014 BioMag Laboratory, Helsinki University Central Hospital
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <iostream>
#include <sys/statvfs.h>
#include <math.h>

#include <QStorageInfo>

#include "config.h"
#include "maindialog.h"

using namespace std;
using namespace VmbCPP;

MainDialog::MainDialog(QWidget *parent)
    : QMainWindow(parent)
{
    Qt::WindowFlags flags = Qt::WindowTitleHint;

    if (settings.controlOnTop)
    {
        flags = flags | Qt::WindowStaysOnTopHint;
    }
    setWindowFlags(flags);

    ui.setupUi(this);

    // Set up status bar
    ui.statusBar->setSizeGripEnabled(false);
    statusLeft = new QLabel("", this);
    statusRight = new QLabel("", this);
    ui.statusBar->addPermanentWidget(statusLeft, 1);
    ui.statusBar->addPermanentWidget(statusRight, 0);
    updateDiskSpace();
    updateTimer = new QTimer(this);
    updateElapsed = new QElapsedTimer();
    updateTimer->setInterval(500);  // every half second
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(updateRunningStatus()));
    ui.clipLabel->setStyleSheet("QLabel { background-color : red; color : black; }");
    ui.clipLabel->setVisible(false);

    // Set up video recording
    initVideo();

    // Set up audio recording
    cycAudioBufRaw = new CycDataBuffer(CIRC_AUDIO_BUFF_SZ);
    cycAudioBufCompressed = new CycDataBuffer(CIRC_AUDIO_BUFF_SZ);
    audioCompressorThread = new AudioCompressorThread(cycAudioBufRaw, cycAudioBufCompressed, N_CHANS, N_CHANS_2_RECORD);
    microphoneThread = new MicrophoneThread(cycAudioBufRaw);
    audioFileWriter = new AudioFileWriter(cycAudioBufCompressed, settings.storagePath.toLocal8Bit().data());
    QObject::connect(cycAudioBufRaw, SIGNAL(chunkReady(unsigned char*)), this, SLOT(onAudioUpdate(unsigned char*)));

    // Set thread names to simplify profiling
    audioCompressorThread->setObjectName("AudComp");
    microphoneThread->setObjectName("Mic");
    audioFileWriter->setObjectName("AudFileWrit");

    // Initialize volume indicator history
    memset(volMaxvals, 0, N_CHANS * N_BUF_4_VOL_IND * sizeof(AUDIO_DATA_TYPE));
    volIndNext = 0;

    // Initialize speaker
    if(settings.useFeedback)
    {
        speakerBuffer = new NonBlockingBuffer(settings.spkBufSz, settings.framesPerPeriod*N_CHANS*sizeof(AUDIO_DATA_TYPE));
        speakerThread = new SpeakerThread(speakerBuffer);

        // Set thread name to simplify profiling
        speakerThread->setObjectName("Speaker");
    }
    else
    {
        speakerBuffer = NULL;
        speakerThread = NULL;
    }

    if (settings.metersUseDB)
    {
        ui.level1->setRange(-60, 0);
        ui.level2->setRange(-60, 0);
        ui.level3->setRange(-60, 0);
        ui.level4->setRange(-60, 0);
        ui.level5->setRange(-60, 0);
        ui.level6->setRange(-60, 0);
        ui.level7->setRange(-60, 0);
        ui.level8->setRange(-60, 0);
        ui.levelBottomLabel->setText("-60 dB");
        ui.levelTopLabel->setText("0 dB");
    }
    else
    {
        ui.level1->setMaximum(MAX_AUDIO_VAL);
        ui.level2->setMaximum(MAX_AUDIO_VAL);
        ui.level3->setMaximum(MAX_AUDIO_VAL);
        ui.level4->setMaximum(MAX_AUDIO_VAL);
        ui.level5->setMaximum(MAX_AUDIO_VAL);
        ui.level6->setMaximum(MAX_AUDIO_VAL);
        ui.level7->setMaximum(MAX_AUDIO_VAL);
        ui.level8->setMaximum(MAX_AUDIO_VAL);
    }

    // Start audio running
    audioFileWriter->start();
    audioCompressorThread->start();
    microphoneThread->start();

    // Start speaker thread
    if(speakerThread)
    {
        speakerThread->start();
    }
    if (settings.controllerRect.isValid())
        this->setGeometry(settings.controllerRect);
}


void MainDialog::closeEvent(QCloseEvent *event)
{
    onExit();
}


double MainDialog::freeSpaceGB()
{
    QStorageInfo storageInfo(settings.storagePath);
    return double(storageInfo.bytesAvailable()) / 1073741824.0;
}


void MainDialog::updateRunningStatus()
{
    int secs = updateElapsed->elapsed() / 1000;
    int mins = (secs / 60) % 60;
    int hours = secs / 3600;
    secs %= 60;
    if (hours > 0)
        statusLeft->setText(QString("Recording (%1:%2:%3)").arg(hours, 2, 10, QLatin1Char('0')).arg(mins, 2, 10, QLatin1Char('0')).arg(secs, 2, 10, QLatin1Char('0')));
    else
        statusLeft->setText(QString("Recording (%1:%2)").arg(mins, 2, 10, QLatin1Char('0')).arg(secs, 2, 10, QLatin1Char('0')));
    updateDiskSpace();
}

void MainDialog::updateDiskSpace()
{
    statusRight->setText(QString("%1 GB free").arg(freeSpaceGB(), 0, 'f', 1));
}


MainDialog::~MainDialog()
{
    audioFileWriter->stop();
    audioCompressorThread->stop();
    microphoneThread->stop();
    if (speakerThread) {
        speakerThread->stop();
    }

    delete cycAudioBufRaw;
    delete cycAudioBufCompressed;
    delete audioCompressorThread;
    delete audioFileWriter;
    delete microphoneThread;
    if (speakerThread) {
        delete speakerThread;
        delete speakerBuffer;
    }

    VmbSystem& system = VmbSystem::GetInstance();
    system.Shutdown();

    // TODO: Implement proper destructor
    delete statusLeft;
    delete statusRight;
    delete updateTimer;
    delete updateElapsed;
}


void MainDialog::onStartRec()
{
    double freeSpace = freeSpaceGB();
    if (freeSpace < settings.lowDiskSpaceWarning)
    {
        if (QMessageBox::warning(this, "Low disk space",
                                 QString("Disk space is low (%1 GB). Do you really want to start recording?").arg(freeSpace, 0, 'f', 1),
                                 QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok)
            != QMessageBox::Ok)
        {
            return;
        }
    }
    ui.stopButton->setEnabled(true);
    ui.startButton->setEnabled(false);
    ui.exitButton->setEnabled(false);

    for (unsigned int i=0; i<numCameras; i++)
    {
        camCheckBoxes[i]->setEnabled(false);
        if (camCheckBoxes[i]->isChecked())
        {
            videoDialogs[i]->setIsRec(true);
        }
    }
    cycAudioBufCompressed->setIsRec(true);
    updateElapsed->start();
    updateTimer->start();
}


void MainDialog::onStopRec()
{
    if (settings.confirmStop &&
        QMessageBox::warning(this, "Confirm recording end",
                             "Are you sure you want to stop recording?",
                             QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel) != QMessageBox::Ok)
        return;
    updateTimer->stop();
    ui.stopButton->setEnabled(false);
    ui.startButton->setEnabled(true);
    ui.exitButton->setEnabled(true);

    for (unsigned int i=0; i<numCameras; i++)
    {
        camCheckBoxes[i]->setEnabled(cameras[i] != NULL);
        if(camCheckBoxes[i]->checkState() == Qt::Checked)
        {
            videoDialogs[i]->setIsRec(false);
        }
    }

    cycAudioBufCompressed->setIsRec(false);
    QString fileName = QString(audioFileWriter->readableFileName);
    fileName.chop(13);
    statusLeft->setText(QString("Saved %1...").arg(fileName));
}


void MainDialog::onExit()
{
    for (unsigned int i=0; i<numCameras; i++)
        if(camCheckBoxes[i]->isChecked())
            delete videoDialogs[i];
    settings.controllerRect = this->geometry();

    close();
}


void MainDialog::onAudioUpdate(unsigned char* _data)
{
    unsigned int    i=0;
    unsigned int    j;
    AUDIO_DATA_TYPE maxvals[N_CHANS]={0};
    AUDIO_DATA_TYPE curval;

    // Update the history
    memset(&(volMaxvals[volIndNext]), 0, N_CHANS * sizeof(AUDIO_DATA_TYPE));
    while(i < settings.framesPerPeriod * N_CHANS)
    {
        for(j=0; j<N_CHANS; j++)
        {
            curval = abs(((AUDIO_DATA_TYPE*)_data)[i++]);
            volMaxvals[volIndNext + j] = (volMaxvals[volIndNext + j] >= curval) ? volMaxvals[volIndNext + j] : curval;
        }
    }

    volIndNext += N_CHANS;
    volIndNext %= (N_CHANS * N_BUF_4_VOL_IND);

    // Compute maxima for all channels
    i = 0;
    while(i < N_CHANS * N_BUF_4_VOL_IND)
    {
        for(j=0; j<N_CHANS; j++)
        {
            curval = volMaxvals[i++];
            maxvals[j] = (maxvals[j] >= curval) ? maxvals[j] : curval;
        }
    }

    // Update only eight level bars
    if (settings.metersUseDB)
    {
        ui.level1->setValue(20 * log10((double)maxvals[0] / MAX_AUDIO_VAL));
        ui.level2->setValue(20 * log10((double)maxvals[1] / MAX_AUDIO_VAL));
        ui.level3->setValue(20 * log10((double)maxvals[2] / MAX_AUDIO_VAL));
        ui.level4->setValue(20 * log10((double)maxvals[3] / MAX_AUDIO_VAL));
        ui.level5->setValue(20 * log10((double)maxvals[4] / MAX_AUDIO_VAL));
        ui.level6->setValue(20 * log10((double)maxvals[5] / MAX_AUDIO_VAL));
        ui.level7->setValue(20 * log10((double)maxvals[6] / MAX_AUDIO_VAL));
        ui.level8->setValue(20 * log10((double)maxvals[7] / MAX_AUDIO_VAL));
    }
    else
    {
        ui.level1->setValue(maxvals[0]);
        ui.level2->setValue(maxvals[1]);
        ui.level3->setValue(maxvals[2]);
        ui.level4->setValue(maxvals[3]);
        ui.level5->setValue(maxvals[4]);
        ui.level6->setValue(maxvals[5]);
        ui.level7->setValue(maxvals[6]);
        ui.level8->setValue(maxvals[7]);
    }
    if (maxvals[0] >= MAX_AUDIO_VAL || maxvals[1] >= MAX_AUDIO_VAL)
        ui.clipLabel->setVisible(true);
    else
        ui.clipLabel->setVisible(false);

    // Feed to the speaker
    if(speakerBuffer)
    {
        speakerBuffer->insertChunk(_data);
    }
}


void MainDialog::initVideo()
{
    CameraPtrVector camerasVec;
    VmbSystem& system = VmbSystem::GetInstance();

    if(system.Startup() != VmbErrorSuccess)
    {
        cerr << "Could not start Vimba system" << endl;
        abort();
    }

    if (system.GetCameras(camerasVec) != VmbErrorSuccess)
    {
        cerr << "Could not get cameras" << endl;
        abort();
    }

    numCameras = MAX_CAMERAS < camerasVec.size() ? MAX_CAMERAS : camerasVec.size();

    for (unsigned int i=0; i < numCameras; i++)
    {
        std::string cameraModel, cameraSN;
        cameras[i] = camerasVec.at(i);
        QString name;
               
        if ((cameras[i]->GetModel(cameraModel) != VmbErrorSuccess) || (cameras[i]->GetSerialNumber(cameraSN) != VmbErrorSuccess))
        {
            clog << "Could not get camera model or serial number. Proceeding nonetheless." << endl;
            cameraModel = "Unknown";
            cameraSN = "Unknown";
        }

        clog << "Using camera [" << cameraModel << "] with serial number [" << cameraSN << "]" << std::endl;

        // Construct and populate camera check boxes
        name = QString::fromStdString(cameraModel + " [S/N: " + cameraSN + "]");
        if (name.length() > 40)
        {
            name.truncate(37);
            name.append("...");
        }
        camCheckBoxes[i] = new QCheckBox(name, this);
        ui.videoVerticalLayout->addWidget(camCheckBoxes[i]);
        camCheckBoxes[i]->setEnabled(true);
        connect(camCheckBoxes[i], SIGNAL(toggled(bool)), this, SLOT(onCamToggled(bool)));
    }

    statusLeft->setText(QString("Found %1 camera%2").arg(numCameras).arg(numCameras != 1 ? "s" : ""));
    vertSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui.videoVerticalLayout->addItem(vertSpacer);
}


void MainDialog::onCamToggled(bool _state)
{
    int idx = -1;
    for (unsigned int i=0; i < numCameras; i++)
    {
        if (sender() == camCheckBoxes[i])
        {
            idx = i;
            break;
        }
    }
    if (idx < 0)
    {
        cerr << "Could not ID camera" << endl;
        abort();
    }

    if(_state)
    {
        videoDialogs[idx] = new VideoDialog(cameras[idx], idx);
        videoDialogs[idx]->show();
    }
    else
    {
        delete videoDialogs[idx];
    }
}
