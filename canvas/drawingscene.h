#pragma once

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsPixmapItem>
#include <QPen>
#include <deque>
#include "drawpixmapitem.h"
#include "layerwidget.h"



/*
 * Welcome to drawingscene.h. Or, as I like to call it:
 * "This amount of inane list trickery and unnecessary optimization should be illegal"
 * Haha!
*/

const size_t MAX_UNDO = 100;



class Layer {
public:
    QPixmap pixmap;
    bool visible = true;
    bool renderable = true;
    qreal opacity = 1.0;
    QString name = "Layer";
    LayerWidget* laywid;

    bool _delete = false;
    std::list<std::unique_ptr<Layer>>::iterator _it; // Every layer must know its own position for movement operations

    std::deque<QPixmap> undoStack;

    Layer(const QSize& size, const QColor& color = Qt::transparent) : pixmap(size){
        pixmap.fill(color);
        MakeWidget();
    }

    operator QPixmap*(){return &pixmap;}

    operator const QPixmap&() const {return pixmap;}

    operator QPixmap&() {return pixmap;}


    void Update()
    {
        laywid->Update(name, pixmap);

    }
    void Snapshot(){
        qDebug() << "Undo added";
        QPixmap deepCopy = pixmap.copy(); // This creates a deep copy
        undoStack.push_back(deepCopy); // Store the deep copy in the undo stack

        if (undoStack.size() > MAX_UNDO) {
            undoStack.pop_front(); // Remove the oldest state from the front
        }
    }
    void Undo(){
        if (!undoStack.empty()) {

            pixmap = undoStack.back(); // Get the last state

            undoStack.pop_back();
            qDebug() << "Undo done.";
        }else{
            qDebug() << "No undos";
        }

    }

private:
    Layer(const Layer& ly) {throw std::exception("Copying layers is verboten");}

    void MakeWidget(){
        laywid = new LayerWidget(nullptr);
        laywid->layer = this;

    }


public:


    void Die(){
        delete laywid;
        _delete = true;
    }
};


enum class ColorCat{
  Primary = 0,
  Secondary
};

using LayerList = std::list<std::unique_ptr<Layer>>;


class DrawingScene : public QGraphicsScene
{
    Q_OBJECT

public: // Events
    DrawingScene(QObject* parent = nullptr);
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void wheelEvent(QGraphicsSceneWheelEvent *event) override;

    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) ;
    bool event(QEvent* event) override;


public: // Fucntions
    QPixmap& Render(bool final = false);
    void Initialize(QSize inSz);
    void takeSnapshot();
    void undo();
    void addLayer();
    void addLayer(const QString& name, const QColor& color = Qt::transparent);
    void moveLayerUp(Layer *lay);

    void moveLayerDown(Layer* lay);


    void deleteLayer(Layer* lay);

    void selectLayer(Layer* lay);
    void selectLayer(int index);

    void setBrushSize(int newsz, bool emitsig = true);


    LayerList& getLayers();

    // Slow, prefer not to use.
    Layer* operator[](size_t i);
    Layer* getCurrentLayer();


    DrawingTool CurrentTool;
    QSize canvasSize;

    // Primary, secondary.
    std::pair<QColor, QColor> ToolColors;
    float Tolerance = 0.5f;

private:
    LayerList layers;
    QPixmap basePixmap;
    Layer* currentLayer;


    QPainterPath drawingPath;


    DrawPixmapItem* pixmapItem;
    QPen pen;
    bool isDrawing;
    QPointF lastPoint;        // Ensure this is declared

    void drawLineTo(const QPointF& endPoint, ColorCat category);

    void updateCursor(const QPointF &position);

    QColor getAssignedColor(ColorCat cat);

signals:
    void brushSizeChanged(int sz);
    void colorPicked(ColorCat category, QColor col);
    void Updated();
};

