#include "whiteboardview.h"
#include "strokeitem.h"
#include "movabletextitem.h"
#include "objectcommands.h"
#include "textcommands.h"

#include <QGraphicsScene>
#include <QUndoStack>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QInputDialog>
#include <QPainter>
#include <QPainterPath>

WhiteboardView::WhiteboardView(const QSize &canvasSize, QWidget *parent)
    : QGraphicsView(parent)
{
    auto *newScene = new QGraphicsScene(this);
    newScene->setSceneRect(0, 0, canvasSize.width(), canvasSize.height());
    newScene->setBackgroundBrush(Qt::white);
    setScene(newScene);

    m_undoStack = new QUndoStack(this);
    m_textFont = QFont(QStringLiteral("SansSerif"), 24);

    setRenderHint(QPainter::Antialiasing, true);
    setDragMode(QGraphicsView::NoDrag);
    setFocusPolicy(Qt::StrongFocus); // 需要能收到键盘事件（Delete 删除文字）
    // 不做任何缩放变换，方便鼠标位置和场景坐标一一对应
    setTransform(QTransform());
}

WhiteboardView::~WhiteboardView()
{
    m_undoStack->clear();
    delete m_undoStack;
    m_undoStack = nullptr;
}

QPointF WhiteboardView::clampToScene(const QPointF &pos) const
{
    const QRectF r = scene()->sceneRect();
    qreal x = qBound(r.left(), pos.x(), r.right());
    qreal y = qBound(r.top(), pos.y(), r.bottom());
    return QPointF(x, y);
}

void WhiteboardView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QGraphicsView::mousePressEvent(event);
        return;
    }

    QPointF scenePos = clampToScene(mapToScene(event->pos()));

    if (m_tool == Tool::Text) {
        // 文字工具下先看看点到的是不是已有对象：点中了就转发给默认
        // 实现，让 MovableTextItem 自己处理拖动；点在空白处才新建。
        QGraphicsItem *hit = itemAt(event->pos());
        if (hit) {
            m_forwardingToScene = true;
            QGraphicsView::mousePressEvent(event);
            return;
        }
        handleTextClick(scenePos);
        return;
    }

    if (m_tool == Tool::Eraser) {
        m_erasing = true;
        eraseAt(scenePos);
        return;
    }

    beginStroke(scenePos);
}

void WhiteboardView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_forwardingToScene) {
        QGraphicsView::mouseMoveEvent(event);
        return;
    }

    QPointF scenePos = clampToScene(mapToScene(event->pos()));

    if (m_erasing) {
        eraseAt(scenePos);
        return;
    }
    if (m_stroking) {
        continueStroke(scenePos);
        return;
    }
    QGraphicsView::mouseMoveEvent(event);
}

void WhiteboardView::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_forwardingToScene) {
        QGraphicsView::mouseReleaseEvent(event);
        if (event->button() == Qt::LeftButton)
            m_forwardingToScene = false;
        return;
    }

    if (event->button() == Qt::LeftButton && m_erasing) {
        m_erasing = false;
        if (m_erasingMacroOpen) {
            m_undoStack->endMacro();
            m_erasingMacroOpen = false;
        }
        return;
    }

    if (event->button() == Qt::LeftButton && m_stroking) {
        endStroke();
        return;
    }

    QGraphicsView::mouseReleaseEvent(event);
}

void WhiteboardView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (m_tool == Tool::Text) {
        if (auto *textItem = qgraphicsitem_cast<MovableTextItem *>(itemAt(event->pos()))) {
            bool ok = false;
            QString newText = QInputDialog::getMultiLineText(
                this, tr(u8"编辑文字"), tr(u8"文字内容："), textItem->text(), &ok);
            if (ok && !newText.isEmpty() && newText != textItem->text())
                m_undoStack->push(new EditTextCommand(textItem, textItem->text(), newText));
            return;
        }
    }
    QGraphicsView::mouseDoubleClickEvent(event);
}

void WhiteboardView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        const auto selected = scene()->selectedItems();
        if (!selected.isEmpty()) {
            m_undoStack->beginMacro(tr(u8"删除选中对象"));
            for (QGraphicsItem *item : selected)
                m_undoStack->push(new RemoveObjectCommand(scene(), item, tr(u8"删除对象")));
            m_undoStack->endMacro();
            return;
        }
    }
    QGraphicsView::keyPressEvent(event);
}

void WhiteboardView::beginStroke(const QPointF &scenePos)
{
    m_stroking = true;
    m_currentStroke = new StrokeItem(m_penColor, m_penWidth);
    m_currentStroke->addPoint(scenePos);
    scene()->addItem(m_currentStroke); // 先直接加入场景，用于拖动时的实时预览
}

void WhiteboardView::continueStroke(const QPointF &scenePos)
{
    m_currentStroke->addPoint(scenePos);
}

void WhiteboardView::endStroke()
{
    m_stroking = false;
    // push() 会立刻调用一次 redo()；AddObjectCommand::redo() 里做了
    // "已经在场景里就不重复添加" 的判断，所以这里不会出现重复添加。
    m_undoStack->push(new AddObjectCommand(scene(), m_currentStroke, tr(u8"画线")));
    m_currentStroke = nullptr;
}

void WhiteboardView::handleTextClick(const QPointF &scenePos)
{
    bool ok = false;
    QString text = QInputDialog::getMultiLineText(this, tr(u8"输入文字"), tr(u8"文字内容："), QString(), &ok);
    if (!ok || text.isEmpty())
        return;

    auto *item = new MovableTextItem(text, m_textFont, m_penColor, m_undoStack);
    item->setPos(scenePos);
    m_undoStack->push(new AddObjectCommand(scene(), item, tr(u8"添加文字")));
}

void WhiteboardView::eraseAt(const QPointF &scenePos)
{
    // 以鼠标位置为圆心画一个小圆，凡是形状（shape()）和这个圆相交的
    // 对象——不管是笔画还是文字——整个删除，而不是"擦掉一部分"。
    QPainterPath hitArea;
    hitArea.addEllipse(scenePos, m_eraserWidth / 2.0, m_eraserWidth / 2.0);

    const auto hits = scene()->items(hitArea, Qt::IntersectsItemShape);
    for (QGraphicsItem *item : hits) {
        if (!m_erasingMacroOpen) {
            // 只有真的命中了东西才开始打包撤销宏，避免空点一下也占一条撤销记录
            m_undoStack->beginMacro(tr(u8"橡皮擦"));
            m_erasingMacroOpen = true;
        }
        m_undoStack->push(new RemoveObjectCommand(scene(), item, tr(u8"删除对象")));
    }
}

void WhiteboardView::clearCanvas()
{
    const auto items = scene()->items();
    if (items.isEmpty())
        return;

    m_undoStack->beginMacro(tr(u8"清空画布"));
    for (QGraphicsItem *item : items)
        m_undoStack->push(new RemoveObjectCommand(scene(), item, tr(u8"删除对象")));
    m_undoStack->endMacro();
}
