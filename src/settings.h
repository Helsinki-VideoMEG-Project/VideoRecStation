/*
 * settings.h
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

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <QString>
#include "config.h"

struct camera_settings {
    int shutter;
    int gain;
    int balance_blue;
    int balance_red;
    int jpeg_quality;

    int width;
    int height;
    int offsetx;
    int offsety;
    bool color;
};


//! Application-wide settings preserved across multiple invocations.
/*!
 * This class contains application-wide settings that are persisted on disc.
 * Create a single instance of this class when the program starts and 
 * destroy it when the program ends to save the updated sttings to the disc.
 */
class Settings {
    // TODO: make the class thread-safe
    // TODO: implement singleton pattern?
public:
    Settings();
    ~Settings();

    // video
    bool            useExternalTrigger;
    QString         externalTriggerSource;

    // audio
    unsigned int    sampRate;
    unsigned int    framesPerPeriod;
    unsigned int    nPeriods;
    unsigned int    spkBufSz;
    QString         inpAudioDev;
    QString         outAudioDev;
    bool            useFeedback;

    // misc
    QString         storagePath;
    double          lowDiskSpaceThreshGB;
    QString         framelockDisplay;

    /*!
     * Load camera-specific settings from disk. User camera serial number to
     * identify the camera.
     */
    struct camera_settings loadCameraSettings(QString _cameraSN);

    /*!
     * Save camera-specific settings to disk. User camera serial number to
     * identify the camera.
     */
    void saveCameraSettings(QString _cameraSN, struct camera_settings _camSettings);
};

#endif /* SETTINGS_H_ */
