# Qt5 对象存储白板

用 Qt5 Graphics View 框架实现的白板。**核心思想：一切都是对象，没有共享的
像素图层**。每一笔画、每一段文字都是场景里一个独立的自定义图元
（都是我们自己写的类，不是 `QGraphicsLineItem`/`QGraphicsTextItem` 这类现成类），
撤销和橡皮擦都是围绕"对象的增删/移动/编辑"来做，而不是像素读写。

## 设计

- `StrokeItem`（自定义 `QGraphicsItem`）：一笔画对应一个对象，内部存的是
  这一笔经过的所有点（`QVector<QPointF>`），`paint()` 里自己把这些点连成
  路径画出来；`shape()` 用 `QPainterPathStroker` 把折线撑成一条"粗路径"，
  用于精确的命中测试。
- `MovableTextItem`（自定义 `QGraphicsItem`）：一段文字对应一个对象，
  `paint()` 里自己调用 `QPainter::drawText`；开启了 `ItemIsMovable`/
  `ItemIsSelectable`，可以拖动、双击编辑、选中后按 Delete 删除。
- **橡皮擦**：以鼠标位置为圆心画一个小圆，用
  `QGraphicsScene::items(圆, Qt::IntersectsItemShape)` 找出形状和这个圆
  相交的所有对象——不管是笔画还是文字——**把命中的对象整个删除**，
  而不是擦掉对象的一部分。拖动橡皮擦划过的一整段命中，会打包成一步
  撤销。
- **撤销**：用 `QUndoStack` + 两类通用命令：
  - `AddObjectCommand` / `RemoveObjectCommand`（`objectcommands.h/.cpp`）：
    对任何对象通用的"加入场景 / 移出场景"，笔画的新建、橡皮擦删除、
    清空画布、文字的新建与删除都复用这两个类。
  - `MoveTextCommand` / `EditTextCommand`（`textcommands.h/.cpp`）：
    文字特有的"移动位置"和"编辑内容"。
- **清空画布**：把场景里所有对象都用 `RemoveObjectCommand` 删除，打包成
  一步撤销。

## 文件说明

- `strokeitem.h/.cpp`：画线用的自定义图元（对象存储一笔的点集）。
- `movabletextitem.h/.cpp`：可拖动的文字图元。
- `objectcommands.h/.cpp`：通用的对象增/删撤销命令。
- `textcommands.h/.cpp`：文字特有的移动/编辑撤销命令。
- `whiteboardview.h/.cpp`：`QGraphicsView` 子类，按当前工具（画笔/橡皮擦/
  文字）分发鼠标事件。
- `mainwindow.h/.cpp`：主窗口 + 工具栏（画笔/橡皮擦/文字/颜色/粗细/撤销/清空）。
- `main.cpp`：程序入口。

已在 Ubuntu + Qt 5.15 下用 `qmake` + `make` 编译通过。

## 编译运行

```bash
qmake whiteboard.pro
make
./whiteboard
```

或者直接用 Qt Creator 打开 `whiteboard.pro`。

## 操作说明

- 工具栏切换「画笔 / 橡皮擦 / 文字」三种工具。
- 「选择颜色」设置画笔/文字颜色；「画笔粗细」调节线宽。
- 「橡皮擦范围」调节命中判定圆的直径：这个范围内碰到的对象（一笔画
  或一段文字）会被整个删除。
- 文字工具下：
  - 点画布空白处会弹出输入框，新建一段文字；
  - 点已有的文字并拖动，可以把它移到别处；
  - 双击已有的文字可以重新编辑内容；
  - 选中文字后按 `Delete`/`Backspace` 也可以删除它。
- 「撤销」（或 Ctrl+Z）撤销上一步操作（画一笔线/橡皮擦删除/新增文字/
  移动文字/编辑文字/清空画布，都可撤销，橡皮擦一次拖动内删除的多个
  对象会作为一步一起撤销）。
- 「清空」会弹出确认框，删除场景里所有对象（这一步本身也可以整体撤销）。

## 与"像素版"的区别（设计取舍）

- 优点：对象天然可以被单独命中、单独删除、单独移动，橡皮擦语义更符合
  "划到哪笔就删哪笔"的直觉，撤销记录也更有意义（每条记录对应一个
  有意义的操作，而不是一块像素区域）。
- 代价：橡皮擦没法再做到"擦掉一条线的一小段"这种像素级的局部擦除——
  只要圆碰到了这个对象的形状，就是整条线/整段文字一起消失。如果之后
  想要"部分擦除一条线"，需要在命中时把 `StrokeItem` 按命中点前后拆成
  两个新的 `StrokeItem`（删除旧的、新增两个新的，同样可以用
  `RemoveObjectCommand` + `AddObjectCommand` 表达），这个可以作为后续
  扩展点。

## 可以按需扩展的点（未包含在本次实现中）

- 没有做"重做"按钮，如需要可以直接调用 `m_view->undoStack()->redo()`
  并加一个按钮。
- 没有做保存/导出为图片：可以用 `QGraphicsScene::render()` 把整个场景
  渲染到一张 `QImage`/`QPixmap` 上再 `save()`。
- 笔画对象目前不支持拖动（只支持被橡皮擦命中删除），如果也想让画好的
  线可以整体拖动，可以参考 `MovableTextItem` 给 `StrokeItem` 也加上
  `ItemIsMovable`/`ItemIsSelectable`。
- 画布大小固定为 1600x1000，可按需调整 `MainWindow` 构造函数里传给
  `WhiteboardView` 的 `QSize`。
