#include "canvastab.h"
#include "ui_canvastab.h"
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include "QtAxodoxInteropCommon.hpp"
#include "canvasrenderpreset.h"

using namespace color_widgets;
using namespace QtAxInterop;

void CanvasTab::NewCanvas(QSize sz, const QColor &fillcol)
{

    Scene->Initialize(sz);

    AddLayer("Background", fillcol);
    AddLayer("Layer 1");
}

CanvasTab::CanvasTab(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::CanvasTab)
{
    ui->setupUi(this);


    SetupColorWidgets();

    SetupCanvas();

    Inferer = nullptr;

    ui->grpFillBucketOptions->hide();


    ui->spbSeed->setMaximum(INT_MAX);
    ui->spbSeed->setMinimum(INT_MIN);

    RefreshPresets();

}

void CanvasTab::SetupColorWidgets()
{
    ui->layColorSel->removeWidget(ui->btnSwitchColors);
    selPrimColor = new ColorSelector(this);
    selSecondColor = new ColorSelector(this);

    selPrimColor->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    selSecondColor->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    ui->layColorSel->addWidget(selPrimColor);
    ui->layColorSel->addWidget(ui->btnSwitchColors);
    ui->layColorSel->addWidget(selSecondColor);

    connect(selPrimColor, &ColorSelector::colorSelected, this, &CanvasTab::onPrimaryColorSelected);
    connect(selSecondColor, &ColorSelector::colorSelected, this, &CanvasTab::onSecondaryColorSelected);

}

void CanvasTab::SetupCanvas()
{
    // Assuming this code is in your main window's constructor or initialization function
    Scene = new DrawingScene(this);
    ui->viewDraw->setScene(Scene);

    NewCanvas(QSize(768,768),
              Qt::white);

    Scene->ToolColors = std::pair<QColor, QColor>(selPrimColor->color(), selSecondColor->color());


    connect(Scene, &DrawingScene::brushSizeChanged, this, &CanvasTab::onBrushSizeChanged);
    connect(Scene, &DrawingScene::Updated, this, &CanvasTab::onCanvasUpdated);
    connect(Scene, &DrawingScene::colorPicked, this, &CanvasTab::onColorPicked);

    ui->viewResult->needsControlScroll = false;
    resultPixmapItem = nullptr;


}

void CanvasTab::SetResult(QImage &img)
{
    // Ensure the QGraphicsView has a QGraphicsScene
    if (ui->viewResult->scene() == nullptr) {
        ui->viewResult->setScene(new QGraphicsScene(this));
    }

    // If the resultPixmapItem hasn't been created yet, create it
    if (!resultPixmapItem) {
        resultPixmapItem = new QGraphicsPixmapItem();
        ui->viewResult->scene()->addItem(resultPixmapItem);
    }

    QPixmap pixmap = QPixmap::fromImage(img);
    resultPixmapItem->setPixmap(pixmap);

    ui->viewResult->scene()->setSceneRect(pixmap.rect());
}



void CanvasTab::RefreshPresets()
{

    QString presetsDirPath = GetPresetsPath();

    // Create a QDir object pointing to the presets directory
    QDir presetsDir(presetsDirPath);

    if (!presetsDir.exists())
        return;

    ui->cbRenderPresets->blockSignals(true);
    ui->cbRenderPresets->clear();

    QStringList filter;
    filter << "*.bin";
    presetsDir.setNameFilters(filter);

    QFileInfoList files = presetsDir.entryInfoList();


    for (const QFileInfo& file : files) {
        QString fileNameWithoutExtension = file.baseName();
        ui->cbRenderPresets->addItem(fileNameWithoutExtension);
    }
    ui->cbRenderPresets->blockSignals(false);
}


void CanvasTab::onUndo(bool checked)
{
    Scene->undo();

}

void CanvasTab::onProgressPoll()
{
    if (!CurrentAsyncSrc)
        return;

    ui->pgbRenderProgress->setValue((int32_t)(CurrentAsyncSrc->state().progress * 100.f));
}

void CanvasTab::onColorPicked(ColorCat cat, QColor col)
{
    // pick the selector
    ColorSelector* selector = cat == ColorCat::Primary ? selPrimColor : selSecondColor;

    selector->setColor(col);
    // Artificially trigger its signal which will then propagate to our slot
    selector->colorSelected(col);


}

void CanvasTab::onCanvasUpdated()
{
    RenderAgain = true;

    if (!Busy && ui->btnLivePreview->isChecked())
        DoRender(true);


}

void CanvasTab::AddLayer(const QString &layname, const QColor &col)
{
    Scene->addLayer(layname, col);

    ui->scraLayout->insertWidget(0, Scene->getCurrentLayer()->laywid);

    connect(Scene->getCurrentLayer()->laywid,&LayerWidget::onSetActive,this,&CanvasTab::onLayerSetActive);
    connect(Scene->getCurrentLayer()->laywid,&LayerWidget::onVisibleChange,this,&CanvasTab::onLayerSetVisible);

    for (auto& lay : Scene->getLayers())
    {
        lay->laywid->SetIsActive(false);


    }

    Scene->getCurrentLayer()->laywid->SetIsActive(true);


}

