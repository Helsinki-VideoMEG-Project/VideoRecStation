/*
 * camerathread.h
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

#ifndef USBCAMERA_H_
#define USBCAMERA_H_

#include <VmbCPP/VmbCPP.h>
#include "cycdatabuffer.h"

class FrameObserver : public VmbCPP::IFrameObserver
{
public:
    FrameObserver(VmbCPP::CameraPtr _camera, CycDataBuffer* _cycBuf, unsigned int _chunkSize);
    virtual ~FrameObserver();

    // Frame callback notifies about incoming frames
    void FrameReceived(const VmbCPP::FramePtr _frame);

private:
    CycDataBuffer*  cycBuf;
    unsigned int    chunkSize;

#ifdef QT_DEBUG
private:
    long frameCnt;
#endif
};

//! A class for controlling a USB camera.
class USBCamera
{
public:
    USBCamera(VmbCPP::CameraPtr _camera, CycDataBuffer* _cycBuf, bool _color);
    virtual ~USBCamera();
    void startAquisition();
    void stopAquisition();
    void setExposureTime(float _exposureTime);

private:
    VmbCPP::CameraPtr   camera;
    CycDataBuffer*      cycBuf;
    bool                color;
    FrameObserver*      frameObserver;
};

#endif /* USBCAMERA_H_ */
