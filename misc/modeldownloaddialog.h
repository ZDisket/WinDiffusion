#ifndef MODELDOWNLOADDIALOG_H
#define MODELDOWNLOADDIALOG_H

#include <QDialog>
#include "modelloadingthreads.h"

namespace Ui {
class ModelDownloadDialog;
}

class ModelDownloadDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ModelDownloadDialog(QWidget *parent = nullptr, const QString& preload = "");
    ~ModelDownloadDialog();


signals:
    void requestModelRefresh();


private slots:
    void on_btnVerify_clicked();

    void on_btnDownload_clicked();

    void on_ledtSaveModelName_editingFinished();

protected slots:
    void onVerificationFinished(ModelVerifStatus Stat);
    void onDownloadCanceled(QString modelId);
    void onProgressPoll();
    void onDownloadFinished(QString modelId, bool success);

private:
    bool DownloadAfterVerify = false;
    std::pair<QString, ModelVerifStatus> lastVerifiedModel = {"", ModelVerifStatus::DoesntExist};
    std::unique_ptr<ModelDownloaderThread> Downloader;
    std::unique_ptr<QTimer> progressPoller;
    std::unique_ptr<Axodox::Threading::async_operation> AsyncOp;

    Ui::ModelDownloadDialog *ui;
};

#endif // MODELDOWNLOADDIALOG_H
