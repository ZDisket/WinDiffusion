#include "resolutionlineedit.h"
#include <QRegularExpression>
#include <QRegularExpressionValidator>

ResolutionLineEdit::ResolutionLineEdit(QWidget *parent) : QLineEdit(parent) {
    // Regular expression to match the format WxH, where W and H are integers
    QRegularExpression regExp("^\\d+x\\d+$");
    QRegularExpressionValidator *validator = new QRegularExpressionValidator(regExp, this);
    setValidator(validator);
}

void ResolutionLineEdit::focusOutEvent(QFocusEvent *event) {
    QString text = this->text();
    if (text.contains('x')) {
        QStringList parts = text.split('x');
        if (parts.size() == 2) {
            int width = parts.at(0).toInt();
            int height = parts.at(1).toInt();

            if (width < 64 || height < 64) {
                // If either value is less than 64, clear the input
                setText("");
            }
        }
    }

    QLineEdit::focusOutEvent(event);
}
