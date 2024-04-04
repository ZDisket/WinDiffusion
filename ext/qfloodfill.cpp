#include "qfloodfill.h"
#include <deque>
#include <unordered_set>
#include <stdexcept>
#include <QPainter>
#include <QDebug>
#include "ext/ColorUtils.hpp"

bool isColorSimilar(const QColor& color1, const QColor& color2, float tolerance) {
    // Convert QColor to the library's rgbColor format
    ColorUtils::rgbColor c1(static_cast<unsigned int>(color1.red()),
                            static_cast<unsigned int>(color1.green()),
                            static_cast<unsigned int>(color1.blue()));

    ColorUtils::rgbColor c2(static_cast<unsigned int>(color2.red()),
                            static_cast<unsigned int>(color2.green()),
                            static_cast<unsigned int>(color2.blue()));

    // Calculate Delta-E for RGB components
    float deltaE = ColorUtils::getColorDeltaE(c1, c2);

    // Calculate Euclidean distance for alpha
    int alphaDiff = color1.alpha() - color2.alpha();

    float normalizedAlphaDistance = static_cast<float>(alphaDiff) / 255.0f;
    float totalDistance = std::sqrt(deltaE * deltaE + normalizedAlphaDistance * normalizedAlphaDistance);
    totalDistance /= 100.f;

    // Adjust tolerance if necessary based on how you want to weight color vs. alpha differences
    return totalDistance <= tolerance;
}


QColor getColorFromPixmap(const QPixmap& pixmap, int x, int y) {
    // Convert QPixmap to QImage
    QImage image = pixmap.toImage();

    // Check if the point is within the image bounds
    if(x >= 0 && y >= 0 && x < image.width() && y < image.height()) {
        // Get the color at the specified point
        QColor color = image.pixelColor(x, y);
        // Return the color
        return color;
    } else {
        // Return a default color (e.g., black) if the point is out of bounds
        return QColor(0, 0, 0);
    }
}

std::deque<QPoint> GetPoints(QImage& image, QPoint seed, float tolerance)
{
    // A really simple algorithm for the time being. Feel free to improve!

    if (image.format() != QImage::Format_ARGB32_Premultiplied) {
        throw std::runtime_error("The only supported input image format is QImage::Format_ARGB32_Premultiplied");
    }

    const int width = image.width();
    const int height = image.height();

    QRgb* bits = reinterpret_cast<QRgb*>(image.bits());

    const auto getPixel = [&](int x, int y) {
        return bits[(y * width) + x];
    };

    const QRgb oldRgba = getPixel(seed.x(), seed.y());

    std::vector<unsigned char> processedAlready(width * height);

    std::deque<QPoint> backlog = { seed };
    std::deque<QPoint> points;

    while (!backlog.empty()) {
        const QPoint& point = backlog.front();
        const int x = point.x();
        const int y = point.y();
        if (x >= 0 && y >= 0 && x < width && y < height) {
            const QRgb rgba = getPixel(x, y);
            if (isColorSimilar(rgba,oldRgba, tolerance)) {
                unsigned char& isProcessedAlready = processedAlready[(y * width) + x];
                if (!isProcessedAlready) {
                    isProcessedAlready = true;
                    points.push_back(point);
                    backlog.push_back(QPoint(x - 1, y));
                    backlog.push_back(QPoint(x, y - 1));
                    backlog.push_back(QPoint(x + 1, y));
                    backlog.push_back(QPoint(x, y + 1));
                }
            }
        }
        backlog.pop_front();
    }

    return points;
}

void FloodFill(QPixmap& pixmap, QPoint seed, QColor newColor, float tolerance, QPainter& painter) {
    QImage image = pixmap.toImage();
    if (image.format() != QImage::Format_ARGB32_Premultiplied) {
        image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    }

    const auto points = GetPoints(image, seed, tolerance); // Pass tolerance here

    painter.setPen(newColor);
    painter.setBrush(newColor);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    for (const QPoint& point : points) {
        painter.drawPoint(point);
    }
}

