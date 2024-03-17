#pragma once

#include <QListWidget>
#include <QMimeData>

class DraggableListWidget : public QListWidget {
    Q_OBJECT

public:
    DraggableListWidget(QWidget *parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
};

