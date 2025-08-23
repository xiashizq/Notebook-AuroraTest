#include "zcenterwidget.h"
#include "widget/folder/zfolderwidget.h"

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
}

