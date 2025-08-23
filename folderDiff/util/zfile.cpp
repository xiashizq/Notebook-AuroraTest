#include "zfile.h"
#include <QTextStream>
#include <QFileIconProvider>
#include <QTemporaryFile>
#include <QDateTime>

// 静态成员初始化
uint ZFile::m_seed = QDateTime::currentDateTime().toTime_t();
QMutex ZFile::m_mutex;

ZFile::ZFile()
{

}

ZFile::~ZFile()
{

}

int ZFile::lines(QFile *file)
{
    int count = -1;
    if(!file->open(QIODevice::ReadOnly))
    {
        return count;
    }
    else
    {
        count = 0;
    }
    QTextStream in(file);
    in.setCodec("UTF-8");
    while(!in.atEnd())
    {
        in.readLine();
        count++;
    }
    file->close();
    return count;
}

int ZFile::linesWithLine(QFile *file, QList<QString> &lineLst)
{
    int count = -1;
    if(!file->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return count;
    }
    else
    {
        count = 0;
    }
    QTextStream in(file);
    in.setCodec("UTF-8");
    while(!in.atEnd())
    {
        lineLst.append(in.readLine());
        count++;
    }
    file->close();
    return count;
}

int ZFile::linesWithHash(QFile *file, QList<unsigned int> &hashLst)
{
    int count = -1;
    if(!file->open(QIODevice::ReadOnly))
    {
        return count;
    }
    else
    {
        count = 0;
    }
    QTextStream in(file);
    in.setCodec("UTF-8");
    while(!in.atEnd())
    {
        QString line = in.readLine();
        hashLst.append(ELFHash(line));  // 直接调用内部方法
        count++;
    }
    file->close();
    return count;
}

QFileInfoList ZFile::files(const QString &path)
{
    QFileInfoList fileList;
    if(!path.isEmpty())
    {
        QDir dir(path);
        fileList = dir.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
        QFileInfoList folderList = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
        for(int i = 0; i < folderList.size(); i++)
        {
             QString path = folderList.at(i).absoluteFilePath();
             QFileInfoList childFileList = files(path);
             fileList.append(childFileList);
        }
    }
    return fileList;
}

QIcon ZFile::icon(const QString &path)
{
    QFileIconProvider provider;
    QIcon icon;
    QTemporaryFile tmpFile(path);
    tmpFile.setAutoRemove(false);
    if (tmpFile.open())
    {
        tmpFile.close();
        icon = provider.icon(QFileInfo(path));
        tmpFile.remove();
    }
    return icon;
}

// 哈希函数实现
unsigned int ZFile::ELFHash(QString key)
{
    unsigned int hash = 0;
    unsigned int x = 0;
    QByteArray ba = key.toLatin1();
    char *str = ba.data();
    while (*str)
    {
        hash = (hash << 4) + (*str++);
        if ((x = hash & 0xF0000000L) != 0)
        {
            hash ^= (x >> 24);
            hash &= ~x;
        }
    }
    return (hash & 0x7FFFFFFF);
}

// 随机字符串生成实现
QString ZFile::randString()
{
    QMutexLocker locker(&m_mutex);
    int max = 8;
    QString tmp = QString("0123456789ABCDEFGHIJKLMNOPQRSTUVWZYZ");
    QString str;
    qsrand(m_seed++);
    for(int i = 0;i < max;i++)
    {
        int ir = qrand() % tmp.length();
        str[i] = tmp.at(ir);
    }
    return str;
}
