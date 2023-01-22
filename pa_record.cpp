// pa_record.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

/** @file paex_record.c
    @ingroup examples_src
    @brief Record input into an array; Save array to a file; Playback recorded data.
    @author Phil Burk  http://www.softsynth.com
*/
/*
 * $Id$
 *
 * This program uses the PortAudio Portable Audio Library.
 * For more information see: http://www.portaudio.com
 * Copyright (c) 1999-2000 Ross Bencina and Phil Burk
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

 /*
  * The text above constitutes the entire PortAudio license; however,
  * the PortAudio community also makes the following non-binding requests:
  *
  * Any person wishing to distribute modifications to the Software is
  * requested to send the modifications to the original developer so that
  * they can be incorporated into the canonical version. It is also
  * requested that these non-binding requests be included along with the
  * license above.
  */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "portaudio.h"
#include "pa_asio.h"
#include "asiosdk/common/asio.h"
#include "../pa_record/pffft.h"

  /* #define SAMPLE_RATE  (17932) // Test failure to open with this value. */
#define SAMPLE_RATE  (44100)
#define FRAMES_PER_BUFFER (128)
#define NUM_SECONDS     (5)
#define NUM_CHANNELS    (8)
#define OUTPUT_CHANNELS (2)
/* #define DITHER_FLAG     (paDitherOff) */
#define DITHER_FLAG     (0) /**/
/** Set to 1 if you want to capture the recording to a file. */
#define WRITE_TO_FILE   (0)

/* Select sample format. */
#if 1
#define PA_SAMPLE_TYPE  paFloat32
typedef float SAMPLE;
#define SAMPLE_SILENCE  (0.0f)
#define PRINTF_S_FORMAT "%.8f"
#elif 1
#define PA_SAMPLE_TYPE  paInt16
typedef short SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#elif 0
#define PA_SAMPLE_TYPE  paInt8
typedef char SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#else
#define PA_SAMPLE_TYPE  paUInt8
typedef unsigned char SAMPLE;
#define SAMPLE_SILENCE  (128)
#define PRINTF_S_FORMAT "%d"
#endif

typedef struct
{
    int          frameIndex;  /* Index into sample array. */
    int          maxFrameIndex;
    SAMPLE*      recordedSamples;
    SAMPLE*      fftinputl;
    SAMPLE*      fftoutputl;
    SAMPLE*      fftinputr;
    SAMPLE*      fftoutputr;
    PFFFT_Setup* fftsetup;
    PFFFT_Setup* fftsetupcpx;
}
paTestData;

