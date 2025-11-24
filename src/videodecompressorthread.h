/*
 * videodecompressorthread.h
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

#ifndef VIDEODECOMPRESSORTHREAD_H_
#define VIDEODECOMPRESSORTHREAD_H_

#include "stoppablethread.h"
#include "cycdatabuffer.h"
#include "libgpujpeg/gpujpeg_decoder.h"
#include "settings.h"

//! A thread that decompresses video as they arrive in the input buffer.
/*!
 * Continuously read JPEG-compressed video frames from the input cyclic buffer,
 * decompresse them using GPUJPEG library and emit frameDecoded() signal with the
 * decompressed frame.
 * 
 * Also copy the input frame to the output buffer so that they can be handled by
 * lower-priority threads (e.g. video file writer).
 * 
 * The main purpose of this class is to decode video frames ASAP so that they
 * can be displayed with minimum latency.
 */
class VideoDecompressorThread : public StoppableThread
{

    Q_OBJECT

public:
    VideoDecompressorThread(CycDataBuffer* _inpBuf, CycDataBuffer* _outBuf, struct camera_settings _camSettings);
    virtual ~VideoDecompressorThread();

signals:
    /*!
     * This signal is raised when a new frame has been decoded.
     */
    void frameDecoded(uint8_t* _data, int _width, int _height, bool _color);

protected:
    virtual void stoppableRun();

private:
    CycDataBuffer*  inpBuf;
    CycDataBuffer*  outBuf;
    struct gpujpeg_decoder* decoder;
    int             width;
    int             height;
    bool            color;
};

#endif /* VIDEODECOMPRESSORTHREAD_H_ */
