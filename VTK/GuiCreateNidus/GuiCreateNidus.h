#ifndef GUICREATENIDUS_H
#define GUICREATENIDUS_H

#include <QCloseEvent>
#include <QDialog>
#include "SketchNode.h"
class CreateNidus;
class QTreeWidgetItem;


namespace Ui {
class GuiCreateNidus;
}

class GuiCreateNidus : public QDialog
{
    Q_OBJECT

public:
    explicit GuiCreateNidus(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    ~GuiCreateNidus();
    void setDirectoryName(QString path);
    QTreeWidgetItem* findItem(QString name);
    void updateSketchNodeIndex();
    QTreeWidgetItem* findTopItem();
private slots:
    void on_btnAddItem_clicked();

    QTreeWidgetItem* addItem(const SketchNode& node);
    QTreeWidgetItem* addItem(const int index,
                             QString name,
                             const int sliceNo,
                             const QString uid);
    void deleteSketchNode();

    void on_btnEditItem_clicked();

    void on_btnSaveItem_clicked();


    void on_verticalScrollBar_valueChanged(int value);

    void on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

    void customContextMenuRequested(const QPoint &pos);
protected:
     void closeEvent(QCloseEvent *event);

private:
    Ui::GuiCreateNidus *ui;
    CreateNidus* createNidus;
    bool EditState = false;
};

#endif // GUICREATENIDUS_H
