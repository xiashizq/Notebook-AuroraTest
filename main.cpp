#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    //高 DPI 设置（Qt 5.14+）
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    // 启用自动高 DPI 缩放
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    // 推荐策略：
    // - Round: 四舍五入（推荐，平衡清晰度和布局）
    // - Floor: 向下取整（更清晰，但可能偏小）
    // - Ceil: 向上取整（更大，但可能偏大）
    // - PassThrough: 精确但可能导致错位
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::Round);
#endif

    //支持高清图标
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);



    //创建应用
    QApplication app(argc, argv);

    QFont font;
    font.setFamily("Microsoft YaHei");     // 推荐：微软雅黑（Windows）
    // font.setFamily("SimHei");           // 黑体（无衬线，清晰）
    // font.setFamily("SimSun");           // 宋体（传统）
    // font.setFamily("Noto Sans CJK SC"); // 谷歌开源字体（跨平台好）
    font.setPointSize(10);                 // 字号（可根据 DPI 调整）
    // font.setBold(true);                 // 可选加粗
    app.setFont(font);

    //设置应用信息（可选）
    app.setApplicationName("MyEditor");
    app.setApplicationVersion("1.0");

    //可选：设置样式（提升跨平台一致性）
    // app.setStyle("Fusion"); // Qt 原生样式，跨平台一致

    MainWindow w;
    w.show();

    return app.exec();
}
