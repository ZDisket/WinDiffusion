#pragma once
#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QPen>
#include <QGraphicsSceneHoverEvent>

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
    int cursorDiameter;
    void updateCursor();
};

