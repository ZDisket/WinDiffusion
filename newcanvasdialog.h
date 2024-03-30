#ifndef NEWCANVASDIALOG_H
#define NEWCANVASDIALOG_H

#include <QDialog>

struct NewCanvasSettings
{
    QSize size;
    QColor color;
};


namespace Ui {
class NewCanvasDialog;
}

class NewCanvasDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewCanvasDialog(QWidget *parent = nullptr);
    ~NewCanvasDialog();

    NewCanvasSettings settings;

private slots:
    void on_buttonBox_accepted();

    void on_NewCanvasDialog_accepted();

private:
    Ui::NewCanvasDialog *ui;
};

#endif // NEWCANVASDIALOG_H
