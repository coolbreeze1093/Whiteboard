#pragma once

#include <QtWidgets/QWidget>
#include "ui_draw.h"

class draw : public QWidget
{
    Q_OBJECT

public:
    draw(QWidget *parent = nullptr);
    ~draw();

private:
    Ui::drawClass ui;
};

