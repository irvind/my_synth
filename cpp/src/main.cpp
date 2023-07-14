#include <iostream>
#include <cstdio>
// #include <cassert>

#include <pulse/mainloop.h>
#include <pulse/context.h>

#include "yswavfile.h"

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

int main(int argc, char* argv[])
{
    YsWavFile *wavFile = getWavFileFromCmdLine(argc, argv);
    if (wavFile == NULL)
        return 1;

    std::cout << "WAV file has been parsed" << std::endl;

    // printWavDataPositions(&wavFile, 500);
    // printUnsignedSignedValues();

    pa_mainloop *mainLoop = pa_mainloop_new();
    if (!mainLoop)
    {
        std::cout << "Cannot create PulseAudio mainloop" << std::endl;
        delete wavFile;
        return 1;
    }

    pa_context *context = pa_context_new(pa_mainloop_get_api(mainLoop), "PulseAudioDemoApp");
    if (!context)
    {
        std::cout << "Cannot create PulseAudio context" << std::endl;
        pa_mainloop_free(mainLoop);
        delete wavFile;
        return 1;
    }

    // TODO

    pa_context_unref(context);
    pa_mainloop_free(mainLoop);
    delete wavFile;

    return 0;
}
