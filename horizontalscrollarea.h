#ifndef HORIZONTALSCROLLAREA_H
#define HORIZONTALSCROLLAREA_H

#include <QScrollArea>

class QWidget;

class HorizontalScrollArea : public QScrollArea {
    Q_OBJECT

public:
    explicit HorizontalScrollArea(QWidget *parent = nullptr);

    void RegisterContentsWidget(QWidget* ScraWidContents);

protected:
    bool eventFilter(QObject *o, QEvent *e) override;

private:
    QWidget *m_scrollAreaWidgetContents;
};

#endif // HORIZONTALSCROLLAREA_H
