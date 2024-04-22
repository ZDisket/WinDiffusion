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


    actSendToImg2Img = new QAction("Send to img2img",this);
    actSendToInpaint = new QAction("Send to inpaint",this);
    actSendToUpscale = new QAction("Send to upscale",this);
    actSendToFavorites = new QAction("Save to favorites",this);


   connect(actSendToImg2Img, &QAction::triggered, this, &ClickableImageLabel::OnClickSendToImg2Img);
   connect(actSendToInpaint, &QAction::triggered, this, &ClickableImageLabel::OnClickSendToInpaint);
   connect(actSendToUpscale, &QAction::triggered, this, &ClickableImageLabel::OnClickSendToUpscale);
   connect(actSendToFavorites, &QAction::triggered, this, &ClickableImageLabel::Favorite);

}

ClickableImageLabel::~ClickableImageLabel()
{



}

void ClickableImageLabel::loadImage(const QString &imagePath)
{

    OriginalFilePath = imagePath;
    SetImage(new QImage(imagePath), &OriginalFilePath, true);

}


void ClickableImageLabel::SetImage(QImage* Img, QString* Path, bool TransferOwnership)
{

    OriginalImage = maybe_ptr<QImage>(Img, TransferOwnership);
    setPixmap(
        QPixmap::fromImage(OriginalImage->scaled(PreviewSize, Qt::KeepAspectRatio, Qt::SmoothTransformation))
        );

    if (Path)
        OriginalFilePath = *Path;
    else
        OriginalFilePath = "";

}

void ClickableImageLabel::ResetImage()
{
    if (OriginalImage)
        OriginalImage.reset();

    OriginalFilePath = "";

    QPixmap EmptyFill(PreviewSize);
    EmptyFill.fill(Qt::white);

    setPixmap(EmptyFill);

}

void ClickableImageLabel::SetImagePreview(QImage &Img)
{
    OriginalImage.reset();
    setPixmap(
        QPixmap::fromImage(Img.scaled(PreviewSize, Qt::KeepAspectRatio, Qt::SmoothTransformation))
        );
}

QSize ClickableImageLabel::getPreviewSize() const
{
    return PreviewSize;
}

void ClickableImageLabel::setPreviewSize(const QSize &newPreviewSize)
{
    PreviewSize = newPreviewSize;

    if (OriginalImage)
        setPixmap(
            QPixmap::fromImage(OriginalImage->scaled(PreviewSize, Qt::KeepAspectRatio, Qt::SmoothTransformation))
            );
    else
        ResetImage();
}

void ClickableImageLabel::mousePressEvent(QMouseEvent* event) {
    QLabel::mousePressEvent(event); // Call the base class implementation

    if (!OriginalImage)
        return;

    if (event->button() != Qt::MouseButton::LeftButton)
        return;

    if (OriginalImage->size().width() > 1024 && OriginalImage->size().height() > 1024)
    {
        // Open the image in windows image viewer
        QDesktopServices::openUrl(QUrl::fromLocalFile(OriginalFilePath));


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
    if (OriginalFilePath.isEmpty())
        return;

    QMenu contextMenu(this);

    QAction action("Show in folder", this);
    connect(&action, &QAction::triggered, this, &ClickableImageLabel::showInFolder);
    contextMenu.addAction(&action);

   // emit OnMenuOpen(&contextMenu);


    if (!isUpscaleResult)
    {

        contextMenu.addAction(actSendToImg2Img);
        contextMenu.addAction(actSendToInpaint);
        contextMenu.addAction(actSendToUpscale);
        contextMenu.addAction(actSendToFavorites);
    }


    contextMenu.exec(event->globalPos());
}

void ClickableImageLabel::showInFolder() {
    if (!OriginalFilePath.isEmpty()) {
        // Ensure the path is in Windows format (backslashes)
        QString winPath = QDir::toNativeSeparators(OriginalFilePath);

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

    if (!OriginalFilePath.isEmpty()) {
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
        bool success = QFile::copy(OriginalFilePath, newFilePath);

        if (!success) {
            qDebug() << "Could not copy the file to the favorites directory: " << newFilePath;
        }
    } else {
        qDebug() << "Original file path is not set.";
    }
}


void ClickableImageLabel::OnClickSendToImg2Img()
{

    emit SendImageToImg2Img(OriginalImage.get());
}

void ClickableImageLabel::OnClickSendToInpaint()
{
    emit SendImageToInpaint(OriginalImage.get());

}

void ClickableImageLabel::OnClickSendToUpscale()
{
    emit SendImageToUpscale(OriginalImage.get(), false);

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
