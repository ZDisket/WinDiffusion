#ifndef LAYERSETTINGSDIALOG_H
#define LAYERSETTINGSDIALOG_H

#include <QDialog>

struct LayerSettings{
    QString name;
    int opacity; // 0 - 100 int range
    bool visible;
    bool render;

};

namespace Ui {
class LayerSettingsDialog;
}

class LayerSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LayerSettingsDialog(QWidget *parent = nullptr);
    ~LayerSettingsDialog();

    LayerSettings settings;

    void Update();

private slots:
    void on_buttonBox_accepted();

    void on_sliLayerOpacity_valueChanged(int value);

private:
    Ui::LayerSettingsDialog *ui;
};

#endif // LAYERSETTINGSDIALOG_H
