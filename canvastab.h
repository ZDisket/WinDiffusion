#ifndef CANVASTAB_H
#define CANVASTAB_H

#include <QMainWindow>
#include "canvas/drawingscene.h"
#include <QtColorWidgets/ColorSelector>
#include <QTimer>
#include <QComboBox>
#include "canvasinferer.h"
#include "inferer.h"
namespace Ui {
class CanvasTab;
}

// This is technically a QMainWindow, for the dock widget areas.
class CanvasTab : public QMainWindow
{
    Q_OBJECT

public:
    StableDiffusionModel* CuMdl;


    void NewCanvas(QSize sz, const QColor& fillcol);

    QComboBox* getPresetsCb();

    explicit CanvasTab(QWidget *parent = nullptr);
    bool KillInferer();
    ~CanvasTab();

private:
    QTimer* progressPoller;

    QList<int> panelSizes;

    void Preinitialize();

    CanvasInferer* Inferer;
    DrawingScene* Scene;
    color_widgets::ColorSelector* selPrimColor;
    color_widgets::ColorSelector* selSecondColor;
    Ui::CanvasTab *ui;
    std::unique_ptr<Axodox::Threading::async_operation_source> CurrentAsyncSrc;

    QGraphicsPixmapItem* resultPixmapItem; // Renamed member variable

    bool Busy = false;

    // For live preview;
    bool RenderAgain = false;


    void DoRender(bool forcePreview = false);
    QString GetPresetsPath();

    // Please cast the result to RenderConfigForm* because I don't feel like including it in the header.
    // Thank you.
    QWidget* GetCurrentConfigWidget();

    void AddLayer(const QString &layname, const QColor &col = Qt::transparent);
    void RefreshLayersList();
    void SetupColorWidgets();
    void SetupCanvas();
    void SetResult(QImage& img);
    void RefreshPresets(const QString& setPreset = "");


public slots:
    void onUndo(bool checked);
    void on_btnRender_clicked();
    void onProgressPoll();
    void on_cbRenderPresets_currentTextChanged(const QString &arg1);

protected slots:


    void onColorPicked(ColorCat cat, QColor col);
    void onCanvasUpdated();
    void onLayerSetActive(bool act, LayerWidget* sendingWid);
    void onLayerSetVisible(bool act, LayerWidget* sendingWid);

    void onPrimaryColorSelected(const QColor& col);
    void onSecondaryColorSelected(const QColor& col);


    // When the brush size is changed via the scroll wheel in the scene, not by the slider.
    void onBrushSizeChanged(int sz);

    void onImageDone(QImage img);
    void onGetPreviews(std::vector<QImage> Imgs);

    void onResultSendToUpscale();
    void onResultSaveAs();

private slots:
    void on_btnBrush_clicked(bool checked);
    void on_btnEraser_clicked(bool checked);
    void on_btnFillBucket_clicked(bool checked);
    void on_btnNewLayer_clicked();
    void on_btnRemoveLayer_clicked();
    void on_btnLayUp_clicked();
    void on_btnLayerDown_clicked();
    void on_sliBrushSize_sliderMoved(int position);
    void on_horizontalSlider_sliderMoved(int position);
    void on_horizontalSlider_valueChanged(int value);
    void on_sliBrushSize_valueChanged(int value);

    void on_btnLivePreview_clicked(bool checked);

    void on_btnColorPicker_clicked(bool checked);

    void on_btnSwitchColors_clicked();

    void on_sliDenoiseStrength_valueChanged(int value);

    void on_btnRandomSeed_clicked();

    void on_spbSeed_valueChanged(int arg1);

    void on_chkViewRenderResults_clicked(bool checked);



    void on_btnSavePreset_clicked();

    void on_chkViewRenderResults_stateChanged(int arg1);

signals:
    void DemandModelLoad();
    void SendImageToUpscale(QImage* Img, bool TransOwnership);
    void Done(QImage Img, StableDiffusionJobType JobType);
};

#endif // CANVASTAB_H
