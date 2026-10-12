#include "movabletextitem.h"
#include "textcommands.h"

#include <QPainter>
#include <QFontMetricsF>
#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>
#include <QCursor>
#include <QUndoStack>

MovableTextItem::MovableTextItem(const QString &text,
                                 const QFont &font,
                                 const QColor &color,
                                 QUndoStack *undoStack)
    : m_text(text)
    , m_font(font)
    , m_color(color)
    , m_undoStack(undoStack)
{
    setFlags(ItemIsMovable | ItemIsSelectable);
    setCursor(Qt::SizeAllCursor);
}

QRectF MovableTextItem::boundingRect() const
{
    QFontMetricsF fm(m_font);

    const QStringList lines =
        m_text.split(QLatin1Char('\n'));

    // 至少一行
    const int lineCount = qMax(1, lines.size());

    qreal width = 0.0;

    for (const QString &line : lines)
    {
        QString text = line;

        // 空行也需要占一定宽度
        if (text.isEmpty())
            text = QStringLiteral(" ");

        const qreal lineWidth =
            fm.width(text);

        width = qMax(width, lineWidth);
    }

    // 如果整个字符串为空
    if (width <= 0.0)
        width = fm.width(QStringLiteral(" "));

    const qreal height =
        fm.height() * lineCount;

    // 四周留 4px padding
    constexpr qreal padding = 4.0;

    return QRectF(
        0.0,
        0.0,
        width + padding * 2.0,
        height + padding * 2.0
    );
}

void MovableTextItem::paint(QPainter *painter,
                            const QStyleOptionGraphicsItem *option,
                            QWidget *)
{
    if (!painter)
        return;

    painter->setRenderHint(
        QPainter::Antialiasing,
        true
    );

    painter->setFont(m_font);
    painter->setPen(m_color);
    painter->setBrush(Qt::NoBrush);

    QFontMetricsF fm(m_font);

    const QStringList lines =
        m_text.split(QLatin1Char('\n'));

    constexpr qreal padding = 4.0;

    // 第一行 baseline
    const qreal firstBaseline =
        padding + fm.ascent();

    // 一行一行绘制
    for (int i = 0; i < lines.size(); ++i)
    {
        const QString &line = lines.at(i);

        painter->drawText(
            QPointF(
                padding,
                firstBaseline + i * fm.height()
            ),
            line
        );
    }

    // 选中状态
    if (option->state & QStyle::State_Selected)
    {
        QPen dashPen(
            Qt::gray,
            1.0,
            Qt::DashLine
        );

        painter->setPen(dashPen);
        painter->setBrush(Qt::NoBrush);

        painter->drawRect(
            boundingRect().adjusted(
                0.5,
                0.5,
                -0.5,
                -0.5
            )
        );
    }
}

void MovableTextItem::setText(const QString &text)
{
    if (m_text == text)
        return;

    // 文字尺寸变化之前通知 QGraphicsScene
    prepareGeometryChange();

    m_text = text;

    update();
}

void MovableTextItem::mousePressEvent(
    QGraphicsSceneMouseEvent *event)
{
    m_pressPos = pos();

    QGraphicsItem::mousePressEvent(event);
}

void MovableTextItem::mouseReleaseEvent(
    QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);

    if (m_undoStack && pos() != m_pressPos)
    {
        m_undoStack->push(
            new MoveTextCommand(
                this,
                m_pressPos,
                pos()
            )
        );
    }
}