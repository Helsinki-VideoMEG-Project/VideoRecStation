/*
 * settings.cpp
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

#include <stdio.h>
#include <QSettings>

#include "settings.h"
#include "config.h"


Settings::Settings()
{
    QSettings settings(ORG_NAME, APP_NAME);

    //---------------------------------------------------------------------
    // Video settings
    //

    // Capture settings
    externalTriggerSource = settings.value("video/external_trigger_source", "Line0").toString();

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
    useFeedback = settings.value("audio/use_speaker_feedback", false).toBool();

    // Speaker buffer size (in frames)
    spkBufSz = settings.value("audio/speaker_buffer_size", 4).toInt();

    // Input/output audio devices
    inpAudioDev = settings.value("audio/input_audio_device", "default").toString();
    outAudioDev = settings.value("audio/output_audio_device", "default").toString();

    //---------------------------------------------------------------------
    // Misc settings
    //

    // Data storage folder
    storagePath = settings.value("misc/data_storage_path", "/tmp").toString();
    lowDiskSpaceThreshGB = settings.value("misc/low_disk_space_warning_threshold_gb", 5).toDouble();

}

Settings::~Settings()
{
    QSettings settings(ORG_NAME, APP_NAME);

    settings.setValue("video/external_trigger_source", externalTriggerSource);

    settings.setValue("audio/sampling_rate", sampRate);
    settings.setValue("audio/frames_per_period", framesPerPeriod);
    settings.setValue("audio/num_periods", nPeriods);
    settings.setValue("audio/use_speaker_feedback", useFeedback);
    settings.setValue("audio/speaker_buffer_size", spkBufSz);

    settings.setValue("audio/input_audio_device", inpAudioDev);
    settings.setValue("audio/output_audio_device", outAudioDev);

    settings.setValue("misc/data_storage_path", storagePath);
    settings.setValue("misc/low_disk_space_warning_threshold_gb", lowDiskSpaceThreshGB);

    settings.sync();
}


struct camera_settings Settings::loadCameraSettings(QString _cameraSN)
{
    QSettings settings(ORG_NAME, APP_NAME);
    struct camera_settings camSettings;

    QString baseKey = QString("video/camera/") + _cameraSN + "/";

    camSettings.shutter = settings.value(baseKey + "shutter", 20).toInt();
    camSettings.gain = settings.value(baseKey + "gain", 10).toInt();
    camSettings.balance_blue = settings.value(baseKey + "balance_blue", 15).toInt();
    camSettings.balance_red = settings.value(baseKey + "balance_red", 15).toInt();
    camSettings.jpeg_quality = settings.value(baseKey + "jpeg_quality", 80).toInt();
    camSettings.width = settings.value(baseKey + "width", 976).toInt();
    camSettings.height = settings.value(baseKey + "height", 736).toInt();
    camSettings.offsetx = settings.value(baseKey + "offset_x", 488).toInt();
    camSettings.offsety = settings.value(baseKey + "offset_y", 368).toInt();
    camSettings.color = settings.value(baseKey + "color", true).toBool();
    camSettings.use_trigger = settings.value(baseKey + "use_external_trigger", false).toBool();

    return camSettings;
}

void Settings::saveCameraSettings(QString _cameraSN, struct camera_settings _camSettings)
{
    QSettings settings(ORG_NAME, APP_NAME);

    QString baseKey = QString("video/camera/") + _cameraSN + "/";

    settings.setValue(baseKey + "shutter", _camSettings.shutter);
    settings.setValue(baseKey + "gain", _camSettings.gain);
    settings.setValue(baseKey + "balance_blue", _camSettings.balance_blue);
    settings.setValue(baseKey + "balance_red", _camSettings.balance_red);
    settings.setValue(baseKey + "jpeg_quality", _camSettings.jpeg_quality);
    settings.setValue(baseKey + "width", _camSettings.width);
    settings.setValue(baseKey + "height", _camSettings.height);
    settings.setValue(baseKey + "offset_x", _camSettings.offsetx);
    settings.setValue(baseKey + "offset_y", _camSettings.offsety);
    settings.setValue(baseKey + "color", _camSettings.color);
    settings.setValue(baseKey + "use_external_trigger", _camSettings.use_trigger);

    settings.sync();
}