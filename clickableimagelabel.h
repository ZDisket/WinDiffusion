#ifndef CLICKABLEIMAGELABEL_H
#define CLICKABLEIMAGELABEL_H

#include <QLabel>
#include <QImage>
#include <QMouseEvent>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>

/*
 ClickableImageLabel: A widget for display of result images
 It should NOT own its images, hence the heavy use of pointers
*/
class ClickableImageLabel : public QLabel {
    Q_OBJECT

public:
    QImage* OriginalImage; // Public member to hold the original image
    QString* pToOriginalFilePath; // Pointer to a QString holding the file path


    explicit ClickableImageLabel(QWidget *parent = nullptr);
    ~ClickableImageLabel() override;

    void loadImage(const QString& imagePath);
    void SetImage(QImage *Img, QString *Path = nullptr);
    void SetImagePreview(QImage& Img);
private:


    bool OwnsImage;

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

    void mousePressEvent(QMouseEvent* event) override; // Handle mouse press event

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;


signals:
    void OnMenuOpen(QMenu* InMenu);

    void SendImageToInpaint(QImage* Img);
    void SendImageToImg2Img(QImage* Img);
    void SendImageToUpscale(QImage* Img);


private slots:
    void showInFolder();
    void Favorite();

    void OnClickSendToImg2Img();
    void OnClickSendToInpaint();
    void OnClickSendToUpscale();

private:
    QAction* actSendToInpaint;
    QAction* actSendToImg2Img;
    QAction* actSendToUpscale;
    QAction* actSendToFavorites;

};

#endif // CLICKABLEIMAGELABEL_H
