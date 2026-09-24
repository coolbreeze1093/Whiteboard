#ifndef CANVASITEM_H
#define CANVASITEM_H

#include <QGraphicsItem>
#include <QImage>
#include <QColor>
#include <QFont>
#include <QRect>

// 画布图元：本身不携带任何"线条/文字"语义，只是把一张 QImage
// 按像素显示出来。所有绘制、擦除、写字操作都是直接对这张 QImage
// 的像素进行读写，而不是往场景里添加 QGraphicsLineItem /
// QGraphicsTextItem 之类的现成图元。

class CanvasItem : public QGraphicsItem
{
public:
    explicit CanvasItem(const QSize &size, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    // 整张画布图像的只读引用
    const QImage &image() const { return m_image; }
    QSize size() const { return m_image.size(); }

    // 截取某个矩形区域的像素（用于撤销时保存"操作前/操作后"的快照）
    QImage copyRegion(const QRect &rect) const;
    // 把一段像素图像原样贴回画布的某个矩形区域（用于撤销/重做）
    void putRegion(const QRect &rect, const QImage &regionImage);

    // 从 from 画一段到 to。eraser = true 时用 CompositionMode_Clear
    // 把经过的像素擦成透明，而不是画一条填充色的线，这样才是
    // 真正"基于像素"的橡皮擦，与画布背景色无关。
    void drawSegment(const QPointF &from, const QPointF &to, const QColor &color, int penWidth, bool eraser);

    void drawRect(const QRectF &rect, const QColor &color, bool eraser);

    // 在指定像素坐标绘制一段文字，返回实际写入的像素矩形
    QRect drawText(const QPointF &pos, const QString &text, const QFont &font, const QColor &color);

    // 清空整张画布（所有像素变透明）
    void clearAll();

private:
    QImage m_image;
};

#endif // CANVASITEM_H
