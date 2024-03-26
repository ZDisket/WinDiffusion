#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <random>
#include <QSpacerItem>
#include <QCoreApplication>
#include <QDir>
#include <algorithm>

#include "ESRGAN.h"

#include "QtAxodoxInteropCommon.hpp"

#include "pathwidgetitem.h"
#include <QFileDialog>
#include "canvastab.h"

// Function to create "outputs" folder if it doesn't exist
QString createOutputsFolder() {
    QString applicationPath = QCoreApplication::applicationDirPath();
    QString OutputsDir = applicationPath + "/outputs";

    QDir dir(OutputsDir);
    if (!dir.exists()) {
        dir.mkpath("."); // Creates the directory including any necessary parent directories
    }

    return OutputsDir;
}



// Function to save an image in the specified directory with a unique name
QString saveImage(const QImage& image, const QString& directoryPath) {
    QDir dir(directoryPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // Find the lowest number for the new image file
    int fileNumber = 1;
    QString filePath;
    do {
        filePath = QString("%1/%2.png").arg(directoryPath).arg(fileNumber, 3, 10, QChar('0'));
        ++fileNumber;
    } while (QFile::exists(filePath));

    // Save the image
    image.save(filePath, "PNG");
    qDebug() << "Image saved to: " << filePath;
    return filePath;
}




void MainWindow::showEvent(QShowEvent *event)
{

    QMainWindow::showEvent(event);

    if (DidFirstShowStuff)
        return;


    ParentGoodWin->resize(1000,500);

    DidFirstShowStuff = true;





}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    UseFirst = true;
    PreviewsSpacer = nullptr;

    OutpsDir = createOutputsFolder();
    CurrentPgb = nullptr;

    CurrentInferThrd = nullptr;


    qRegisterMetaType< std::vector<Axodox::Graphics::TextureData> >("std::vector<Axodox::Graphics::TextureData>");
    //qRegisterMetaType< std::vector<Axodox::Graphics::TextureData>& >("std::vector<Axodox::Graphics::TextureData>&");

    progressPoller = new QTimer(this); // Create the timer with MainWindow as the parent
    progressPoller->setInterval(100); // Set the timer interval to 100 ms

    // Connect the timeout signal of the timer to the OnProgressPoll slot
    connect(progressPoller, &QTimer::timeout, this, &MainWindow::OnProgressPoll);

    progressPoller->start(); // Start the timer

    IsProcessing = false;
    CurrentItemNumber = 0;
    CurrentImageNumber = 0;
    CurrentImgDisplayIndex = 0;

    ui->scraImgPreviews->RegisterContentsWidget(ui->scrollAreaWidgetContents);
    CurrentAsyncSrc = std::make_unique<Axodox::Threading::async_operation_source>();


    UpdateModelListing();

    ResetViewports();

    ui->grpImg2Img->hide();

    ui->widInpaintCanvas->hide();
    ui->lblImgArrow->hide();

    ui->btnCancel->hide();
    ui->btnClearInpaint->hide();
    DidFirstShowStuff = false;

    connect(ui->widInpaintCanvas,&PaintableCanvas::OnImageSet, this, &MainWindow::OnInpaintWidImageSet);

    connect(ui->lblImg,&ClickableImageLabel::SendImageToInpaint,this,&MainWindow::OnImageSendToInpaint);
    connect(ui->lblImg,&ClickableImageLabel::SendImageToImg2Img,this,&MainWindow::OnImageSendToImg2Img);
    connect(ui->lblImg,&ClickableImageLabel::SendImageToUpscale,this,&MainWindow::OnImageSendToUpscale);


    connect(ui->lblLeftImg,&ClickableImageLabel::SendImageToInpaint,this,&MainWindow::OnImageSendToInpaint);
    connect(ui->lblLeftImg,&ClickableImageLabel::SendImageToImg2Img,this,&MainWindow::OnImageSendToImg2Img);
    connect(ui->lblLeftImg,&ClickableImageLabel::SendImageToUpscale,this,&MainWindow::OnImageSendToUpscale);


    UpdateUpscalerListing();

    CoInitializeEx( 0, COINIT_APARTMENTTHREADED );

    CurrentGlobalPgb = ui->pgbAllGens;


    canvasTab = new CanvasTab(this);
    canvasTab->setWindowFlags(Qt::Widget);
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(canvasTab);
    ui->tabwCanvas->setLayout(layout);

    ((CanvasTab*)canvasTab)->CuMdl = &CurrentMdl;



    connect((CanvasTab*)canvasTab, &CanvasTab::DemandModelLoad, this, &MainWindow::ModelLoadDemanded);

}

