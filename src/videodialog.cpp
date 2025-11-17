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
    cycVideoBufRaw = new CycDataBuffer(CIRC_VIDEO_BUFF_SZ);
    cycVideoBufJpeg = new CycDataBuffer(CIRC_VIDEO_BUFF_SZ);
    usbcamera = new USBCamera(_camera, cycVideoBufRaw, settings.color);
    videoFileWriter = new VideoFileWriter(cycVideoBufJpeg, settings.storagePath.toLocal8Bit().data(), cameraIdx + 1);
    videoCompressorThread = new VideoCompressorThread(cycVideoBufRaw, cycVideoBufJpeg, settings.color, settings.jpgQuality);

    // Set thread names to simplify profiling
    videoFileWriter->setObjectName(QString("VidFileWrit_%1").arg(cameraIdx + 1));
    videoCompressorThread->setObjectName(QString("VidComp_%1").arg(cameraIdx + 1));

    QObject::connect(cycVideoBufJpeg, SIGNAL(chunkReady(unsigned char*)), ui.videoWidget, SLOT(onDrawFrame(unsigned char*)));
    QObject::connect(cycVideoBufJpeg, SIGNAL(chunkReady(unsigned char*)), this, SLOT(onNewFrame(unsigned char*)));

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
    videoCompressorThread->start();
    usbcamera->startAquisition();
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
    videoCompressorThread->stop();
    usbcamera->stopAquisition();

    delete cycVideoBufRaw;
    delete cycVideoBufJpeg;
    delete usbcamera;
    delete videoFileWriter;
    delete videoCompressorThread;
}


void VideoDialog::closeEvent(QCloseEvent *event)
{
    event->ignore(); // Ignore the close event to prevent the window from closing
}


void VideoDialog::onShutterChanged(int _newVal)
{
    float       shutterVal;

    shutterVal = _newVal * SHUTTER_SCALE;  // msec -> usec
    usbcamera->setExposureTime(shutterVal);
}


void VideoDialog::onGainChanged(int _newVal)
{
    float       gainVal;

    gainVal = _newVal * GAIN_SCALE;
    usbcamera->setGain(gainVal);
}


void VideoDialog::onBalanceRedChanged(int _newVal)
{
    float       balanceVal;

    balanceVal = _newVal * BALANCE_SCALE;
    usbcamera->setBalance(balanceVal, "Red");
}


void VideoDialog::onBalanceBlueChanged(int _newVal)
{
    float       balanceVal;

    balanceVal = _newVal * BALANCE_SCALE;
    usbcamera->setBalance(balanceVal, "Blue");

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
    cycVideoBufJpeg->setIsRec(_isRec);
}

void VideoDialog::onLdsBoxToggled(bool _checked)
{
    ui.videoWidget->limitDisplaySize = _checked;
}
