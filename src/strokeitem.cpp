#include "strokeitem.h"

#include <QPainter>
#include <QPainterPathStroker>

StrokeItem::StrokeItem(const QColor &color, int penWidth)
    : m_color(color)
    , m_penWidth(qMax(1, penWidth))
{
}

void StrokeItem::addPoint(const QPointF &scenePoint)
{
    prepareGeometryChange();
    m_points.append(scenePoint);
    m_shapeDirty = true;
    update();
}

void StrokeItem::rebuildShapeIfNeeded() const
{
    if (!m_shapeDirty)
        return;

    m_cachedShape = QPainterPath();
    if (m_points.isEmpty()) {
        m_shapeDirty = false;
        return;
    }
    if (m_points.size() == 1) {
        // 只点了一下没拖动：用一个小圆当作命中区域和显示效果
        qreal r = m_penWidth / 2.0 + 2;
        m_cachedShape.addEllipse(m_points.first(), r, r);
        m_shapeDirty = false;
        return;
    }

    QPainterPath raw(m_points.first());
    for (int i = 1; i < m_points.size(); ++i)
        raw.lineTo(m_points.at(i));

    QPainterPathStroker stroker;
    // 命中区域比实际显示的线宽稍微放宽一点，橡皮擦/点击更容易碰到
    stroker.setWidth(m_penWidth + 4);
    stroker.setCapStyle(Qt::RoundCap);
    stroker.setJoinStyle(Qt::RoundJoin);
    m_cachedShape = stroker.createStroke(raw);
    m_shapeDirty = false;
}

QRectF StrokeItem::boundingRect() const
{
    rebuildShapeIfNeeded();
    return m_cachedShape.boundingRect();
}

QPainterPath StrokeItem::shape() const
{
    rebuildShapeIfNeeded();
    return m_cachedShape;
}

void StrokeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setRenderHint(QPainter::Antialiasing, true);

    if (m_points.size() == 1) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(m_color);
        painter->drawEllipse(m_points.first(), m_penWidth / 2.0, m_penWidth / 2.0);
        return;
    }

    QPainterPath path(m_points.first());
    for (int i = 1; i < m_points.size(); ++i)
        path.lineTo(m_points.at(i));

    QPen pen(m_color, m_penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(path);
}
