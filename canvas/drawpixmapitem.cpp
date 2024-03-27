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
    QPixmap cursorPixmap(14, 14); // Start with a base size for cursors
    cursorPixmap.fill(Qt::transparent); // Ensure the background is transparent

    if (*currentTool == DrawingTool::PenBrush) {
        cursorPixmap = QPixmap(cursorDiameter + 2, cursorDiameter + 2);
        cursorPixmap.fill(Qt::transparent);
        QPainter painter(&cursorPixmap);
        painter.setPen(QPen(Qt::black, 2));
        painter.drawEllipse(1, 1, cursorDiameter, cursorDiameter);
    }
    else if (*currentTool == DrawingTool::Remover) {
        cursorPixmap = QPixmap(cursorDiameter + 2, cursorDiameter + 2);
        cursorPixmap.fill(Qt::transparent);
        QPainter painter(&cursorPixmap);
        painter.setPen(QPen(Qt::black, 2));
        painter.drawEllipse(1, 1, cursorDiameter, cursorDiameter);
        painter.drawLine(cursorDiameter / 4, cursorDiameter / 2, 3 * cursorDiameter / 4, cursorDiameter / 2);
    }
    else if (*currentTool == DrawingTool::FillBucket) {
        QPainter painter(&cursorPixmap);
        painter.setPen(QPen(Qt::black, 2));
        painter.drawEllipse(1, 1, 12, 12); // Draw smaller circle for the fill bucket tool
        int crossCenter = cursorPixmap.width() / 2;
        int crossArmLength = 4;
        painter.drawLine(crossCenter, crossCenter - crossArmLength, crossCenter, crossCenter + crossArmLength);
        painter.drawLine(crossCenter - crossArmLength, crossCenter, crossCenter + crossArmLength, crossCenter);
    }
    else if (*currentTool == DrawingTool::ColorPicker) {
        // The ColorPicker tool has the same base size but only the crosshair, no circle.
        QPainter painter(&cursorPixmap);
        painter.setPen(QPen(Qt::black, 2));
        int crossCenter = cursorPixmap.width() / 2;
        int crossArmLength = 4;
        painter.drawLine(crossCenter, crossCenter - crossArmLength, crossCenter, crossCenter + crossArmLength);
        painter.drawLine(crossCenter - crossArmLength, crossCenter, crossCenter + crossArmLength, crossCenter);
    }

    QCursor cursor(cursorPixmap);
    QApplication::setOverrideCursor(cursor);
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
