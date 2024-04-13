#ifndef MODELBROWSERDIALOG_H
#define MODELBROWSERDIALOG_H

#include <QDialog>
#include "modelloadingthreads.h"
#include <QTimer>
#include <QListWidget>

namespace Ui {
class ModelBrowserDialog;
}

class ModelBrowserDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ModelBrowserDialog(QWidget *parent = nullptr);
    ~ModelBrowserDialog();

private slots:
    void on_btnSearch_clicked();


    void on_chkHideNonQual_clicked(bool checked);

    void on_ledtModelSearch_returnPressed();

    void on_btnExit_clicked();

    void on_lstModels_itemDoubleClicked(QListWidgetItem *item);

protected slots:
    void onModelsFound(QStringList models);
    void onRequestMaxProgressBar(int maxpg);
    void onProgressPoll();

private:
    std::unique_ptr<QTimer> progressPoller;
    std::unique_ptr<ModelSearcherThread> Searcher;
    Ui::ModelBrowserDialog *ui;
};

#endif // MODELBROWSERDIALOG_H
