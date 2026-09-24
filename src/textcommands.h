#ifndef TEXTCOMMANDS_H
#define TEXTCOMMANDS_H

#include <QUndoCommand>
#include <QPointF>
#include <QString>

class MovableTextItem;

// 拖动文字：只记录拖动前、后的位置。（新增/删除文字用的是通用的
// AddObjectCommand / RemoveObjectCommand，这里只放文字独有的行为。）
class MoveTextCommand : public QUndoCommand
{
public:
    MoveTextCommand(MovableTextItem *item, const QPointF &oldPos, const QPointF &newPos,
                     QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    MovableTextItem *m_item;
    QPointF m_oldPos;
    QPointF m_newPos;
};

// 双击编辑文字内容：只记录修改前、后的字符串。
class EditTextCommand : public QUndoCommand
{
public:
    EditTextCommand(MovableTextItem *item, const QString &oldText, const QString &newText,
                     QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    MovableTextItem *m_item;
    QString m_oldText;
    QString m_newText;
};

#endif // TEXTCOMMANDS_H
