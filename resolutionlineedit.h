#pragma once

#include <QLineEdit>

#include <QFocusEvent>

class ResolutionLineEdit : public QLineEdit {
    Q_OBJECT

public:
    explicit ResolutionLineEdit(QWidget *parent = nullptr);

protected:
    void focusOutEvent(QFocusEvent *event) override;
};

