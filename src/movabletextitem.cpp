#include "movabletextitem.h"
#include "textcommands.h"

#include <QPainter>
#include <QFontMetrics>
#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>
#include <QCursor>
#include <QUndoStack>

MovableTextItem::MovableTextItem(const QString &text, const QFont &font, const QColor &color, QUndoStack *undoStack)
    : m_text(text), m_font(font), m_color(color), m_undoStack(undoStack)
{
    setFlags(ItemIsMovable | ItemIsSelectable);
    setCursor(Qt::SizeAllCursor);
}

QRectF MovableTextItem::boundingRect() const
{
    QFontMetrics fm(m_font);
    const QStringList lines = m_text.split(QLatin1Char('\n'));

    qreal width = 0;

    for (const QString &line : lines)
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        const qreal w = fm.horizontalAdvance(
            line.isEmpty() ? QStringLiteral(" ") : line
        );
#else
        const qreal w = fm.width(
            line.isEmpty() ? QStringLiteral(" ") : line
        );
#endif

        width = qMax(width, w);
    }

    const qreal height = fm.height() * qMax(1, lines.size());

    return QRectF(0, 0, width + 8, height + 8);
}

void MovableTextItem::paint(QPainter *painter,
                            const QStyleOptionGraphicsItem *option,
                            QWidget *)
{
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setFont(m_font);
    painter->setPen(m_color);

    const QRectF textRect = boundingRect().adjusted(4, 4, -4, -4);

    painter->drawText(
        textRect,
        Qt::AlignLeft | Qt::AlignTop,
        m_text
    );

    if (option->state & QStyle::State_Selected)
    {
        QPen dashPen(Qt::gray, 1, Qt::DashLine);
        painter->setPen(dashPen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(
            boundingRect().adjusted(0.5, 0.5, -0.5, -0.5)
        );
    }
}

void MovableTextItem::setText(const QString &text)
{
    prepareGeometryChange(); // 文字变了，包围盒可能也变了，先通知场景
    m_text = text;
    update();
}

void MovableTextItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    m_pressPos = pos();
    QGraphicsItem::mousePressEvent(event); // 交给默认实现处理选中/开始拖动
}

void MovableTextItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event); // 默认实现已经把位置移动到位
    if (m_undoStack && pos() != m_pressPos)
        m_undoStack->push(new MoveTextCommand(this, m_pressPos, pos()));
}
