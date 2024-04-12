#include "modelbrowserdialog.h"
#include "ui_modelbrowserdialog.h"

ModelBrowserDialog::ModelBrowserDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ModelBrowserDialog)
{
    ui->setupUi(this);


    Searcher = new ModelSearcherThread(this);
    connect(Searcher, &ModelSearcherThread::modelsFound, this, &ModelBrowserDialog::onModelsFound);
    connect(Searcher, &ModelSearcherThread::reqMaxProgressBar, this, &ModelBrowserDialog::onRequestMaxProgressBar);

    Searcher->start();





}

ModelBrowserDialog::~ModelBrowserDialog()
{

    // if we don't do this the GUI thread hangs after closing the dialog.
    Searcher->terminate();
    delete Searcher;

    if (progressPoller)
        delete progressPoller;

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
        progressPoller = new QTimer(this);
        progressPoller->setInterval(100);

        connect(progressPoller, &QTimer::timeout, this, &ModelBrowserDialog::onProgressPoll);
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

