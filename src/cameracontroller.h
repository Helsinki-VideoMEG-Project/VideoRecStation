/*
 * cameracontroller.h
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

#ifndef CAMERACONTROLLER_H_
#define CAMERACONTROLLER_H_

#include <VmbCPP/VmbCPP.h>
#include "frameobserver.h"

//! A class for controlling a USB camera.
class CameraController
{
public:
    CameraController(VmbCPP::CameraPtr _camera, FrameObserver* _frameObserver, CameraSettings _camSettings);
    virtual ~CameraController();
    void startAquisition();
    void stopAquisition();
    void setExposureTime(float _exposureTime);
    void setGain(float _gain);
    void setBalance(float _balance, char* _color);

private:
    VmbCPP::CameraPtr   camera;
    FrameObserver*      frameObserver;
};

#endif /* CAMERACONTROLLER_H_ */
