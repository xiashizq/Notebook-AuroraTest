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
    // 注意：由于 mTopWidget 和 mCenterWidget 都设置了 this 为 parent，
    // 在 QWidget 析构时会自动 delete 子对象，所以这里可以省略 delete。
    // 如果你没有在 initUI 中设置 parent，请保留 delete。
    // 当前写法中 new 时传了 this，因此无需手动删除。
}

void FolderCompareWidget::initData()
{
    // 初始化数据（保留原有逻辑）
}

void FolderCompareWidget::initUI()
{
    // ✅ 移除这一行，或设为 0
    this->setContentsMargins(0, 0, 0, 0); // 不要在这里设边距！
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);

    // ✅ 只在这里统一设置布局边距（推荐）
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
