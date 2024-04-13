#include "modelbrowserdialog.h"
#include "ui_modelbrowserdialog.h"




ModelBrowserDialog::ModelBrowserDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ModelBrowserDialog)
{
    ui->setupUi(this);


    Searcher =  std::make_unique<ModelSearcherThread>(this);
    connect(Searcher.get(), &ModelSearcherThread::modelsFound, this, &ModelBrowserDialog::onModelsFound);
    connect(Searcher.get(), &ModelSearcherThread::reqMaxProgressBar, this, &ModelBrowserDialog::onRequestMaxProgressBar);
    Searcher->hideNonQualifying = false;

    Searcher->start();





}

ModelBrowserDialog::~ModelBrowserDialog()
{

    // if we don't do this the GUI thread hangs after closing the dialog.
    Searcher->terminate();


    delete ui;
}

void ModelBrowserDialog::on_btnSearch_clicked()
{
    ui->lstModels->clear();

    if (Searcher->Busy())
        Searcher->Skip();

    Searcher->enqueueSearch(ui->ledtModelSearch->text());





    if (!progressPoller)
    {
        progressPoller = std::make_unique<QTimer>(this);
        progressPoller->setInterval(100);

        connect(progressPoller.get(), &QTimer::timeout, this, &ModelBrowserDialog::onProgressPoll);
        progressPoller->start();

    }

}

void ModelBrowserDialog::onModelsFound(QStringList models)
{


    ui->lstModels->addItems(models);


}

void ModelBrowserDialog::onRequestMaxProgressBar(int maxpg)
{
    ui->pgbSearchProg->setMaximum(maxpg);

}

void ModelBrowserDialog::onProgressPoll()
{
    ui->pgbSearchProg->setValue(Searcher->numProcessed());

}


void ModelBrowserDialog::on_chkHideNonQual_clicked(bool checked)
{
    Searcher->hideNonQualifying = checked;
}


void ModelBrowserDialog::on_ledtModelSearch_returnPressed()
{
    on_btnSearch_clicked();
}


void ModelBrowserDialog::on_btnExit_clicked()
{
    accept();
}


void ModelBrowserDialog::on_lstModels_itemDoubleClicked(QListWidgetItem *item)
{

    emit requestModelDownload(item->text());

}


void ModelBrowserDialog::on_btnDownload_clicked()
{
    if (ui->lstModels->selectedItems().isEmpty())
        return;

    on_lstModels_itemDoubleClicked(ui->lstModels->selectedItems()[0]);
}

