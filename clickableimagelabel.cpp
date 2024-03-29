#include "ClickableImageLabel.h"
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QProcess>
#include <QMimeData>
#include <QImageReader>

ClickableImageLabel::ClickableImageLabel(QWidget *parent) : QLabel(parent) {
    OriginalImage = nullptr;
    pToOriginalFilePath = nullptr;

    actSendToImg2Img = new QAction("Send to img2img",this);
    actSendToInpaint = new QAction("Send to inpaint",this);
    actSendToUpscale = new QAction("Send to upscale",this);
    actSendToFavorites = new QAction("Save to favorites",this);

   connect(actSendToImg2Img, &QAction::triggered, this, &ClickableImageLabel::OnClickSendToImg2Img);
   connect(actSendToInpaint, &QAction::triggered, this, &ClickableImageLabel::OnClickSendToInpaint);
   connect(actSendToUpscale, &QAction::triggered, this, &ClickableImageLabel::OnClickSendToUpscale);
   connect(actSendToFavorites, &QAction::triggered, this, &ClickableImageLabel::Favorite);

   OwnsImage = false;
}

ClickableImageLabel::~ClickableImageLabel()
{
    if (OwnsImage && OriginalImage)
    {
        delete OriginalImage;
        delete pToOriginalFilePath;
    }


}

void ClickableImageLabel::loadImage(const QString &imagePath)
{
    OriginalImage = new QImage(imagePath);
    pToOriginalFilePath = new QString(imagePath);

    OwnsImage = true;
    SetImage(OriginalImage);

}

void ClickableImageLabel::SetImage(QImage* Img)
{
    OriginalImage = Img;
    setPixmap(
        QPixmap::fromImage(OriginalImage->scaled(pixmap().size(), Qt::KeepAspectRatio, Qt::SmoothTransformation))
        );
}

void ClickableImageLabel::SetImagePreview(QImage &Img)
{
    OriginalImage = nullptr;
    setPixmap(
        QPixmap::fromImage(Img.scaled(pixmap().size(), Qt::KeepAspectRatio, Qt::SmoothTransformation))
        );
}

void ClickableImageLabel::mousePressEvent(QMouseEvent* event) {
    QLabel::mousePressEvent(event); // Call the base class implementation

    if (!OriginalImage)
        return;

    if (event->button() != Qt::MouseButton::LeftButton)
        return;

    if (OriginalImage->size().width() > 1024 && OriginalImage->size().height() > 1024)
    {
        // Open the image (path is defined by class member QString* pToOriginalFilePath) in windows image viewer
        QDesktopServices::openUrl(QUrl::fromLocalFile(*pToOriginalFilePath));


        return;
    }


    // Create and show the dialog when the label is clicked
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Image Preview");
    QVBoxLayout *layout = new QVBoxLayout(dialog);

    QLabel *imageLabel = new QLabel(dialog);
    imageLabel->setPixmap(QPixmap::fromImage(*OriginalImage));
    layout->addWidget(imageLabel);


    dialog->resize(OriginalImage->size());

    dialog->exec(); // Show the dialog modally
}



void ClickableImageLabel::contextMenuEvent(QContextMenuEvent *event) {
    QLabel::contextMenuEvent(event);
    if (!pToOriginalFilePath)
        return;

    QMenu contextMenu(this);

    QAction action("Show in folder", this);
    connect(&action, &QAction::triggered, this, &ClickableImageLabel::showInFolder);
    contextMenu.addAction(&action);

   // emit OnMenuOpen(&contextMenu);


    contextMenu.addAction(actSendToImg2Img);
    contextMenu.addAction(actSendToInpaint);
    contextMenu.addAction(actSendToUpscale);
    contextMenu.addAction(actSendToFavorites);

    contextMenu.exec(event->globalPos());
}

void ClickableImageLabel::showInFolder() {
    if (pToOriginalFilePath && !pToOriginalFilePath->isEmpty()) {
        // Ensure the path is in Windows format (backslashes)
        QString winPath = QDir::toNativeSeparators(*pToOriginalFilePath);

        QStringList args;
        args << "/select," << winPath;
        QProcess::startDetached("explorer", args);

    }
}


void ClickableImageLabel::Favorite() {
    QString FavesDir = QCoreApplication::applicationDirPath() + "/favorites/";

    // Ensure the favorites directory exists; create it if it doesn't
    QDir dir(FavesDir);
    if (!dir.exists()) {
        dir.mkpath(FavesDir);
    }

    if (pToOriginalFilePath && !pToOriginalFilePath->isEmpty()) {
        // Construct the base filename for the new favorite file
        QString baseFileName = "fave_";
        QString fileExtension = ".png";

        // Find the lowest number that can be used without overwriting an existing file
        int num = 1;
        QString newFileName;
        do {
            newFileName = baseFileName + QString::number(num++) + fileExtension;
        } while (dir.exists(newFileName));

        // Construct the full new file path
        QString newFilePath = FavesDir + newFileName;

        // Copy the original image file to the new favorite file path
        bool success = QFile::copy(*pToOriginalFilePath, newFilePath);

        if (!success) {
            qDebug() << "Could not copy the file to the favorites directory: " << newFilePath;
        }
    } else {
        qDebug() << "Original file path is not set.";
    }
}


void ClickableImageLabel::OnClickSendToImg2Img()
{

    emit SendImageToImg2Img(OriginalImage);
}

void ClickableImageLabel::OnClickSendToInpaint()
{
    emit SendImageToInpaint(OriginalImage);

}

void ClickableImageLabel::OnClickSendToUpscale()
{
    emit SendImageToUpscale(OriginalImage);

}


void ClickableImageLabel::dragEnterEvent(QDragEnterEvent *event) {
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

void ClickableImageLabel::dropEvent(QDropEvent *event) {
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
