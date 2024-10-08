#include "GuiCreateNidus.h"
#include "ui_GuiCreateNidus.h"

#include <QLineEdit>
#include <QMessageBox>
#include <QTreeWidgetItem>

#include "createnidus.h"
GuiCreateNidus::GuiCreateNidus(QWidget *parent,Qt::WindowFlags flags)
    : QDialog(parent), ui(new Ui::GuiCreateNidus)
{
    ui->setupUi(this);
    flags |= Qt::WindowCloseButtonHint;
    flags |= Qt::WindowMinimizeButtonHint;
    flags |= Qt::WindowMaximizeButtonHint;
    setWindowFlags(flags);
    //    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeWidget->header()->setSectionResizeMode(QHeaderView::Interactive);
    ui->treeWidget->header()->setStretchLastSection(true);
    ui->treeWidget->setColumnWidth(0,70);
    ui->treeWidget->setColumnWidth(1,100);
    ui->treeWidget->setColumnWidth(2,50);
    ui->treeWidget->setColumnWidth(3,32);
    QGridLayout* layout = new QGridLayout(ui->vtkOpenGLWidget);
    createNidus = new CreateNidus(ui->vtkOpenGLWidget);
    layout->addWidget(createNidus);


    connect(createNidus, SIGNAL(addItem(const SketchNode&)), this, SLOT(addItem(const SketchNode&)));
    connect(createNidus, SIGNAL(setSliceNo(int)), ui->verticalScrollBar, SLOT(setValue(int)));
    connect(createNidus, SIGNAL(setRange(int, int)), ui->verticalScrollBar, SLOT(setRange(int, int)));
}

GuiCreateNidus::~GuiCreateNidus()
{
    delete ui;
}

void GuiCreateNidus::setDirectoryName(QString path)
{
    createNidus->SetDirectoryName(qUtf8Printable(path));
}

void GuiCreateNidus::on_btnAddItem_clicked()
{
    auto name = createNidus->CreateNodeName();
    if(name.isEmpty())
    {
        return;
    }
    auto res = createNidus->AddSketchNode(name);
    auto index = createNidus->getSketchNodeSize();
    auto uid = createNidus->CreateUUID();
    addItem(index, name, 0, uid);
    createNidus->insertSketchNode(uid, name);
}

QTreeWidgetItem* GuiCreateNidus::findItem(QString name)
{
    QList<QTreeWidgetItem*> items = ui->treeWidget->findItems(name, Qt::MatchExactly, 1);
    if (0 < items.size())
    {
        return items[0];
    }
    else
    {
        return nullptr;
    }
}

QTreeWidgetItem* GuiCreateNidus::addItem(const int index,QString name, const int sliceNo,const QString uid)
{
    if(true == name.isEmpty())
    {
        return nullptr;
    }
    QTreeWidgetItem* rootItem = findItem(name);
    if (nullptr == rootItem)
    {
        //添加新结节
        QStringList labels;
        labels << QString::number(index) << name << "";
        rootItem = new QTreeWidgetItem(ui->treeWidget->invisibleRootItem(), labels);
        ui->treeWidget->addTopLevelItem(rootItem);
        QPushButton* btnDelete= new QPushButton("",ui->treeWidget);
        btnDelete->setFixedSize(32, 32);
        btnDelete->setFlat(true);
        btnDelete->setIcon(QIcon(":/delete.png"));
//        btnDelete->setStyleSheet("background: transparent;");
        connect(btnDelete, &QPushButton::clicked, this, &GuiCreateNidus::deleteSketchNode);
        ui->treeWidget->setItemWidget(rootItem, 3, btnDelete);
        btnDelete->setProperty("row", ui->treeWidget->currentIndex().row());
        btnDelete->setProperty("uid", uid);
        btnDelete->setProperty("sliceNo", sliceNo);
        btnDelete->setProperty("name", name);
        btnDelete->setProperty("index", index);
        btnDelete->setProperty("isTop", true);
        ui->treeWidget->setCurrentItem(rootItem);
        return rootItem;
    }
    //添加新子结节
    QStringList labels;
    labels << QString::number(index) << "" << QString::number(sliceNo);
    QTreeWidgetItem* item = new QTreeWidgetItem(rootItem, labels);
    ui->treeWidget->addTopLevelItem(item);
    QPushButton* btnDelete= new QPushButton("",ui->treeWidget);
    btnDelete->setFixedSize(32, 32);
    btnDelete->setFlat(true);
    btnDelete->setIcon(QIcon(":/delete.png"));
//    btnDelete->setStyleSheet("background: transparent;");
    connect(btnDelete, &QPushButton::clicked, this, &GuiCreateNidus::deleteSketchNode);

    ui->treeWidget->setItemWidget(item, 3, btnDelete);
    btnDelete->setProperty("row", ui->treeWidget->currentIndex().row());
    btnDelete->setProperty("uid", uid);
    btnDelete->setProperty("name", name);
    btnDelete->setProperty("sliceNo", sliceNo -1);
    btnDelete->setProperty("index", index);
    btnDelete->setProperty("isTop", false);
    ui->treeWidget->setCurrentItem(item);
    return rootItem;
}

QTreeWidgetItem* GuiCreateNidus::addItem(const SketchNode& node)
{
    return addItem(node.index,node.name,node.sliceNo,node.uid);
}

