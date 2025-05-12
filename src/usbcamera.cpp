/*
 * usbcamera.cpp
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
#include <sched.h>
#include <time.h>
#include <QCoreApplication>
#include <stdlib.h>
#include <thread>
#include <chrono>

#include "usbcamera.h"
#include "config.h"

using namespace std;
using namespace VmbCPP;


FrameObserver::FrameObserver(CameraPtr _camera, CycDataBuffer* _cycBuf, unsigned int _chunkSize) : IFrameObserver(_camera)
{
    cycBuf = _cycBuf;
    chunkSize = _chunkSize;

    #ifdef QT_DEBUG
        frameCnt = 0;
    #endif
}


FrameObserver::~FrameObserver()
{

}


void FrameObserver::FrameReceived(const FramePtr _frame)
{
    struct timespec timestamp;
    ChunkAttrib     chunkAttrib;
    VmbUchar_t*     image;

    clock_gettime(CLOCK_REALTIME, &timestamp);
    chunkAttrib.timestamp = timestamp.tv_nsec / 1000000 + timestamp.tv_sec * 1000;
    chunkAttrib.chunkSize = chunkSize;

    if (_frame->GetImage(image) != VmbErrorSuccess)
    {
        cerr << "Could not get the image data" << endl;
        abort();
    }

    cycBuf->insertChunk(image, chunkAttrib);

    // When the frame has been processed, requeue it
    if (m_pCamera->QueueFrame(_frame) != VmbErrorSuccess)
    {
        clog << "Could not requeue frame. Possibly acquisition has been stopped." << endl;
    }

    #ifdef QT_DEBUG
        frameCnt++;
        clog << ".";
        if (frameCnt % 100 == 0)
        {
            clog << " : " << frameCnt << " frames" << endl;
        }
    #endif
}


USBCamera::USBCamera(CameraPtr _camera, CycDataBuffer* _cycBuf, bool _color)
{
    FeaturePtr      feature;
    VmbUint32_t     payloadSize;

    cycBuf = _cycBuf;
    color = _color;

    camera = _camera;

    /*-----------------------------------------------------------------------
     *  setup capture
     *-----------------------------------------------------------------------*/
    if (camera->Open(VmbAccessModeFull) != VmbErrorSuccess)
    {
        cerr << "Could not open camera" << endl;
        abort();
    }

    if ((camera->GetFeatureByName("Width", feature) != VmbErrorSuccess) ||
        (feature->SetValue(settings.width) != VmbErrorSuccess) ||
        (camera->GetFeatureByName("Height", feature) != VmbErrorSuccess) ||
        (feature->SetValue(settings.height) != VmbErrorSuccess) ||
        (camera->GetFeatureByName("OffsetX", feature) != VmbErrorSuccess) ||
        (feature->SetValue(settings.offsetx) != VmbErrorSuccess) ||
        (camera->GetFeatureByName("OffsetY", feature) != VmbErrorSuccess) ||
        (feature->SetValue(settings.offsety) != VmbErrorSuccess))
    {
        cerr << "Could not set up the ROI size/offset" << endl;
        abort();
    }

    if ((camera->GetFeatureByName("PixelFormat", feature) != VmbErrorSuccess) ||
        (feature->SetValue(color ? "RGB8" : "Mono8") != VmbErrorSuccess))
    {
        cerr << "Could not set up the video format" << endl;
        abort();
    }

    // Turn off the USB trottling
    // This is needed to get the maximum throughput from the camera
    if ((camera->GetFeatureByName("DeviceLinkThroughputLimitMode", feature) != VmbErrorSuccess) ||
    (feature->SetValue("Off") != VmbErrorSuccess))
    {
        cerr << "Could not set up device throughput limit mode" << endl;
        abort();
    }

    // Make sure the exposure can be controlled by the slider
    if ((camera->GetFeatureByName("ExposureMode", feature) != VmbErrorSuccess) ||
    (feature->SetValue("Timed") != VmbErrorSuccess))
    {
        cerr << "Could not set up device exposure mode" << endl;
        abort();
    }

    if ((camera->GetFeatureByName("ExposureAuto", feature) != VmbErrorSuccess) ||
    (feature->SetValue("Off") != VmbErrorSuccess))
    {
        cerr << "Could not turn off the autoexposure" << endl;
        abort();
    }


    if (camera->GetPayloadSize(payloadSize) != VmbErrorSuccess)
    {
        cerr << "Could not get the camera payload size" << endl;
        abort();
    }

    frameObserver = new FrameObserver(camera, cycBuf, (unsigned int)payloadSize);
}


USBCamera::~USBCamera()
// This code will be either refactored or removed. Maybe.
{   
    //camera->EndCapture();
    //camera->FlushQueue();
    //camera->RevokeAllFrames();
    camera->Close();
    // Sleep for some time to allow the last frame to be processed
    //std::this_thread::sleep_for(std::chrono::milliseconds(10000));

    //delete frameObserver;
}


void USBCamera::startAquisition()
{
    if (camera->StartContinuousImageAcquisition(1, IFrameObserverPtr(frameObserver)) != VmbErrorSuccess)
    {
        cerr << "Could not start camera acquisition" << endl;
        abort();
    }
}


void USBCamera::stopAquisition()
{
    if (camera->StopContinuousImageAcquisition() != VmbErrorSuccess)
    {
        cerr << "Could not stop camera acquisition" << endl;
        abort();
    }
}

void USBCamera::setExposureTime(float _exposureTime)
{
    FeaturePtr  feature;

    if ((camera->GetFeatureByName("ExposureTime", feature) != VmbErrorSuccess) ||
        (feature->SetValue(_exposureTime) != VmbErrorSuccess))
    {
        cerr << "Could not set exposure time" << endl;
        abort();
    }
}

void USBCamera::setGain(float _gain)
{
    FeaturePtr  feature;

    if ((camera->GetFeatureByName("Gain", feature) != VmbErrorSuccess) ||
        (feature->SetValue(_gain) != VmbErrorSuccess))
    {
        cerr << "Could not set gain" << endl;
        abort();
    }
}

void USBCamera::setBalance(float _balance, char* _color)
{
    FeaturePtr  feature;

    if ((camera->GetFeatureByName("BalanceRatioSelector", feature) != VmbErrorSuccess) ||
        (feature->SetValue(_color) != VmbErrorSuccess))
    {
        cerr << "Could not set balance ratio selector" << endl;
        abort();
    }

    if ((camera->GetFeatureByName("BalanceRatio", feature) != VmbErrorSuccess) ||
        (feature->SetValue(_balance) != VmbErrorSuccess))
    {
        cerr << "Could not set balance ratio" << endl;
        abort();
    }
}