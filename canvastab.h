#ifndef CANVASTAB_H
#define CANVASTAB_H

#include <QMainWindow>
#include "canvas/drawingscene.h"
#include <QtColorWidgets/ColorSelector>
#include <QTimer>

#include "canvasinferer.h"
namespace Ui {
class CanvasTab;
}

class CanvasTab : public QMainWindow
{
    Q_OBJECT

public:
    StableDiffusionModel* CuMdl;


    void NewCanvas(QSize sz, const QColor& fillcol);

    explicit CanvasTab(QWidget *parent = nullptr);
    ~CanvasTab();

private:
    QTimer* progressPoller;

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

    // Please cast the result to RenderConfigForm* because I don't feel like including it in the header.
    // Thank you.
    QWidget* GetCurrentConfigWidget();

    void AddLayer(const QString &layname, const QColor &col = Qt::transparent);
    void RefreshLayersList();
    void SetupColorWidgets();
    void SetupCanvas();
    void SetResult(QImage& img);


public slots:
    void onUndo(bool checked);
    void on_btnRender_clicked();
    void onProgressPoll();

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

signals:
    void DemandModelLoad();
};

#endif // CANVASTAB_H
