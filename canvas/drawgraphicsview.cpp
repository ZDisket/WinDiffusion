#include "drawgraphicsview.h"

DrawGraphicsView::DrawGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
{
    setMouseTracking(true);
    viewport()->setMouseTracking(true);
    needsControlScroll = true;
    // You can add more initialization code here if needed

}

void DrawGraphicsView::wheelEvent(QWheelEvent *pWheelEvent)
{
    if (!needsControlScroll || (pWheelEvent->modifiers() & Qt::ControlModifier)) {
        // Do a wheel-based zoom about the cursor position
        double angle = pWheelEvent->angleDelta().y();
        double factor = qPow(1.0015, angle);

        auto targetViewportPos = pWheelEvent->position();
        auto targetScenePos = pWheelEvent->scenePosition();

        scale(factor, factor);
        centerOn(targetScenePos);
        QPointF deltaViewportPos = targetViewportPos - QPointF(viewport()->width() / 2.0, viewport()->height() / 2.0);
        QPointF viewportCenter = mapFromScene(targetScenePos) - deltaViewportPos;
        centerOn(mapToScene(viewportCenter.toPoint()));

        return;
    }

    // Call the base class method to ensure that non-zoom events are processed
    QGraphicsView::wheelEvent(pWheelEvent);
}

bool DrawGraphicsView::event(QEvent *event)
{
    return QGraphicsView::event(event);
}
