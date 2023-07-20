#include <iostream>
#include <ctime>
#include "PulsePlayer.h"

PulsePlayerError::PulsePlayerError(const std::string& message) : message(message) {}

const char* PulsePlayerError::what() 
{
    return message.c_str();
}

PulsePlayer::PulsePlayer()
{
    mainLoop = NULL;
    context = NULL;
    contextIsConnected = false;
}

PulsePlayer::~PulsePlayer()
{
    Free();
}

void PulsePlayer::Initialize()
{
    std::cout << "Initialize" << std::endl;

    mainLoop = pa_mainloop_new();
    if (mainLoop == NULL)
        throw PulsePlayerError("Cannot create PulseAudio mainloop");

    context = pa_context_new(pa_mainloop_get_api(mainLoop), "PulseAudioDemoApp");
    if (context == NULL)
        throw PulsePlayerError("Cannot create PulseAudio context");

    if (pa_context_connect(context, NULL, (pa_context_flags_t)0, NULL) < 0)
        throw PulsePlayerError("Cannot connect context to default server");

    contextIsConnected = true;
    WaitForContextConnect();
}

void PulsePlayer::WaitForContextConnect()
{
    bool contextIsReady = false;
    time_t readyTimeLimit = time(NULL) + 30;

    while (time(NULL) <= readyTimeLimit) {
        pa_mainloop_iterate(mainLoop, 0, NULL);
        pa_context_state_t state = pa_context_get_state(context);
        if (state == PA_CONTEXT_READY) {
            contextIsReady = true;
            break;
        }
    }

    if (!contextIsReady)
        throw PulsePlayerError("PulseAudio context is not ready for too long");
}

void PulsePlayer::Play()
{
    std::cout << "Play" << std::endl;
}

void PulsePlayer::Free()
{
    if (context != NULL && contextIsConnected)
        pa_context_disconnect(context);
    if (context != NULL)
        pa_context_unref(context);
    if (mainLoop != NULL)
        pa_mainloop_free(mainLoop);

    context = NULL;
    mainLoop = NULL;
    contextIsConnected = false;
}

pa_mainloop* PulsePlayer::getMainLoop()
{
    return mainLoop;
}
pa_context* PulsePlayer::getContext()
{
    return context;
}
