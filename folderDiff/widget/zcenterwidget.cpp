#include "zcenterwidget.h"
#include "widget/folder/zfolderwidget.h"
#include "../DiffFunction/DiffWidget.h"
#pragma execution_character_set("utf-8")
ZCenterWidget::ZCenterWidget(QWidget *parent)
    :QWidget(parent)
{
    initData();
    initUI();
    initConnect();
}

ZCenterWidget::~ZCenterWidget()
{
    delete mFolderWidget;
}

void ZCenterWidget::folderComparison()
{
    if (mFolderWidget != nullptr)
    {
        delete mFolderWidget;
    }

    mFolderWidget = new ZFolderWidget;
    mFolderWidget->setObjectName(OBJECT_FOLDER_COMPARISON);
    mLayout->addWidget(mFolderWidget);
}

void ZCenterWidget::startOrRecompare()
{
    if (mFolderWidget != nullptr)
    {
        mFolderWidget->compare();
    }
}

void ZCenterWidget::stopCompare()
{
    if (mFolderWidget != nullptr)
    {
        mFolderWidget->stopCompare();
    }
}

void ZCenterWidget::paintEvent(QPaintEvent *event){
    QWidget::paintEvent(event);
}

void ZCenterWidget::initData()
{
    mFolderCount = 1;
    mFolderWidget = nullptr;
}

void ZCenterWidget::initUI()
{
    mLayout = new QVBoxLayout;
    mLayout->setContentsMargins(0, STANDARD_MARGIN, 0, 0);
    this->setLayout(mLayout);

    // 创建初始的文件夹对比窗口
    folderComparison();
}

void ZCenterWidget::initConnect()
{
    // 不需要连接标签页相关信号
    connect(mFolderWidget, &ZFolderWidget::fileCompare, this, &ZCenterWidget::onFileCompare);
}

void ZCenterWidget::onFileCompare(ZPathDiffModel pathDiffModel)
{
    // 打印两个文件的路径
    QString srcPath = pathDiffModel.srcFileInfo().absoluteFilePath();
    QString dstPath = pathDiffModel.dstFileInfo().absoluteFilePath();
    qDebug() << srcPath;
    qDebug() << dstPath;
    openFileDiffDialog(srcPath,dstPath);
}

void ZCenterWidget::openFileDiffDialog(QString srcPath,QString dstPath){
    DiffWidget* w = new DiffWidget();
    w->setAttribute(Qt::WA_DeleteOnClose); // 自动释放
    w->setWindowTitle("行级字符/代码对比结果");
    w->setFiles(srcPath,dstPath);
    w->resize(900,600);
    w->show();
}


