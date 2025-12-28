/*
 * videodialog.h
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


#ifndef VIDEODIALOG_H
#define VIDEODIALOG_H

#include <QDialog>
#include <QCloseEvent>

#include "ui_videodialog.h"
#include "cameracontroller.h"
#include "frameobserver.h"
#include "cycdatabuffer.h"
#include "videofilewriter.h"
#include "gpujpegencoder.h"
#include "videodecompressorthread.h"
#include "settings.h"
#include "config.h"


class VideoDialog : public QDialog
{
    Q_OBJECT

public:
    VideoDialog(VmbCPP::CameraPtr _camera, int _cameraId, QString _cameraSN, QWidget *parent = 0);
    virtual ~VideoDialog();
    void setIsRec(bool _isRec);

public slots:
    void onShutterChanged(int _newVal);
    void onGainChanged(int _newVal);
    void onBalanceRedChanged(int _newVal);
    void onBalanceBlueChanged(int _newVal);
    void onJpegQualityChanged(int _newVal);
    void onNewFrame(unsigned char* _jpegBuf);
    void onShowControlsToggled(bool _checked);
    void closeEvent(QCloseEvent* _event);

private:
    Ui::VideoDialogClass ui;

    unsigned int                cameraIdx;
    QString                     cameraSN;
    CameraSettings              camSettings;
    CameraController*           cameraController;
    FrameObserver*              frameObserver;
    CycDataBuffer*              cycVideoBufDisp;
    CycDataBuffer*              cycVideoBufWrite;
    VideoFileWriter*            videoFileWriter;
    GPUJPEGEncoder*             gpuJpegEncoder;
    VideoDecompressorThread*    videoDecompressorThread;

    // These variables are used for computing realtime stats
    u_int64_t   timeStamps[N_FRAMES_FOR_STATS] = {0};
    size_t      frameSizes[N_FRAMES_FOR_STATS] = {0};
    uint64_t    runningFrameCnt = 0;
    int         statsIdx = 0;
};

#endif // VIDEODIALOG_H
