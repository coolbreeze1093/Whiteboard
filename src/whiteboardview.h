#ifndef WHITEBOARDVIEW_H
#define WHITEBOARDVIEW_H

#include <QGraphicsView>
#include <QColor>
#include <QFont>
#include <QPointF>

class QUndoStack;
class StrokeItem;

class WhiteboardView : public QGraphicsView
{
    Q_OBJECT
public:
    enum class Tool { Pen, Eraser, Text };

    explicit WhiteboardView(const QSize &canvasSize, QWidget *parent = nullptr);
    ~WhiteboardView();

    QUndoStack *undoStack() const { return m_undoStack; }

    void setTool(Tool tool) { m_tool = tool; }
    void setPenColor(const QColor &c) { m_penColor = c; }
    void setPenWidth(int w) { m_penWidth = qMax(1, w); }
    // 橡皮擦不再是"擦的宽度"，而是命中判定的直径：以鼠标为圆心，
    // 这个范围内碰到的对象（笔画/文字）整个删除。
    void setEraserWidth(int w) { m_eraserWidth = qMax(4, w); }
    void setTextFont(const QFont &f) { m_textFont = f; }

    void clearCanvas();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    QPointF clampToScene(const QPointF &scenePos) const;
    void beginStroke(const QPointF &scenePos);
    void continueStroke(const QPointF &scenePos);
    void endStroke();
    void handleTextClick(const QPointF &scenePos);
    void eraseAt(const QPointF &scenePos);

    QUndoStack *m_undoStack;

    Tool m_tool = Tool::Pen;
    QColor m_penColor = Qt::black;
    int m_penWidth = 4;
    int m_eraserWidth = 24;
    QFont m_textFont;

    // 画笔：正在画的这一笔，抬笔前会先直接加入场景做实时预览，
    // 抬笔时才打包成一条可撤销命令。
    bool m_stroking = false;
    StrokeItem *m_currentStroke = nullptr;

    // 橡皮擦：一次拖动过程中命中的多个对象打包成一步撤销
    bool m_erasing = false;
    bool m_erasingMacroOpen = false;

    // 文字工具下，点在已有文字上准备拖动时，把事件转发给
    // QGraphicsView 的默认实现，让文字对象自己处理拖动。
    bool m_forwardingToScene = false;
};

#endif // WHITEBOARDVIEW_H
