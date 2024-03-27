#include "drawingscene.h"
#include "qcursor.h"
#include <QPainter>
#include "../ext/qfloodfill.h"

const size_t MAX_BRUSH_SIZE = 90;

DrawingScene::DrawingScene(QObject* parent)
    : QGraphicsScene(parent), pixmapItem(nullptr), isDrawing(false)
{

    pen = QPen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap); // Adjust pen properties as needed

    CurrentTool = DrawingTool::PenBrush;
    // I LOVE pointers!!!


  //  Render();


}

void DrawingScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{

    if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton) {
        lastPoint = event->scenePos();

        if (!isDrawing)
            currentLayer->Snapshot();

        isDrawing = true;


    }
    QGraphicsScene::mousePressEvent(event);
}


void DrawingScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    bool isUsingLeftBtn = event->buttons() & Qt::LeftButton;
    bool isUsingRightBtn = event->buttons() & Qt::RightButton;

    if ((isUsingLeftBtn || isUsingRightBtn) && isDrawing && CurrentTool != DrawingTool::FillBucket) {


        // By basing the ternary on whether we're using the right button we prioritize it in case
        // both are held down at once
        drawLineTo(event->scenePos(), isUsingRightBtn ? ColorCat::Secondary : ColorCat::Primary);
    }
    QGraphicsScene::mouseMoveEvent(event);

}

void DrawingScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (
        (event->button() == Qt::LeftButton || event->button() == Qt::RightButton)
        && isDrawing
        )
    {
        drawLineTo(event->scenePos(),
                   event->button() == Qt::RightButton ? ColorCat::Secondary : ColorCat::Primary);

        isDrawing = false;

        currentLayer->Update();
        emit Updated();

    }
    QGraphicsScene::mouseReleaseEvent(event);
}

void DrawingScene::takeSnapshot()
{
    currentLayer->Snapshot();
}

void DrawingScene::undo()
{
    currentLayer->Undo();
    Render();
    currentLayer->Update();
}

void DrawingScene::addLayer()
{
    layers.push_back(std::make_unique<Layer>(canvasSize));
    currentLayer = layers.back().get();
    Render();

}

void DrawingScene::addLayer(const QString &name, const QColor &color)
{
    layers.push_back(std::make_unique<Layer>(canvasSize));
    currentLayer = layers.back().get();

    currentLayer->name = name;
    currentLayer->pixmap.fill(color);


    currentLayer->Update();

    Render();

}

void DrawingScene::moveLayerUp(Layer *lay) {
    auto it = lay->_it;
    if (it == layers.begin()) return; // Can't move up the first element
    auto prev = std::prev(it);
    layers.splice(prev, layers, it); // Moves the element pointed to by 'it' before 'prev'
    // Update iterators for affected layers
    lay->_it = prev;
    (*prev)->_it = it;

    Render();
}

void DrawingScene::moveLayerDown(Layer *lay) {
    auto it = lay->_it;
    auto next = std::next(it);

    // Check if the layer is the last element
    if (next == layers.end()) return; // Can't move down the last element

    // The actual moving operation. With std::list, we have to adjust positions in a way
    // that does not invalidate iterators. std::list::splice is a good tool for this.
    // We move 'next' before 'it', effectively swapping their positions.
    auto nextNext = std::next(next); // To splice at the correct position
    layers.splice(it, layers, next, nextNext);

    // Update iterators for both layers involved in the swap
    lay->_it = next;
    (*next)->_it = it;

    Render();
}

void DrawingScene::deleteLayer(Layer *lay)
{
    lay->Die();
    Render();

    currentLayer = layers.back().get();
    currentLayer->laywid->SetIsActive(true, true);

}

void DrawingScene::selectLayer(Layer *lay)
{
    currentLayer = lay;

    for (auto& layer : layers)
    {
        if (layer.get() != lay)
            layer->laywid->SetIsActive(false);

    }

}

void DrawingScene::selectLayer(int index)
{

}

void DrawingScene::setBrushSize(int newsz, bool emitsig)
{
    pen.setWidth(newsz);

    pixmapItem->cursorDiameter = pen.width() * 1.5f;


    if (emitsig)
    {
        pixmapItem->updateCursor();
        emit brushSizeChanged(newsz);
    }



}

LayerList &DrawingScene::getLayers()
{
    return layers;

}

Layer *DrawingScene::operator[](size_t i)
{
    size_t r = 0;
    for (auto& lay : layers)
    {
        if (r == i)
            return lay.get();

        r++;
    }

    return nullptr;
}

