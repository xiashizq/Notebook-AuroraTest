#include "SqlParserWindow.h"
#include "../CodeEditor/CodeEditor.h"
#include "../SqlFormat/SqlHighlighter.h"
#include "../SqlFormat/SqlParser.h"
#pragma execution_character_set("utf-8")

SqlParserWindow::SqlParserWindow(QWidget *parent) : QWidget(parent) {
    setWindowTitle("SqlParser");
    textEdit = new CodeEditor(this);

    QFont font;
    #if defined(Q_OS_WIN)
            // Windows: 优先使用微软雅黑，如果没有则使用默认系统字体
        if (QFontDatabase().families().contains("Consolas")) {
            font = QFont("Consolas");
        } else {
            font = QFont();  // 默认字体
        }
    #elif defined(Q_OS_LINUX)
        // Linux: 使用 WenQuanYi Micro Hei 或 Noto Sans CJK
    if (QFontDatabase().families().contains("Consolas")) {
        font = QFont("Consolas");
    } else if (QFontDatabase().families().contains("WenQuanYi Micro Hei")) {
        font = QFont("WenQuanYi Micro Hei");
    } else {
        font = QFont();  // 默认字体
    }
    #elif defined(Q_OS_MACOS)
            // macOS: 使用苹方或系统默认字体
        if (QFontDatabase().families().contains("PingFang SC")) {
            font = QFont("PingFang SC");
        } else {
            font = QFont();  // 默认字体
        }
    #else
        font = QFont();  // 其他平台也使用默认字体
    #endif
    font.setPointSize(12);
    textEdit->setFont(font);

    resize(900, 500);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(textEdit);
    setLayout(layout);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    highlighter = new SqlHighlighter(textEdit->document());

    textEdit->setPlainText(
        "INSERT INTO sys_user\n"
        "(user_id, username, password, nickname, avatar, sex, phone, email, email_verified, real_name, id_card, birthday, introduction, status, deleted, create_time, update_time)\n"
        "VALUES(1, 'admin', 'xxxx', '管理员', 'xxx', 1, '12345678901', NULL, 0, NULL, NULL, '2021-05-21', '遗其欲，则心静！', 0, 0, '2020-01-13 14:43:52', '2025-07-05 14:22:12');"
        );

    connect(textEdit, &QPlainTextEdit::cursorPositionChanged, this, &SqlParserWindow::onCursorChanged);
}



void SqlParserWindow::onCursorChanged()
{
    QString sql = textEdit->toPlainText();
    QTextCursor cursor = textEdit->textCursor();
    int pos = cursor.position();

    // 1. 按 ; 分割所有SQL
    QList<int> stmtStarts, stmtEnds;
    int last = 0;
    for (int i=0; i<sql.length(); ++i) {
        if (sql[i] == ';') {
            stmtStarts << last;
            stmtEnds << i;
            last = i+1;
        }
    }
    // 2. 找到光标在哪条语句区间
    int stmtIdx = -1;
    for (int i=0; i<stmtStarts.size(); ++i) {
        if (pos >= stmtStarts[i] && pos <= stmtEnds[i]) {
            stmtIdx = i;
            break;
        }
    }
    if(stmtIdx == -1) {
        highlighter->clearHighlight();
        return;
    }

    int stmtStart = stmtStarts[stmtIdx];
    int stmtEnd = stmtEnds[stmtIdx];
    QString stmt = sql.mid(stmtStart, stmtEnd-stmtStart+1); // 不要trimmed
    QList<FieldValuePos> pairs = SqlParser::parseInsertWithPos(stmt);
    // 位置偏移
    for(auto &pair:pairs) {
        pair.fieldStart += stmtStart;
        pair.fieldEnd += stmtStart;
        pair.valueStart += stmtStart;
        pair.valueEnd += stmtStart;
    }

    bool found = false;
    for (const auto &pair : pairs) {
        if ((pair.fieldStart <= pos && pos <= pair.fieldEnd) ||
            (pair.valueStart <= pos && pos <= pair.valueEnd)) {
            QColor color = QColor::fromHsv(qHash(pair.field) % 360, 180, 230);
            highlighter->setHighlightRegion(pair.fieldStart, pair.fieldEnd, pair.valueStart, pair.valueEnd, color);
            found = true;
            break;
        }
    }
    if (!found) {
        highlighter->clearHighlight();
    }
}
