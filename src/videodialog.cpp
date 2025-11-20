/*
 * videodialog.cpp
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

#include "videodialog.h"
#include "config.h"
#include "settings.h"

using namespace std;

VideoDialog::VideoDialog(VmbCPP::CameraPtr _camera, int _cameraIdx, QWidget *parent)
    : QDialog(parent)
{
    Settings    settings;
    cameraIdx = _cameraIdx;
    prevFrameTstamp = 0;
    frameCnt = 0;

    ui.setupUi(this);
    ui.videoWidget->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint| Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);
    setWindowTitle(QString("Camera %1").arg(cameraIdx + 1));

    // Set up video recording
    cycVideoBufDisp = new CycDataBuffer(CIRC_VIDEO_BUFF_SZ);
    cycVideoBufWrite = new CycDataBuffer(CIRC_VIDEO_BUFF_SZ);
    videoFileWriter = new VideoFileWriter(cycVideoBufWrite, settings.storagePath.toLocal8Bit().data(), cameraIdx + 1);
    gpuJpegEncoder = new GPUJPEGEncoder(settings.width, settings.height, settings.color, settings.jpgQuality);
    frameObserver = new FrameObserver(_camera, cycVideoBufDisp, gpuJpegEncoder, settings.width, settings.height, settings.color);
    cameraController = new CameraController(_camera, frameObserver, settings.color);
    videoDecompressorThread = new VideoDecompressorThread(cycVideoBufDisp, cycVideoBufWrite, settings.width, settings.height, settings.color);

    // Set thread names to simplify profiling
    videoFileWriter->setObjectName(QString("VidFileWrit_%1").arg(cameraIdx + 1));
    videoDecompressorThread->setObjectName(QString("VidDecompTh_%1").arg(cameraIdx + 1));

    QObject::connect(videoDecompressorThread, SIGNAL(frameDecoded(uint8_t*, int, int, bool)), ui.videoWidget, SLOT(onDrawFrame(uint8_t*, int, int, bool)));
    QObject::connect(cycVideoBufWrite, SIGNAL(chunkReady(unsigned char*)), this, SLOT(onNewFrame(unsigned char*)));

    // Setup gain/shutter sliders
    ui.shutterSlider->setMinimum(SHUTTER_SLIDER_MIN);
    ui.shutterSlider->setMaximum(SHUTTER_SLIDER_MAX);

    ui.gainSlider->setMinimum(GAIN_SLIDER_MIN);
    ui.gainSlider->setMaximum(GAIN_SLIDER_MAX);

    ui.balanceRedSlider->setMinimum(BALANCE_SLIDER_MIN);
    ui.balanceRedSlider->setMaximum(BALANCE_SLIDER_MAX);

    ui.balanceBlueSlider->setMinimum(BALANCE_SLIDER_MIN);
    ui.balanceBlueSlider->setMaximum(BALANCE_SLIDER_MAX);


    //if(settings.videoRects[idx].isValid())
    //    videoDialogs[idx]->setGeometry(settings.videoRects[idx]);
    //videoDialogs[idx]->findChild<QSlider*>("shutterSlider")->setValue(settings.videoShutters[idx]);
    //videoDialogs[idx]->findChild<QSlider*>("gainSlider")->setValue(settings.videoGains[idx]);
    //videoDialogs[idx]->findChild<QSlider*>("uvSlider")->setValue(settings.videoUVs[idx]);
    //videoDialogs[idx]->findChild<QSlider*>("vrSlider")->setValue(settings.videoVRs[idx]);
    //videoDialogs[idx]->findChild<QCheckBox*>("ldsBox")->setChecked(settings.videoLimits[idx]);

    ui.balanceRedSlider->setEnabled(settings.color);
    ui.balanceBlueSlider->setEnabled(settings.color);
    ui.balanceRedLabel->setEnabled(settings.color);
    ui.balanceBlueLabel->setEnabled(settings.color);
    ui.wbLabel->setEnabled(settings.color);

    // Start video running
    videoFileWriter->start();
    videoDecompressorThread->start();
    cameraController->startAquisition();
}


VideoDialog::~VideoDialog()
{
    //settings.videoRects[idx] = videoDialogs[idx]->geometry();
    //settings.videoShutters[idx] = videoDialogs[idx]->findChild<QSlider*>("shutterSlider")->value();
    //settings.videoGains[idx] = videoDialogs[idx]->findChild<QSlider*>("gainSlider")->value();
    //settings.videoUVs[idx] = videoDialogs[idx]->findChild<QSlider*>("uvSlider")->value();
    //settings.videoVRs[idx] = videoDialogs[idx]->findChild<QSlider*>("vrSlider")->value();
    //settings.videoLimits[idx] = videoDialogs[idx]->findChild<QCheckBox*>("ldsBox")->isChecked();

    // The piece of code stopping the threads should execute fast enough,
    // otherwise cycVideoBufRaw or cycVideoBufJpeg buffer might overflow. The
    // order of stopping the threads is important.
    videoFileWriter->stop();
    videoDecompressorThread->stop();
    cameraController->stopAquisition();

    delete cycVideoBufDisp;
    delete cycVideoBufWrite;
    delete cameraController;
    delete videoFileWriter;

    // TODO: Fix the segfault issue:
    // Deleteing the frame observer causes segfault. No idea why. Not deleting it causes
    // a memory leak, but it's better than a crash.
    //delete frameObserver;

    // Wait a bit to let the frameObserver finish processing any remaining frames
    QThread::msleep(200);
    delete gpuJpegEncoder;

}


void VideoDialog::closeEvent(QCloseEvent *event)
{
    event->ignore(); // Ignore the close event to prevent the window from closing
}


void VideoDialog::onShutterChanged(int _newVal)
{
    float       shutterVal;

    shutterVal = _newVal * SHUTTER_SCALE;  // msec -> usec
    cameraController->setExposureTime(shutterVal);
}


void VideoDialog::onGainChanged(int _newVal)
{
    float       gainVal;

    gainVal = _newVal * GAIN_SCALE;
    cameraController->setGain(gainVal);
}


void VideoDialog::onBalanceRedChanged(int _newVal)
{
    float       balanceVal;

    balanceVal = _newVal * BALANCE_SCALE;
    cameraController->setBalance(balanceVal, (char*)"Red");
}


void VideoDialog::onBalanceBlueChanged(int _newVal)
{
    float       balanceVal;

    balanceVal = _newVal * BALANCE_SCALE;
    cameraController->setBalance(balanceVal, (char*)"Blue");

}


void VideoDialog::onNewFrame(unsigned char* _jpegBuf)
{
    ChunkAttrib chunkAttrib;
    float       fps;

    chunkAttrib = *((ChunkAttrib*)(_jpegBuf-sizeof(ChunkAttrib)));

    if (prevFrameTstamp)
    {
        if(frameCnt == 10)
        {
            fps = 1 / (float(chunkAttrib.timestamp - prevFrameTstamp) / 1000);
            ui.fpsLabel->setText(QString("FPS: %1").arg(fps, 0, 'f', 2));
            frameCnt = 0;
        }
    }

    prevFrameTstamp = chunkAttrib.timestamp;
    frameCnt++;
}


void VideoDialog::setIsRec(bool _isRec)
{
    cycVideoBufWrite->setIsRec(_isRec);
}

void VideoDialog::onLdsBoxToggled(bool _checked)
{
    ui.videoWidget->limitDisplaySize = _checked;
}
