#ifndef TOPBARIMG_H
#define TOPBARIMG_H

#include <QLabel>
#include <QObject>
#include <QImage>
class TopBarImg : public QLabel
{
    Q_OBJECT
public:
    explicit TopBarImg(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    ~TopBarImg() override;

    TopBarImg();
protected:
    void enterEvent(QEnterEvent* event) override; // Mouse enters the widget
    void leaveEvent(QEvent* event) override; // Mouse leaves the widget
    void mousePressEvent(QMouseEvent* event) override; // Add this line

public:
    size_t VecIndex;
    QImage OriginalImg;
    QString FilePath;

    void SetHoveringBorder(bool IsHovering);
    void SetSelectedBorder(bool IsSelected);

signals:
    void HoverEnter(size_t LblIndex);
    void HoverExit(size_t LblIndex);
    void MouseClicked(size_t LblIndex);
private:
    bool IsCurrentlySelected;
};

#endif // TOPBARIMG_H
