/*
 * maindialog.h
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


#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QMainWindow>
#include <QMessageBox>
#include <QTimer>
#include <QElapsedTimer>
#include <QCloseEvent>

#include "config.h"
#include "ui_maindialog.h"
#include "cameracontroller.h"
#include "microphonethread.h"
#include "cycdatabuffer.h"
#include "videofilewriter.h"
#include "audiofilewriter.h"
#include "speakerthread.h"
#include "audiocompressorthread.h"
#include "videodialog.h"
#include "settings.h"


class MainDialog : public QMainWindow
{
    Q_OBJECT

public:
    MainDialog(QWidget *parent = 0);
    ~MainDialog();
    Settings            settings;

public slots:
    void onStartRec();
    void onStopRec();
    void onExit();
    void onFramelockToggled(bool enabled);
    void onAudioUpdate(unsigned char* _data);
    void onCamToggled(bool _state);
    void updateDiskSpace();
    void updateRunningStatus();
    double freeSpaceGB();
    void closeEvent(QCloseEvent* _event);

private:
    void initVideo();
    Ui::MainDialogClass ui;

    // TODO: Replace arrays with std::vectors
    VmbCPP::CameraPtr   cameras[MAX_CAMERAS];
    VideoDialog*        videoDialogs[MAX_CAMERAS];
    QCheckBox*          camCheckBoxes[MAX_CAMERAS];
    QString             cameraSNs[MAX_CAMERAS];
    unsigned int        numCameras;
    QSpacerItem*        vertSpacer;

    MicrophoneThread*       microphoneThread;
    CycDataBuffer*          cycAudioBufRaw;
    CycDataBuffer*          cycAudioBufCompressed;
    AudioFileWriter*        audioFileWriter;
    AudioCompressorThread*  audioCompressorThread;

    QLabel*         statusLeft;
    QLabel*         statusRight;
    QTimer*         updateTimer;
    QElapsedTimer*  updateElapsed;

    // Data structures for volume indicator. volMaxvals is a cyclic buffer
    // that stores maximal values for the last N_BUF_4_VOL_IND periods for
    // all channels in an interleaved fashion.
    AUDIO_DATA_TYPE     volMaxvals[N_CHANS * N_BUF_4_VOL_IND];
    int                 volIndNext;
    SpeakerThread*      speakerThread;
    NonBlockingBuffer*  speakerBuffer;
};

#endif // MAINDIALOG_H
