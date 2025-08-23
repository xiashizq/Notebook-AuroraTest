#include "ztopwidget.h"
#pragma execution_character_set("utf-8")
ZTopWidget::ZTopWidget(QWidget *parent)
    :QToolBar(parent)
{
    initData();
    initUI();
    initConnect();
}

ZTopWidget::~ZTopWidget()
{
    delete mRefreshButton;
    delete mStopButton;
}

void ZTopWidget::initData()
{

}

void ZTopWidget::initUI()
{
    mRefreshButton = new QToolButton;
    QIcon refreshIcon(":/imageSrc/diffstart.png");
    mRefreshButton->setIcon(refreshIcon);
    mRefreshButton->setText("开始对比");
    mRefreshButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
//    mRefreshButton->setFixedSize(QSize(95, 100));

    mStopButton = new QToolButton;
    QIcon stopIcon(":/imageSrc/stopdiff.png");
    mStopButton->setIcon(stopIcon);
    mStopButton->setText("停止对比");
    mStopButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
//    mStopButton->setFixedSize(QSize(95, 100));

    this->addWidget(mRefreshButton);
    this->addWidget(mStopButton);
    this->setIconSize(QSize(45, 45));
}

void ZTopWidget::initConnect()
{
    connect(mRefreshButton, SIGNAL(clicked()), this, SIGNAL(startOrRecompare()));
    connect(mStopButton, SIGNAL(clicked()), this, SIGNAL(stopCompare()));
}
