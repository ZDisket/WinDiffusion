#ifndef PATHWIDGETITEM_H
#define PATHWIDGETITEM_H
#include <QListWidgetItem>
#include <QFileInfo>

class PathWidgetItem : public QListWidgetItem {
public:
    // Constructor takes the full file path and the parent QListWidget (optional)
    PathWidgetItem(const QString &fullFilePath, QListWidget *parent = nullptr);

    // Getter for the full path
    QString getFullPath() const;

private:
    QString fullPath; // Member variable to store the full path
};

#endif // PATHWIDGETITEM_H
