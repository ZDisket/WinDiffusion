#ifndef CLICKABLEIMAGELABEL_H
#define CLICKABLEIMAGELABEL_H

#include <QLabel>
#include <QImage>
#include <QMouseEvent>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include "ext/maybe_ptr.hpp"

/*
 ClickableImageLabel: A widget for display of result images
 It's designed to only own its images in specific scenarios, hence the heavy use of pointers
*/
class ClickableImageLabel : public QLabel {
    Q_OBJECT

public:


    explicit ClickableImageLabel(QWidget *parent = nullptr);
    ~ClickableImageLabel() override;

    void loadImage(const QString& imagePath);
    void SetImage(QImage *Img, QString *Path = nullptr, bool TransferOwnership = false);
    void ResetImage();
    void SetImagePreview(QImage& Img);

    QImage* GetOriginalImage() {return OriginalImage.get();};

    QSize getPreviewSize() const;
    void setPreviewSize(const QSize &newPreviewSize);

private:

    // shared_ptr is not applicable here since TopBarImg allocates its image as an owning class member
    // also, it would take too much refactoring
    maybe_ptr<QImage> OriginalImage;

    QString OriginalFilePath = "";

    QSize PreviewSize = QSize(430,430);


protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

    void mousePressEvent(QMouseEvent* event) override; // Handle mouse press event

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;


signals:
    void OnMenuOpen(QMenu* InMenu);

    void SendImageToInpaint(QImage* Img);
    void SendImageToImg2Img(QImage* Img);
    void SendImageToUpscale(QImage* Img, bool TransOwnership);


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
