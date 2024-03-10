#include "paintablecanvas.h"
#include <QPainter>
#include <QMimeData>
#include <QFileInfo>
#include <QImageReader>

PaintableCanvas::PaintableCanvas(QWidget *parent) : QWidget(parent), paintingEnabled(false)
{
    brushSize = 10;
    setAcceptDrops(true);


}


void PaintableCanvas::loadImage(const QString &imagePath) {
    originalImage.load(imagePath);
    Initialize();
    emit OnImageSet();
}

void PaintableCanvas::loadImage(const QImage &Img)
{
    originalImage = Img.copy();
    Initialize();
    emit OnImageSet();
}

void PaintableCanvas::setPaintingEnabled(bool enabled) {
    paintingEnabled = enabled;
}

QImage PaintableCanvas::getImage() const
{
    return originalImage;
}

QImage PaintableCanvas::getMask() const {
    return maskImage;
}

void PaintableCanvas::enterEvent(QEnterEvent *event) {
    QWidget::enterEvent(event);
    if (!paintingEnabled)
        return;
    setCustomCursor();
}

void PaintableCanvas::leaveEvent(QEvent *event) {
    QWidget::leaveEvent(event);
    unsetCursor(); // Revert to the default cursor
}

void PaintableCanvas::setCustomCursor() {
    QPixmap pixmap(brushSize * 2 + 2, brushSize * 2 + 2); // +2 for the outline width
    pixmap.fill(Qt::transparent); // Ensure the pixmap is transparent

    // Draw a white circle outline in the center of the pixmap
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    QPen pen(Qt::white, 2); // White pen for the outline, 2 pixels wide
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(1, 1, brushSize * 2 - 1, brushSize * 2 - 1);

    // Set the custom cursor
    QCursor cursor(pixmap);
    setCursor(cursor);
}


void PaintableCanvas::paintEvent(QPaintEvent *event) {
    QPainter painter(this);

    painter.setRenderHint(QPainter::SmoothPixmapTransform); // Enable smooth pixmap transformation
    painter.setRenderHint(QPainter::Antialiasing); // Enable antialiasing

    painter.drawImage(0, 0, displayedImage); // Draw the scaled image for display

    if (!paintingEnabled)
        return;

    // Optionally: draw the scaled mask over it
    QImage scaledMask = maskImage.scaled(460, 460, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    painter.setOpacity(0.65f);
    painter.drawImage(0, 0, scaledMask);
    painter.setOpacity(1.0f);
}
void PaintableCanvas::mousePressEvent(QMouseEvent *event) {
    if (!paintingEnabled) return;
    lastPoint = event->pos();
}

void PaintableCanvas::mouseMoveEvent(QMouseEvent *event) {
    if (!paintingEnabled || !(event->buttons() & Qt::LeftButton)) return;

    // Calculate the scale factor
    double scaleFactorX = static_cast<double>(originalImage.width()) / displayedImage.width();
    double scaleFactorY = static_cast<double>(originalImage.height()) / displayedImage.height();

    // Adjust lastPoint and current point according to the scale factor
    QPoint scaledLastPoint(lastPoint.x() * scaleFactorX, lastPoint.y() * scaleFactorY);
    QPoint scaledCurrentPoint(event->pos().x() * scaleFactorX, event->pos().y() * scaleFactorY);

    QPainter painter(&maskImage);
    // Set pen for smoother, semi-transparent strokes
    QPen pen(QColor(255, 255, 255, 255), (brushSize + 4) * scaleFactorX, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin); // 50% opacity
    painter.setPen(pen);
    painter.drawLine(scaledLastPoint, scaledCurrentPoint);

    lastPoint = event->pos();
    update(); // Trigger a repaint to show the changes
}

void PaintableCanvas::mouseReleaseEvent(QMouseEvent *event) {
    if (!paintingEnabled) return;
}




void PaintableCanvas::dragEnterEvent(QDragEnterEvent *event) {
    // Check if event contains URLs and accept it if there's at least one image file
    if (event->mimeData()->hasUrls() && !event->mimeData()->urls().isEmpty()) {
        QList<QUrl> urls = event->mimeData()->urls();
        for (const QUrl &url : urls) {
            QFileInfo fileInfo(url.toLocalFile());
            if (fileInfo.exists() && fileInfo.isFile() && QImageReader::imageFormat(fileInfo.filePath()).isEmpty() == false) {
                event->acceptProposedAction();
                return;
            }
        }
    }
}

void PaintableCanvas::dropEvent(QDropEvent *event) {
    // Process the first valid image URL from the dropped items
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urls = mimeData->urls();
        for (const QUrl &url : urls) {
            QString filePath = url.toLocalFile();
            QFileInfo fileInfo(filePath);
            if (fileInfo.exists() && fileInfo.isFile() && QImageReader::imageFormat(filePath).isEmpty() == false) {
                loadImage(filePath); // Assuming loadImage() loads and displays the image
                event->acceptProposedAction();
                return;
            }
        }
    }
}

void PaintableCanvas::wheelEvent(QWheelEvent *event) {
    const int delta = event->angleDelta().y();
    if (delta > 0) {
        brushSize += 2; // Increase brush size
    } else if (delta < 0 && brushSize > 2) {
        brushSize -= 2; // Decrease brush size but prevent it from becoming too small
    }

    setCustomCursor(); // Update the cursor to reflect the new brush size
    event->accept();
}

void PaintableCanvas::clearStrokes()
{
    maskImage = QImage(originalImage.size(), QImage::Format_ARGB32_Premultiplied);
    maskImage.fill(Qt::transparent);
    update(); // Trigger a repaint
}

void PaintableCanvas::Initialize()
{

    // Scale the original image for display

    displayedImage = originalImage.scaled(460, 460, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    setMinimumSize(displayedImage.size());


    // Initialize maskImage as transparent and the same size as the original image
    maskImage = QImage(originalImage.size(), QImage::Format_ARGB32_Premultiplied);
    maskImage.fill(Qt::transparent);
    update(); // Trigger a repaint
}
