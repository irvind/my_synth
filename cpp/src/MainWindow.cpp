#include <iostream>
#include "MainWindow.h"

MainWindow::MainWindow()
: mButton("Play")
{
    set_title("PulseAudio playback");
    // set_default_size(800, 450);

    mButton.set_margin(10);
    mButton.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onButtonClicked));

    set_child(mButton);
}

MainWindow::~MainWindow()
{ 
}

void MainWindow::onButtonClicked()
{
  std::cout << "MainWindow ping" << std::endl;
}
