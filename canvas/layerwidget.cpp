#include "layerwidget.h"
#include "ui_layerwidget.h"
#include "drawingscene.h"

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
            SetIsActive(!isActive);
            // Change the border color to deep blue when active
            ui->frame->setStyleSheet("QFrame { border: 2px solid blue; }");
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

