#ifndef COMPILERDLG_H
#define COMPILERDLG_H

#include <QDialog>
#include <QVector>

namespace Ui {
class CompilerDlg;
}

/**
 * @brief 编译器对话框
 */
class CompilerDlg : public QDialog
{
    Q_OBJECT

public:
    explicit CompilerDlg(QWidget *parent = nullptr);
    ~CompilerDlg();

    /**
     * @brief 编译器信息
     */
    struct CompilerInfo {
        CompilerInfo() {}
        CompilerInfo(const QString& _platform, const QString& _path)
            : platform(_platform)
            , path(_path) {}
        QString platform; // 编译器平台
        QString path;     // 编译器路径
    };

signals:
    /**
     * @brief 添加编译器类型的信号
     * @param name 编译器名称
     * @param is86 是否是x86平台
     * @param path 编译器路径
     */
    void addComiplerType(const QString& name, const CompilerInfo& info);

private slots:
    /**
     * @brief 添加编译器
     */
    void addCompiler();

    /**
     * @brief 选择编译器路径
     */
    void selectCompilerPath();

private:
    Ui::CompilerDlg *ui;
};

#endif // COMPILERDLG_H
