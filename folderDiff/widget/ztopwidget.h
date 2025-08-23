#ifndef ZTOPWIDGET
#define ZTOPWIDGET

#include <QtWidgets>

class ZTopWidget : public QToolBar{
    Q_OBJECT
public:
    ZTopWidget(QWidget *parent = 0);
    ~ZTopWidget();

signals:
    void startOrRecompare();
    void stopCompare();

private:
    void initData();
    void initUI();
    void initConnect();

private:
    QToolButton *mRefreshButton;
    QToolButton *mStopButton;
};

#endif // ZTOPWIDGET

