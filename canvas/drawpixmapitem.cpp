#include "drawpixmapitem.h"
#include <QCursor>
#include <QApplication>
DrawPixmapItem::DrawPixmapItem(const QPixmap &pixmap) : QGraphicsPixmapItem(pixmap)
{
    // Enable handling hover events
    setAcceptHoverEvents(true);
    cursorDiameter = 10;
}

void DrawPixmapItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    updateCursor();
}

void DrawPixmapItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    // Reset to default cursor when leaving the item
    QApplication::setOverrideCursor(Qt::ArrowCursor);
}

void DrawPixmapItem::updateCursor()
{
    QPixmap cursorPixmap(cursorDiameter + 2, cursorDiameter + 2); // +2 for the outline
    cursorPixmap.fill(Qt::transparent); // Transparent background

    QPainter painter(&cursorPixmap);
    painter.setPen(QPen(Qt::black, 2)); // Black outline for the circle
    painter.setBrush(Qt::transparent); // No fill for the circle
    painter.drawEllipse(1, 1, cursorDiameter, cursorDiameter); // Draw the circle

    QCursor cursor(cursorPixmap);

    // Normally we use the pixmap item-specific setCursor, but that results in it resetting to normal cursor when we hold click.
    QApplication::setOverrideCursor(cursor);
//    setCursor(cursor);
}


void DrawPixmapItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsPixmapItem::mousePressEvent(event); // Call base class implementation
    // Reapply the custom cursor
}

void DrawPixmapItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsPixmapItem::mouseReleaseEvent(event); // Call base class implementation
}
