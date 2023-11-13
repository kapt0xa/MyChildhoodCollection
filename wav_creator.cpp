#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <stdint.h>
#include <math.h>

//===========================================================================

union _2bytes
{
    uint8_t b[2];
    int8_t c[2];
    uint16_t u;
    int16_t i;
};

union _4bytes
{
    _2bytes s[2];
    uint8_t b[4];
    int8_t c[4];
    uint32_t u;
    int32_t i;
    float f;
};

struct _4bytes_array
{
    uint32_t size;
    _4bytes * ptr;
};

struct wav_header_inf
{
    uint8_t chunkId [4];//="RIFF"
    uint32_t chunkSize;//size of other part of file, sizeof file - 8
    uint8_t format [4];//="WAVE"
    uint8_t subchunk1Id [4];//="fmt "
    uint32_t subchunk1size;//=16
     int16_t audioFormat;//=1?
    uint16_t numChannels;//std-1,2...
    uint32_t samplerate;//std-8000, 44100
    uint32_t byteRate;//bytes per second
    uint16_t blockAlign;//bytes per sample, including all channels
    uint16_t bitsPerSample;//sound depth
    uint8_t subchunk2Id[4];//="data"
    uint32_t subchunk2size;//size of sound data
};

union wav_header
{
    wav_header_inf i;
    uint8_t b[sizeof(wav_header_inf)];
    _2bytes s[sizeof(wav_header_inf)/2];
    _4bytes u[sizeof(wav_header_inf)/4];
};

//=============================================================================

wav_header WAVheader = {};
unsigned headersize = sizeof(wav_header);
const uint32_t STDsamplerate[6] = {8000, 16000, 32000, 11025, 22050, 44100};
uint32_t obertones = 20,/*<-look here!!!*/ samples = 0;
const uint8_t STDsoundDepth[3] = {8, 16, 32};
uint8_t STDsamplerate_t = 5;//5-max
uint8_t STDsoundDepth_t = 2;//2-max
uint16_t numChannels = 1;
int i = 0, j = 0, k = 0;
float t = 1, hz = 30,/*<-and here->*/ amplitude = 0.1, distortion = 0.08, turn = 6.283185307179586;
const char * soundaddress = "sound.wav";
FILE * audiofile = 0;
_4bytes buf = {0};
_4bytes_array x = {0, 0};

//=============================================================================

int wav_create_interface();
int wav_create_build ();
int wav_create_do ();

//=============================================================================

int main()
{
    printf("\nthe program started!\n");
    buf.i = wav_create_build ();
    printf ("\nbuild done\n");
    if(buf.i != 0) return buf.i;
    buf.i = wav_create_do();
    printf("\nfile created!\n\npress [ESC]");
    while (getch() != '\e');
    return buf.i;
}

//==============================================================================

int wav_create_do ()
{
    if(audiofile != 0)
    {
        printf("\nerror! audiofile != 0\n");
        return 1;
    }
    else
    {
        audiofile = fopen(soundaddress, "wb");
        if(audiofile == 0)
        {
            printf("\nerror! can't open %s\n", soundaddress);
            return -1;
        }
        else
        {
            for (i = 0; i < headersize; i++)
            {
                fputc(WAVheader.b[i], audiofile);
            }
            if(x.ptr != 0) x.ptr[0].f = 0;
            else
            {
                printf ("\nerrorx.ptr is free\n");
                return 2;
            }
            for (i = 0; i < samples; i++)
            {
                buf = x.ptr[i];
                for(j = 0; j < numChannels; j++)
                {
                    for(k = 0; j < 4; j++) fputc(buf.b[j], audiofile);
                    buf.f = 0;
                }
            }
        }
    }
    return 0;
}

int wav_create_build ()
{
    for(i = 0; i < 4; i++) WAVheader.i.chunkId[i]="RIFF"[i];
    for(i = 0; i < 4; i++) WAVheader.i.format[i]="WAVE"[i];
    for(i = 0; i < 4; i++) WAVheader.i.subchunk1Id[i]="fmt "[i];
    for(i = 0; i < 4; i++) WAVheader.i.subchunk2Id[i]="data"[i];
    WAVheader.i.subchunk1size = 16;
    WAVheader.i.audioFormat = 3;
    WAVheader.i.numChannels = numChannels;
    WAVheader.i.samplerate = STDsamplerate[STDsamplerate_t];
    WAVheader.i.blockAlign = numChannels*STDsoundDepth[STDsoundDepth_t]/8;
    WAVheader.i.byteRate = STDsamplerate[STDsamplerate_t] * WAVheader.i.blockAlign;
    WAVheader.i.bitsPerSample = STDsoundDepth[STDsoundDepth_t];
    WAVheader.i.subchunk2size = 4 * ((uint32_t(t * WAVheader.i.byteRate)) / 4);
    WAVheader.i.chunkSize = WAVheader.i.subchunk2size + headersize - 8;
    if (x.ptr != 0)
    {
        printf("\nerror! x.ptr != 0\n");
        return 1;
    }
    else
    {
        samples = t * WAVheader.i.samplerate;
        x.size = samples;
        x.ptr = (_4bytes *)malloc(x.size * sizeof(_4bytes));
        if (x.ptr == 0)
        {
            printf("\nerror! malloc failed, x.ptr is still free\n");
            return -1;
        }
        else
        {
            for(i = 0; i < x.size; i++) x.ptr[i].f = 0;
            if (obertones == 0 || WAVheader.i.samplerate == 0)
            {
                printf ("\nerror! divide by zerro!\nobertones = % d\nsamplerate = %d\n", obertones, WAVheader.i.samplerate);
                return 2;
            }
            for(j = 1; j <= obertones; j++) for(i = 0; i < samples; i++)
            {
                x.ptr[i].f += sin((turn*i*obertones*hz)/WAVheader.i.samplerate)*(amplitude/(j*j));
            }
            j = 0;
            k = 0;
            for(i = 0; i < samples; i++)
            {
                if(x.ptr[i].f > distortion) {x.ptr[i].f = distortion; j++;}
                if(x.ptr[i].f <-distortion) {x.ptr[i].f =-distortion; k++;}
            }
            printf("\nj = %d, k = %d\n", j, k);
        }
    }
    return 0;
}