void CanvasTab::RefreshLayersList()
{
    // First, remove all widgets from the layout

    for (auto& lay : Scene->getLayers())
    {

        ui->scraLayout->removeWidget(lay->laywid);


    }

    for (auto& lay : Scene->getLayers())
    {
        ui->scraLayout->insertWidget(0, lay->laywid);


    }


}



CanvasTab::~CanvasTab()
{
    delete ui;
}

void CanvasTab::Preinitialize()
{

    if (!CuMdl->IsLoaded())
        emit DemandModelLoad();

    if (!Inferer)
    {
        Inferer = new CanvasInferer;
        CurrentAsyncSrc = std::make_unique<Axodox::Threading::async_operation_source>();


        Inferer->AsyncSrc = CurrentAsyncSrc.get();
        Inferer->Model = CuMdl;
        connect(Inferer, &CanvasInferer::Done, this, &CanvasTab::onImageDone);
        connect(CuMdl, &StableDiffusionModel::PreviewAvailable, Inferer, &CanvasInferer::OnPreviewsAvailable);
        connect(Inferer, &CanvasInferer::PreviewsAvailable, this, &CanvasTab::onGetPreviews);


        Inferer->start();





    }

}

void CanvasTab::DoRender(bool forcePreview)
{
    Preinitialize();


    // Get the scene
    QImage CanvasFull = Scene->Render(true).toImage();
    CanvasOrder Ord;


    Ord.BatchCount = 1;

    Ord.InputImage = CanvasFull;
    Ord.NegativePrompt = ui->edtNegPrompt->toPlainText();
    Ord.Prompt = ui->edtPrompt->toPlainText();


    auto CurrentOptions = forcePreview ? ui->widPreviewConf->GetConfig() : ((RenderConfigForm*)GetCurrentConfigWidget())->GetConfig();

    Ord.Vae = (VaeMode)CurrentOptions.Vae;
    Ord.Options.StepCount = CurrentOptions.NumSteps;
    Ord.Options.Scheduler = InterOpHelper::ComboBoxIDToScheduler()[CurrentOptions.Sampler];

    Ord.Options.DenoisingStrength = InterOpHelper::SliderToZeroOneRange(ui->sliDenoiseStrength->value());
    Ord.Options.GuidanceScale = (float)ui->spbCFGScale->value();
    QStringList WidthHeight = ui->ledtResolution->text().split("x");

    Ord.Options.Height = WidthHeight[1].toInt(); Ord.Options.Width = WidthHeight[0].toInt();
    Ord.Options.BatchSize = 1;
    Ord.Options.Seed = ui->spbSeed->value();

    Inferer->Queue.push(Ord);

    Busy = true;
    RenderAgain = false;

    if (!ui->chkViewRenderResults->isChecked())
    {
        ui->chkViewRenderResults->setChecked(true);
        ui->viewResult->show();

    }

}

QString CanvasTab::GetPresetsPath()
{
    // Get the executable's directory
    QString exePath = QCoreApplication::applicationDirPath();
    // Define the presets folder path
    QString presetsDirPath = exePath + "/presets/canvas";

    return presetsDirPath;

}

QWidget *CanvasTab::GetCurrentConfigWidget()
{
    if (ui->rbConfFinal->isChecked())
        return ui->widFinalConf;

    return ui->widPreviewConf;

}

void CanvasTab::on_btnBrush_clicked(bool checked)
{
    if (!checked){return;}

    Scene->CurrentTool = DrawingTool::PenBrush;

    ui->grpBrushOpts->setVisible(checked);
    ui->grpFillBucketOptions->setVisible(!checked);

}


void CanvasTab::on_btnEraser_clicked(bool checked)
{
    if (!checked){return;}

    Scene->CurrentTool = DrawingTool::Remover;

    ui->grpBrushOpts->setVisible(checked);
    ui->grpFillBucketOptions->setVisible(!checked);


}


void CanvasTab::on_btnFillBucket_clicked(bool checked)
{
    if (!checked){return;}

    Scene->CurrentTool = DrawingTool::FillBucket;

    ui->grpBrushOpts->setVisible(!checked);
    ui->grpFillBucketOptions->setVisible(checked);

}



void CanvasTab::on_btnNewLayer_clicked()
{
    AddLayer("Layer " + QString::number(Scene->getLayers().size()));
}


void CanvasTab::on_btnRemoveLayer_clicked()
{
    Scene->deleteLayer(Scene->getCurrentLayer());
}

void CanvasTab::onLayerSetActive(bool act, LayerWidget *sendingWid)
{
    if (!act)
        return;

    Scene->selectLayer((Layer*)sendingWid->layer);



}

