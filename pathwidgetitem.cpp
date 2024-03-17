#include "pathwidgetitem.h"

PathWidgetItem::PathWidgetItem(const QString &fullFilePath, QListWidget *parent)
    : QListWidgetItem(parent), fullPath(fullFilePath) {
    // Set the item's displayed text to just the filename
    setText(QFileInfo(fullFilePath).fileName());
}

QString PathWidgetItem::getFullPath() const {
    return fullPath;
}
