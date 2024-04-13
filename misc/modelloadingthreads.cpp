#include "modelloadingthreads.h"



using namespace Axodox;
using namespace Axodox::Web;

ModelSearcherThread::ModelSearcherThread(QObject *parent)
    : QThread(parent), running(true) {}

ModelSearcherThread::~ModelSearcherThread() {
    running = false;

    wait();
}

void ModelSearcherThread::run() {
    while (running) {
        QString searchTerm;
        searchQueue.wait_and_pop(searchTerm);

        isBusy = true;
        Processed = 0;

        auto models = client.GetModels(searchTerm.toStdString());

        emit reqMaxProgressBar(models.size());

        QStringList modelList;
        for (const auto &model : models) {

            if (skipThisOne)
            {
                skipThisOne = false;
                break;

            }


            if (modelList.size() > 4)
                Flush(modelList);

            if (hideNonQualifying)
            {
                auto modelDetails = client.GetModel(model);
                if (!modelDetails->IsValidModel(StableDiffusionOnnxFileset))
                {
                    Processed++;
                    continue;

                }

            }

            modelList << QString::fromStdString(model);
            Processed++;
        }
        Flush(modelList);
        isBusy = false;
    }
}

void ModelSearcherThread::enqueueSearch(const QString &searchTerm) {
    searchQueue.push(searchTerm);
}

void ModelSearcherThread::Skip()
{
    skipThisOne = true;
}

void ModelSearcherThread::Flush(QStringList &modelList)
{
    emit modelsFound(modelList);
    modelList.clear();


}


// #########################################################################################

ModelDownloaderThread::ModelDownloaderThread(QObject *parent)
    : QThread(parent), running(true) {

     qRegisterMetaType<ModelVerifStatus>("ModelVerifStatus");

}

ModelDownloaderThread::~ModelDownloaderThread() {
    running = false;


    wait();
}

void ModelDownloaderThread::run() {
    while (running) {
        DownloadRequest request;
        downloadQueue.wait_and_pop(request);

        if (request.justVerify)
        {

            ModelVerifStatus Stat = VerifyModel(request.modelId);
            emit verificationFinished(Stat);




            continue;
        }


        emit downloadStarted(request.modelId);
        Downloading = true;


        bool success = client.TryDownloadModel(request.modelId.toStdString(), StableDiffusionOnnxFileset, StableDiffusionOnnxOptionals, request.targetPath, *AsOp);

        Downloading = false;

        if (AsOp->state().is_cancelled)
            emit downloadCanceled(request.modelId);
        else
            emit downloadFinished(request.modelId, success);


    }
}

void ModelDownloaderThread::enqueueDownload(const QString &modelId, const std::filesystem::path &path) {
    DownloadRequest request{modelId, path};
    downloadQueue.push(request);
}

void ModelDownloaderThread::enqueueVerification(const QString &modelId)
{
    // Clang doesn't mind mixing designated and non-designated initializers, but MSVC bitches with an error
    // Dumb.
    DownloadRequest reqvef{.modelId = modelId, .justVerify = true};
    downloadQueue.push(reqvef);
}

bool ModelDownloaderThread::isDownloading() const
{
    return Downloading;
}

ModelVerifStatus ModelDownloaderThread::VerifyModel(const QString &modelId)
{
    auto modelDetails = client.GetModel(modelId.toStdString());

    if (!modelDetails.has_value())
        return ModelVerifStatus::DoesntExist;

    if (!modelDetails->IsValidModel(StableDiffusionOnnxFileset))
        return ModelVerifStatus::Invalid;

    return ModelVerifStatus::Good;
}

