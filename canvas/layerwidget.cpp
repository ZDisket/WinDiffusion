#include "layerwidget.h"
#include "ui_layerwidget.h"
#include "drawingscene.h"
#include "layersettingsdialog.h"
#include "QtAxodoxInteropCommon.hpp"

using namespace QtAxInterop;

#define GETLAYER ((Layer*)layer)


LayerWidget::LayerWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LayerWidget)
{
    ui->setupUi(this);

    this->setMouseTracking(true); // Enable mouse tracking for the widget
    this->installEventFilter(this); // Install event filter on itself

    isActive = false;

}

bool LayerWidget::eventFilter(QObject *watched, QEvent *event) {
    if (watched == this) { // Check if the event is for the LayerWidget itself
        switch (event->type()) {
        case QEvent::Enter:
            // Change the border color to yellow on hover
            ui->frame->setStyleSheet("QFrame { border: 1px solid yellow; }");
            break;
        case QEvent::Leave:
            // Revert the border color based on the active state
            ui->frame->setStyleSheet(isActive ? "QFrame { border: 2px solid blue; }" : "QFrame { border: 1px solid black; }");
            break;
        case QEvent::MouseButtonPress:
            if (!isActive)
                 SetIsActive(true);
            break;
        case QEvent::MouseButtonDblClick:
            onDoubleClick();
            break;
        default:
            break;
        }
    }
    ui->lblImageLayer->setStyleSheet("QFrame { border: none; }");

    ui->lblLayerName->setStyleSheet("QFrame { border: none; }");
    return QWidget::eventFilter(watched, event); // Make sure to call the base class eventFilter
}




void LayerWidget::Update(const QString &layname, const QPixmap &pix)
{
    ui->lblLayerName->setText(layname);
    ui->lblImageLayer->setPixmap(pix.scaled(32,32, Qt::IgnoreAspectRatio).copy());

}

void LayerWidget::SetIsActive(bool active, bool purecosmetic)
{
    isActive = active;
    ui->frame->setStyleSheet(isActive ? "QFrame { border: 2px solid blue; }" : "QFrame { border: 1px solid black; }");

    if (purecosmetic)
        return;

    emit onSetActive(active,this);

}

LayerWidget::~LayerWidget()
{
    delete ui;
}

void LayerWidget::on_btnVisible_clicked(bool checked)
{
    emit onVisibleChange(checked, this);
}


void LayerWidget::on_btnRenderable_clicked(bool checked)
{
    GETLAYER->renderable = checked;
    emit onRenderableChange(checked, this);

}


void LayerWidget::onDoubleClick()
{
    LayerSettingsDialog dialog(this);

    dialog.settings.name = ui->lblLayerName->text();
    dialog.settings.render = ui->btnRenderable->isChecked();
    dialog.settings.opacity = InterOpHelper::ZeroOneToSliderRange(GETLAYER->opacity);
    dialog.settings.visible = ui->btnVisible->isChecked();

    dialog.Update();




    int result = dialog.exec();

    if (result != QDialog::Accepted)
        return;

    GETLAYER->name = dialog.settings.name;
    GETLAYER->opacity = InterOpHelper::SliderToZeroOneRange(dialog.settings.opacity);



    ui->lblLayerName->setText(dialog.settings.name);
    ui->btnRenderable->setChecked(dialog.settings.render);
    ui->btnVisible->setChecked(dialog.settings.visible);

    // This is important because it triggers a rerender, in case the opacity/visibility was changed.
    on_btnVisible_clicked(dialog.settings.visible);
    on_btnRenderable_clicked(dialog.settings.render);

    // Reset active
    SetIsActive(true);


}


