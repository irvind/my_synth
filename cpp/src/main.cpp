#include <iostream>
#include <cstdio>
#include <ctime>
#include <cassert>
#include <csignal>

#include <pulse/mainloop.h>
#include <pulse/context.h>
#include <pulse/stream.h>

#include <gtkmm/application.h>

#include "yswavfile.h"
#include "PulsePlayer.h"
#include "MainWindow.h"

PulsePlayer *gPlayer = NULL;

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

YsWavFile* getWavFileFromCmdLine(int argc, char* argv[]);
void signalCallbackHandler(int signum);
pa_sample_format_t getSampleFormatFromFile(YsWavFile *wavFile);

int runAudioPlayback(int argc, char* argv[])
{
    YsWavFile *wavFile = getWavFileFromCmdLine(argc, argv);
    if (wavFile == NULL)
        return 1;

    std::cout << "WAV file has been parsed" << std::endl;

    signal(SIGINT, signalCallbackHandler);

    PulsePlayer *player = new PulsePlayer();
    gPlayer = player;
    try {
        player->Initialize();
    } catch (PulsePlayerError error) {
        std::cout << "Initialization error: " << error.what() << std::endl;
        return 1;
    }

    pa_sample_format_t format = getSampleFormatFromFile(wavFile);
    if (format == PA_SAMPLE_INVALID) {
        std::cout << "Wav file bit format is invalid" << std::endl;
        return 1;
    }

    player->Play(
        format,
        wavFile->PlayBackRate(),
        wavFile->Stereo() == YSTRUE,
        wavFile->SizeInByte(),
        wavFile->DataPointer()
    );

    delete player;

    return 0;
}

YsWavFile* getWavFileFromCmdLine(int argc, char* argv[])
{
    if (argc < 2) {
        std::cout << "Audio file is required" << std::endl;
        return NULL;
    }

    YsWavFile *wavFile = new YsWavFile();
    YSRESULT loadResult = wavFile->LoadWav(argv[1]);
    if (loadResult != YSOK) {
        delete wavFile;
        std::cout << "Error occured during wav file parsing" << std::endl;
        return NULL;
    }

    return wavFile;
}

void signalCallbackHandler(int signum)
{
    if (gPlayer != NULL)
        gPlayer->Stop();
}

pa_sample_format_t getSampleFormatFromFile(YsWavFile *wavFile)
{
    pa_sample_format_t format;
    switch(wavFile->BitPerSample()) {
    case 8:
        format = PA_SAMPLE_U8;
        break;
    case 16:
        format = PA_SAMPLE_S16LE;
        break;
    default:
        format = PA_SAMPLE_INVALID;
    }

    return format;
}

int main(int argc, char* argv[])
{
    auto app = Gtk::Application::create("org.gtkmm.audioplayback");
    return app->make_window_and_run<MainWindow>(argc, argv);

    // int returnCode = runAudioPlayback(argc, argv);
    // return returnCode;
}
