#include "textcommands.h"
#include "movabletextitem.h"

MoveTextCommand::MoveTextCommand(MovableTextItem *item, const QPointF &oldPos, const QPointF &newPos, QUndoCommand *parent)
    : QUndoCommand(parent), m_item(item), m_oldPos(oldPos), m_newPos(newPos)
{
    setText(QObject::tr(u8"移动文字"));
}

void MoveTextCommand::undo() { m_item->setPos(m_oldPos); }
void MoveTextCommand::redo() { m_item->setPos(m_newPos); }

EditTextCommand::EditTextCommand(MovableTextItem *item, const QString &oldText, const QString &newText, QUndoCommand *parent)
    : QUndoCommand(parent), m_item(item), m_oldText(oldText), m_newText(newText)
{
    setText(QObject::tr(u8"编辑文字"));
}

void EditTextCommand::undo() { m_item->setText(m_oldText); }
void EditTextCommand::redo() { m_item->setText(m_newText); }