MainWindow::~MainWindow()
{
    on_actionClear_current_outputs_2_triggered();
    CurrentMdl.Destroy();

    try {
        delete ui;

    } catch (...) {
        // ...
    }




}

std::vector<QString> JobTypeToOutdir = {"txt2img", "img2img", "upscale"};

void MainWindow::OnImageDone(QImage InImg, StableDiffusionJobType JobType)
{
    const int PREVIEW_RES = 430;

    QString dateSubfolder = QDate::currentDate().toString("dd-MM-yyyy");
    QString ImmediateFolder = JobTypeToOutdir[(size_t)JobType];
    QString targetDirectory = QString("%1/%3/%2").arg(OutpsDir, dateSubfolder, ImmediateFolder);


    QString OutPath = saveImage(InImg, targetDirectory);

    if (JobType == StableDiffusionJobType::Upscale)
    {
        ui->lblUpscalePostImg->loadImage(OutPath);
        return;
    }




    ++CurrentImageNumber;


    if (PreviewsSpacer){
        ui->scraLayout->removeItem(PreviewsSpacer);
        delete PreviewsSpacer;
        PreviewsSpacer = nullptr;
    }


    TopBarImg* ImgPreviewTop = new TopBarImg(Q_NULLPTR);
    ImgPreviewTop->setPixmap(QPixmap::fromImage(InImg).scaled(64,64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ImgPreviewTop->setSizePolicy(QSizePolicy(QSizePolicy::Policy::Maximum,QSizePolicy::Policy::Maximum));


    TopBarImages.push_back(ImgPreviewTop);

    ImgPreviewTop->VecIndex = TopBarImages.size() - 1;
    ImgPreviewTop->OriginalImg = InImg.copy();
    ImgPreviewTop->FilePath = OutPath;

    connect(ImgPreviewTop, &TopBarImg::HoverChange, this, &MainWindow::OnTopBarHoverChange);
    connect(ImgPreviewTop, &TopBarImg::MouseClicked, this, &MainWindow::OnTopBarClick);

    ui->scraLayout->addWidget(ImgPreviewTop);




    OnTopBarClick(ImgPreviewTop->VecIndex);




    ui->lblAllGensProgress->setText(
        QString::number(TaskQueue.size()) + " orders in queue\n" +
        "Image " + QString::number(CurrentImageNumber) + "/"  + QString::number(CurrentGlobalPgb->maximum())
        );

    CurrentGlobalPgb->setValue(CurrentImageNumber);

    PreviewsSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    ui->scraLayout->addSpacerItem(PreviewsSpacer);



    //childWidget - QLabel you want to move to
    //area - QScrollArea
    ui->scraImgPreviews->updateGeometry();

    ImgPreviewTop->updateGeometry();
    // calculate childWidget position in coordinates of the viewport
   // const QPoint p = ImgPreviewTop->mapTo( ui->scraImgPreviews, QPoint(0,0));

    // move scroll bar
  //  ui->scraImgPreviews->horizontalScrollBar()->setValue(p.x() +  ui->scraImgPreviews->horizontalScrollBar()->value());

    QRect r = ImgPreviewTop->geometry(); ui->scraImgPreviews->horizontalScrollBar()->setValue(r.right());

//    ui->scraImgPreviews->ensureWidgetVisible(ImgPreviewTop);
   // ui->scraImgPreviews->scroll(64 * TopBarImages.size(),0);


}

void MainWindow::OnBulkImageDone(QImage InImg, std::string OutputPath, QListWidgetItem* Itm)
{

    ui->lblImgBulkUps->setPixmap(QPixmap::fromImage(InImg.scaled(512,512, Qt::KeepAspectRatio, Qt::SmoothTransformation))
                                 );



    CurrentGlobalPgb->setValue(CurrentItemNumber);

    Itm->setBackground(QBrush(Qt::green));

    ui->lblTotalUpscaleProg->setText("Upscale: " + QString::number(CurrentItemNumber) + "/" + QString::number(CurrentGlobalPgb->maximum()) + " files.");


    if (CurrentItemNumber == CurrentGlobalPgb->maximum())
    {
        SetControls(true);

        if (ui->chkAutoBrowseFolder->isChecked())
            OpenDirectory(ui->ledtBulkOutputFolder->text());

    }



}

void MainWindow::OnInpaintWidImageSet()
{
    ui->lblImg2ImgAssist->hide();
}

void MainWindow::OnImageSendToImg2Img(QImage *SndImg)
{
    ui->chkImg2Img->setChecked(true);
    ui->widInpaintCanvas->loadImage(*SndImg);
    ui->chkInpaint->setChecked(false);

}

void MainWindow::OnImageSendToInpaint(QImage *SndImg)
{
    ui->chkImg2Img->setChecked(true);
    ui->widInpaintCanvas->loadImage(*SndImg);
    ui->chkInpaint->setChecked(true);

}

void MainWindow::OnImageSendToUpscale(QImage *SndImg)
{
    ui->tabsMain->setCurrentIndex(1);
    ui->tabsUpsOptions->setCurrentIndex(0);

    ui->lblUpscalePreImage->SetImage(SndImg);


}

void MainWindow::OnPreviewsAvailable(std::vector<QImage> Previews)
{
    QImage& PreviewImage = Previews[0];

     ClickableImageLabel* Viewport = ui->lblLeftImg;

    if (!UseFirst)
        Viewport = ui->lblImg;

    Viewport->SetImagePreview(PreviewImage);

}

void MainWindow::ModelLoadDemanded()
{
    on_btnLoadModel_clicked();

}


void MainWindow::OnProgressPoll()
{
    if (!CurrentPgb)
        return;

    CurrentPgb->setValue((int32_t)(CurrentAsyncSrc->state().progress * 100.f));

}

void MainWindow::OnThreadDone()
{
    IsProcessing = false;
    IterateQueue();
}


void MainWindow::OnTopBarHoverChange(size_t LblIndex, bool Hovering)
{
    int32_t Neighbor = GetNeighbor(LblIndex);
    if (Neighbor == -1)
        return;

    TopBarImages[Neighbor]->SetHoveringBorder(Hovering);
}

void MainWindow::OnTopBarClick(size_t LblIndex)
{


    if (!TopBarImages.size())
        return;

    const int PREVIEW_RES = 430;

    QPixmap ViewportImg = QPixmap::fromImage(TopBarImages[LblIndex]->OriginalImg).scaled(PREVIEW_RES,PREVIEW_RES,Qt::KeepAspectRatio,Qt::SmoothTransformation);


    int32_t Neighbor = GetNeighbor(LblIndex);

    if (Neighbor == -1)
    {
        ClickableImageLabel* Viewport = ui->lblLeftImg;

        if (!UseFirst)
            Viewport = ui->lblImg;

        Viewport->setPixmap(ViewportImg);
        Viewport->OriginalImage = &TopBarImages[LblIndex]->OriginalImg;
        Viewport->pToOriginalFilePath = &TopBarImages[LblIndex]->FilePath;




        UseFirst = !UseFirst;

    }else
    {
        QPixmap SecondViewportImg = QPixmap::fromImage(TopBarImages[Neighbor]->OriginalImg).scaled(PREVIEW_RES,PREVIEW_RES,Qt::KeepAspectRatio,Qt::SmoothTransformation);

        ui->lblLeftImg->setPixmap(ViewportImg);
        ui->lblLeftImg->OriginalImage = &TopBarImages[LblIndex]->OriginalImg;
        ui->lblLeftImg->pToOriginalFilePath = &TopBarImages[LblIndex]->FilePath;

        ui->lblImg->setPixmap(SecondViewportImg);
        ui->lblImg->OriginalImage = &TopBarImages[Neighbor]->OriginalImg;
        ui->lblImg->pToOriginalFilePath = &TopBarImages[Neighbor]->FilePath;

        UseFirst = true;

    }
    UpdateSelectedTopBarImg(LblIndex);

    CurrentImgDisplayIndex = LblIndex;

}

std::vector<Axodox::MachineLearning::StableDiffusionSchedulerKind> ComboBoxIDToScheduler = {Axodox::MachineLearning::StableDiffusionSchedulerKind::DpmPlusPlus2M , Axodox::MachineLearning::StableDiffusionSchedulerKind::EulerAncestral};

void MainWindow::on_btnGenerate_clicked()
{
    if (!CurrentMdl.IsLoaded())
        on_btnLoadModel_clicked();

    CurrentGlobalPgb = ui->pgbAllGens;


    Axodox::MachineLearning::StableDiffusionOptions Options;

    Options.BatchSize = ui->spbBatchSize->value();
    Options.GuidanceScale = (float)ui->spbCFGScale->value();

    QString Resolution = ui->edtResolution->text();
    QStringList WidthHeight= Resolution.split("x");

    Options.Height = WidthHeight[1].toInt(); Options.Width = WidthHeight[0].toInt();
    Options.PredictionType = Axodox::MachineLearning::StableDiffusionSchedulerPredictionType::V;
    Options.StepCount = ui->spbSamplingSteps->value();

    if (ui->chkImg2Img->isChecked())
    {
        float BaseDenStrength = (float)ui->sliDenoiseStrength->value();
        Options.DenoisingStrength = BaseDenStrength / 100.f;


    }


    if (ui->edtSeed->text().isEmpty())
        Options.Seed = UINT32_MAX;
    else
        Options.Seed = ui->edtSeed->text().toUInt();

    Options.Scheduler = ComboBoxIDToScheduler[ui->cbSampler->currentIndex()];





    SDOrder Ord{ui->edtPrompt->toPlainText().toStdString(), ui->edtNegPrompt->toPlainText().toStdString(), Options, (uint32_t)ui->spbBatchCount->value(), ui->edtSeed->text().isEmpty()};
    if (ui->chkImg2Img->isChecked())
        Ord.InputImage = ui->widInpaintCanvas->getImage().copy();
    if (ui->chkInpaint->isChecked())
        Ord.InputMask = ui->widInpaintCanvas->getMask().copy();





    TaskQueue.push(Ord);

    ui->lblAllGensProgress->setText(
        QString::number(TaskQueue.size() + 1) + " orders in queue\n" +
        "Image " + QString::number(CurrentImageNumber) + "/"  + QString::number(CurrentGlobalPgb->maximum())
        );

    IterateQueue();



    //InferThrd->DoInference();
}


void MainWindow::on_btnLoadModel_clicked()
{
    // Get the current application directory path
    QString appDirPath = QCoreApplication::applicationDirPath();


    QString fullModelPath = appDirPath + "/models/" + ui->edtModelPath->currentText().trimmed();

    if (!LoadingFromModelsFolder)
        fullModelPath = ui->edtModelPath->currentText().trimmed();

    // Load the model using the full path
    CurrentMdl.Load(fullModelPath.toStdString(), QCoreApplication::applicationDirPath().toStdString() + "/auxiliary/");

}

int32_t MainWindow::GetNeighbor(size_t InIdx)
{
    size_t MaxIndex = TopBarImages.size() - 1;

    // Try going forward
    size_t AttemptIndex = InIdx + 1;

    if (AttemptIndex > MaxIndex)
        return -1;

    return (int32_t) AttemptIndex;



}

void MainWindow::IterateQueue()
{
    if (!TaskQueue.size() && !IsProcessing)
        ui->btnCancel->hide();
    else
        ui->btnCancel->show();

    if (IsProcessing)
        return;

    if (!TaskQueue.size()){
        CurrentItemNumber = 0;
        return;
    }


    SDOrder CuOrd = TaskQueue.front();

    Inferer* InferThrd = new Inferer;



    // we can't "un-cancel" an async source, so we make a new one.
    if (CurrentAsyncSrc->is_cancelled())
        CurrentAsyncSrc = std::make_unique<Axodox::Threading::async_operation_source>();

    InferThrd->Opts = CuOrd.Options;
    InferThrd->Model = &CurrentMdl;
    InferThrd->Prompt = CuOrd.Prompt;
    InferThrd->NegativePrompt = CuOrd.NegativePrompt;
    InferThrd->BatchCount = CuOrd.BatchCount;
    InferThrd->RandomSeed = CuOrd.RandomSeed;

    if (!CuOrd.InputImage.isNull())
        InferThrd->InputImage = CuOrd.InputImage.copy();
    if (!CuOrd.InputMask.isNull())
        InferThrd->InputMask = CuOrd.InputMask.copy();




    if (CuOrd.IsUpscale)
    {
        CurrentUpscaler.SetEnv(CurrentMdl.GetEnv());
        InferThrd->EsrGan = &CurrentUpscaler;

        CurrentPgb = ui->pgbUpscaleProg;

        InferThrd->OutputPath = CuOrd.OutputPath;
        InferThrd->itmInput = CuOrd.itmUpscaleInput;

        if (CuOrd.itmUpscaleInput){
            CuOrd.itmUpscaleInput->setBackground(QBrush(Qt::blue));
            CurrentGlobalPgb->setRange(0, ui->lstInputBulkFiles->count());
        }




    }
    else{
        InferThrd->EsrGan = nullptr;
        CurrentPgb = ui->pgbCurrentGen;
        CurrentGlobalPgb->setRange(0, CuOrd.BatchCount);
    }



    connect(InferThrd, &Inferer::Done, this, &MainWindow::OnImageDone);
    connect(InferThrd, &Inferer::DoneBulk, this, &MainWindow::OnBulkImageDone);
    connect(InferThrd, &Inferer::ThreadFinished, this, &MainWindow::OnThreadDone);


    /*
     * previews go from model -> inferer (convert to qimage) -> window
    */
    connect(&CurrentMdl, &StableDiffusionModel::PreviewAvailable, InferThrd, &Inferer::OnPreviewsAvailable);
    connect(InferThrd,&Inferer::PreviewsAvailable, this, &MainWindow::OnPreviewsAvailable);

    // Otherwise the thread lingers and causes a memory leak
    connect(InferThrd, &Inferer::finished, InferThrd, &QObject::deleteLater);

    CurrentInferThrd = InferThrd;


    InferThrd->AsyncSrc = CurrentAsyncSrc.get();
    InferThrd->start();


    ++CurrentItemNumber;

    ui->pgbCurrentGen->setRange(0, 100);
    ui->pgbUpscaleProg->setRange(0, 100);



    TaskQueue.pop();
    IsProcessing = true;


    ui->lblAllGensProgress->setText(
        QString::number(TaskQueue.size()) + " orders in queue\n" +
            "Image 0/" + QString::number(CuOrd.BatchCount)
        );

    CurrentImageNumber = 0;
}

void MainWindow::UpdateModelListing()
{
    // Get the current executable's directory
    QDir appDir(QCoreApplication::applicationDirPath());

    QDir modelsDir(appDir.absoluteFilePath("models"));

    // Check if the "models" folder exists
    if (!modelsDir.exists()) {
        LoadingFromModelsFolder = false;
        ui->edtModelPath->setEditable(!LoadingFromModelsFolder);
        return;
    }

    modelsDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

    QFileInfoList folders = modelsDir.entryInfoList();

    ui->edtModelPath->setCurrentText("");
    ui->edtModelPath->clear();


    foreach (const QFileInfo &folder, folders) {
        ui->edtModelPath->addItem(folder.fileName());
    }

    LoadingFromModelsFolder = true;
    ui->edtModelPath->setEditable(!LoadingFromModelsFolder);
}

void MainWindow::UpdateUpscalerListing() {
    QDir appDir(QCoreApplication::applicationDirPath());

    QDir upscalersDir(appDir.absoluteFilePath("upscalers"));

    if (!upscalersDir.exists()) {
        // Handle the case where the directory doesn't exist
        ui->cbUpscalerModels->setDisabled(true);
        return;
    }

    upscalersDir.setNameFilters(QStringList() << "*.onnx");
    upscalersDir.setFilter(QDir::Files | QDir::NoDotAndDotDot);

    QFileInfoList files = upscalersDir.entryInfoList();

    ui->cbUpscalerModels->setCurrentText("");
    ui->cbUpscalerModels->clear();

    // Iterate over each file and add its name to the comboBox
    foreach (const QFileInfo &file, files) {
        // Remove the ".onnx" extension from the file name
        QString modelName = file.baseName();
        ui->cbUpscalerModels->addItem(modelName);
    }

    ui->cbUpscalerModels->setDisabled(false);

    // If needed, set the first item as the current selection
    if (!files.isEmpty()) {
        ui->cbUpscalerModels->setCurrentIndex(0);
    }
}



void MainWindow::UpdateSelectedTopBarImg(size_t NewSelected)
{
    for (auto*& ImgWid : TopBarImages)
        ImgWid->SetSelectedBorder(false);

    TopBarImages[NewSelected]->SetSelectedBorder(true);

    int32_t Neighbor = GetNeighbor(NewSelected);

    if (Neighbor != -1)
        TopBarImages[Neighbor]->SetSelectedBorder(true);



}

void MainWindow::ResetViewports()
{
    const int PREVIEW_RES = 430;


    QPixmap WhiteFillPixm = QPixmap(PREVIEW_RES,PREVIEW_RES);
    WhiteFillPixm.fill(Qt::white);

    ui->lblLeftImg->setPixmap(WhiteFillPixm);
    ui->lblLeftImg->OriginalImage = nullptr;
    ui->lblLeftImg->pToOriginalFilePath = nullptr;

    ui->lblImg->setPixmap(WhiteFillPixm);
    ui->lblImg->OriginalImage = nullptr;
    ui->lblImg->pToOriginalFilePath = nullptr;
}

void MainWindow::OnImg2ImgEnabled()
{
    const int PREVIEW_RES = 430;

    QImage white(PREVIEW_RES,PREVIEW_RES, QImage::Format_RGBA8888);
    white.fill(Qt::white);
    ui->widInpaintCanvas->loadImage(white);
}

void MainWindow::OpenDirectory(const QString &dir)
{
    QString winPath = QDir::toNativeSeparators(dir);

    QStringList args;
    args << winPath;
    QProcess::startDetached("explorer", args);

}


void MainWindow::on_btnImagesForward_clicked()
{
    size_t MaxIndex = TopBarImages.size() - 1;

    // Try double-hop first

    size_t DoubleHopIndex = CurrentImgDisplayIndex + 2;
    size_t SingleHopIndex = CurrentImgDisplayIndex + 1;

    if (DoubleHopIndex < MaxIndex)
        OnTopBarClick(DoubleHopIndex);
    else if (SingleHopIndex < MaxIndex)
        OnTopBarClick(SingleHopIndex);
    else
        return;




}


void MainWindow::on_btnImagesBackwards_clicked()
{
    size_t MinIndex = 0; // Sneed

    // Try double-hop first

    int32_t DoubleHopIndex = CurrentImgDisplayIndex - 2;


    DoubleHopIndex = std::max<int32_t>(DoubleHopIndex,0);

    OnTopBarClick(DoubleHopIndex);


}


void MainWindow::on_actionScroll_Left_triggered()
{
    on_btnImagesBackwards_clicked();
}


void MainWindow::on_actionScroll_Right_triggered()
{
    on_btnImagesForward_clicked();
}


void MainWindow::on_actionOpen_outputs_directory_triggered()
{
    OpenDirectory(OutpsDir);
}


void MainWindow::on_actionClear_current_outputs_2_triggered()
{
    if (PreviewsSpacer){
        ui->scraLayout->removeItem(PreviewsSpacer);
        delete PreviewsSpacer;
        PreviewsSpacer = nullptr;
    }

    for (auto* ImgPreview : TopBarImages)
    {
        ui->scraLayout->removeWidget(ImgPreview);
        delete ImgPreview;

    }
    TopBarImages.clear();

    ResetViewports();



    CurrentImgDisplayIndex = 0;


}


void MainWindow::on_btnCancel_clicked()
{
    CurrentAsyncSrc->cancel();


}


void MainWindow::on_actionRefresh_model_listing_triggered()
{
    UpdateModelListing();
}


void MainWindow::on_chkImg2Img_stateChanged(int arg1)
{

    bool Enabled = (bool)arg1;

    ui->grpImg2Img->setVisible(Enabled);

    ui->widInpaintCanvas->setVisible(Enabled);
    ui->lblImgArrow->setVisible(Enabled);

    ui->btnClearInpaint->setVisible(Enabled);


    if (Enabled){
        ui->horizontalSpacer->changeSize(0,0, QSizePolicy::Ignored, QSizePolicy::Ignored);
        ui->horizontalSpacer_2->changeSize(0,0, QSizePolicy::Ignored, QSizePolicy::Ignored);
        OnImg2ImgEnabled();

    }else{
        ui->horizontalSpacer->changeSize(40,20, QSizePolicy::Expanding);
        ui->horizontalSpacer_2->changeSize(40,20, QSizePolicy::Maximum);

    }
    update();

}


void MainWindow::on_sliDenoiseStrength_valueChanged(int value)
{
    ui->lblDenoisePercShow->setText(QString::number(value) + "%");
}


void MainWindow::on_chkInpaint_stateChanged(int arg1)
{
    ui->widInpaintCanvas->setPaintingEnabled((bool)arg1);
}


void MainWindow::on_btnClearInpaint_clicked()
{

    ui->widInpaintCanvas->clearStrokes();
}


void MainWindow::on_btnUpscale_clicked()
{

    if (!CurrentUpscaler.IsLoaded())
        on_btnLoadUpscaler_clicked();


    if (ui->tabsUpsOptions->currentIndex() == 0)// Single
    {
        SDOrder Ord{ui->edtPrompt->toPlainText().toStdString(), ui->edtNegPrompt->toPlainText().toStdString(), Axodox::MachineLearning::StableDiffusionOptions{}, (uint32_t)ui->spbBatchCount->value(), ui->edtSeed->text().isEmpty()};

        Ord.InputImage = *ui->lblUpscalePreImage->OriginalImage;
        Ord.IsUpscale = true;


        TaskQueue.push(Ord);

    }else
    { // Bulk
        if (ui->ledtBulkOutputFolder->text().isEmpty()) {
            // Get the application directory path
            QString appDirPath = QCoreApplication::applicationDirPath();

            // Construct the path for the new folder using the current date
            QString dateStr = QDate::currentDate().toString("dd-MM-yyyy");
            QString newFolderPath = appDirPath + "/outputs/bulk-upscale/" + dateStr;

            // Create the directory (and any necessary parent directories)
            QDir dir;
            if (dir.mkpath(newFolderPath)) {
                // If the directory was successfully created, set the QLineEdit's text
                ui->ledtBulkOutputFolder->setText(newFolderPath);
            } else {
                // Handle error if the directory could not be created
                // This could be due to permissions or other issues.
                qDebug() << "Could not create directory at path:" << newFolderPath;
            }
        }

        CurrentGlobalPgb = ui->pgbBulkUpscales;

        std::vector<QListWidgetItem*> itemsToDelete;

        for (int i = 0; i < ui->lstInputBulkFiles->count(); ++i)
        {
            QListWidgetItem* cuItem = ui->lstInputBulkFiles->item(i);

            auto pathItem = (PathWidgetItem*)cuItem;

            SDOrder Ord{ui->edtPrompt->toPlainText().toStdString(), ui->edtNegPrompt->toPlainText().toStdString(), Axodox::MachineLearning::StableDiffusionOptions{}, (uint32_t)ui->spbBatchCount->value(), ui->edtSeed->text().isEmpty()};

            Ord.IsUpscale = true;
            Ord.itmUpscaleInput = cuItem;

            Ord.OutputPath = (
                              std::filesystem::path(ui->ledtBulkOutputFolder->text().toStdString()) / pathItem->text().toStdString()
                              ).string();


            if (ui->chkSkipAlreadyUpscaled->isChecked() && std::filesystem::exists(Ord.OutputPath))
            {
                // We flag these items for deletion later; if we do so right now, we have to deal with the rest shifting position within the loop
                // We can do that, but it's cleaner and less headache-y code to just delete them later.
                itemsToDelete.push_back(cuItem);
                continue;


            }

            TaskQueue.push(Ord);

        }


        for (QListWidgetItem* itm : itemsToDelete)
             delete itm;


        ui->lstInputBulkFiles->update();

        if (ui->chkAutoUnloadModel->isChecked() && ui->lstInputBulkFiles->count() > 5)
            CurrentMdl.Destroy();

        if (TaskQueue.size())
            SetControls(false);

    }



    IterateQueue();

}


void MainWindow::on_btnLoadUpscaler_clicked()
{
    if (!CurrentMdl.GetEnv())
        CurrentMdl.LoadMinimal();

    CurrentUpscaler.SetEnv(CurrentMdl.GetEnv());

    CurrentUpscaler.Load(
        QString(QCoreApplication::applicationDirPath() + "/upscalers/" + ui->cbUpscalerModels->currentText() + ".onnx").toStdString()
        );

}



void MainWindow::on_btnUpsBrowseFolder_clicked() {

    QString folderPath = QFileDialog::getExistingDirectory(this, tr("Select Folder"), QDir::currentPath(),
                                                           QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks |  QFileDialog::DontUseNativeDialog);

    if (folderPath.isEmpty())
        return;


    ui->ledtBulkAddFolderPath->setText(folderPath);

}


void MainWindow::on_btnAddBulkFolder_clicked() {
    // Clear the list widget to avoid duplicating items if the button is clicked multiple times
    //ui->lstInputBulkFiles->clear();

    QString folderPath = ui->ledtBulkAddFolderPath->text();

    // Check if the folder path is not empty
    if (!folderPath.isEmpty()) {
        QDir directory(folderPath);

        QStringList imageFilters;
        imageFilters << "*.png" << "*.jpg" << "*.jpeg";

        QFileInfoList fileList = directory.entryInfoList(imageFilters, QDir::Files | QDir::NoDotAndDotDot);

        foreach (const QFileInfo &file, fileList) {
            new PathWidgetItem(file.absoluteFilePath(), ui->lstInputBulkFiles);
        }
    }
}


void MainWindow::on_btnBrowseBulkUpsOutFolder_clicked()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, tr("Select Folder"), QDir::currentPath(),             // COM issues mean we can't use native dialog
                                                           QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | QFileDialog::DontUseNativeDialog);

    if (folderPath.isEmpty())
        return;


    ui->ledtBulkOutputFolder->setText(folderPath);

}


void MainWindow::on_btnAddFromSpecialFolder_clicked()
{
    QString FavesDir = QCoreApplication::applicationDirPath() + "/favorites/";

    QDir dir(FavesDir);

    if (!dir.exists())
        return;

    ui->ledtBulkAddFolderPath->setText(FavesDir);
    on_btnAddBulkFolder_clicked();

}


void MainWindow::on_actionOpen_favorites_directory_triggered()
{
    OpenDirectory(QCoreApplication::applicationDirPath() + "/favorites/");

}


void MainWindow::on_actionWhat_s_this_triggered(bool checked)
{
    if (checked)
        QWhatsThis::enterWhatsThisMode();
    else
        QWhatsThis::leaveWhatsThisMode();


}


void MainWindow::on_actionWhat_s_this_triggered()
{

}

void MainWindow::SetControls(bool Enabled)
{
    ui->btnUpscale->setEnabled(Enabled);
    ui->btnGenerate->setEnabled(Enabled);
    ui->btnAddBulkFolder->setEnabled(Enabled);

}


void MainWindow::on_btnClearUpscaleAdds_clicked()
{
    ui->lstInputBulkFiles->clear();
}

