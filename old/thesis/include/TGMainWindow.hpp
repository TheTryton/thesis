#pragma once

#include <QMainWindow>

#include <ui_TGMainWindow.h>

class TGMainWindow : public QMainWindow, private Ui::TGMainWindow
{
public:
    TGMainWindow();
};