/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int recordCallback(const void* inputBuffer, void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData)
{
    paTestData* data = (paTestData*)userData;
    const SAMPLE* rptr = (const SAMPLE*)inputBuffer;
    SAMPLE* wptr = &data->recordedSamples[data->frameIndex * NUM_CHANNELS];
    SAMPLE* fftptr = wptr;
    long framesToCalc;
    long i;
    int finished;
    unsigned long framesLeft = data->maxFrameIndex - data->frameIndex;

    (void)outputBuffer; /* Prevent unused variable warnings. */
    (void)timeInfo;
    (void)statusFlags;
    (void)userData;

    if (framesLeft < framesPerBuffer)
    {
        framesToCalc = framesLeft;
        finished = paComplete;
    }
    else
    {
        framesToCalc = framesPerBuffer;
        finished = paContinue;
    }

    if (inputBuffer == NULL)
    {
        for (i = 0; i < framesToCalc; i++)
        {
            data->fftinputl[i] = SAMPLE_SILENCE;  /*to fft array*/

            *wptr++ = SAMPLE_SILENCE;  /* left */

            if (NUM_CHANNELS == 2) {
                data->fftinputr[i] = SAMPLE_SILENCE;  /*to fft array*/
                *wptr++ = SAMPLE_SILENCE;
            } /* right */
            else if (NUM_CHANNELS == 3) {
                *wptr++ = SAMPLE_SILENCE;
                *wptr++ = SAMPLE_SILENCE;
            }
            else if (NUM_CHANNELS == 4) {
                *wptr++ = SAMPLE_SILENCE;
                *wptr++ = SAMPLE_SILENCE;
                *wptr++ = SAMPLE_SILENCE;
            }
            else if (NUM_CHANNELS == 5) {
                *wptr++ = SAMPLE_SILENCE;
                *wptr++ = SAMPLE_SILENCE;
                *wptr++ = SAMPLE_SILENCE;
                *wptr++ = SAMPLE_SILENCE;
            }
            else if (NUM_CHANNELS == 6) {
                *wptr++ = SAMPLE_SILENCE;
                *wptr++ = SAMPLE_SILENCE;
                *wptr++ = SAMPLE_SILENCE;
                *wptr++ = SAMPLE_SILENCE;
                *wptr++ = SAMPLE_SILENCE;
            }
            else if (NUM_CHANNELS == 7) {
                data->fftinputr[i] = SAMPLE_SILENCE;  /*to fft array*/
                *wptr++ = SAMPLE_SILENCE;
                *wptr++ = SAMPLE_SILENCE;
                *wptr++ = SAMPLE_SILENCE;
                *wptr++ = SAMPLE_SILENCE;
                *wptr++ = SAMPLE_SILENCE;
                *wptr++ = SAMPLE_SILENCE;
            }
            else if (NUM_CHANNELS == 8) {
                data->fftinputr[i] = SAMPLE_SILENCE;  /*to fft array*/
                *wptr++ = SAMPLE_SILENCE;
                *wptr++ = SAMPLE_SILENCE;
                *wptr++ = SAMPLE_SILENCE;
                *wptr++ = SAMPLE_SILENCE;
                *wptr++ = SAMPLE_SILENCE;
                *wptr++ = SAMPLE_SILENCE;
                *wptr++ = SAMPLE_SILENCE;
            }
        }
    }
    else
    {
        for (i = 0; i < framesToCalc; i++)
        {

            data->fftinputl[i] = *rptr;   /*to fft array*/
            *wptr++ = *rptr++;  /* left */
            if (NUM_CHANNELS == 2) {
                data->fftinputr[i] = *rptr;   /*to fft array*/
                *wptr++ = *rptr++;
            }/* right */
            else if (NUM_CHANNELS == 3) {
                *wptr++ = *rptr++;
                *wptr++ = *rptr++;
            }
            else if (NUM_CHANNELS == 4) {
                *wptr++ = *rptr++;
                *wptr++ = *rptr++;
                *wptr++ = *rptr++;
            }
            else if (NUM_CHANNELS == 5) {
                *wptr++ = *rptr++;
                *wptr++ = *rptr++;
                *wptr++ = *rptr++;
                *wptr++ = *rptr++;
            }
            else if (NUM_CHANNELS == 6) {
                *wptr++ = *rptr++;
                *wptr++ = *rptr++;
                *wptr++ = *rptr++;
                *wptr++ = *rptr++;
                *wptr++ = *rptr++;
            }
            else if (NUM_CHANNELS == 7) {
                data->fftinputr[i] = *rptr;   /*to fft array*/
                *wptr++ = *rptr++;
                *wptr++ = *rptr++;
                *wptr++ = *rptr++;
                *wptr++ = *rptr++;
                *wptr++ = *rptr++;
                *wptr++ = *rptr++;
            }
            else if (NUM_CHANNELS == 8) {
                data->fftinputr[i] = *rptr;   /*to fft array*/
                *wptr++ = *rptr++;
                *wptr++ = *rptr++;
                *wptr++ = *rptr++;
                *wptr++ = *rptr++;
                *wptr++ = *rptr++;
                *wptr++ = *rptr++;
                *wptr++ = *rptr++;
            }
        }
    }

    //forward fft
    pffft_transform_ordered(data->fftsetup, data->fftinputl, data->fftoutputl, 0, PFFFT_FORWARD);
    pffft_transform_ordered(data->fftsetup, data->fftinputr, data->fftoutputr, 0, PFFFT_FORWARD);

    /* do fft processing */

    //backward fft
    pffft_transform_ordered(data->fftsetupcpx, data->fftoutputl, data->fftinputl, 0, PFFFT_BACKWARD);
    pffft_transform_ordered(data->fftsetupcpx, data->fftoutputr, data->fftinputr, 0, PFFFT_BACKWARD);


    data->frameIndex += framesToCalc; //  comment out this code and uncomment the next line of code to loop indefinitely

    //continuously looping
    //data->frameIndex = 0;

    return finished;
}

