#ifndef MOVABLETEXTITEM_H
#define MOVABLETEXTITEM_H

#include <QGraphicsItem>
#include <QFont>
#include <QColor>
#include <QPointF>
#include <QString>

class QUndoStack;

// 可拖动的文字图元：完全是我们自己实现的类（不是 QGraphicsTextItem），
// paint() 里直接调用 QPainter::drawText 把文字画出来。
//
// 之所以没有像画笔/橡皮擦那样把文字直接烧进 CanvasItem 的共享像素图里，
// 是因为"可移动"这个需求本质上要求文字在被挪走之前必须一直保留
// 位置、内容等语义信息——而像素一旦画上去，就已经和背景融合、
// 不可逆了，没法再单独把"这段文字"挑出来移动。
class MovableTextItem : public QGraphicsItem
{
public:
    enum { Type = QGraphicsItem::UserType + 1 };

    MovableTextItem(const QString &text, const QFont &font, const QColor &color, QUndoStack *undoStack);

    int type() const override { return Type; }
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    QString text() const { return m_text; }
    void setText(const QString &text);

    QFont font() const { return m_font; }
    QColor color() const { return m_color; }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QString m_text;
    QFont m_font;
    QColor m_color;
    QUndoStack *m_undoStack;
    QPointF m_pressPos; // 记录一次拖动开始时的位置，抬起时用来生成撤销命令
};

#endif // MOVABLETEXTITEM_H
