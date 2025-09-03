#ifndef ZCENTERWIDGET
#define ZCENTERWIDGET

#include <QtWidgets>
#include "diff/zpathdiffmodel.h"
#include "widget/folder/zfolderwidget.h"

class ZCenterWidget : public QWidget{
    Q_OBJECT

public:
    ZCenterWidget(QWidget *parent = nullptr);
    ~ZCenterWidget();

public slots:
    void folderComparison();
    void startOrRecompare();
    void stopCompare();
    void onFileCompare(ZPathDiffModel pathDiffModel);
    void openFileDiffDialog(QString srcPath,QString dstPath);


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