/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int playCallback(const void* inputBuffer, void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData)
{
    paTestData* data = (paTestData*)userData;
    SAMPLE* rptr = &data->recordedSamples[data->frameIndex * NUM_CHANNELS];
    SAMPLE* wptr = (SAMPLE*)outputBuffer;
    unsigned int i;
    int finished;
    unsigned int framesLeft = data->maxFrameIndex - data->frameIndex;

    (void)inputBuffer; /* Prevent unused variable warnings. */
    (void)timeInfo;
    (void)statusFlags;
    (void)userData;

    if (framesLeft < framesPerBuffer)
    {
        /* final buffer... */
        for (i = 0; i < framesLeft; i++)
        { 
            
            *wptr++ = *rptr++;  /* left */
            if (OUTPUT_CHANNELS == 2) *wptr++ = *rptr++;  /* right */
            rptr +=(NUM_CHANNELS-OUTPUT_CHANNELS);
        }
        for (; i < framesPerBuffer; i++)
        {
            *wptr++ = 0;  /* left */
            if (OUTPUT_CHANNELS == 2)
            *wptr++ = 0;  /* right */
        }
        data->frameIndex += framesLeft;
        finished = paComplete;
    }
    else
    {
        for (i = 0; i < framesPerBuffer;i++)
        {
            *wptr++ = *rptr++;  /* left */           
            if (OUTPUT_CHANNELS == 2) *wptr++ = *rptr++;  /* right */
            rptr +=(NUM_CHANNELS-OUTPUT_CHANNELS);
        }
        data->frameIndex += framesPerBuffer;
        finished = paContinue;
    }
    return finished;
}

