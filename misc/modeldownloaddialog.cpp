#include "modeldownloaddialog.h"
#include "ui_modeldownloaddialog.h"

#include "../QtAxodoxInteropCommon.hpp"

using namespace Axodox;
using namespace Threading;
using namespace QtAxInterop;
using namespace std;

#include <QTimer>
#include <QMessageBox>

ModelDownloadDialog::ModelDownloadDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ModelDownloadDialog)
{
    ui->setupUi(this);

    Downloader = std::make_unique<ModelDownloaderThread>(this);
    AsyncOp = std::make_unique<async_operation>();



    connect(Downloader.get(), &ModelDownloaderThread::verificationFinished, this, &ModelDownloadDialog::onVerificationFinished);
    connect(Downloader.get(), &ModelDownloaderThread::downloadCanceled, this, &ModelDownloadDialog::onDownloadCanceled);

    Downloader->AsOp = AsyncOp.get();
    Downloader->start();

    progressPoller = std::make_unique<QTimer>(this);

    progressPoller->setInterval(100);

    connect(progressPoller.get(), &QTimer::timeout, this, &ModelDownloadDialog::onProgressPoll);

    progressPoller->start();





}

ModelDownloadDialog::~ModelDownloadDialog()
{
    Downloader->terminate();
    delete ui;
}

void ModelDownloadDialog::on_btnVerify_clicked()
{

    Downloader->enqueueVerification(ui->ledtModelName->text());

}

const QStringList VerifPixmaps = {":/res/check.png", ":/res/cross.png", ":/res/warning.png"};
const QStringList VerifTexts = {"Valid model", "This model doesn't exist!", "This does not appear to be a Stable Diffusion ONNX model"};


void ModelDownloadDialog::onVerificationFinished(ModelVerifStatus Stat)
{

    ui->lblImgCheck->setPixmap(
        QPixmap(VerifPixmaps[(int)Stat]).scaled(64,64, Qt::KeepAspectRatio, Qt::SmoothTransformation)
    );

    ui->lblProgressShow->setText(VerifTexts[(int)Stat]);

    // set the default save model name to the model name without the author name
    if (ui->ledtSaveModelName->text().isEmpty() || ui->chkAutoSaveName->isChecked())
        ui->ledtSaveModelName->setText(ui->ledtModelName->text().split("/")[1]);


    lastVerifiedModel = {ui->ledtModelName->text(), Stat};

    if (DownloadAfterVerify)
    {
        if (Stat == ModelVerifStatus::Good)
            on_btnDownload_clicked();


        DownloadAfterVerify = false;
    }


}

void ModelDownloadDialog::onDownloadCanceled(QString modelId)
{
    AsyncOp = std::make_unique<async_operation>();
    Downloader->AsOp = AsyncOp.get();

    ui->btnDownload->setEnabled(true);

}

void ModelDownloadDialog::onProgressPoll()
{

    ui->lblImgCheck->setVisible(!Downloader->isDownloading());
    ui->btnDownload->setText(Downloader->isDownloading() ? "Cancel" : "Download");

    if (!Downloader->isDownloading())
        return;


    async_operation_info inf = AsyncOp->state();

    ui->pgbDownProgress->setValue(
        InterOpHelper::ZeroOneToSliderRange(inf.progress)
        );

    ui->lblProgressShow->setText(
        QString::fromStdString(inf.status_message)
        );


}


void ModelDownloadDialog::on_btnDownload_clicked()
{

    if (Downloader->isDownloading())
    {
        AsyncOp->cancel();
        ui->btnDownload->setEnabled(false);

        return;

    }

    if (lastVerifiedModel.first != ui->ledtModelName->text())
    {
        on_btnVerify_clicked();
        DownloadAfterVerify = true;
        return;

    }

    if (lastVerifiedModel.second == ModelVerifStatus::DoesntExist)
    {
        QMessageBox::critical(this, "Error", "Cannot download a model that doesn't exist. Please review your entry");
        return;

    }
    else if (lastVerifiedModel.second == ModelVerifStatus::Invalid)
    {
        int butt = QMessageBox::warning(this, "Warning!", "This is not a valid Stable Diffusion ONNX model. Do you want to download anyway?\nOnly press Yes if you REALLY know what you're doing!",
                                      QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

        if (butt != QMessageBox::Yes)
            return;



    }



    filesystem::path appDirPath = QCoreApplication::applicationDirPath().toStdString();




    Downloader->enqueueDownload(ui->ledtModelName->text(),
                                appDirPath / "models" / ui->ledtSaveModelName->text().toStdString()
                                );

}


void ModelDownloadDialog::on_ledtSaveModelName_editingFinished()
{
    ui->chkAutoSaveName->setChecked(false);
}

