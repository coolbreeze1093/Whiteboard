#ifndef STROKEITEM_H
#define STROKEITEM_H

#include <QGraphicsItem>
#include <QVector>
#include <QPointF>
#include <QColor>
#include <QPainterPath>

// 一笔"画线"对应一个 StrokeItem 对象，内部只存这一笔经过的所有点
// （对象存储，而不是像素）。这是我们自己写的图元类（不是
// QGraphicsLineItem / QGraphicsPathItem 之类的现成类），paint() 里
// 自己根据点集画路径。橡皮擦碰到这整个对象，就把它整个删除，
// 而不是"擦掉一部分像素"。
class StrokeItem : public QGraphicsItem
{
public:
    enum { Type = QGraphicsItem::UserType + 2 };

    StrokeItem(const QColor &color, int penWidth);

    int type() const override { return Type; }
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    // 追加一个新的路径点（画笔拖动过程中持续调用）
    void addPoint(const QPointF &scenePoint);

    QColor color() const { return m_color; }
    int penWidth() const { return m_penWidth; }

private:
    void rebuildShapeIfNeeded() const;

    QVector<QPointF> m_points;
    QColor m_color;
    int m_penWidth;

    // shape() 里要用 QPainterPathStroker 把折线撑成一个"粗路径"用来做
    // 精确的命中测试（橡皮擦/点击判定），这个过程有一点点开销，加个
    // 缓存避免同一帧内被反复调用时重复计算。
    mutable QPainterPath m_cachedShape;
    mutable bool m_shapeDirty = true;
};

#endif // STROKEITEM_H
