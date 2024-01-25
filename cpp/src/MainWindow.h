#pragma once

#include <gtkmm.h>
#include <gtkmm/button.h>
#include <gtkmm/window.h>
#include <gtkmm/application.h>

#include "yswavfile.h"
#include "PulsePlayer.h"

class MainWindow : public Gtk::Window
{
protected:
    Gtk::Button mButton;
    bool isPlaying;
    YsWavFile *wavFile;
    PulsePlayer *player;

    void onButtonClicked();
    bool onIdle();

public:
    MainWindow(PulsePlayer *inPlayer);
    ~MainWindow() override;
};
