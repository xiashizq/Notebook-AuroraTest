#ifndef FOLDERCOMPAREWIDGET_H
#define FOLDERCOMPAREWIDGET_H

#include <QWidget>
#include "widget/ztopwidget.h"
#include "widget/zcenterwidget.h"

class ZTopWidget;
class ZCenterWidget;

class FolderCompareWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FolderCompareWidget(QWidget *parent = nullptr);
    ~FolderCompareWidget();

private:
    void initData();
    void initUI();
    void initConnect();

private:
    ZTopWidget     *mTopWidget;
    ZCenterWidget  *mCenterWidget;
};

#endif // FolderCompareWidget_H
