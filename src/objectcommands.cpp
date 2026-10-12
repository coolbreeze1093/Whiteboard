#include "objectcommands.h"

#include <QGraphicsScene>
#include <QGraphicsItem>

AddObjectCommand::AddObjectCommand(QGraphicsScene *scene, QGraphicsItem *item, const QString &text, QUndoCommand *parent)
    : QUndoCommand(parent), m_scene(scene), m_item(item)
{
    setText(text.isEmpty() ? QObject::tr(u8"添加对象") : text);
}

AddObjectCommand::~AddObjectCommand()
{
    // 命令被销毁时，如果对象当前不在任何场景里（意味着最后一次操作
    // 是 undo），说明没人再持有它了，这时才真正 delete。
    if (!m_item->scene())
        delete m_item;
}

void AddObjectCommand::undo()
{
    m_scene->removeItem(m_item);
}

void AddObjectCommand::redo()
{
    // 允许对象在被手动加入场景做实时预览之后（比如正在画的笔画），
    // 再被 QUndoStack::push() 自动调用一次 redo()：这里判断一下场景，
    // 保证重复调用是幂等的，不会出现"重复添加"的警告。
    if (m_item->scene() != m_scene)
        m_scene->addItem(m_item);
}

RemoveObjectCommand::RemoveObjectCommand(QGraphicsScene *scene, QGraphicsItem *item, const QString &text, QUndoCommand *parent)
    : QUndoCommand(parent), m_scene(scene), m_item(item)
{
    setText(text.isEmpty() ? QObject::tr(u8"删除对象") : text);
}

RemoveObjectCommand::~RemoveObjectCommand()
{
    /* if (!m_item->scene())
        delete m_item; */
}

void RemoveObjectCommand::undo()
{
    if (m_item->scene() != m_scene)
        m_scene->addItem(m_item);
}

void RemoveObjectCommand::redo()
{
    m_scene->removeItem(m_item);
}
