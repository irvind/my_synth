#pragma once

#include <iostream>
#include <pulse/mainloop.h>
#include <pulse/context.h>
#include <pulse/stream.h>

class PulsePlayerError : public std::exception
{
private:
    std::string message;

public:
    PulsePlayerError(const std::string& message);
    const char* what();
};

class PulsePlayer
{
private:
    pa_mainloop* mainLoop;
    pa_context* context;
    pa_stream* stream;
    bool contextIsConnected;
    bool streamIsConnected;

    void WaitForContextConnect();
    void PlaybackLoop(unsigned int sizeInBytes, const unsigned char *dataPointer);

public:
    PulsePlayer();
    ~PulsePlayer();

    void Initialize();
    void Play(pa_sample_format_t sampleFormat, unsigned int playbackRate, bool isStereo,
        unsigned int sizeInBytes, const unsigned char *dataPointer);
    void Free();

    pa_mainloop* getMainLoop();
    pa_context* getContext();
};
