#pragma once
#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QPen>
#include <QGraphicsSceneHoverEvent>

enum class DrawingTool{
    PenBrush = 0,
    Remover, // I'd name this Eraser but I want to avoid name collisions with Qt namespaces
    FillBucket,
    ColorPicker
};

class DrawPixmapItem : public QGraphicsPixmapItem
{
public:
    explicit DrawPixmapItem(const QPixmap &pixmap);

protected:
    // Override these methods to handle hover events
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

public:
    DrawingTool* currentTool = nullptr;
    int cursorDiameter;
    void updateCursor();
};

