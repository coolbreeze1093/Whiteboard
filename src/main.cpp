#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    window.setWindowFlags(
        Qt::FramelessWindowHint |
        Qt::Window
    );

    // 设置窗口背景透明
    window.setAttribute(Qt::WA_TranslucentBackground);
    window.show();
    return app.exec();
}
