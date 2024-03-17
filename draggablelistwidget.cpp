// DraggableListWidget.cpp
#include "draggablelistwidget.h"
#include "pathwidgetitem.h"
#include "qevent.h"
#include <QFileInfo>

DraggableListWidget::DraggableListWidget(QWidget *parent) : QListWidget(parent) {
    // Enable drag and drop
    setAcceptDrops(true);
}

void DraggableListWidget::dragEnterEvent(QDragEnterEvent *event) {
    // Accept the drag event if it contains URLs (file paths)
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void DraggableListWidget::dragMoveEvent(QDragMoveEvent *event) {
    // Accept the drag move event if it contains URLs
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void DraggableListWidget::dropEvent(QDropEvent *event) {
    const QMimeData *mimeData = event->mimeData();

    // Check if the dropped data contains URLs
    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();

        // Process each URL
        for (const QUrl &url : urlList) {
            QFileInfo fileInfo(url.toLocalFile());
            QString filePath = fileInfo.absoluteFilePath();

            // Filter for .png, .jpg, .jpeg files
            if (fileInfo.suffix().compare("png", Qt::CaseInsensitive) == 0 ||
                fileInfo.suffix().compare("jpg", Qt::CaseInsensitive) == 0 ||
                fileInfo.suffix().compare("jpeg", Qt::CaseInsensitive) == 0) {

                // Add the file as a PathWidgetItem
                new PathWidgetItem(filePath, this);
            }
        }
    }
}
