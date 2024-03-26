#ifndef CANVASTAB_H
#define CANVASTAB_H

#include <QMainWindow>
#include "canvas/drawingscene.h"
#include <QtColorWidgets/ColorSelector>


#include "canvasinferer.h"
namespace Ui {
class CanvasTab;
}

class CanvasTab : public QMainWindow
{
    Q_OBJECT

public:
    StableDiffusionModel* CuMdl;



    explicit CanvasTab(QWidget *parent = nullptr);
    ~CanvasTab();

private:

    void Preinitialize();

    CanvasInferer* Inferer;
    DrawingScene* Scene;
    color_widgets::ColorSelector* selPrimColor;
    color_widgets::ColorSelector* selSecondColor;
    Ui::CanvasTab *ui;
    std::unique_ptr<Axodox::Threading::async_operation_source> CurrentAsyncSrc;

    QGraphicsPixmapItem* resultPixmapItem; // Renamed member variable



    // Please cast the result to RenderConfigForm* because I don't feel like including it in the header.
    // Thank you.
    QWidget* GetCurrentConfigWidget();

    void AddLayer(const QString &layname, const QColor &col = Qt::transparent);
    void RefreshLayersList();
    void SetupColorWidgets();
    void SetupCanvas();


public slots:
    void onUndo(bool checked);

protected slots:


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
    void on_btnRender_clicked();
signals:
    void DemandModelLoad();
};

#endif // CANVASTAB_H
