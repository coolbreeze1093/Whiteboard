#ifndef OBJECTCOMMANDS_H
#define OBJECTCOMMANDS_H

#include <QUndoCommand>
#include <QString>

class QGraphicsScene;
class QGraphicsItem;

// 通用的"把一个对象加入场景"撤销命令，适用于任何自定义图元
// （笔画 StrokeItem、文字 MovableTextItem……）——白板里的一切现在
// 都是"对象"，不再有共享的像素图层，所以增/删逻辑可以完全统一。
class AddObjectCommand : public QUndoCommand
{
public:
    AddObjectCommand(QGraphicsScene *scene, QGraphicsItem *item,
                      const QString &text = QString(), QUndoCommand *parent = nullptr);
    ~AddObjectCommand() override;
    void undo() override;
    void redo() override;

private:
    QGraphicsScene *m_scene;
    QGraphicsItem *m_item;
};

// 通用的"从场景移除一个对象"撤销命令，逻辑与 AddObjectCommand 相反。
// 橡皮擦命中删除、清空画布、Delete 键删文字，用的都是这一个命令类。
class RemoveObjectCommand : public QUndoCommand
{
public:
    RemoveObjectCommand(QGraphicsScene *scene, QGraphicsItem *item,
                         const QString &text = QString(), QUndoCommand *parent = nullptr);
    ~RemoveObjectCommand() override;
    void undo() override;
    void redo() override;

private:
    QGraphicsScene *m_scene;
    QGraphicsItem *m_item;
};

#endif // OBJECTCOMMANDS_H
