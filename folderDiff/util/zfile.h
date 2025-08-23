#ifndef ZFILE
#define ZFILE

#include <QFile>
#include <QList>
#include <QFileInfo>
#include <QDir>
#include <QIcon>
#include <QMutex>

class ZFile{
public:
    ZFile();
    ~ZFile();

    // 文件操作相关功能
    static int lines(QFile *file);
    static int linesWithLine(QFile *file, QList<QString> &lineLst);
    static int linesWithHash(QFile *file, QList<unsigned int> &hashLst);
    static QFileInfoList files(const QString &path);
    static QIcon icon(const QString &path);
    
    // 哈希计算功能（原ZHash）
    static unsigned int ELFHash(QString key);
    
    // 随机字符串生成功能（原ZRand）
    static QString randString();
    
private:
    // 随机数生成相关静态成员
    static uint m_seed;
    static QMutex m_mutex;
};

#endif // ZFILE

