/*
 * videodecompressorthread.cpp
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

#include <cstdlib>
#include <stdio.h>
#include <assert.h>
#include <iostream>

#include "config.h"
#include "videodecompressorthread.h"

VideoDecompressorThread::VideoDecompressorThread(CycDataBuffer* _inpBuf, CycDataBuffer* _outBuf, int _width, int _height, bool _color)
{
    inpBuf = _inpBuf;
    outBuf = _outBuf;
    width = _width;
    height = _height;
    color = _color;

    decoder = gpujpeg_decoder_create(0);
}


VideoDecompressorThread::~VideoDecompressorThread()
{
    gpujpeg_decoder_destroy(decoder);
}


void VideoDecompressorThread::stoppableRun()
{
    while(!shouldStop)
    {
        uint8_t*      jpegImage;
        ChunkAttrib   chunkAttrib;
        struct gpujpeg_decoder_output dec_output;

        // Get the next jpeg image from the input buffer
        jpegImage = inpBuf->getChunk(&chunkAttrib);

        // Decompress the jpeg image
        gpujpeg_decoder_output_set_default(&dec_output);

        if ( gpujpeg_decoder_decode(decoder, jpegImage, chunkAttrib.chunkSize, &dec_output) != 0) {
            std::cerr << "Error decoding frame" << std::endl;
            exit(1);
        }

        // Emit the signal notifying about the new decoded frame
        emit frameDecoded(dec_output.data, width, height, color);

        // Copy the input jpeg image to the output buffer for further processing
        outBuf->insertChunk(jpegImage, chunkAttrib);
    }
}
