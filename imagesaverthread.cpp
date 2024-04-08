#include "imagesaverthread.h"
#include <QDebug>

ImageSaverThread::ImageSaverThread() {}
using namespace std;
void ImageSaverThread::Push(const std::variant<QImage, QPixmap> &Img, const QString &Path)
{
    if (holds_alternative<QImage>(Img)){

        Queue.push(
            {QPixmap::fromImage(get<QImage>(Img)),
             Path}
            );

    }
    else if (holds_alternative<QPixmap>(Img))
    {
        Queue.push({get<QPixmap>(Img), Path});

    }else{

        throw std::invalid_argument("Unknown variant type for ImageSaverThread::Push; valid are QImage and QPixmap");
    }



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
