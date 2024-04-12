#include "modelloadingthreads.h"

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
