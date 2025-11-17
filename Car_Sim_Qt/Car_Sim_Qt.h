#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Car_Sim_Qt.h"

class Car_Sim_Qt : public QMainWindow
{
    Q_OBJECT

public:
    Car_Sim_Qt(QWidget *parent = nullptr);
    ~Car_Sim_Qt();

private:
    Ui::Car_Sim_QtClass ui;
};

