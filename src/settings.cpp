/*
 * settings.cpp
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

#include <stdio.h>
#include <QSettings>
#include <QRect>

#include "settings.h"
#include "config.h"

Settings::Settings()
{
    QSettings settings(ORG_NAME, APP_NAME);


    //---------------------------------------------------------------------
    // Video settings
    //

    // JPEG quality
    jpgQuality = settings.value("video/jpeg_quality", 80).toInt();

    // Use color mode
    color = settings.value("video/color", true).toBool();

    // Video size
    width = settings.value("video/width", 640).toInt();
    height = settings.value("video/height", 480).toInt();
    offsetx = settings.value("video/offset_x", 0).toInt();
    offsety = settings.value("video/offset_y", 0).toInt();

    // Capture settings
    for (unsigned int i=0; i<MAX_CAMERAS; i++)
    {
        videoRects[i] = settings.value(QString("control/viewer_%1_window").arg(i+1), QRect(-1, -1, -1, -1)).toRect();
        videoLimits[i] = settings.value(QString("control/viewer_%1_limit_display_size").arg(i+1), false).toBool();
    }

    // Window pos and size
    controllerRect = settings.value("control/controller_window", QRect(-1, -1, -1, -1)).toRect();
    controlOnTop = settings.value("control/controller_on_top", false).toBool();
    lowDiskSpaceWarning = settings.value("control/low_disk_space_warning", 0).toDouble();
    confirmStop = settings.value("control/confirm_on_stop", false).toBool();
    metersUseDB = settings.value("control/meters_use_db", false).toBool();

    //---------------------------------------------------------------------
    // Audio settings
    //

    // Sampling rate
    sampRate = settings.value("audio/sampling_rate", 44100).toInt();

    // Frames per period
    framesPerPeriod = settings.value("audio/frames_per_period", 940).toInt();

    // Number of periods
    nPeriods = settings.value("audio/num_periods", 10).toInt();

    // Enable/disable speaker feedback
    useFeedback = settings.value("audio/use_speaker_feedback", true).toBool();

    // Speaker buffer size (in frames)
    spkBufSz = settings.value("audio/speaker_buffer_size", 4).toInt();

    // Input/output audio devices
    inpAudioDev = settings.value("audio/input_audio_device", "default").toString();
    outAudioDev = settings.value("audio/output_audio_device", "default").toString();

    //---------------------------------------------------------------------
    // Misc settings
    //

    // Data storage folder
    storagePath = settings.value("misc/data_storage_path", "/videodat").toString();
}

Settings::~Settings()
{
    QSettings settings(ORG_NAME, APP_NAME);

    settings.setValue("video/jpeg_quality", jpgQuality);
    settings.setValue("video/color", color);
    settings.setValue("video/width", width);
    settings.setValue("video/height", height);
    settings.setValue("video/offset_x", offsetx);
    settings.setValue("video/offset_y", offsety);
    for (unsigned int i=0; i<MAX_CAMERAS; i++)
    {
        settings.setValue(QString("control/viewer_%1_window").arg(i+1), videoRects[i]);
        settings.setValue(QString("control/viewer_%1_limit_display_size").arg(i+1), videoLimits[i]);
    }
    settings.setValue("control/controller_window", controllerRect);
    settings.setValue("control/controller_on_top", controlOnTop);
    settings.setValue("control/low_disk_space_warning", lowDiskSpaceWarning);
    settings.setValue("control/confirm_on_stop", confirmStop);
    settings.setValue("control/meters_use_db", metersUseDB);

    settings.setValue("audio/sampling_rate", sampRate);
    settings.setValue("audio/frames_per_period", framesPerPeriod);
    settings.setValue("audio/num_periods", nPeriods);
    settings.setValue("audio/use_speaker_feedback", useFeedback);
    settings.setValue("audio/speaker_buffer_size", spkBufSz);

    settings.setValue("audio/input_audio_device", inpAudioDev);
    settings.setValue("audio/output_audio_device", outAudioDev);

    settings.setValue("misc/data_storage_path", storagePath);

    settings.sync();
}
