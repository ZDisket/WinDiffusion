#ifndef LAYERWIDGET_H
#define LAYERWIDGET_H

#include <QWidget>

// Can't directly include the layer header because the layer header already includes this (circular dependency)
// We going C mode but with this little help.
using LayerPtr = void*;

namespace Ui {
class LayerWidget;
}

class LayerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LayerWidget(QWidget *parent = nullptr);

    LayerPtr layer;

    void Update(const QString& layname, const QPixmap& pix);
    void SetIsActive(bool active, bool purecosmetic = false);
    ~LayerWidget();


signals:
    void onSetActive(bool act, LayerWidget* sending);
    void onVisibleChange(bool vis, LayerWidget* sending);
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
private slots:
    void on_btnVisible_clicked(bool checked);

private:
    Ui::LayerWidget *ui;
    bool isActive;
};

#endif // LAYERWIDGET_H
