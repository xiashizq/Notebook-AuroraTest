#include "env/zcons.h"
#include "diff/zfilediffmodel.h"
#include "util/zfile.h"
#include "zfolderctl.h"

ZFolderCtl::ZFolderCtl(QString srcBasePath, QString dstBasePath, QList<ZPathDiffModel> &mPathModelLst, QObject *parent)
    : QThread(parent)
    , mPathModelLst(mPathModelLst)
{
    mSrcBasePath = srcBasePath;
    mDstBasePath = dstBasePath;
    mIsRunning = false;
}

ZFolderCtl::~ZFolderCtl()
{

}

void ZFolderCtl::run()
{
    mIsRunning = true;
    int modelCount = mPathModelLst.size();
    for(int i = 0;i < modelCount;i++)
    {
        if(!mIsRunning)
        {
            break;
        }
        ZPathDiffModel pathDiffModel = mPathModelLst[i];
        int no = i + 1;
        QString path;
        QIcon icon;
        QString extension;
        Status status = pathDiffModel.status();
        QString sts;
        QColor color = STATUS_CLR[0];

        if(status == Same)
        {
            path = pathDiffModel.srcFileInfo().absoluteFilePath().remove(0, mSrcBasePath.length());
            extension = pathDiffModel.srcFileInfo().suffix();
            icon = ZFile::icon(pathDiffModel.srcFileInfo().absoluteFilePath());

            // 检查文件内容是否有差异，使用简化的比较方法
            QFile file1(pathDiffModel.srcFileInfo().absoluteFilePath());
            QFile file2(pathDiffModel.dstFileInfo().absoluteFilePath());
            bool hasDiff = false;
            
            if(file1.open(QIODevice::ReadOnly) && file2.open(QIODevice::ReadOnly))
            {
                QByteArray content1 = file1.readAll();
                QByteArray content2 = file2.readAll();
                hasDiff = (content1 != content2);
                file1.close();
                file2.close();
            }
            if(hasDiff)
            {
                status = Modified;
                pathDiffModel.setStatus(status);
                mPathModelLst[i] = pathDiffModel;
            }
        }
        else if(status == Removed)
        {
            path = pathDiffModel.srcFileInfo().absoluteFilePath().remove(0, mSrcBasePath.length());
            extension = pathDiffModel.srcFileInfo().suffix();
            icon = ZFile::icon(pathDiffModel.srcFileInfo().absoluteFilePath());
        }
        else if(status == Added)
        {
            path = pathDiffModel.dstFileInfo().absoluteFilePath().remove(0, mDstBasePath.length());
            extension = pathDiffModel.dstFileInfo().suffix();
            icon = ZFile::icon(pathDiffModel.dstFileInfo().absoluteFilePath());
        }
        else
        {

        }

        sts = STATUS_STR[(int)status];
        color = STATUS_CLR[(int)status];

        QList<ZTreeItemModel> itemModelList;

        ZTreeItemModel noItemModel;
        noItemModel.setHasIcon(false);
        noItemModel.setValue(no);
        noItemModel.setColor(color);
        itemModelList.append(noItemModel);

        ZTreeItemModel pathItemModel;
        pathItemModel.setHasIcon(true);
        pathItemModel.setIcon(icon);
        pathItemModel.setValue(path);
        pathItemModel.setColor(color);
        itemModelList.append(pathItemModel);

        ZTreeItemModel extItemModel;
        extItemModel.setHasIcon(false);
        extItemModel.setValue(extension);
        extItemModel.setColor(color);
        itemModelList.append(extItemModel);

        ZTreeItemModel stsItemModel;
        stsItemModel.setHasIcon(false);
        stsItemModel.setValue(sts);
        stsItemModel.setColor(color);
        itemModelList.append(stsItemModel);



        emit diffMessage(itemModelList);
        emit progress(i + 1, modelCount);
    }
    stopRunAndDelete();
}

void ZFolderCtl::stopRunAndDelete()
{
    mIsRunning = false;
    quit();
    wait();

    emit progress(100, 100);
    emit diffEnd();
}
