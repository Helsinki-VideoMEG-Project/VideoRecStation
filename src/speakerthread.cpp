/*
 * speaker.cpp
 *
 * Author: Andrey Zhdanov
s *
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
#include <QMessageBox>
#include <cstdlib>

#include "config.h"
#include "speakerthread.h"
#include "settings.h"

SpeakerThread::SpeakerThread(NonBlockingBuffer* _buffer)
{
    int                     rc;
    snd_pcm_hw_params_t*    params;
    unsigned int            sampRate;
    snd_pcm_uframes_t       framesPerPeriod;
    AudioSettings           audioSettings = Settings::getInstance().getAudioSettings();

    buffer = _buffer;

    // Open PCM device for playback
    rc = snd_pcm_open(&sndHandle, audioSettings.outDev.toLocal8Bit().data(), SND_PCM_STREAM_PLAYBACK, 0);
    if (rc < 0)
    {
        QMessageBox::critical(nullptr, "SpeakerThread Error", QString("Unable to open PCM device: %1").arg(snd_strerror(rc)));
        std::exit(EXIT_FAILURE);
    }

    snd_pcm_hw_params_alloca(&params);          // Allocate a hardware parameters object
    snd_pcm_hw_params_any(sndHandle, params);   // Fill it in with default values

    // Set the desired hardware parameters
    snd_pcm_hw_params_set_access(sndHandle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(sndHandle, params, AUDIO_FORMAT);
    snd_pcm_hw_params_set_channels(sndHandle, params, N_CHANS);

    // Set sampling rate
    sampRate = audioSettings.sampRate;
    snd_pcm_hw_params_set_rate_near(sndHandle, params, &sampRate, NULL);

    // Set period size
    framesPerPeriod = audioSettings.framesPerPeriod;
    snd_pcm_hw_params_set_period_size_near(sndHandle, params, &framesPerPeriod, NULL);

    /* Set number of periods */
    if (snd_pcm_hw_params_set_periods(sndHandle, params, audioSettings.nPeriods, 0) < 0)
    {
        QMessageBox::critical(nullptr, "SpeakerThread Error", QString("Error setting periods"));
        std::exit(EXIT_FAILURE);
    }

    // Write the parameters to the driver
    rc = snd_pcm_hw_params(sndHandle, params);
    if (rc < 0)
    {
        QMessageBox::critical(nullptr, "SpeakerThread Error", QString("Unable to set HW parameters: %1").arg(snd_strerror(rc)));
        std::exit(EXIT_FAILURE);
    }

    // Verify the parameters
    sampRate = 0;
    snd_pcm_hw_params_get_rate(params, &sampRate, NULL);
    if (sampRate != audioSettings.sampRate)
    {
        QMessageBox::critical(nullptr, "SpeakerThread Error", QString("Unable to set sampling rate: requested %1, actual %2").arg(audioSettings.sampRate).arg(sampRate));
        std::exit(EXIT_FAILURE);
    }

    framesPerPeriod = 0;
    snd_pcm_hw_params_get_period_size(params, &framesPerPeriod, NULL);
    if (audioSettings.framesPerPeriod != framesPerPeriod)
    {
        QMessageBox::critical(nullptr, "SpeakerThread Error", QString("Unable to set frames per period: requested %1, actual %2").arg(audioSettings.framesPerPeriod).arg(framesPerPeriod));
        std::exit(EXIT_FAILURE);
    }
}


SpeakerThread::~SpeakerThread()
{
    // TODO Add code for releasing the sound card
}


void SpeakerThread::stoppableRun()
{
    int             rc;
    AudioSettings   audioSettings = Settings::getInstance().getAudioSettings();

    // TODO: Consider increasing the thread's priority 

    // Start the playback loop
    while(!shouldStop)
    {
        rc = snd_pcm_writei(sndHandle, buffer->getChunk(), audioSettings.framesPerPeriod);
        if (rc == -EPIPE)
        {
            /* EPIPE means underrun */
            std::cerr << "underrun occurred" << std::endl;
            snd_pcm_prepare(sndHandle);
        }
        else if (rc < 0)
        {
            std::cerr << "error from write: " << snd_strerror(rc) << std::endl;
        }
        else if (rc != (int)audioSettings.framesPerPeriod)
        {
            std::cerr << "short write, write " << rc << " frames" << std::endl;
        }
    }
}
