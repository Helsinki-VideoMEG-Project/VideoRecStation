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

using namespace std;

VideoDialog::VideoDialog(VmbCPP::CameraPtr _camera, int _cameraIdx, QString _cameraSN, QWidget *parent)
    : QDialog(parent)
{
    Settings  settings;
    cameraIdx = _cameraIdx;
    cameraSN = _cameraSN;

    ui.setupUi(this);
    ui.videoWidget->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint| Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);
    setWindowTitle(QString("Camera %1").arg(cameraIdx + 1));

    // EXplicitly set gpuJpegEncoder and cameraController to nullptr
    // to make sure that setting min/max values for sliders below
    // does not affect camera settings
    gpuJpegEncoder = nullptr;
    cameraController = nullptr;

    // Setup gain/shutter sliders
    ui.shutterSlider->setMinimum(SHUTTER_SLIDER_MIN);
    ui.shutterSlider->setMaximum(SHUTTER_SLIDER_MAX);
    ui.shutterSpinBox->setMinimum(SHUTTER_SLIDER_MIN);
    ui.shutterSpinBox->setMaximum(SHUTTER_SLIDER_MAX);

    ui.gainSlider->setMinimum(GAIN_SLIDER_MIN);
    ui.gainSlider->setMaximum(GAIN_SLIDER_MAX);
    ui.gainSpinBox->setMinimum(GAIN_SLIDER_MIN);
    ui.gainSpinBox->setMaximum(GAIN_SLIDER_MAX);

    ui.balanceRedSlider->setMinimum(BALANCE_SLIDER_MIN);
    ui.balanceRedSlider->setMaximum(BALANCE_SLIDER_MAX);
    ui.balanceRedSpinBox->setMinimum(BALANCE_SLIDER_MIN);
    ui.balanceRedSpinBox->setMaximum(BALANCE_SLIDER_MAX);

    ui.balanceBlueSlider->setMinimum(BALANCE_SLIDER_MIN);
    ui.balanceBlueSlider->setMaximum(BALANCE_SLIDER_MAX);
    ui.balanceBlueSpinBox->setMinimum(BALANCE_SLIDER_MIN);
    ui.balanceBlueSpinBox->setMaximum(BALANCE_SLIDER_MAX);

    ui.jpegQualitySlider->setMinimum(JPEG_QUALITY_MIN);
    ui.jpegQualitySlider->setMaximum(JPEG_QUALITY_MAX);
    ui.jpegQualitySpinBox->setMinimum(JPEG_QUALITY_MIN);
    ui.jpegQualitySpinBox->setMaximum(JPEG_QUALITY_MAX);

    camSettings = settings.loadCameraSettings(cameraSN);

    // Set up video recording
    cycVideoBufDisp = new CycDataBuffer(CIRC_VIDEO_BUFF_SZ);
    cycVideoBufWrite = new CycDataBuffer(CIRC_VIDEO_BUFF_SZ);
    videoFileWriter = new VideoFileWriter(cycVideoBufWrite, settings.storagePath.toLocal8Bit().data(), cameraIdx + 1);
    gpuJpegEncoder = new GPUJPEGEncoder(camSettings);
    frameObserver = new FrameObserver(_camera, cycVideoBufDisp, gpuJpegEncoder, camSettings);
    cameraController = new CameraController(_camera, frameObserver, camSettings);
    videoDecompressorThread = new VideoDecompressorThread(cycVideoBufDisp, cycVideoBufWrite, camSettings);

    // Set thread names to simplify profiling
    videoFileWriter->setObjectName(QString("VidFileWrit_%1").arg(cameraIdx + 1));
    videoDecompressorThread->setObjectName(QString("VidDecompTh_%1").arg(cameraIdx + 1));

    QObject::connect(videoDecompressorThread, SIGNAL(frameDecoded(uint8_t*, int, int, bool)), ui.videoWidget, SLOT(onDrawFrame(uint8_t*, int, int, bool)));
    QObject::connect(cycVideoBufWrite, SIGNAL(chunkReady(unsigned char*)), this, SLOT(onNewFrame(unsigned char*)));

    // Make controls match the showControlsBox state
    this->onShowControlsToggled(ui.showControlsBox->isChecked());

    // Set the controls according to the values loaded from disk
    ui.shutterSlider->setValue(camSettings.shutter);
    ui.shutterSpinBox->setValue(camSettings.shutter);
    ui.gainSlider->setValue(camSettings.gain);
    ui.gainSpinBox->setValue(camSettings.gain);
    ui.balanceRedSlider->setValue(camSettings.balance_red);
    ui.balanceRedSpinBox->setValue(camSettings.balance_red);
    ui.balanceBlueSlider->setValue(camSettings.balance_blue);
    ui.balanceBlueSpinBox->setValue(camSettings.balance_blue);
    
    ui.jpegQualitySlider->setValue(camSettings.jpeg_quality);
    ui.jpegQualitySpinBox->setValue(camSettings.jpeg_quality);

    // Enable/disable color controls
    ui.balanceRedSlider->setEnabled(camSettings.color);
    ui.balanceRedSpinBox->setEnabled(camSettings.color);
    ui.balanceRedLabel->setEnabled(camSettings.color);
    ui.balanceBlueSlider->setEnabled(camSettings.color);
    ui.balanceBlueSpinBox->setEnabled(camSettings.color);
    ui.balanceBlueLabel->setEnabled(camSettings.color);
    ui.wbLabel->setEnabled(camSettings.color);

    // Start video running
    videoFileWriter->start();
    videoDecompressorThread->start();
    cameraController->startAquisition();
}


