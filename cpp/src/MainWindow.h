#pragma once

#include <gtkmm/button.h>
#include <gtkmm/window.h>

class MainWindow : public Gtk::Window
{
protected:
    Gtk::Button mButton;

    void onButtonClicked();

public:
    MainWindow();
    ~MainWindow() override;
};
