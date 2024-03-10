#ifndef PAINTABLECANVAS_H
#define PAINTABLECANVAS_H

#include <QWidget>
#include <QImage>
#include <QMouseEvent>
#include <QPainter>

class PaintableCanvas : public QWidget {
    Q_OBJECT

public:
    explicit PaintableCanvas(QWidget *parent = nullptr);
    void loadImage(const QString &imagePath);
    void loadImage(const QImage& Img);
    void setPaintingEnabled(bool enabled);
    void clearStrokes();

    QImage getImage() const;
    QImage getMask() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    // Implementation in PaintableCanvas.cpp
    void wheelEvent(QWheelEvent *event) override;
private:

    void Initialize();
    void setCustomCursor();
    int brushSize;
    QImage canvasImage;
    QImage originalImage;  // Stores the original full-resolution image
    QImage maskImage;      // Stores the mask at full resolution
    QImage displayedImage; // Stores the scaled version of the image for display

    bool paintingEnabled = false;
    QPoint lastPoint;

signals:
    void OnImageSet();
};

#endif // PAINTABLECANVAS_H
