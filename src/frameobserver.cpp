/*
 * frameobserver.cpp
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

#include <iostream>
#include <sched.h>
#include <time.h>
#include <QCoreApplication>
#include <stdlib.h>
#include <thread>
#include <chrono>

#include "frameobserver.h"
#include "config.h"

using namespace std;
using namespace VmbCPP;


FrameObserver::FrameObserver(CameraPtr _camera, CycDataBuffer* _cycBuf, GPUJPEGEncoder* _encoder, struct camera_settings _camSettings) : IFrameObserver(_camera)
{
    cycBuf = _cycBuf;
    rawImageSize = (size_t)(_camSettings.width) * _camSettings.height * (_camSettings.color ? 3 : 1);

    encoder = _encoder;

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
    VmbUchar_t*     rawImage;

    uint8_t*        jpegImage;
    size_t          jpegSize;

    clock_gettime(CLOCK_REALTIME, &timestamp);
    chunkAttrib.timestamp = timestamp.tv_nsec / 1000000 + timestamp.tv_sec * 1000;

    if (_frame->GetImage(rawImage) != VmbErrorSuccess)
    {
        cerr << "Could not get the image data" << endl;
        abort();
    }

    jpegImage = encoder->encodeFrame(rawImage, jpegSize);
    chunkAttrib.chunkSize = jpegSize;

    cycBuf->insertChunk(jpegImage, chunkAttrib);

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

