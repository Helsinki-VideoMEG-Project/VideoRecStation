/*
 * cameracontroller.cpp
 *
 * Author: Andrey Zhdanov
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

#include <sched.h>
#include <time.h>
#include <QCoreApplication>
#include <stdlib.h>
#include <thread>
#include <chrono>
#include <QMessageBox>
#include <cstdlib>

#include "cameracontroller.h"
#include "config.h"
#include "settings.h"

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
    if (camera->Open(VmbAccessModeFull) != VmbErrorSuccess) {
        QMessageBox::critical(nullptr, "CameraController Error", QString("Could not open camera"));
        std::exit(EXIT_FAILURE);
    }

    // Reset the ROI offset to 0/0 first, otherwise setting the width/height might fail
    if ((camera->GetFeatureByName("OffsetX", feature) != VmbErrorSuccess) ||
        (feature->SetValue(0) != VmbErrorSuccess) ||
        (camera->GetFeatureByName("OffsetY", feature) != VmbErrorSuccess) ||
        (feature->SetValue(0) != VmbErrorSuccess))
    {
        QMessageBox::critical(nullptr, "CameraController Error", QString("Could not reset ROI offset to 0/0"));
        std::exit(EXIT_FAILURE);
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
        QMessageBox::critical(nullptr, "CameraController Error", QString("Could not set up the ROI size/offset"));
        std::exit(EXIT_FAILURE);
    }

    if ((camera->GetFeatureByName("PixelFormat", feature) != VmbErrorSuccess) ||
        (feature->SetValue("BayerRG8") != VmbErrorSuccess))
    {
        QMessageBox::critical(nullptr, "CameraController Error", QString("Could not set up the video format"));
        std::exit(EXIT_FAILURE);
    }

    // Turn off the USB trottling
    // This is needed to get the maximum throughput from the camera
    if ((camera->GetFeatureByName("DeviceLinkThroughputLimitMode", feature) != VmbErrorSuccess) ||
        (feature->SetValue("Off") != VmbErrorSuccess))
    {
        QMessageBox::critical(nullptr, "CameraController Error", QString("Could not set up device throughput limit mode"));
        std::exit(EXIT_FAILURE);
    }

    // Make sure the exposure can be controlled by the slider
    if ((camera->GetFeatureByName("ExposureMode", feature) != VmbErrorSuccess) ||
        (feature->SetValue("Timed") != VmbErrorSuccess))
    {
        QMessageBox::critical(nullptr, "CameraController Error", QString("Could not set up device exposure mode"));
        std::exit(EXIT_FAILURE);
    }

    if ((camera->GetFeatureByName("ExposureAuto", feature) != VmbErrorSuccess) ||
        (feature->SetValue("Off") != VmbErrorSuccess))
    {
        QMessageBox::critical(nullptr, "CameraController Error", QString("Could not turn off the autoexposure"));
        std::exit(EXIT_FAILURE);
    }

    if (camera->GetPayloadSize(payloadSize) != VmbErrorSuccess) {
        QMessageBox::critical(nullptr, "CameraController Error", QString("Could not get the camera payload size"));
        std::exit(EXIT_FAILURE);
    }

    if (payloadSize != _camSettings.width * _camSettings.height) {
        QMessageBox::critical(nullptr, "CameraController Error", QString("Error: the actual payload size %1 does not match the expected size %2").arg(payloadSize).arg(_camSettings.width * _camSettings.height));
        std::exit(EXIT_FAILURE);
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
            QMessageBox::critical(nullptr, "CameraController Error", QString("Could not set up external trigger mode"));
            std::exit(EXIT_FAILURE);
        }
    }
    else
    {
        // Set the camera to free run mode
        if ((camera->GetFeatureByName("TriggerMode", feature) != VmbErrorSuccess) ||
            (feature->SetValue("Off") != VmbErrorSuccess))
        {
            QMessageBox::critical(nullptr, "CameraController Error", QString("Could not set up free run mode"));
            std::exit(EXIT_FAILURE);
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
    if (camera->StartContinuousImageAcquisition(5, IFrameObserverPtr(frameObserver)) != VmbErrorSuccess) {
        QMessageBox::critical(nullptr, "CameraController Error", QString("Could not start camera acquisition"));
        std::exit(EXIT_FAILURE);
    }
}


void CameraController::stopAquisition()
{
    if (camera->StopContinuousImageAcquisition() != VmbErrorSuccess) {
        QMessageBox::critical(nullptr, "CameraController Error", QString("Could not stop camera acquisition"));
        std::exit(EXIT_FAILURE);
    }
}

void CameraController::setExposureTime(float _exposureTime)
{
    FeaturePtr  feature;

    if ((camera->GetFeatureByName("ExposureTime", feature) != VmbErrorSuccess) ||
        (feature->SetValue(_exposureTime) != VmbErrorSuccess))
    {
        QMessageBox::critical(nullptr, "CameraController Error", QString("Could not set exposure time"));
        std::exit(EXIT_FAILURE);
    }
}

void CameraController::setGain(float _gain)
{
    FeaturePtr  feature;

    if ((camera->GetFeatureByName("Gain", feature) != VmbErrorSuccess) ||
        (feature->SetValue(_gain) != VmbErrorSuccess))
    {
        QMessageBox::critical(nullptr, "CameraController Error", QString("Could not set gain"));
        std::exit(EXIT_FAILURE);
    }
}


void CameraController::setBalance(float _balance, char* _color)
{
    FeaturePtr  feature;

    if ((camera->GetFeatureByName("BalanceRatioSelector", feature) != VmbErrorSuccess) ||
        (feature->SetValue(_color) != VmbErrorSuccess))
    {
        QMessageBox::critical(nullptr, "CameraController Error", QString("Could not set balance ratio selector"));
        std::exit(EXIT_FAILURE);
    }

    if ((camera->GetFeatureByName("BalanceRatio", feature) != VmbErrorSuccess) ||
        (feature->SetValue(_balance) != VmbErrorSuccess))
    {
        QMessageBox::critical(nullptr, "CameraController Error", QString("Could not set balance ratio"));
        std::exit(EXIT_FAILURE);
    }
}