VideoDialog::~VideoDialog()
{
    Settings  settings;

    // Save camera-specific settings to disk
    settings.saveCameraSettings(cameraSN, camSettings);

    // The piece of code stopping the threads should execute fast enough,
    // otherwise cycVideoBufRaw or cycVideoBufJpeg buffer might overflow. The
    // order of stopping the threads is important.
    videoFileWriter->stop();
    videoDecompressorThread->stop();
    cameraController->stopAquisition();

    // TODO: Fix the segfault issue:
    // Deleteing the frame observer causes segfault. No idea why. Not deleting it causes
    // a memory leak, but it's better than a crash.
    //delete frameObserver;

    // Wait a bit to let the frameObserver finish processing any remaining frames
    QThread::msleep(200);
    delete gpuJpegEncoder;

    delete cycVideoBufDisp;
    delete cycVideoBufWrite;
    delete cameraController;
    delete videoFileWriter;
    delete videoDecompressorThread;
}


void VideoDialog::closeEvent(QCloseEvent *event)
{
    event->ignore(); // Ignore the close event to prevent the window from closing
}


void VideoDialog::onShutterChanged(int _newVal)
{
    float       shutterVal;

    if (cameraController != nullptr)
    {
        shutterVal = _newVal * SHUTTER_SCALE;  // msec -> usec
        cameraController->setExposureTime(shutterVal);
        camSettings.shutter = _newVal;
    }
}

void VideoDialog::onGainChanged(int _newVal)
{
    float       gainVal;

    if (cameraController != nullptr)
    {
        gainVal = _newVal * GAIN_SCALE;
        cameraController->setGain(gainVal);
        camSettings.gain = _newVal;
    }
}

void VideoDialog::onBalanceRedChanged(int _newVal)
{
    float       balanceVal;

    if (cameraController != nullptr)
    {
        balanceVal = _newVal * BALANCE_SCALE;
        cameraController->setBalance(balanceVal, (char*)"Red");
        camSettings.balance_red = _newVal;
    }
}

void VideoDialog::onBalanceBlueChanged(int _newVal)
{
    float       balanceVal;

    if (cameraController != nullptr)
    {
        balanceVal = _newVal * BALANCE_SCALE;
        cameraController->setBalance(balanceVal, (char*)"Blue");
        camSettings.balance_blue = _newVal;
    }
}

void VideoDialog::onJpegQualityChanged(int _newVal)
{
    if (gpuJpegEncoder != nullptr)
    {
        gpuJpegEncoder->setJPEGQuality(_newVal);
        camSettings.jpeg_quality = _newVal;
    }
}

void VideoDialog::onNewFrame(unsigned char* _jpegBuf)
{
    ChunkAttrib chunkAttrib;
    int         prevStatsIdx = statsIdx;

    // Update the data for computing realtime stats
    chunkAttrib = *((ChunkAttrib*)(_jpegBuf-sizeof(ChunkAttrib)));

    timeStamps[statsIdx] = chunkAttrib.timestamp;
    frameSizes[statsIdx] = chunkAttrib.chunkSize + sizeof(ChunkAttrib);
    statsIdx = (statsIdx + 1) % N_FRAMES_FOR_STATS;
    runningFrameCnt++;

    // Compute and display stats every 10 frames after we have enough frames to compute stats
    if ((runningFrameCnt >= N_FRAMES_FOR_STATS) && (runningFrameCnt % 10 == 0))
    {
        // Compute average FPS and bitrate
        uint64_t    timeDiff;
        size_t      sizeSum = 0;
        float       fps;
        float       mbps; // megabytes per second

        timeDiff = timeStamps[prevStatsIdx] - timeStamps[statsIdx]; // At this point statsIdx points to the oldest entry and prevStatsIdx to the newest

        for (int i = 0; i < N_FRAMES_FOR_STATS; i++)
        {
            if (i != statsIdx) // Skip the oldest frame
                sizeSum += frameSizes[i];
        }

        fps = double(N_FRAMES_FOR_STATS-1) / (double(timeDiff) / 1000);
        mbps = double(sizeSum) / (double(timeDiff) / 1000) / 1e6; // in MB/s 

        ui.fpsLabel->setText(QString("FPS: %1").arg(fps, 0, 'f', 2) + QString(", Data rate: %1 MB/s").arg(mbps, 0, 'f', 2));
    }
}


void VideoDialog::setIsRec(bool _isRec)
{
    cycVideoBufWrite->setIsRec(_isRec);
}


void VideoDialog::onShowControlsToggled(bool _checked)
{
    // Hide all widgets in the grid layout
    for (int i = 0; i < ui.gridLayout->count(); ++i) {
        QWidget *widget = ui.gridLayout->itemAt(i)->widget();
        if (widget) {
            if (_checked)
                widget->show();
            else
                widget->hide();
        }
    }
}
