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



#include <cstdlib>
#include <QMessageBox>

#include <opencv2/opencv.hpp>
#include "gpujpegencoder.h"

GPUJPEGEncoder::GPUJPEGEncoder(CameraSettings _camSettings)
{
    // Initialize GPUJPEG encoder
    if ((encoder = gpujpeg_encoder_create(0)) == NULL) {
        QMessageBox::critical(nullptr, "GPUJPEGEncoder Error", QString("Error creating GPUJPEG encoder"));
        std::exit(EXIT_FAILURE);
    }

    quality = _camSettings.jpegQuality;

    // Set image parameters
    gpujpeg_image_set_default_parameters(&param_image);
    param_image.width = _camSettings.width;
    param_image.height = _camSettings.height;
    param_image.color_space = _camSettings.color ? GPUJPEG_RGB : GPUJPEG_NONE;
    param_image.pixel_format = _camSettings.color ? GPUJPEG_444_U8_P012 : GPUJPEG_U8;

    debayeredData = (unsigned char*)malloc(_camSettings.width*_camSettings.height*3);   // 3 bytes per pixel should be enough for any pixel format, either color or monochrome
    memset(debayeredData, 42, _camSettings.width*_camSettings.height*3);
    debayeredMatType = _camSettings.color ? CV_8UC3 : CV_8UC1;
    colorConv = _camSettings.color ? cv::COLOR_BayerBG2RGB : cv::COLOR_BayerBG2GRAY;
}


GPUJPEGEncoder::~GPUJPEGEncoder()
{
    gpujpeg_encoder_destroy(encoder);
    free(debayeredData);
}


uint8_t* GPUJPEGEncoder::encodeFrame(uint8_t* _frameData, size_t& _outSize)
{
        struct gpujpeg_encoder_input encoder_input;
        uint8_t* image_compressed = NULL;
        size_t image_compressed_size = 0;

        // Debayer
        cv::Mat bayerMat(param_image.height, param_image.width, CV_8UC1, _frameData);
        cv::Mat debayerMat(param_image.height, param_image.width, debayeredMatType, debayeredData);
        cv::cvtColor(bayerMat, debayerMat, colorConv);

        // Set encode parameters
        gpujpeg_set_default_parameters(&param);
        param.quality = quality;

        // Set the raw data for the image to be compressed
        gpujpeg_encoder_input_set_image(&encoder_input, debayeredData);

        // compress the image
        if (gpujpeg_encoder_encode(encoder, &param, &param_image, &encoder_input, &image_compressed, &image_compressed_size) != 0) {
            QMessageBox::critical(nullptr, "GPUJPEGEncoder Error", QString("Error encoding frame"));
            std::exit(EXIT_FAILURE);
        }

        _outSize = image_compressed_size;
        return image_compressed;
}


void GPUJPEGEncoder::setJPEGQuality(int _quality)
{
    quality = _quality;
}