#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "inferer.h"
#include <QHBoxLayout>
#include <QTimer>
#include <queue>
#include "topbarimg.h"
#include <QGoodWindow>
#include "imagesaverthread.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE


using CanvasTabPtr = QWidget*;

struct SDOrder{
    std::string Prompt;
    std::string NegativePrompt;
    Axodox::MachineLearning::StableDiffusionOptions Options;
    uint32_t BatchCount;
    bool RandomSeed;
    QImage InputImage;
    QImage InputMask;
    bool IsUpscale = false;
    std::string OutputPath = "";
    QListWidgetItem* itmUpscaleInput = nullptr;

};


class MainWindow : public QMainWindow
{
    Q_OBJECT

protected:
    void showEvent(QShowEvent* event) override;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QGoodWindow* ParentGoodWin;

    bool IsExiting(){return Exiting;};

public slots:
    void OnImageDone(QImage InImg, StableDiffusionJobType JobType);
    void OnBulkImageDone(QImage InImg, std::string OutputPath, QListWidgetItem *Itm);
    void OnInpaintWidImageSet();
    void OnImageSendToImg2Img(QImage* SndImg);
    void OnImageSendToInpaint(QImage* SndImg);
    void OnImageSendToUpscale(QImage* SndImg, bool TransferOwnership = false);
    void OnPreviewsAvailable(std::vector<QImage> Previews);
    void ModelLoadDemanded();


private slots:
    void OnProgressPoll(); // Slot to be called by QTimer
    void OnThreadDone();

    void OnTopBarHoverChange(size_t LblIndex, bool Hovering);

    void OnTopBarClick(size_t LblIndex);

    void OnRequestModelDownload(QString MdlName);

private slots:
    void on_btnGenerate_clicked();

    void on_btnLoadModel_clicked();


    void on_btnImagesForward_clicked();

    void on_btnImagesBackwards_clicked();

    void on_actionScroll_Left_triggered();

    void on_actionScroll_Right_triggered();

    void on_actionOpen_outputs_directory_triggered();

    void on_actionClear_current_outputs_2_triggered();

    void on_btnCancel_clicked();

    void on_actionRefresh_model_listing_triggered();

    void on_chkImg2Img_stateChanged(int arg1);

    void on_sliDenoiseStrength_valueChanged(int value);

    void on_chkInpaint_stateChanged(int arg1);

    void on_btnClearInpaint_clicked();

    void on_btnUpscale_clicked();

    void on_btnLoadUpscaler_clicked();

    void on_btnUpsBrowseFolder_clicked();

    void on_btnAddBulkFolder_clicked();

    void on_btnBrowseBulkUpsOutFolder_clicked();

    void on_btnAddFromSpecialFolder_clicked();

    void on_actionOpen_favorites_directory_triggered();

    void on_actionWhat_s_this_triggered(bool checked);

    void on_actionWhat_s_this_triggered();

    void on_btnClearUpscaleAdds_clicked();

    void on_actUndo_triggered();

    void on_actRender_triggered();

    void on_actNewCanvas_triggered();

    void on_actionDownload_models_triggered();

    void on_actionDownload_a_model_from_Huggingface_triggered();

    void on_actCheckGPU_triggered();

private:

    void OpenDownloadModelDlg(const QString& DownMdlName = "");
    void SetControls(bool Enabled);
    bool DidFirstShowStuff;
    int32_t GetNeighbor(size_t InIdx);
    void IterateQueue();
    std::queue<SDOrder> TaskQueue; // STL queue for tasks
    QTimer* ProcessTimer; // Timer to trigger processing of the next task
    bool IsProcessing = false; // Flag to indicate if a task is currently being processed
    uint32_t CurrentItemNumber = 0;
    uint32_t CurrentImageNumber = 0;

    bool LoadingFromModelsFolder;
    size_t CurrentImgDisplayIndex = 0;
    std::vector<TopBarImg*> TopBarImages;


    QProgressBar* CurrentPgb = nullptr;
    QProgressBar* CurrentGlobalPgb = nullptr;

    void UpdateModelListing();
    void UpdateSelectedTopBarImg(size_t NewSelected);
    void ResetViewports();
    void OnImg2ImgEnabled();
    void OpenDirectory(const QString& dir);

private:
    bool Exiting = false;
    QTimer* progressPoller; // QTimer object
    std::unique_ptr<Axodox::Threading::async_operation_source> CurrentAsyncSrc;
    QString OutpsDir;
    QSpacerItem* PreviewsSpacer = nullptr;
    bool UseFirst = true;
    StableDiffusionModel CurrentMdl;
    Upscaler CurrentUpscaler;
    Ui::MainWindow *ui;
    Inferer* CurrentInferThrd = nullptr;
    CanvasTabPtr canvasTab;
    ImageSaverThread* imageSaver = nullptr;

    void UpdateUpscalerListing();

    QString saveImage(const QImage &image, const QString &directoryPath);
    void SetupUI();
    void SetupCanvas();
};
#endif // MAINWINDOW_H
