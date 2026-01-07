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

#include "settings.h"
#include "config.h"


Settings::Settings()
{
    settings = new QSettings(ORG_NAME, APP_NAME);

    audioSettings.sampRate = settings->value("audio/sampling_rate", 44100).toInt();
    audioSettings.framesPerPeriod = settings->value("audio/frames_per_period", 940).toInt();
    audioSettings.nPeriods = settings->value("audio/num_periods", 10).toInt();
    audioSettings.spkBufSize = settings->value("audio/speaker_buffer_size", 4).toInt();          // Speaker buffer size (in frames), used for speaker feedback
    audioSettings.inpDev = settings->value("audio/input_audio_device", "default").toString();
    audioSettings.outDev = settings->value("audio/output_audio_device", "default").toString();
    audioSettings.useFeedback = settings->value("audio/use_speaker_feedback", false).toBool();   // Enable/disable speaker feedback

    miscSettings.externalTriggerSource = settings->value("video/external_trigger_source", "Line0").toString();
    miscSettings.storagePath = settings->value("misc/data_storage_path", "/tmp").toString();
    miscSettings.lowDiskSpaceThreshGB = settings->value("misc/low_disk_space_warning_threshold_gb", 5).toDouble();
    miscSettings.framelockDisplay = settings->value("misc/framelock_display", ":2").toString();
}

Settings::~Settings()
{
    settings->setValue("audio/sampling_rate", audioSettings.sampRate);
    settings->setValue("audio/frames_per_period", audioSettings.framesPerPeriod);
    settings->setValue("audio/num_periods", audioSettings.nPeriods);
    settings->setValue("audio/speaker_buffer_size", audioSettings.spkBufSize);
    settings->setValue("audio/input_audio_device", audioSettings.inpDev);
    settings->setValue("audio/output_audio_device", audioSettings.outDev);
    settings->setValue("audio/use_speaker_feedback", audioSettings.useFeedback);
    
    settings->setValue("video/external_trigger_source", miscSettings.externalTriggerSource);
    settings->setValue("misc/data_storage_path", miscSettings.storagePath);
    settings->setValue("misc/low_disk_space_warning_threshold_gb", miscSettings.lowDiskSpaceThreshGB);
    settings->setValue("misc/framelock_display", miscSettings.framelockDisplay);

    settings->sync();
}

Settings& Settings::getInstance()
{
    static Settings instance;
    return instance;
}

CameraSettings Settings::getCameraSettings(QString _cameraSN)
{
    QWriteLocker locker(&rwLock);

    CameraSettings camSettings;
    QString baseKey = QString("video/camera/") + _cameraSN + "/";

    camSettings.shutter = settings->value(baseKey + "shutter", 20).toInt();
    camSettings.gain = settings->value(baseKey + "gain", 10).toInt();
    camSettings.balanceBlue = settings->value(baseKey + "balance_blue", 15).toInt();
    camSettings.balanceRed = settings->value(baseKey + "balance_red", 15).toInt();
    camSettings.jpegQuality = settings->value(baseKey + "jpeg_quality", 80).toInt();
    camSettings.width = settings->value(baseKey + "width", 976).toInt();
    camSettings.height = settings->value(baseKey + "height", 736).toInt();
    camSettings.offsetX = settings->value(baseKey + "offset_x", 488).toInt();
    camSettings.offsetY = settings->value(baseKey + "offset_y", 368).toInt();
    camSettings.color = settings->value(baseKey + "color", true).toBool();
    camSettings.useTrigger = settings->value(baseKey + "use_external_trigger", false).toBool();

    return camSettings;
}

void Settings::setCameraSettings(QString _cameraSN, CameraSettings _camSettings)
{
    QWriteLocker locker(&rwLock);

    QString baseKey = QString("video/camera/") + _cameraSN + "/";

    settings->setValue(baseKey + "shutter", _camSettings.shutter);
    settings->setValue(baseKey + "gain", _camSettings.gain);
    settings->setValue(baseKey + "balance_blue", _camSettings.balanceBlue);
    settings->setValue(baseKey + "balance_red", _camSettings.balanceRed);
    settings->setValue(baseKey + "jpeg_quality", _camSettings.jpegQuality);
    settings->setValue(baseKey + "width", _camSettings.width);
    settings->setValue(baseKey + "height", _camSettings.height);
    settings->setValue(baseKey + "offset_x", _camSettings.offsetX);
    settings->setValue(baseKey + "offset_y", _camSettings.offsetY);
    settings->setValue(baseKey + "color", _camSettings.color);
    settings->setValue(baseKey + "use_external_trigger", _camSettings.useTrigger);

    settings->sync();
}

AudioSettings Settings::getAudioSettings()
{
    QReadLocker locker(&rwLock);

    return(audioSettings);
}

MiscSettings Settings::getMiscSettings()
{
    QReadLocker locker(&rwLock);

    return(miscSettings);
}

void Settings::setMiscSettings(MiscSettings _miscSettings)
{
    QWriteLocker locker(&rwLock);

    miscSettings = _miscSettings;
}