void GuiCreateNidus::deleteSketchNode()
{
    if(false == EditState) return;

    auto button  = QMessageBox::question(this,
                                         tr("删除确认"),
                                         QString(tr("是否删除?")),
                                         tr("确定"),tr("取消"));

    if(button==1)
    {
        return;
    }

    auto btnDelete = qobject_cast<QPushButton*>(sender());
    if (nullptr == btnDelete) return;

    auto isTopNode = btnDelete->property("isTop").toBool();

    auto index = ui->treeWidget->indexAt(QPoint(btnDelete->frameGeometry().x(),
                                                btnDelete->frameGeometry().y()));
    auto item = ui->treeWidget->itemAt(QPoint(btnDelete->frameGeometry().x(),
                                              btnDelete->frameGeometry().y()));
    if(nullptr == item) return;
    auto sindex = item->text(2);
    auto row = index.row();
    if (nullptr == item)
    {
        return;
    }
    auto name = btnDelete->property("name").toString();
    auto uid = btnDelete->property("uid").toString();
    createNidus->deleteSketchChildNode(name, uid, isTopNode);//删除XML

    if (nullptr == item->parent())
    {
        //没有父节点就直接删除
        delete ui->treeWidget->takeTopLevelItem(row);
    }
    else
    {
        //如果有父节点就要用父节点的takeChild删除节点
        delete item->parent()->takeChild(row);
    }
    updateSketchNodeIndex();
}

void GuiCreateNidus::on_btnEditItem_clicked()
{
    this->EditState = !EditState;
    createNidus->SetEnabledTracer();
    QString txt =this->EditState ? "关闭编辑" : "开启编辑";
    ui->btnSaveItem->setEnabled(EditState);
    ui->btnAddItem->setEnabled(EditState);
    ui->btnEditItem->setText(txt);
}

void GuiCreateNidus::on_btnSaveItem_clicked()
{
    createNidus->saveSketchNode(true);
    QMessageBox::information(this,"提示","保存完成!");
}

QTreeWidgetItem* GuiCreateNidus::findTopItem()
{
    QTreeWidgetItem* currentItem = ui->treeWidget->currentItem();//获得当前节点
    if (nullptr == currentItem)
    {
        return currentItem;
    }

    QTreeWidgetItem* parent = currentItem->parent();//获得当前节点的父节点

    while (nullptr != parent)
    {
        currentItem = parent;
        parent = currentItem->parent();//获得当前节点的父节点
    }

    return currentItem;
}

void GuiCreateNidus::updateSketchNodeIndex()
{
    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); ++i)
    {
        ui->treeWidget->topLevelItem(i)->setText(0, QString::number(i + 1));
    }
    QTreeWidgetItem* currentItem = findTopItem();
    if (nullptr == currentItem)return;
    for (int i = 0; i < currentItem->childCount(); ++i)
    {
        auto index = i + 1;
        auto child = currentItem->child(i);
        auto btnDelete = ui->treeWidget->itemWidget(child,3);
        if(nullptr != btnDelete)
        {
            auto name = btnDelete->property("name").toString();
            auto uid = btnDelete->property("uid").toString();
            btnDelete->setProperty("index", index);
            child->setText(0, QString::number(index));
            createNidus->updateSketchNodeIndex(name, uid,index);
        }
    }
}

void GuiCreateNidus::on_verticalScrollBar_valueChanged(int value)
{
    if(nullptr != createNidus)
    {
        createNidus->setSliceNumber(value);
    }
}

void GuiCreateNidus::closeEvent(QCloseEvent *event)
{
//    createNidus->saveSketchNode(false);// 不修改PatientInfoXml
    //    QMessageBox::StandardButton button  = QMessageBox::question(this,
    //                                                                tr("退出程序"),
    //                                                                QString(tr("确认退出程序")),
    //                                                                QMessageBox::Yes|QMessageBox::No);

    //    if(button==QMessageBox::No)
    //    {
    //        event->ignore(); // 忽略退出信号，程序继续进行
    //    }
    //    else if(button==QMessageBox::Yes)
    //    {
    //        event->accept(); // 接受退出信号，程序退出
    //    }
}

void GuiCreateNidus::on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    if( nullptr == current)return;
    auto btnDelete = ui->treeWidget->itemWidget(current,3);
    auto name = btnDelete->property("name").toString();
    auto uid = btnDelete->property("uid").toString();
    auto sliceNo = btnDelete->property("sliceNo").toInt();
    auto isTop = btnDelete->property("isTop").toBool();
    createNidus->changeSketchNodeName(name);
    if(true ==isTop )return;
    ui->verticalScrollBar->setValue(sliceNo);
    createNidus->setSliceNumber(sliceNo);
}

void GuiCreateNidus::customContextMenuRequested(const QPoint &pos)
{
    //    if(false == EditState) return;
    //    QTreeWidgetItem* currentItem = ui->treeWidget->itemAt(pos);
    //    if (nullptr != currentItem)
    //    {
    //        auto index = ui->treeWidget->indexAt(pos);
    //        auto row = index.row();
    //        auto menu = new QMenu(this);
    //        auto deleteTreeNode = new QAction(tr("deleteTreeNode"));
    //        auto stlName = currentItem->text(1);

    //        menu->addAction(deleteTreeNode);
    //        connect(deleteTreeNode, &QAction::triggered, [=]()
    //        {

    //        });

    //        menu->exec(QCursor::pos());//菜单出现的位置为当前鼠标的位置
    //    }
}
