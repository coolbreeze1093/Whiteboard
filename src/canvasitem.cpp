#include "canvasitem.h"
#include <QPainter>
#include <QFontMetrics>

CanvasItem::CanvasItem(const QSize &size, QGraphicsItem *parent)
    : QGraphicsItem(parent)
{
    // 带 alpha 通道的格式：没画过的地方是完全透明的像素，
    // 这样橡皮擦才能把像素真正"擦透"，而不是涂成某个背景色。
    m_image = QImage(size, QImage::Format_ARGB32_Premultiplied);
    m_image.fill(Qt::transparent);
}

QRectF CanvasItem::boundingRect() const
{
    return QRectF(0, 0, m_image.width(), m_image.height());
}

void CanvasItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->drawImage(0, 0, m_image);
}

QImage CanvasItem::copyRegion(const QRect &rect) const
{
    return m_image.copy(rect);
}

void CanvasItem::putRegion(const QRect &rect, const QImage &regionImage)
{
    QPainter p(&m_image);
    // Source 模式：直接覆盖像素（包括 alpha 通道），
    // 这样撤销才能把"被擦成透明的像素"精确还原回来。
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.drawImage(rect.topLeft(), regionImage);
    p.end();
    update(rect);
}

void CanvasItem::drawSegment(const QPointF &from, const QPointF &to, const QColor &color, int penWidth, bool eraser)
{
    QPainter p(&m_image);
    p.setRenderHint(QPainter::Antialiasing, true);
    QPen pen(color, penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p.setPen(pen);

    if (eraser) {
        // 真正的"擦除"：把画笔经过的像素清成透明。
        p.setCompositionMode(QPainter::CompositionMode_Clear);
    } else {
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    }
    p.drawLine(from, to);
    p.end();

    int pad = penWidth / 2 + 2;
    QRect dirty = QRectF(from, to).normalized().adjusted(-pad, -pad, pad, pad).toAlignedRect();
    update(dirty.intersected(m_image.rect()));
}

void CanvasItem::drawRect(const QRectF &rect,
                          const QColor &color,
                          bool eraser)
{
    QPainter p(&m_image);
    p.setRenderHint(QPainter::Antialiasing, true);

    if (eraser) {
        // 擦除：清除 rect 区域
        p.setCompositionMode(QPainter::CompositionMode_Clear);
        p.fillRect(rect, Qt::transparent);
    } else {
        // 正常填充
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        p.fillRect(rect, color);
    }

    p.end();

    update(rect.toAlignedRect().intersected(m_image.rect()));
}

QRect CanvasItem::drawText(const QPointF &pos, const QString &text, const QFont &font, const QColor &color)
{
    QPainter p(&m_image);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setFont(font);
    p.setPen(color);
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);

    QFontMetrics fm(font);
    QRect textRect = fm.boundingRect(QRect(pos.toPoint(), QSize(2000, 800)), Qt::TextWordWrap, text);
    textRect.moveTopLeft(pos.toPoint());
    p.drawText(textRect, Qt::TextWordWrap, text);
    p.end();

    QRect dirty = textRect.adjusted(-2, -2, 2, 2).intersected(m_image.rect());
    update(dirty);
    return dirty;
}

void CanvasItem::clearAll()
{
    m_image.fill(Qt::transparent);
    update();
}
