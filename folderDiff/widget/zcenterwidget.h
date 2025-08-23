#ifndef ZCENTERWIDGET
#define ZCENTERWIDGET

#include <QtWidgets>
#include "diff/zpathdiffmodel.h"
#include "widget/folder/zfolderwidget.h"

class ZCenterWidget : public QWidget{
    Q_OBJECT

public:
    ZCenterWidget(QWidget *parent = 0);
    ~ZCenterWidget();

public slots:
    void folderComparison();
    void startOrRecompare();
    void stopCompare();
    void onFileCompare(ZPathDiffModel pathDiffModel);

protected:
    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;

private:
    void initData();
    void initUI();
    void initConnect();

private:
    ZFolderWidget *mFolderWidget;
    QVBoxLayout *mLayout;
    int mFolderCount;
};

#endif // ZCENTERWIDGET

