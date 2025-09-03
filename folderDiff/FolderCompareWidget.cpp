#include "FolderCompareWidget.h"
#include <QVBoxLayout>

FolderCompareWidget::FolderCompareWidget(QWidget *parent)
    : QWidget(parent)
{
    initData();
    initUI();
    initConnect();
}

FolderCompareWidget::~FolderCompareWidget()
{

}

void FolderCompareWidget::initData()
{
    // 初始化数据（保留原有逻辑）
}

void FolderCompareWidget::initUI()
{
    this->setContentsMargins(0, 0, 0, 0); // 不要在这里设边距！
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);

    mainLayout->setContentsMargins(
        STANDARD_MARGIN,  // left
        STANDARD_MARGIN,  // top    → 这才是你想要的顶部边距
        STANDARD_MARGIN,  // right
        STANDARD_MARGIN   // bottom
        );

    mTopWidget = new ZTopWidget(this);
    mCenterWidget = new ZCenterWidget(this);

    mainLayout->addWidget(mTopWidget);
    mainLayout->addWidget(mCenterWidget);

    setLayout(mainLayout);
}

void FolderCompareWidget::initConnect()
{
    connect(mTopWidget, &ZTopWidget::startOrRecompare,
            mCenterWidget, &ZCenterWidget::startOrRecompare);

    connect(mTopWidget, &ZTopWidget::stopCompare,
            mCenterWidget, &ZCenterWidget::stopCompare);
}
