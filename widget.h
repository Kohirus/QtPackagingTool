#ifndef WIDGET_H
#define WIDGET_H

#include "compilerdlg.h"
#include <QHash>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class CompilerDlg;
class QSettings;
class QZipWriter;

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    /**
     * @brief 初始化信号和槽
     */
    void initConnections();

    /**
     * @brief 加载配置文件
     */
    void loadConfigIni();

private slots:
    /**
     * @brief 开始打包
     */
    void startPacking();

    /**
     * @brief 添加编译器类型
     */
    void showCompilerDialog();

    /**
     * @brief 选择可执行文件
     */
    void selectExecutableFile();

    /**
     * @brief 设置输出路径
     */
    void setOutputPath();

    /**
     * @brief 添加第三方库文件
     */
    void addThirdPartyLibraryFile();

    /**
     * @brief 显示右键菜单
     */
    void showContextMenu(const QPoint& pos);

private:
    /**
     * @brief 拷贝文件到指定路径
     * @param srcPath 源路径
     * @param dstPath 目标路径
     * @param coverFileIfExist 文件存在是否覆盖
     * @return 拷贝失败返回false，否则返回true
     */
    bool copyFile(const QString& srcPath, const QString& dstPath, bool coverFileIfExist);

    /**
     * @brief 压缩指定目录
     * @param dstPath 目标路径
     * @param srcPath 源路径
     */
    void compresseDirectory(const QString& dstPath, const QString& srcPath);

private:
    Ui::Widget*  ui;
    QSettings*   settings_;    // 配置文件
    CompilerDlg* compilerDlg_; // 编译器对话框

    QHash<QString, CompilerDlg::CompilerInfo> compilers_;
};
#endif // WIDGET_H
