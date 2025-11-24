/*
 * frameobserver.h
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

#ifndef FRAMEOBSERVER_H_
#define FRAMEOBSERVER_H_

#include <VmbCPP/VmbCPP.h>
#include "cycdatabuffer.h"
#include "settings.h"
#include "gpujpegencoder.h"
#include "settings.h"

class FrameObserver : public VmbCPP::IFrameObserver
{
public:
    FrameObserver(VmbCPP::CameraPtr _camera, CycDataBuffer* _cycBuf, GPUJPEGEncoder* _encoder, struct camera_settings _camSettings);
    virtual ~FrameObserver();

    // Frame callback notifies about incoming frames
    void FrameReceived(const VmbCPP::FramePtr _frame);

private:
    CycDataBuffer*  cycBuf;
    size_t          rawImageSize;

    GPUJPEGEncoder* encoder;

#ifdef QT_DEBUG
private:
    long frameCnt;
#endif
};


#endif /* FRAMEOBSERVER_H_ */