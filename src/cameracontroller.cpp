/*
 * cameracontroller.cpp
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

#include "cameracontroller.h"
#include "config.h"
#include "settings.h"

using namespace std;
using namespace VmbCPP;


CameraController::CameraController(CameraPtr _camera, FrameObserver* _frameObserver, CameraSettings _camSettings)
{
    FeaturePtr      feature;
    VmbUint32_t     payloadSize;
    MiscSettings    miscSettings = Settings::getInstance().getMiscSettings();

    frameObserver = _frameObserver;

    camera = _camera;

    /*-----------------------------------------------------------------------
     *  setup capture
     *-----------------------------------------------------------------------*/
    if (camera->Open(VmbAccessModeFull) != VmbErrorSuccess)
    {
        cerr << "Could not open camera" << endl;
        abort();
    }

    // Reset the ROI offset to 0/0 first, otherwise setting the width/height might fail
    if ((camera->GetFeatureByName("OffsetX", feature) != VmbErrorSuccess) ||
        (feature->SetValue(0) != VmbErrorSuccess) ||
        (camera->GetFeatureByName("OffsetY", feature) != VmbErrorSuccess) ||
        (feature->SetValue(0) != VmbErrorSuccess))
    {
        cerr << "Could not reset ROI offset to 0/0" << endl;
        abort();
    }

    if ((camera->GetFeatureByName("Width", feature) != VmbErrorSuccess) ||
        (feature->SetValue(_camSettings.width) != VmbErrorSuccess) ||
        (camera->GetFeatureByName("Height", feature) != VmbErrorSuccess) ||
        (feature->SetValue(_camSettings.height) != VmbErrorSuccess) ||
        (camera->GetFeatureByName("OffsetX", feature) != VmbErrorSuccess) ||
        (feature->SetValue(_camSettings.offsetX) != VmbErrorSuccess) ||
        (camera->GetFeatureByName("OffsetY", feature) != VmbErrorSuccess) ||
        (feature->SetValue(_camSettings.offsetY) != VmbErrorSuccess))
    {
        cerr << "Could not set up the ROI size/offset" << endl;
        abort();
    }

    if ((camera->GetFeatureByName("PixelFormat", feature) != VmbErrorSuccess) ||
        (feature->SetValue(_camSettings.color ? "RGB8" : "Mono8") != VmbErrorSuccess))
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
    if (payloadSize != _camSettings.width * _camSettings.height * (_camSettings.color ? 3 : 1))
    {
        cerr << "Error: the actual payload size " << payloadSize << " does not match the expected size "
             << (_camSettings.width * _camSettings.height * (_camSettings.color ? 3 : 1)) << endl;
        abort();
    }

    if (_camSettings.useTrigger)
    {
        // Set the camera to external trigger mode
        if ((camera->GetFeatureByName("TriggerMode", feature) != VmbErrorSuccess) ||
            (feature->SetValue("On") != VmbErrorSuccess) ||
            (camera->GetFeatureByName("TriggerSource", feature) != VmbErrorSuccess) ||
        (feature->SetValue(miscSettings.externalTriggerSource.toUtf8().constData()) != VmbErrorSuccess) ||
        (camera->GetFeatureByName("TriggerActivation", feature) != VmbErrorSuccess) ||
        (feature->SetValue("RisingEdge") != VmbErrorSuccess))
    {
        cerr << "Could not set up external trigger mode" << endl;
            abort();
        }
    }
    else
    {
        // Set the camera to free run mode
        if ((camera->GetFeatureByName("TriggerMode", feature) != VmbErrorSuccess) ||
            (feature->SetValue("Off") != VmbErrorSuccess))
        {
            cerr << "Could not set up free run mode" << endl;
            abort();
        }
    }
}


CameraController::~CameraController()
// This code will be either refactored or removed. Maybe.
{   
    // TODO: check whether more cleanup is needed here
    camera->Close();
}


void CameraController::startAquisition()
{
    if (camera->StartContinuousImageAcquisition(5, IFrameObserverPtr(frameObserver)) != VmbErrorSuccess)
    {
        cerr << "Could not start camera acquisition" << endl;
        abort();
    }
}


void CameraController::stopAquisition()
{
    if (camera->StopContinuousImageAcquisition() != VmbErrorSuccess)
    {
        cerr << "Could not stop camera acquisition" << endl;
        abort();
    }
}

void CameraController::setExposureTime(float _exposureTime)
{
    FeaturePtr  feature;

    if ((camera->GetFeatureByName("ExposureTime", feature) != VmbErrorSuccess) ||
        (feature->SetValue(_exposureTime) != VmbErrorSuccess))
    {
        cerr << "Could not set exposure time" << endl;
        abort();
    }
}

void CameraController::setGain(float _gain)
{
    FeaturePtr  feature;

    if ((camera->GetFeatureByName("Gain", feature) != VmbErrorSuccess) ||
        (feature->SetValue(_gain) != VmbErrorSuccess))
    {
        cerr << "Could not set gain" << endl;
        abort();
    }
}


void CameraController::setBalance(float _balance, char* _color)
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