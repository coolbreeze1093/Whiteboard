# Qt5 像素白板

用 Qt5 Graphics View 框架实现的白板，核心设计：

- 场景（`QGraphicsScene`）里只放**一个**自定义图元 `CanvasItem`（继承 `QGraphicsItem`，
  不是 `QGraphicsLineItem`/`QGraphicsTextItem` 这类现成图元），内部是一张带 alpha 通道
  的 `QImage`。
- 画线、写字、擦除，全部是直接用 `QPainter` 在这张 `QImage` 上做**像素级**读写：
  - 画笔：`CompositionMode_SourceOver` 画一段圆头圆角的线。
  - 橡皮擦：`CompositionMode_Clear` 把经过的像素直接擦成透明，而不是画白线，
    所以是真正基于像素的擦除。
  - 文字：`QPainter::drawText` 直接把字"烧"进图像像素里。
- 撤销：用 Qt 的 `QUndoStack` + 自定义 `PixelEditCommand`。每次操作只记录受影响
  的最小矩形区域的"操作前/操作后"像素快照，undo/redo 就是把快照贴回去，
  不区分是线还是文字，实现统一。
- 清空画布本身也是一条可撤销命令。

## 文件说明

- `canvasitem.h/.cpp`：像素画布图元。
- `pixeleditcommand.h/.cpp`：通用像素级撤销命令。
- `whiteboardview.h/.cpp`：`QGraphicsView` 子类，处理鼠标事件、当前工具状态。
- `mainwindow.h/.cpp`：主窗口 + 工具栏（画笔/橡皮擦/文字/颜色/粗细/撤销/清空）。
- `main.cpp`：程序入口。

## 编译运行

已在 Ubuntu + Qt 5.15 下用 `qmake` + `make` 编译通过。

```bash
qmake whiteboard.pro
make
./whiteboard
```

或者直接用 Qt Creator 打开 `whiteboard.pro`。

## 操作说明

- 工具栏切换「画笔 / 橡皮擦 / 文字」三种工具。
- 「选择颜色」按钮设置画笔/文字颜色。
- 「画笔粗细」「橡皮粗细」分别调节两者的笔触大小。
- 文字工具下点击画布会弹出输入框，输入完成后文字按当前颜色和字号绘制在
  点击位置。
- 「撤销」（或 Ctrl+Z）撤销上一步操作（画线/擦除/写字/清空都可撤销）。
- 「清空」会弹出确认框，清空整个白板（这一步本身也可以撤销）。

## 可以按需扩展的点（未包含在本次实现中）

- 目前只做了一级撤销栈，没有做"重做"按钮，如需要可以直接调用
  `m_view->undoStack()->redo()` 并加一个按钮。
- 没有做保存/导出为图片，可以在 `CanvasItem::image()` 基础上很容易加
  `image().save(path)`。
- 画布大小固定为 1600x1000，可按需调整 `MainWindow` 构造函数里传给
  `WhiteboardView` 的 `QSize`。
