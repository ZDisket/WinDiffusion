#include "imagesaverthread.h"
#include <QDebug>

ImageSaverThread::ImageSaverThread() {}

void ImageSaverThread::Push(const QImage &Img, const QString &Path)
{
    Push(QPixmap::fromImage(Img), Path);

}

void ImageSaverThread::Push(const QPixmap &Pix, const QString &Path)
{

    Queue.push({Pix, Path});

}

void ImageSaverThread::Stop()
{
    Running = false;

}

void ImageSaverThread::run()
{
    while (Running)
    {
        ImageSaveOrder Ord;

        Queue.wait_and_pop(Ord);

        QPixmap& Pix = Ord.first;
        QString& Path = Ord.second;

        Pix.save(Path, "PNG");

        qDebug() << "Saver thread: Image saved to: " << Path;




    }

}
