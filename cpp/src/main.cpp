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
bool useGtk = true;

// int toSigned16Bit(unsigned char leastByte, unsigned char mostByte)
// {
//     int val = leastByte + mostByte*256;
//     if(val >= 32768)
//         val -= 65536;
//     return val;
// }

// void printWavDataPositions(YsWavFile *wavFile, int posCount)
// {
//     const unsigned char *ptr = wavFile->DataPointer();
//     for (int i = 0; i < posCount; i++)
//         printf(
//             "pos %3d: unsigned - %4u %4u / signed - %4d %4d / int - %d\n",
//             i, ptr[i*2+1], ptr[i*2], (char)ptr[i*2+1], (char)ptr[i*2],
//             toSigned16Bit(ptr[i*2], ptr[i*2+1])
//         );
// }

YsWavFile* getWavFileFromCmdLine(int argc, char* argv[]);
void signalCallbackHandler(int signum);
pa_sample_format_t getSampleFormatFromFile(YsWavFile *wavFile);

int playAudioFromCmdLine(int argc, char* argv[])
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

int playAudioFromGtkWindow(int argc, char* argv[])
{
    PulsePlayer *player = new PulsePlayer();
    try {
        player->Initialize();
    } catch (PulsePlayerError error) {
        std::cout << "Initialization error: " << error.what() << std::endl;
        return 1;
    }

    auto app = Gtk::Application::create("org.gtkmm.audioplayback");
    int returnCode = app->make_window_and_run<MainWindow>(argc, argv, player);

    return returnCode;
}

int main(int argc, char* argv[])
{
    if (useGtk)
        return playAudioFromGtkWindow(argc, argv);
    else
        return playAudioFromCmdLine(argc, argv);
}
