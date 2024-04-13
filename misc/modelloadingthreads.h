#pragma once

/*
 I am the THREADMAXXER
*/

#include <QThread>
#include <QStringList>
#include "Web/HuggingFaceClient.h"
#include "../threadsafequeue.hpp"



// Define our own
const std::set<std::string> StableDiffusionOnnxFileset = {
    "scheduler/scheduler_config.json",
    "text_encoder/model.onnx",
    "tokenizer/merges.txt",
    "tokenizer/special_tokens_map.json",
    "tokenizer/tokenizer_config.json",
    "tokenizer/vocab.json",
    "unet/model.onnx",
    "vae_decoder/model.onnx",
    "vae_encoder/model.onnx"
};

const std::set<std::string> StableDiffusionOnnxOptionals = {
    "feature_extractor/preprocessor_config.json",
    "controlnet/model.onnx",
    "safety_checker/model.onnx",
    "text_encoder_2/model.onnx",
    "text_encoder_2/model.onnx.data",
    "tokenizer_2/merges.txt",
    "tokenizer_2/special_tokens_map.json",
    "tokenizer_2/tokenizer_config.json",
    "tokenizer_2/vocab.json",
    "unet/model.onnx.data",
    "windiffusion.json"
};




class ModelSearcherThread : public QThread {
    Q_OBJECT

public:
    explicit ModelSearcherThread(QObject *parent = nullptr);
    ~ModelSearcherThread();

    void run() override;
    void enqueueSearch(const QString &searchTerm);

    std::atomic<bool> hideNonQualifying = true;

    bool Busy() const {return isBusy.load();};
    int numProcessed() const {return Processed.load();}
    void Skip();

signals:
    void modelsFound(QStringList modelList);
    void reqMaxProgressBar(int maxPg);

private:
    std::atomic<bool> skipThisOne = false;

    std::atomic<bool> isBusy = false;
    std::atomic<int> Processed = 0;

    void Flush(QStringList& modelList);

    ThreadSafeQueue<QString> searchQueue;
    Axodox::Web::HuggingFaceClient client;
    bool running;
};


struct DownloadRequest {
    QString modelId;
    std::filesystem::path targetPath;
    bool justVerify = false;
};

enum class ModelVerifStatus{
  Good = 0,
  DoesntExist,
  Invalid
};


class ModelDownloaderThread : public QThread {
    Q_OBJECT

public:


    explicit ModelDownloaderThread(QObject *parent = nullptr);

    Axodox::Threading::async_operation* AsOp = nullptr;

    ~ModelDownloaderThread();

    void run() override;
    void enqueueDownload(const QString &modelId, const std::filesystem::path &path);
    void enqueueVerification(const QString& modelId);

    bool isDownloading() const;

signals:
    void downloadStarted(QString modelId);
    void downloadFinished(QString modelId, bool success);
    void verificationFinished(ModelVerifStatus Stat);
    void downloadCanceled(QString modelId);

private:

    std::atomic<bool> Downloading = false;

    ModelVerifStatus VerifyModel(const QString& modelId);

    ThreadSafeQueue<DownloadRequest> downloadQueue;
    Axodox::Web::HuggingFaceClient client;
    std::atomic<bool> running;
};
