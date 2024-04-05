#pragma once
#include <QGraphicsView>
#include <QWheelEvent>
#include <QtWidgets>

class DrawGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit DrawGraphicsView(QWidget *parent = nullptr);

    bool needsControlScroll;
    bool acceptContextMenu = false;

protected:
    void wheelEvent(QWheelEvent *pWheelEvent) override;
    bool event(QEvent* event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;



public:
    QAction* actSaveAs;
    QAction* actSendToUpscale;

};

