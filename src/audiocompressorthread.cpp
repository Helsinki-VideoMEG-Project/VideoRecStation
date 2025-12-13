/*
 * audiocompressorthread.cpp
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

#include <cstdlib>
#include <stdio.h>
#include <assert.h>

#include "config.h"
#include "audiocompressorthread.h"

AudioCompressorThread::AudioCompressorThread(CycDataBuffer* _inpBuf, CycDataBuffer* _outBuf, int _nChanInp, int _nChanOut)
{
    inpBuf = _inpBuf;
    outBuf = _outBuf;
    nChanInp = _nChanInp;
    nChanOut = _nChanOut;
}


AudioCompressorThread::~AudioCompressorThread()
{
}


void AudioCompressorThread::stoppableRun()
{
    while(!shouldStop)
    {
        unsigned char*      outChunk=NULL;
        size_t              outChunkLen;
        size_t              nSamples;

        unsigned char*      inpChunk;
        ChunkAttrib         chunkAttrib;

        // Get raw data from the input buffer
        if ((inpChunk = inpBuf->getChunk(&chunkAttrib, BUFF_SEM_TIMEOUT_MS)) == nullptr) {
            continue; // timeout
        }

        // Check that the chunk size makes sense
        assert(chunkAttrib.chunkSize % (N_CHANS * sizeof(AUDIO_DATA_TYPE)) == 0);

        // TODO: this is not an efficient implementation, better to allocate the buffer once
        outChunkLen = (chunkAttrib.chunkSize / N_CHANS) * N_CHANS_2_RECORD;
        outChunk = (unsigned char*)malloc(outChunkLen);

        nSamples = chunkAttrib.chunkSize / (N_CHANS * sizeof(AUDIO_DATA_TYPE));

        for (size_t i = 0; i < nSamples; i++)
        {
            memcpy(outChunk + i * N_CHANS_2_RECORD * sizeof(AUDIO_DATA_TYPE),
                   inpChunk + i * N_CHANS * sizeof(AUDIO_DATA_TYPE),
                   N_CHANS_2_RECORD * sizeof(AUDIO_DATA_TYPE));
        }

        chunkAttrib.chunkSize = outChunkLen;
        outBuf->insertChunk(outChunk, chunkAttrib);

        free(outChunk);
    }
}