void CanvasTab::onLayerSetVisible(bool act, LayerWidget *sendingWid)
{

    ((Layer*)sendingWid->layer)->visible = act;

    Scene->Render();

}

void CanvasTab::onPrimaryColorSelected(const QColor &col)
{

    Scene->ToolColors.first = col;

}

void CanvasTab::onSecondaryColorSelected(const QColor &col)
{
    Scene->ToolColors.second = col;

}

void CanvasTab::onBrushSizeChanged(int sz)
{
    ui->sliBrushSize->setValue(sz);

}

void CanvasTab::onImageDone(QImage img)
{
    Busy = false;
    SetResult(img);

    if (RenderAgain && ui->btnLivePreview->isChecked())
        DoRender(true);

}

void CanvasTab::onGetPreviews(std::vector<QImage> Imgs)
{
    SetResult(Imgs[0]);

}


void CanvasTab::on_btnLayUp_clicked()
{
    Scene->moveLayerDown(Scene->getCurrentLayer());
    RefreshLayersList();


}


void CanvasTab::on_btnLayerDown_clicked()
{
    Scene->moveLayerUp(Scene->getCurrentLayer());
    RefreshLayersList();
}

void CanvasTab::on_sliBrushSize_sliderMoved(int position)
{
    Scene->setBrushSize(position, false);
}


void CanvasTab::on_horizontalSlider_sliderMoved(int position)
{

    Scene->Tolerance = ((float)position) / 100.f;
}


void CanvasTab::on_horizontalSlider_valueChanged(int value)
{
    ui->lblTolerance->setText(QString::number(value) + "%");
}


void CanvasTab::on_sliBrushSize_valueChanged(int value)
{
    ui->lblBrushSize->setText(QString::number(value));
}


void CanvasTab::on_btnRender_clicked()
{

    DoRender();

}


void CanvasTab::on_btnLivePreview_clicked(bool checked)
{
    if (checked && !Busy)
        DoRender(true);
}


void CanvasTab::on_btnColorPicker_clicked(bool checked)
{
    if (!checked){return;}

    Scene->CurrentTool = DrawingTool::ColorPicker;

    ui->grpBrushOpts->setVisible(false);
    ui->grpFillBucketOptions->setVisible(false);
}


void CanvasTab::on_btnSwitchColors_clicked()
{
    QColor primcolor = selPrimColor->color();

    // Reuse the color picked handler
    onColorPicked(ColorCat::Primary, selSecondColor->color());
    onColorPicked(ColorCat::Secondary, primcolor);

}


void CanvasTab::on_sliDenoiseStrength_valueChanged(int value)
{
    ui->lblDenStrengthShow->setText(QString::number(value) + "%");
    onCanvasUpdated();
}


void CanvasTab::on_btnRandomSeed_clicked()
{
    ui->spbSeed->setValue(
        InterOpHelper::getRandomNum<int>()
        );
}


void CanvasTab::on_spbSeed_valueChanged(int arg1)
{
    onCanvasUpdated();
}


void CanvasTab::on_chkViewRenderResults_clicked(bool checked)
{
    ui->viewResult->setVisible(checked);
}


void CanvasTab::on_cbRenderPresets_currentTextChanged(const QString &arg1)
{

    QString rawFilename = arg1 + ".bin";

    QString fullFilename = GetPresetsPath() + "/" + rawFilename;

    ZFile filePresetIn;

    if (!filePresetIn.Open(fullFilename.toStdString(), EZFOpenMode::BinaryRead))
        throw std::runtime_error("Could not open preset file.");


    CanvasRenderPreset Pres;

    filePresetIn >> Pres;

    filePresetIn.Close();

    ui->spbCFGScale->setValue(Pres.CFGScale);
    ui->ledtResolution->setText(Pres.Resolution);

    ui->widFinalConf->SetConfig(Pres.Final);
    ui->widPreviewConf->SetConfig(Pres.Preview);








}


void CanvasTab::on_btnSavePreset_clicked()
{
    bool ok;
    QString presetName = QInputDialog::getText(this, tr("Save Preset"),
                                               tr("Preset Name:"), QLineEdit::Normal,
                                               QString(), &ok);

    if (!ok || presetName.isEmpty())
        return;



    QString presetsDirPath = GetPresetsPath();

    // Verify that the folder exists, or create it
    QDir dir(presetsDirPath);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            QMessageBox::critical(this, tr("Error"), tr("Failed to create presets directory."));
            return;
        }
    }

    // Define the full filename with the path and .bin extension
    QString filename = dir.filePath(presetName + ".bin");

    CanvasRenderPreset Pres;

    Pres.CFGScale = ui->spbCFGScale->value();
    Pres.Final = ui->widFinalConf->GetConfig();
    Pres.Preview = ui->widPreviewConf->GetConfig();
    Pres.Resolution = ui->ledtResolution->text();

    ZFile presout(filename.toStdString(), EZFOpenMode::BinaryWrite);

    presout << Pres;

    presout.Close();

    RefreshPresets();


}
