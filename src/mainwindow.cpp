#include "mainwindow.h"
#include "whiteboardview.h"

#include <QToolBar>
#include <QAction>
#include <QActionGroup>
#include <QSpinBox>
#include <QLabel>
#include <QToolButton>
#include <QColorDialog>
#include <QUndoStack>
#include <QPixmap>
#include <QPainter>
#include <QIcon>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{

    m_view = new WhiteboardView(QSize(1600, 1000), this);
    setCentralWidget(m_view);
    resize(1650, 1050);
    setWindowTitle(tr(u8"白板 (Qt5 QGraphicsView 对象存储白板)"));

    setupToolBar();
}

void MainWindow::setupToolBar()
{
    auto *toolBar = addToolBar(tr(u8"工具"));
    toolBar->setMovable(false);

    m_toolGroup = new QActionGroup(this);
    m_toolGroup->setExclusive(true);

    m_penAction = toolBar->addAction(tr(u8"画笔"));
    m_penAction->setCheckable(true);
    m_penAction->setChecked(true);
    m_toolGroup->addAction(m_penAction);

    m_eraserAction = toolBar->addAction(tr(u8"橡皮擦"));
    m_eraserAction->setCheckable(true);
    m_eraserAction->setToolTip(tr(u8"碰到哪个对象（一笔画/一段文字）就把它整个删除"));
    m_toolGroup->addAction(m_eraserAction);

    m_textAction = toolBar->addAction(tr(u8"文字"));
    m_textAction->setCheckable(true);
    m_textAction->setToolTip(tr(u8"点空白处新建文字；点已有文字可拖动；双击编辑；选中后按 Delete 删除"));
    m_toolGroup->addAction(m_textAction);

    connect(m_toolGroup, &QActionGroup::triggered, this, &MainWindow::onToolChanged);

    toolBar->addSeparator();

    m_colorButton = new QToolButton(this);
    m_colorButton->setToolTip(tr(u8"选择颜色"));
    toolBar->addWidget(m_colorButton);
    connect(m_colorButton, &QToolButton::clicked, this, &MainWindow::choosePenColor);
    updateColorButtonIcon();

    toolBar->addSeparator();

    toolBar->addWidget(new QLabel(tr(u8" 画笔粗细："), this));
    auto *penWidthSpin = new QSpinBox(this);
    penWidthSpin->setRange(1, 60);
    penWidthSpin->setValue(4);
    toolBar->addWidget(penWidthSpin);
    connect(penWidthSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onPenWidthChanged);

    toolBar->addWidget(new QLabel(tr(u8" 橡皮擦范围："), this));
    auto *eraserWidthSpin = new QSpinBox(this);
    eraserWidthSpin->setRange(4, 150);
    eraserWidthSpin->setValue(24);
    toolBar->addWidget(eraserWidthSpin);
    connect(eraserWidthSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            m_view, &WhiteboardView::setEraserWidth);

    toolBar->addSeparator();

    QAction *undoAction = toolBar->addAction(tr(u8"撤销"));
    undoAction->setShortcut(QKeySequence::Undo);
    connect(undoAction, &QAction::triggered, m_view->undoStack(), &QUndoStack::undo);

    QAction *clearAction = toolBar->addAction(tr(u8"清空"));
    connect(clearAction, &QAction::triggered, this, [this]()
            {
        auto btn = QMessageBox::question(this, tr(u8"清空画布"), tr(u8"确定要清空整个白板吗？（此操作可撤销）"));
        if (btn == QMessageBox::Yes)
            m_view->clearCanvas(); });

    QAction *saveAction = toolBar->addAction(tr(u8"保存"));
    connect(saveAction, &QAction::triggered, this, [this]()
            {
                QImage image(m_view->viewport()->size(),
                             QImage::Format_ARGB32);

                image.fill(Qt::transparent);

                QPainter painter(&image);
                m_view->render(&painter);
                painter.end();
                image.save("whiteboard.png");
            });

    QAction *closeAction = toolBar->addAction(tr(u8"关闭"));
    connect(closeAction, &QAction::triggered, this, [this]()
            {
        MainWindow::close(); });

    toolBar->setStyleSheet(
        "QToolBar {"
        "    background-color: #FFFFFF;"
        "    border: none;"
        "}");

    // 初始化默认工具与默认粗细
    onToolChanged(m_penAction);
    onPenWidthChanged(penWidthSpin->value());
}

void MainWindow::onToolChanged(QAction *action)
{
    if (action == m_penAction)
        m_view->setTool(WhiteboardView::Tool::Pen);
    else if (action == m_eraserAction)
        m_view->setTool(WhiteboardView::Tool::Eraser);
    else if (action == m_textAction)
        m_view->setTool(WhiteboardView::Tool::Text);
}

void MainWindow::onPenWidthChanged(int w)
{
    m_view->setPenWidth(w);
}

void MainWindow::choosePenColor()
{
    QColor c = QColorDialog::getColor(m_currentColor, this, tr(u8"选择颜色"));
    if (!c.isValid())
        return;
    m_currentColor = c;
    m_view->setPenColor(c);
    updateColorButtonIcon();
}

void MainWindow::updateColorButtonIcon()
{
    QPixmap pm(20, 20);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setBrush(m_currentColor);
    p.setPen(Qt::darkGray);
    p.drawRect(1, 1, 17, 17);
    p.end();
    m_colorButton->setIcon(QIcon(pm));
}
