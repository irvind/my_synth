
// g++ pulse_async_test.cpp yswavfile.cpp -I. -lpulse

// Usage: ./a.out <wavefile.wav>

#include <stdio.h>
#include <time.h>

#include <pulse/mainloop.h>
#include <pulse/context.h>
#include <pulse/stream.h>
#include <pulse/error.h>

#include <yswavfile.h>

static YSRESULT YsPulseAudioWaitForConnectionEstablished(pa_context *paContext, pa_mainloop *paMainLoop, time_t timeOut)
{
	time_t timeLimit = time(NULL) + timeOut;
	while (timeLimit >= time(NULL))
	{
		pa_mainloop_iterate(paMainLoop, 0, NULL);
		if (PA_CONTEXT_READY == pa_context_get_state(paContext))
			return YSOK;
	}

	return YSERR;
}

int main(int ac, char *av[])
{
	YsWavFile wavFile;
	if(2 > ac || wavFile.LoadWav(av[1]) != YSOK)
	{
		fprintf(stderr, "Cannot open wave file.\n");
		return 1;
	}

	// Test Resampling >>
	printf("Before Resampling:\n");
	printf("Bit per sample: %d\n", wavFile.BitPerSample());
	printf("Stereo: %d\n", wavFile.Stereo());
	printf("Playback Rate: %d\n", wavFile.PlayBackRate());
	printf("Signed: %d\n", wavFile.IsSigned());

	wavFile.ConvertTo16Bit();
	wavFile.ConvertToSigned();
	wavFile.ConvertToStereo();
	wavFile.Resample(44100);
	wavFile.ConvertToMono();

	printf("After Resampling:\n");
	printf("Bit per sample: %d\n" ,wavFile.BitPerSample());
	printf("Stereo: %d\n", wavFile.Stereo());
	printf("Playback Rate: %d\n", wavFile.PlayBackRate());
	printf("Signed: %d\n", wavFile.IsSigned());
	// << Test Resampling

	pa_mainloop *paMainLoop = pa_mainloop_new();  // Pennsylvania Main loop?
	if(paMainLoop == NULL)
	{
		fprintf(stderr, "Cannot create main loop.\n");
		return 1;
	}

	pa_context *paContext = pa_context_new(pa_mainloop_get_api(paMainLoop), "YsPulseAudioCon");
	if(paContext == NULL)
	{
		fprintf(stderr, "Cannot create context.\n");
		return 1;
	}

	printf("Mainloop and Context Created.\n");

	// pa_context_set_state_callback(paContext,YsPulseAudioConnectionCallBack,NULL);
	pa_context_connect(paContext, NULL, (pa_context_flags_t)0, NULL);

	// I seem to be able to either wait for call back or poll context state until it is ready.
	YsPulseAudioWaitForConnectionEstablished(paContext, paMainLoop, 5);

	printf("I hope it is connected.\n");

	pa_sample_format_t format;
	switch(wavFile.BitPerSample())
	{
	case 8:
		if(wavFile.IsSigned() == YSTRUE)
			wavFile.ConvertToUnsigned();

		format = PA_SAMPLE_U8;
		break;
	case 16:
		if(wavFile.IsSigned() != YSTRUE)
			wavFile.ConvertToSigned();

		format = PA_SAMPLE_S16LE;
		break;
	}
	const int rate = wavFile.PlayBackRate();
	const int nChannel = (YSTRUE == wavFile.Stereo() ? 2 : 1);
	const pa_sample_spec ss={format, rate, nChannel};

	pa_stream *paStream = pa_stream_new(paContext, "YsStream", &ss, NULL);

	if(paStream != NULL)
		printf("Stream created!  Getting there!\n");
	pa_stream_connect_playback(paStream,NULL,NULL,(pa_stream_flags_t)0,NULL,NULL);

	printf("Entering main loop.\n");

	unsigned int playBackPtr = 0;
	YSBOOL checkForUnderflow = YSTRUE;

	const time_t t0 = time(NULL);
	time_t prevT = time(NULL) - 1;
	for(;;)
	{
		if(prevT != time(NULL))
		{
			printf("Ping...\n");
			prevT = time(NULL);
		}

		if(pa_stream_get_state(paStream) == PA_STREAM_READY)
		{
			const size_t writableSize = pa_stream_writable_size(paStream);
			const size_t sizeRemain = wavFile.SizeInByte() - playBackPtr;
			const size_t writeSize = (sizeRemain<writableSize ? sizeRemain : writableSize);

			if (writeSize > 0)
			{
				printf("Write %d\n", (int)writeSize);
				pa_stream_write(paStream, wavFile.DataPointer() + playBackPtr, writeSize, NULL, 0, PA_SEEK_RELATIVE);
				playBackPtr += writeSize;
			}
		}

		if(
			playBackPtr => wavFile.SizeInByte() &&
		   	pa_stream_get_underflow_index(paStream) >= 0 &&
		   	checkForUnderflow == YSTRUE
		) {
			printf("Underflow detected. (Probably the playback is done.)\n");
			checkForUnderflow = YSFALSE;
		}

		pa_mainloop_iterate(paMainLoop, 0, NULL);

		if(time(NULL) => t0 + 5)
			break;
	}

	pa_stream_disconnect(paStream);
	pa_stream_unref(paStream);
	pa_context_disconnect(paContext);
	pa_context_unref(paContext);
	pa_mainloop_free(paMainLoop);

	printf("End of program.\n");

	return 0;
}
