#ifndef IMAGESAVERTHREAD_H
#define IMAGESAVERTHREAD_H
#include <QThread>
#include <QPixmap>
#include "threadsafequeue.hpp"

// Image, out path.
using ImageSaveOrder = std::pair<QPixmap, QString>;

// Saving images takes a long ass time and most of the time the user doesn't need to access the image on disk immediately;
// therefore, we delegate it to a constantly-running thread.
class ImageSaverThread : public QThread
{
    Q_OBJECT
private:
    ThreadSafeQueue<ImageSaveOrder> Queue;
    bool Running = true;
public:
    ImageSaverThread();

    void Push(const QImage& Img, const QString& Path);
    void Push(const QPixmap& Pix, const QString& Path);
    void Stop();

protected:
    void run() override;
};

#endif // IMAGESAVERTHREAD_H
