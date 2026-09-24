#ifndef PIXELEDITCOMMAND_H
#define PIXELEDITCOMMAND_H

#include <QUndoCommand>
#include <QImage>
#include <QRect>
#include <QString>

class CanvasItem;

// 通用的"像素级"撤销命令：只记录一次操作影响到的矩形区域，
// 以及这块区域"操作前"和"操作后"的像素快照。
// 无论是画一笔、写一段文字，还是点了"清空"，撤销/重做都只是把对应的像素快照贴回画布，
// 逻辑统一，且完全不关心画的是线还是文字（本来也没有"图元类型"这回事）。

class PixelEditCommand : public QObject, public QUndoCommand
{
    Q_OBJECT
public:
    PixelEditCommand(CanvasItem *canvas, const QRect &rect,
                      const QImage &before, const QImage &after, uint32_t id,
                      const QString &text = QString(),
                      QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;
signals:
    void undoSignal(uint32_t id);
    void redoSignal(uint32_t id);
private:
    CanvasItem *m_canvas;
    QRect m_rect;
    QImage m_before;
    QImage m_after;
    uint32_t m_id=0;
};

#endif // PIXELEDITCOMMAND_H
