#include "pixeleditcommand.h"
#include "canvasitem.h"


PixelEditCommand::PixelEditCommand(CanvasItem *canvas, const QRect &rect,
                                    const QImage &before, const QImage &after, uint32_t id,
                                    const QString &text, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_canvas(canvas)
    , m_rect(rect)
    , m_before(before)
    , m_after(after)
    , m_id(id)
{
    setText(text.isEmpty() ? QObject::tr(u8"编辑") : text);
}

void PixelEditCommand::undo()
{
    m_canvas->putRegion(m_rect, m_before);
    emit undoSignal(m_id);
}

void PixelEditCommand::redo()
{
    m_canvas->putRegion(m_rect, m_after);
    emit redoSignal(m_id);
}
