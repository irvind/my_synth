#include <iostream>
#include <glibmm/main.h>

#include "MainWindow.h"
#include "PulsePlayer.h"

extern PulsePlayer *gPlayer;

MainWindow::MainWindow()
: mButton("Play")
{
    isPlaying = false;
    wavFile = NULL;
    set_title("PulseAudio playback");
    // set_default_size(800, 450);

    mButton.set_margin(10);
    mButton.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onButtonClicked));

    set_child(mButton);
}

MainWindow::~MainWindow()
{ 
    std::cout << "MainWindow destructor" << std::endl;
    if (wavFile != NULL)
        delete wavFile;
}

void MainWindow::onButtonClicked()
{
    if (isPlaying || gPlayer == NULL)
        return;

    wavFile = new YsWavFile();
    YSRESULT loadResult = wavFile->LoadWav("song.wav");
    if (loadResult != YSOK) {
        delete wavFile;
        std::cout << "Error occured during wav file parsing" << std::endl;
        return;
    }

    gPlayer->PlayStart(
        wavFile->SampleFormat(),
        wavFile->PlayBackRate(),
        wavFile->Stereo() == YSTRUE,
        wavFile->SizeInByte(),
        wavFile->DataPointer()
    );
    isPlaying = true;
    Glib::signal_idle().connect(sigc::mem_fun(*this, &MainWindow::onIdle));
}

bool MainWindow::onIdle()
{
    if (!isPlaying)
        return false;

    bool ended = gPlayer->PlayTick();
    if (ended) {
        delete wavFile;
        wavFile = NULL;
        isPlaying = false;
    }
        
    return !ended;
}
