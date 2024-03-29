#include <iostream>
#include <cstdio>
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
    stream = NULL;
    contextIsConnected = false;
    streamIsConnected = false;
    stopped = false;
}

PulsePlayer::~PulsePlayer()
{
    Free();
}

void PulsePlayer::Initialize()
{
    mainLoop = pa_threaded_mainloop_new();
    if (mainLoop == NULL)
        throw PulsePlayerError("Cannot create PulseAudio mainloop");

    if (pa_threaded_mainloop_start(mainLoop) != 0)
        throw PulsePlayerError("Cannot start mainLoop thread");

    context = pa_context_new(pa_threaded_mainloop_get_api(mainLoop), "PulseAudioDemoApp");
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
        pa_context_state_t state = pa_context_get_state(context);
        if (state == PA_CONTEXT_READY) {
            contextIsReady = true;
            break;
        }
    }

    if (!contextIsReady)
        throw PulsePlayerError("PulseAudio context is not ready for too long");
}

void PulsePlayer::CreateAndConnectStream(
    pa_sample_format_t sampleFormat,
    unsigned int playbackRate,
    bool isStereo
) {
    pa_sample_spec sampleSpec = {
        sampleFormat,
        playbackRate,
        (unsigned char)(isStereo ? 2 : 1)
    };

    stream = pa_stream_new(context, "DemoStream", &sampleSpec, NULL);
    if (stream == NULL)
        throw PulsePlayerError("Cannot create PulseAudio stream");

    pa_cvolume vol;
    pa_cvolume_set(&vol, 2, PA_VOLUME_NORM / 2);

    if (pa_stream_connect_playback(stream, NULL, NULL, (pa_stream_flags_t)0, &vol, NULL) != 0)
        throw PulsePlayerError("Cannot connect PulseAudio stream to the sink");

    streamIsConnected = true;
}

void PulsePlayer::Play(
    pa_sample_format_t sampleFormat,
    unsigned int playbackRate,
    bool isStereo,
    unsigned int sizeInBytes,
    const unsigned char *dataPointer
) {
    CreateAndConnectStream(sampleFormat, playbackRate, isStereo);
    PlaybackLoop(sizeInBytes, dataPointer);
}

void PulsePlayer::PlaybackLoop(unsigned int sizeInBytes, const unsigned char *dataPointer)
{
    unsigned int playbackPtr = 0;
    double progress = -1;
    while (!stopped) {
        double newProgress = ((double)playbackPtr / (double)sizeInBytes) * 100;
        if (newProgress > progress + 1) {
            progress = newProgress;
            printf("progress: %.0lf%%\n", progress);
        }
        if (playbackPtr >= sizeInBytes)
            break;

        pa_threaded_mainloop_lock(mainLoop);

        pa_stream_state_t state = pa_stream_get_state(stream);

        if (state == PA_STREAM_READY) {
            const size_t writableSize = pa_stream_writable_size(stream);

            const size_t sizeRemain = sizeInBytes - playbackPtr;
            const size_t writeSize = (sizeRemain < writableSize ? sizeRemain : writableSize);
            if (writeSize > 0) {
                pa_stream_write(
                    stream,
                    dataPointer + playbackPtr,
                    writeSize,
                    NULL,
                    0,
                    PA_SEEK_RELATIVE
                );
                playbackPtr += writeSize;
            }
        }

        pa_threaded_mainloop_unlock(mainLoop);
    }
}

void PulsePlayer::PlayStart(
    pa_sample_format_t sampleFormat,
    unsigned int playbackRate,
    bool isStereo,
    unsigned int sizeInBytes,
    const unsigned char *dataPointer
) {
    CreateAndConnectStream(sampleFormat, playbackRate, isStereo);
    playbackByteIndex = 0;
    playbackSizeInBytes = sizeInBytes;
    playbackDataPointer = dataPointer;
}

bool PulsePlayer::PlayTick()
{
    if (stopped || (playbackByteIndex >= playbackSizeInBytes)) {
        return true;
    }

    pa_threaded_mainloop_lock(mainLoop);

    pa_stream_state_t state = pa_stream_get_state(stream);
    if (state == PA_STREAM_READY) {
        const size_t writableSize = pa_stream_writable_size(stream);

        const size_t sizeRemain = playbackSizeInBytes - playbackByteIndex;
        const size_t writeSize = (sizeRemain < writableSize ? sizeRemain : writableSize);
        if (writeSize > 0) {
            pa_stream_write(
                stream,
                playbackDataPointer + playbackByteIndex,
                writeSize,
                NULL,
                0,
                PA_SEEK_RELATIVE
            );
            playbackDataPointer += writeSize;
        }
    }

    pa_threaded_mainloop_unlock(mainLoop);
    return false;
}

void PulsePlayer::Stop()
{
    stopped = true;
}

void PulsePlayer::Free()
{
    if (streamIsConnected)
        pa_stream_disconnect(stream);
    if (stream != NULL)
        pa_stream_unref(stream);
    if (context != NULL && contextIsConnected)
        pa_context_disconnect(context);
    if (context != NULL)
        pa_context_unref(context);
    if (mainLoop != NULL) {
        pa_threaded_mainloop_stop(mainLoop);
        pa_threaded_mainloop_free(mainLoop);
    }

    stream = NULL;
    context = NULL;
    mainLoop = NULL;
    contextIsConnected = false;
    streamIsConnected = false;
}

pa_threaded_mainloop* PulsePlayer::getMainLoop()
{
    return mainLoop;
}

pa_context* PulsePlayer::getContext()
{
    return context;
}
