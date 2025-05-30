/*
 * config.h
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

#ifndef CONFIG_H_

// Buffer parameters
#define CIRC_BUF_MARG       0.2         // When less than this fraction of the buffer
                                        // is left free, overflow error is generated.
                                        // Used as a safety measure against race
                                        // condition between consumer and producer
                                        // threads when the circular buffer is close
                                        // to full.

#define MAX_CHUNK_SIZE      (CIRC_BUF_MARG/10)
                                        // Maximum chunk size as a fraction of the
                                        // circular buffer size. Should be at least
                                        // several times smaller than CIRC_BUF_MARG
                                        // for the buffer to work properly.

// Buffer sizes
#define CIRC_VIDEO_BUFF_SZ  2000000000  // in bytes, must be smaller than the largest int (2147483647 ?)
#define CIRC_AUDIO_BUFF_SZ  1000000000  // in bytes, must be smaller than the largest int (2147483647 ?)

// Thread priorities
#define CAM_THREAD_PRIORITY 10
#define MIC_THREAD_PRIORITY 15
#define SPK_THREAD_PRIORITY 5

#define SHUTTER_SLIDER_MIN    1           // Milliseconds, need to be multiplied by 1000 before sending to the camera
#define SHUTTER_SLIDER_MAX    200
#define SHUTTER_SCALE         1000.0

#define GAIN_SLIDER_MIN       0           // Doesn't need any conversion, can be sent to the camera as is
#define GAIN_SLIDER_MAX       48
#define GAIN_SCALE            1.0

#define BALANCE_SLIDER_MIN    0           // Need to be divided by 100 before sending to the camera
#define BALANCE_SLIDER_MAX    800
#define BALANCE_SCALE         0.01

// Used for storing application settings
#define ORG_NAME "Helsinki VideoMEG Project"
#define APP_NAME "VideoRecStation"

// Camera configuration
#define MAX_CAMERAS         6

// Audio configuration
#define N_CHANS             20          // number of channels provided by the sound card
#define N_CHANS_2_RECORD    8           // number of channels to record
#define N_BUF_4_VOL_IND     10          // number of buffers used by volume indicator

#define AUDIO_FORMAT        SND_PCM_FORMAT_S32_LE   // from <alsa/asoundlib.h>
#define AUDIO_DATA_TYPE     int32_t                 // should match AUDIO_FORMAT
#define MAX_AUDIO_VAL       INT32_MAX               // should match AUDIO_FORMAT
#define AUDIO_DATA_TYPE_STR "<i"                    // Python struct format for a single sample, should match AUDIO_FORMAT. Must be 2 byes long.

#define AUDIO_FILE_VERSION  0
#define VIDEO_FILE_VERSION  0

#define MAGIC_VIDEO_STR     "HELSINKI_VIDEO_MEG_PROJECT_VIDEO_FILE"
#define MAGIC_AUDIO_STR     "HELSINKI_VIDEO_MEG_PROJECT_AUDIO_FILE"

#define CONFIG_H_

#endif /* CONFIG_H_ */
