/*
 * gpujpegencoder.cpp
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
#include "gpujpegencoder.h"

GPUJPEGEncoder::GPUJPEGEncoder(struct camera_settings _camSettings)
{
    // Initialize GPUJPEG encoder
    if ((encoder = gpujpeg_encoder_create(0)) == NULL) {
        std::cerr << "Error creating GPUJPEG encoder" << std::endl;
        exit(1);
    }

    quality = _camSettings.jpeg_quality;

    // Set image parameters
    gpujpeg_image_set_default_parameters(&param_image);
    param_image.width = _camSettings.width;
    param_image.height = _camSettings.height;
    param_image.color_space = _camSettings.color ? GPUJPEG_RGB : GPUJPEG_NONE;
    param_image.pixel_format = _camSettings.color ? GPUJPEG_444_U8_P012 : GPUJPEG_U8;
}


GPUJPEGEncoder::~GPUJPEGEncoder()
{
    gpujpeg_encoder_destroy(encoder);
}


uint8_t* GPUJPEGEncoder::encodeFrame(uint8_t* _frameData, size_t& _outSize)
{
        struct gpujpeg_encoder_input encoder_input;
        uint8_t* image_compressed = NULL;
        size_t image_compressed_size = 0;

        // Set encode parameters
        gpujpeg_set_default_parameters(&param);
        param.quality = quality;

        // Set the raw data for the image to be compressed
        gpujpeg_encoder_input_set_image(&encoder_input, _frameData);

        // compress the image
        if (gpujpeg_encoder_encode(encoder, &param, &param_image, &encoder_input, &image_compressed, &image_compressed_size) != 0) {
            std::cerr << "Error encoding frame" << std::endl;
            exit(1);
        }

        _outSize = image_compressed_size;
        return image_compressed;
}


void GPUJPEGEncoder::setJPEGQuality(int _quality)
{
    quality = _quality;
}