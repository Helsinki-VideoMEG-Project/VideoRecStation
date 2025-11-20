/*
 * gpujpegencoder.h
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

#ifndef GPUJPEGENCODER_H_
#define GPUJPEGENCODER_H_

#include <QAtomicInteger>
#include "libgpujpeg/gpujpeg_encoder.h"

class GPUJPEGEncoder
{
public:
    GPUJPEGEncoder(int _width, int _height, bool _color, int _quality);
    virtual ~GPUJPEGEncoder();
    uint8_t* encodeFrame(uint8_t* _frameData, size_t& _outSize);
    void setJPEGQuality(int _quality);

private:
    struct gpujpeg_encoder* encoder;
    struct gpujpeg_parameters param;
    struct gpujpeg_image_parameters param_image;
    QAtomicInteger<int> quality;
};

#endif /* GPUJPEGENCODER_H_ */