/*******************************************************************/
int main(void);
int main(void)
{
    PaStreamParameters  inputParameters,
        outputParameters;
    PaStream* stream;
    PaError             err = paNoError;
    paTestData          data;
    int                 i;
    int                 totalFrames;
    int                 numSamples;
    int                 numBytes;
    SAMPLE              max, val;
    double              average;
    double              rms;

    printf("patest_record.c\n"); fflush(stdout);

    data.maxFrameIndex = totalFrames = NUM_SECONDS * SAMPLE_RATE; /* Record for a few seconds. */
    data.frameIndex = 0;
    numSamples = totalFrames * NUM_CHANNELS;
    numBytes = numSamples * sizeof(SAMPLE);
    data.recordedSamples = (SAMPLE*)malloc(numBytes); /* From now on, recordedSamples is initialised. */

    data.fftsetup = pffft_new_setup(FRAMES_PER_BUFFER, PFFFT_REAL);
    data.fftsetupcpx = pffft_new_setup(FRAMES_PER_BUFFER/2, PFFFT_COMPLEX);
    data.fftinputl = (SAMPLE*)pffft_aligned_malloc(sizeof(SAMPLE) * FRAMES_PER_BUFFER);
    data.fftoutputl = (SAMPLE*)pffft_aligned_malloc(sizeof(SAMPLE) * FRAMES_PER_BUFFER);
    data.fftinputr = (SAMPLE*)pffft_aligned_malloc(sizeof(SAMPLE) * FRAMES_PER_BUFFER);
    data.fftoutputr = (SAMPLE*)pffft_aligned_malloc(sizeof(SAMPLE) * FRAMES_PER_BUFFER);

    for (int i = 0; i < FRAMES_PER_BUFFER; i++)
    {   
        data.fftinputl[i] = 0.f;
        data.fftinputr[i] = 0.f;
        data.fftoutputl[i] = 0.f;
        data.fftoutputr[i] = 0.f;
    }

    if (data.recordedSamples == NULL)
    {
        printf("Could not allocate record array.\n");
        goto done;
    }
    for (i = 0; i < numSamples; i++) data.recordedSamples[i] = 0;

    err = Pa_Initialize();
    if (err != paNoError) goto done;

    inputParameters.device = 0; /* default input device, may need to change this depending on input device enumeration */
    if (inputParameters.device == paNoDevice) {
        fprintf(stderr, "Error: No default input device.\n");
        goto done;
    }
    inputParameters.channelCount = NUM_CHANNELS;
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    /* Record some audio. -------------------------------------------- */
    err = Pa_OpenStream(
        &stream,
        &inputParameters,
        NULL,                  /* &outputParameters, */
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        paClipOff,      /* we won't output out of range samples so don't bother clipping them */
        recordCallback,
        &data);
    if (err != paNoError) goto done;

    err = Pa_StartStream(stream);
    if (err != paNoError) goto done;
    printf("\n=== Now recording!! Please speak into the microphone. ===\n"); fflush(stdout);

    while ((err = Pa_IsStreamActive(stream)) == 1)
    {
        Pa_Sleep(1000);
        printf("index = %d\n", data.frameIndex); fflush(stdout);
    }
    if (err < 0) goto done;

    err = Pa_CloseStream(stream);
    if (err != paNoError) goto done;

    /* Measure maximum peak amplitude. */
    max = 0;
    average = 0.0;
    for (i = 0; i < numSamples; i++)
    {
        val = data.recordedSamples[i];
        if (val < 0) val = -val; /* ABS */
        if (val > max)
        {
            max = val;
        }
        average += val;
    }

    average = average / (double)numSamples;
    rms = sqrt(average);

    printf("sample max amplitude = %.8f\n", max);
    printf("sample average = %lf\n", average);
    printf("sample rms = %lf\n", rms);

    /* Write recorded data to a file. */
#if WRITE_TO_FILE
    {
        FILE* fid;
        fid = fopen("recorded.raw", "wb");
        if (fid == NULL)
        {
            printf("Could not open file.");
        }
        else
        {
            fwrite(data.recordedSamples, NUM_CHANNELS * sizeof(SAMPLE), totalFrames, fid);
            fclose(fid);
            printf("Wrote data to 'recorded.raw'\n");
        }
    }
#endif

    /* Playback recorded data.  -------------------------------------------- */
    data.frameIndex = 0;

    outputParameters.device = 9; /* default output device, may need to change this depending on output device enumeration */
    if (outputParameters.device == paNoDevice) {
        fprintf(stderr, "Error: No default output device.\n");
        goto done;
    }
    outputParameters.channelCount = OUTPUT_CHANNELS;
    outputParameters.sampleFormat = PA_SAMPLE_TYPE;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    printf("\n=== Now playing back. ===\n"); fflush(stdout);
    err = Pa_OpenStream(
        &stream,
        NULL, /* no input */
        &outputParameters,
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        paClipOff,      /* we won't output out of range samples so don't bother clipping them */
        playCallback,
        &data);
    if (err != paNoError) goto done;

    if (stream)
    {
        err = Pa_StartStream(stream);
        if (err != paNoError) goto done;

        printf("Waiting for playback to finish.\n"); fflush(stdout);

        while ((err = Pa_IsStreamActive(stream)) == 1) Pa_Sleep(100);
        if (err < 0) goto done;

        err = Pa_CloseStream(stream);
        if (err != paNoError) goto done;

        printf("Done.\n"); fflush(stdout);
    }

done:
    Pa_Terminate();
    if (data.recordedSamples)       /* Sure it is NULL or valid. */
        free(data.recordedSamples);

    if (data.fftinputl)
        pffft_aligned_free(data.fftinputl);
    if (data.fftoutputr)
        pffft_aligned_free(data.fftoutputr);
    if (data.fftinputr)
        pffft_aligned_free(data.fftinputr);
    if (data.fftoutputl)
        pffft_aligned_free(data.fftoutputl);
    if (data.fftsetup)
        pffft_destroy_setup(data.fftsetup);
    if (data.fftsetupcpx)
        pffft_destroy_setup(data.fftsetupcpx);

    if (err != paNoError)
    {
        fprintf(stderr, "An error occurred while using the portaudio stream\n");
        fprintf(stderr, "Error number: %d\n", err);
        fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
        err = 1;          /* Always return 0 or 1, but no other return codes. */
    }
    return err;
}


// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
