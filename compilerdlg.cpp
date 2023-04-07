#include "compilerdlg.h"
#include "ui_compilerdlg.h"
#include <QCloseEvent>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QtGui/private/qzipreader_p.h>
#include <QtGui/private/qzipwriter_p.h>
#pragma execution_character_set("utf-8")

CompilerDlg::CompilerDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CompilerDlg)
{
    ui->setupUi(this);
    this->setWindowTitle("添加编译器");
    connect(ui->pbt_ok, &QPushButton::clicked, this, &CompilerDlg::addCompiler);
    connect(ui->pbt_cancel, &QPushButton::clicked, this, &CompilerDlg::close);
    connect(ui->pbt_select, &QPushButton::clicked, this, &CompilerDlg::selectCompilerPath);
}

CompilerDlg::~CompilerDlg()
{
    delete ui;
}

void CompilerDlg::addCompiler() {
    if (ui->lne_name->text() == "") {
        QMessageBox::information(this, "提示", "请输入编译器名称!");
        return;
    }
    if (ui->lne_path->text() == "") {
        QMessageBox::information(this, "提示", "请选择编译器路径!");
        return;
    }

    CompilerInfo info(ui->cbx_bits->currentText(), ui->lne_path->text() + "/windeployqt.exe");

    emit addComiplerType(ui->lne_name->text(), info);

    auto res = QMessageBox::information(this, "提示", "添加成功，是否继续添加?", QMessageBox::Yes | QMessageBox::No);
    if (res == QMessageBox::No) {
        this->close();
    }
}

void CompilerDlg::selectCompilerPath() {
    QString pathname = QFileDialog::getExistingDirectory(
        this,
        "选择编译器路径",
        ".",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    QDir dir(pathname);
    if (!dir.exists()) {
        QMessageBox::critical(this, "错误", "无效的编译器路径!");
        return;
    }
    bool          exist = false;
    QFileInfoList list  = dir.entryInfoList();
    for (auto& fileinfo : list) {
        if (fileinfo.isFile() && fileinfo.baseName() == "windeployqt") {
            exist = true;
            break;
        }
    }
    if (!exist) {
        QMessageBox::critical(this, "错误", "未在所选路径下寻找到windeployqt可执行文件!");
        return;
    }
    ui->lne_path->setText(pathname);
}