Layer *DrawingScene::getCurrentLayer()
{
    return currentLayer;
}


void DrawingScene::drawLineTo(const QPointF& endPoint, ColorCat category)
{
    QPainter painter(*currentLayer);
    painter.setRenderHint(QPainter::Antialiasing, true); // Enable antialiasing

    QColor color = getAssignedColor(category);

    pen.setColor(color);

    // Maybe I should make this a switch-case.
    if (CurrentTool == DrawingTool::Remover)
    {
        // Set the composition mode to erase the drawing
        painter.setCompositionMode(QPainter::CompositionMode_Clear);

        // Set the pen to transparent; it works in combination with the composition mode
        painter.setPen(QPen(Qt::transparent, pen.width(), pen.style(), pen.capStyle(), pen.joinStyle()));
    }
    else if (CurrentTool == DrawingTool::FillBucket)
    {
        QPointF pixmapPos = pixmapItem->mapFromScene(endPoint);

        QPoint pntPixmap((int)round(pixmapPos.x()),
                         (int)round(pixmapPos.y()));



        /*
         * Use the canvas with background as the workspace, while passing the drawingpixmap's painter
         * so it only operates on the current drawing area.
        */
        QPixmap Rendered = Render();
        FloodFill(Rendered, pntPixmap, pen.color(), Tolerance, painter);

        goto finish; // the EVIL goto keyword
    }
    else if (CurrentTool == DrawingTool::ColorPicker)
    {
        QPointF pixmapPos = pixmapItem->mapFromScene(endPoint);

        QPoint pntPixmap((int)round(pixmapPos.x()),
                         (int)round(pixmapPos.y()));

        emit colorPicked(category,
                         getColorFromPixmap(*currentLayer, pntPixmap.x(), pntPixmap.y())
                         );

        goto finish;



    }
    else
    {
        // Regular drawing
        painter.setPen(pen);
    }



    painter.drawLine(lastPoint, endPoint);
    lastPoint = endPoint;

    finish:
    Render();
}

QColor DrawingScene::getAssignedColor(ColorCat cat)
{
    return cat == ColorCat::Primary ? ToolColors.first : ToolColors.second;
}

QPixmap& DrawingScene::Render(bool final)
{

    if (basePixmap.isNull())
        throw std::exception("Cannot render into a null pixmap! Use Initialize()!");

    basePixmap.fill(Qt::transparent);

    QPainter painter(&basePixmap);

    for (auto it = layers.begin(); it != layers.end();)
    {
        Layer* lay = (*it).get();

        lay->_it = it;

        /*
         * Layers are flagged for deletion and then deleted properly in the composition loop
         * This is because deleting a pointer owned by a unique_ptr results in errors and I don't like
         * iterating through lists more than absolutely necessary.
         *
         * (honestly, since these are pointers, it'd be much more str8forward to just use std::vector and recreate it every time, but
         * I want to challenge myself and use other things)
        */
        if (lay->_delete){ // It was deleted
            (*it).reset();
            it = layers.erase(it);
            continue;
        }

        if (!lay->visible){
            it = std::next(it);
            continue;
        }

        if (final && !lay->renderable)
        {
            it = std::next(it);
            continue;
        }


        painter.setOpacity(lay->opacity);
        painter.drawPixmap(0,0, *lay);

        it = std::next(it);

    }

    if (!final)
    {

        pixmapItem->setPixmap(basePixmap); // Display the merged image

        update();
        pixmapItem->update();


    }


    return basePixmap;
}

void DrawingScene::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if (!(event->modifiers() & Qt::ControlModifier)) {
        // Adjust the pen width based on the wheel delta
        int delta = event->delta();
        int newWidth = pen.width() + delta / 120; // Each wheel notch is typically 120 units

        // Clamp the new width to a reasonable range to prevent it from becoming too large or negative
        if (newWidth < 1) newWidth = 1;
        if (newWidth > MAX_BRUSH_SIZE) newWidth = MAX_BRUSH_SIZE; // Set maximum brush size as needed

        setBrushSize(newWidth);

        event->accept();
    } else {
        // Forward the event to the base class for default processing (e.g., zooming)
        QGraphicsScene::wheelEvent(event);
    }
}

bool DrawingScene::event(QEvent *event)
{

    return QGraphicsScene::event(event);
}

void DrawingScene::Initialize(QSize inSz)
{
    canvasSize = inSz;

    basePixmap = QPixmap(canvasSize).copy();

    pixmapItem = new DrawPixmapItem(basePixmap);
    pixmapItem->currentTool = &CurrentTool;
    addItem(pixmapItem);

}

