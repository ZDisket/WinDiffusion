#ifndef QFLOODFILL_H
#define QFLOODFILL_H
// Thanks https://github.com/reunanen/qt-image-flood-fill/tree/master

#include <QPixmap>

void FloodFill(QPixmap& pixmap, QPoint seed, QColor newColor, float tolerance, QPainter& painter);

#endif // QFLOODFILL_H
