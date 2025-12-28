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
#include <QSettings>
#include <QReadWriteLock>

struct CameraSettings
{
    unsigned int shutter;
    unsigned int gain;
    unsigned int balanceBlue;
    unsigned int balanceRed;
    unsigned int jpegQuality;

    unsigned int width;
    unsigned int height;
    unsigned int offsetX;
    unsigned int offsetY;
    bool color;
    bool useTrigger;
};


struct AudioSettings
{
    unsigned int    sampRate;
    unsigned int    framesPerPeriod;
    unsigned int    nPeriods;
    unsigned int    spkBufSize;
    QString         inpDev;
    QString         outDev;
    bool            useFeedback;
};


struct MiscSettings
{
    QString         externalTriggerSource;
    QString         storagePath;
    double          lowDiskSpaceThreshGB;
};


struct CameraSettingConstraints
{
    unsigned int maxWidth;
    unsigned int maxHeight;
};


//! Application-wide settings preserved across multiple invocations.
/*!
 * This class contains application-wide settings that are persisted on disc.
 * Uses a singleton pattern, and should be thread-safe for typical usage
 * scenarios.
 * 
 * NOTE: The class uses QSettings under the hood. QSettings might cause
 * problems when constucted/destroyed outside of lifescope of
 * QCoreApplication/QApplication. To solve the problem we pass 
 * organization name and application name to QSettings explicitly,
 * so that it should not rely on QCoreApplication/QApplication.
 * This seems to work, but watch out for problems nevertheless.
 */
class Settings {

public:
    // Delete copy constructor and assignment operator
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;

    // Get singleton instance
    static Settings& getInstance();

    /*!
     * Load camera-specific settings from disk. User camera serial number to
     * identify the camera.
     */
    CameraSettings getCameraSettings(QString _cameraSN);

    /*!
     * Save camera-specific settings to disk. User camera serial number to
     * identify the camera.
     */
    void setCameraSettings(QString _cameraSN, CameraSettings _camSettings);

    AudioSettings getAudioSettings();
    MiscSettings getMiscSettings();
    void setMiscSettings(MiscSettings _miscSettings);

private:
    explicit Settings();
    ~Settings();

    QReadWriteLock rwLock;  // Read-write lock for thread safety
    QSettings* settings;
    AudioSettings audioSettings;
    MiscSettings miscSettings;
};

#endif /* SETTINGS_H_ */
