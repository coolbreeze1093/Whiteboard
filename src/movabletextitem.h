#ifndef MOVABLETEXTITEM_H
#define MOVABLETEXTITEM_H

#include <QGraphicsItem>
#include <QString>
#include <QFont>
#include <QColor>
#include <QPointF>

class QUndoStack;

class MovableTextItem : public QGraphicsItem
{
public:
    MovableTextItem(const QString &text,
                    const QFont &font,
                    const QColor &color,
                    QUndoStack *undoStack = nullptr);

    QRectF boundingRect() const override;

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

    void setText(const QString &text);

    QString text() const
    {
        return m_text;
    }

private:
    void mousePressEvent(
        QGraphicsSceneMouseEvent *event) override;

    void mouseReleaseEvent(
        QGraphicsSceneMouseEvent *event) override;

private:
    QString m_text;
    QFont m_font;
    QColor m_color;

    QUndoStack *m_undoStack = nullptr;

    QPointF m_pressPos;
};

#endif // MOVABLETEXTITEM_H