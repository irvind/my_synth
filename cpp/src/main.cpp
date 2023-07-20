#include <iostream>
#include <cstdio>
#include <ctime>
#include <cassert>

#include <pulse/mainloop.h>
#include <pulse/context.h>
#include <pulse/stream.h>

#include "yswavfile.h"
#include "PulsePlayer.h"

int toSigned16Bit(unsigned char leastByte, unsigned char mostByte)
{
    int val = leastByte + mostByte*256;
    if(val >= 32768)
        val -= 65536;

    return val;
}

void printWavDataPositions(YsWavFile *wavFile, int posCount)
{
    const unsigned char *ptr = wavFile->DataPointer();

    for (int i = 0; i < posCount; i++)
        printf(
            "pos %3d: unsigned - %4u %4u / signed - %4d %4d / int - %d\n",
            i, ptr[i*2+1], ptr[i*2], (char)ptr[i*2+1], (char)ptr[i*2],
            toSigned16Bit(ptr[i*2], ptr[i*2+1])
        );
}

void printUnsignedSignedValues()
{
    unsigned char chars[6] = {255, 254, 128, 127, 126, 0};
    for (int i = 0; i < 6; i++)
        printf("signed: %d / unsigned: %u\n", (char)chars[i], chars[i]);

    unsigned char ch1 = 255;
    char ch2 = ch1;
    unsigned char ch3 = ch2; 
    printf("signed ch1 d: %d / unsigned ch2 d: %d\n", ch1, ch2);
    printf("unsigned ch3 d: %d\n", ch3);

    unsigned char ch4 = 128;
    int num = ch4;  
    printf("num - %d\n", num);

    unsigned char ch5 = 128;
    char ch6 = ch5;
    int num2 = ch6;
    // printf("ch6 - %d\n", ch6);
    printf("num2 - %d\n", num2);
}

YsWavFile* getWavFileFromCmdLine(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Audio file is required" << std::endl;
        return NULL;
    }

    YsWavFile *wavFile = new YsWavFile();
    YSRESULT loadResult = wavFile->LoadWav(argv[1]);
    if(loadResult != YSOK)
    {
        delete wavFile;
        std::cout << "Error occured during wav file parsing" << std::endl;
        return NULL;
    }

    return wavFile;
}

void freeResources(
    PulsePlayer* player,
    pa_stream* stream,
    YsWavFile* wavFile,
    bool disconnectStream
)
{
    if (wavFile == NULL)
        delete wavFile;
    if (disconnectStream)
        pa_stream_disconnect(stream);
    if (stream != NULL)
        pa_stream_unref(stream);
    player->Free();
}

int main(int argc, char* argv[])
{
    YsWavFile *wavFile = getWavFileFromCmdLine(argc, argv);
    if (wavFile == NULL)
        return 1;

    std::cout << "WAV file has been parsed" << std::endl;

    PulsePlayer player;
    try {
        player.Initialize();
    } catch (PulsePlayerError error) {
        std::cout << "Initialization error: " << error.what() << std::endl;
        return 1;
    }

    pa_mainloop* mainLoop = player.getMainLoop();
    pa_context* context = player.getContext();

    pa_sample_format_t format;
    switch(wavFile->BitPerSample()) {
    case 8:
        format = PA_SAMPLE_U8;
        break;
    case 16:
        format = PA_SAMPLE_S16LE;
        break;
    default:
        std::cout << "Bit size must be 8 or 16" << std::endl;
        freeResources(&player, NULL, wavFile, false);
        return 1;
    }

     pa_sample_spec sampleSpec = {
        format,
        wavFile->PlayBackRate(),                                // Rate, Ex: 44Hz
        (unsigned char)(wavFile->Stereo() == YSTRUE ? 2 : 1)    // Channels, Ex: 1 or 2
    };

    pa_stream *stream = pa_stream_new(context, "DemoStream", &sampleSpec, NULL);
    if (stream == NULL) {
        std::cout << "Cannot create PulseAudio stream" << std::endl;
        freeResources(&player, NULL, wavFile, false);
        return 1;
    }

    if (pa_stream_connect_playback(stream, NULL, NULL, (pa_stream_flags_t)0, NULL, NULL) != 0) {
        std::cout << "Cannot connect PulseAudio stream to the sink" << std::endl;
        freeResources(&player, stream, wavFile, false);
        return 1;
    }

    // const time_t startTime = time(NULL);
    // bool mainLoopFirst = true;
    // pa_stream_state_t prevState;
    // int logIdx = 0;
    unsigned int playbackPtr = 0;
    double progress = -1;

    while (1) {
        // if (time(NULL) >= startTime + 30)
        //     break;

        double newProgress = ((double)playbackPtr / (double)wavFile->SizeInByte()) * 100;
        if (newProgress > progress + 1) {
            progress = newProgress;
            printf("progress: %.0lf%%\n", progress);
        }
        if (playbackPtr >= wavFile->SizeInByte())
            break;

        pa_stream_state_t state = pa_stream_get_state(stream);

        // assert(PA_STREAM_IS_GOOD(state));
        // if (mainLoopFirst || state != prevState)
        //    std::cout << "state: " << state << std::endl;
        // mainLoopFirst = false;
        // prevState = state;

        if (state == PA_STREAM_READY) {
            const size_t writableSize = pa_stream_writable_size(stream);
            // if (writableSize != 0) {
            //     std::cout << logIdx << ", writableSize: " << writableSize << std::endl;
            //     logIdx++;
            // }

            const size_t sizeRemain = wavFile->SizeInByte() - playbackPtr;
            const size_t writeSize = (sizeRemain < writableSize ? sizeRemain : writableSize);
            if (writeSize > 0) {
                pa_stream_write(
                    stream,
                    wavFile->DataPointer() + playbackPtr,
                    writeSize,
                    NULL,
                    0,
                    PA_SEEK_RELATIVE
                );
                playbackPtr += writeSize;
            }
        }

        pa_mainloop_iterate(mainLoop, 0, NULL);
    }

    freeResources(&player, stream, wavFile, true);

    return 0;
}
