#include "topbarimg.h"

TopBarImg::TopBarImg(QWidget *parent, Qt::WindowFlags f)
    : QLabel(parent, f) {
    setAttribute(Qt::WA_Hover); // Enable hover events
    IsCurrentlySelected = false;
}

TopBarImg::~TopBarImg()
{

}
TopBarImg::TopBarImg() {}

void TopBarImg::enterEvent(QEnterEvent *event)
{
    QLabel::enterEvent(event); // Ensure proper event processing

    if (IsCurrentlySelected)
        return;

    SetHoveringBorder(true);

    emit HoverEnter(VecIndex);
}

void TopBarImg::leaveEvent(QEvent *event)
{
    QLabel::leaveEvent(event); // Ensure proper event processing

    if (IsCurrentlySelected)
        return;

    SetHoveringBorder(false);

    emit HoverExit(VecIndex);

}

void TopBarImg::mousePressEvent(QMouseEvent *event)
{
    QLabel::mousePressEvent(event); // Ensure proper event processing
    emit MouseClicked(VecIndex);
}

void TopBarImg::SetHoveringBorder(bool IsHovering)
{

    if (IsHovering)
        setStyleSheet("border: 2px solid yellow;");
    else
        setStyleSheet("");
}

void TopBarImg::SetSelectedBorder(bool IsSelected)
{
    if (IsSelected)
        setStyleSheet("border: 3px solid blue;");
    else
        setStyleSheet("");

    IsCurrentlySelected = IsSelected;

}
