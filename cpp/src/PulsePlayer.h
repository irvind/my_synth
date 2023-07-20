#pragma once

#include <iostream>
#include <pulse/mainloop.h>
#include <pulse/context.h>

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
    bool contextIsConnected;

    void WaitForContextConnect();

public:
    PulsePlayer();
    ~PulsePlayer();

    void Initialize();
    void Play();
    void Free();

    pa_mainloop* getMainLoop();
    pa_context* getContext();
};
