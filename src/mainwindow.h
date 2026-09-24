#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QColor>

class WhiteboardView;
class QAction;
class QActionGroup;
class QToolButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void choosePenColor();
    void onPenWidthChanged(int w);
    void onToolChanged(QAction *action);

private:
    void setupToolBar();
    void updateColorButtonIcon();

    WhiteboardView *m_view;
    QAction *m_penAction;
    QAction *m_eraserAction;
    QAction *m_textAction;
    QActionGroup *m_toolGroup;
    QToolButton *m_colorButton;
    QColor m_currentColor = Qt::black;
};

#endif // MAINWINDOW_H
