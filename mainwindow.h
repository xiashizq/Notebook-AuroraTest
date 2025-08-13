#pragma once
#include <QMainWindow>
#include <QDebug>
#include <QFileInfo>
#include <QEvent>
class QsciScintilla;
class SqlHighlighter;
class SqlLexer;
#pragma execution_character_set("utf-8")

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    //    void onCursorChanged();
    void openNewWindow();
    void updateLineNumberWidth(QsciScintilla *textEdit);
    void addNewTabMneu();
    void addNewTab(const QString &filePath);
    void closeTab(int index);
    bool saveFile(int index);
    int extractNumberFromUntitled(const QString &filename);
    void openRecentFile();

    void openFileNoPar();           // 无参数：弹出文件选择对话框
    void openFile(const QString &path); // 有路径：直接打开

protected:
    void closeEvent(QCloseEvent *event) override;  // ✅ 声明为 protected

private:
    QsciScintilla *textEdit;
    SqlHighlighter *highlighter;
    int m_untitledCount;
    struct TabInfo {
        QsciScintilla *editor;
        QString filePath;     // 实际文件路径，恢复的未命名文件为空
        QString baseName;     // 显示名称，如 "未命名1" 或 "report.sql"
        bool isModified;
        QString autoSavePath;
        bool neverSavedByUser = true;
        // 获取显示名称（用于 tab 标签）
        QString displayName() const {
            return baseName.isEmpty() ? "未命名" : baseName;
        }
    };
    QVector<TabInfo> m_tabs;        // 存储所有 tab 信息
    QTabWidget *m_tabWidget;        // 指向 tab widget

    QTimer *m_autoSaveTimer; // 自动保存定时器
    QString autoSaveDir();   // 返回自动保存目录（如 temp 或 app dir）
    void autoSaveAll();      // 保存所有可保存的 tab
    QString autoSavePathForTab(const TabInfo &info); // 为每个 tab 生成自动保存路径

    void saveCurrentSession();    // 关闭前保存当前会话
    void restoreLastSession();    // 启动时恢复会话
    QStringList getLastOpenFiles(); // 从设置中读取
    void setLastOpenFiles(const QStringList &files); // 写入设置
    bool closeTabWithPrompt(int index); // ✅ 返回 false 表示取消关闭

    QMenu *m_recentFilesMenu;
    void setupRecentFilesMenu();
    void updateRecentFilesMenu();

    void onSave();           // Ctrl+S
    void onSaveAs();         // Ctrl+Shift+S
    void onCloseTab();       // Ctrl+W
    void onNewFile();        // Ctrl+N
    void onOpenFile();       // Ctrl+O
};
