#ifndef MODELBROWSERDIALOG_H
#define MODELBROWSERDIALOG_H

#include <QDialog>
#include "modelloadingthreads.h"
#include <QTimer>

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

protected slots:
    void onModelsFound(QStringList models);
    void onRequestMaxProgressBar(int maxpg);
    void onProgressPoll();

private:
    QTimer* progressPoller = nullptr;
    ModelSearcherThread* Searcher = nullptr;
    Ui::ModelBrowserDialog *ui;
};

#endif // MODELBROWSERDIALOG_H
