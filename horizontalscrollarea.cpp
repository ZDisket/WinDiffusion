#include "HorizontalScrollArea.h"
#include <QWidget>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QEvent>

HorizontalScrollArea::HorizontalScrollArea(QWidget *parent)
    : QScrollArea(parent)
{
    setWidgetResizable(true);
    setFrameStyle(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollAreaWidgetContents = nullptr;


}

void HorizontalScrollArea::RegisterContentsWidget(QWidget *ScraWidContents)
{
    m_scrollAreaWidgetContents = ScraWidContents;

    m_scrollAreaWidgetContents->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_scrollAreaWidgetContents->installEventFilter(this);
}

bool HorizontalScrollArea::eventFilter(QObject *o, QEvent *e) {
    if (o == m_scrollAreaWidgetContents && e->type() == QEvent::Resize) {
        // Adjust the minimum height of the scroll area based on the content's height
        int32_t ScrollBarHeight = horizontalScrollBar()->height();

        if (!horizontalScrollBar()->isVisible())
            ScrollBarHeight = 0;

        setMinimumHeight(m_scrollAreaWidgetContents->minimumSizeHint().height() + ScrollBarHeight);
    }
    return QScrollArea::eventFilter(o, e);
}
