#include "widget.h"
#include "ui_widget.h"
#include <QAction>
#include <QDebug>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>
#include <QTextCodec>
#include <QtGui/private/qzipreader_p.h>
#include <QtGui/private/qzipwriter_p.h>
#pragma execution_character_set("utf-8")

Widget::Widget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , settings_(nullptr)
    , compilerDlg_(nullptr) {
    ui->setupUi(this);
    this->setWindowTitle("QT打包工具");
    loadConfigIni();
    initConnections();

    ui->lst_third->setContextMenuPolicy(Qt::CustomContextMenu); // 开启自定义菜单选项
}

Widget::~Widget()
{
    delete ui;
    delete settings_;
}

void Widget::initConnections() {
    connect(ui->pbt_exit, &QPushButton::clicked, this, &Widget::close);
    connect(ui->pbt_start, &QPushButton::clicked, this, &Widget::startPacking);
    connect(ui->pbt_compile, &QPushButton::clicked, this, &Widget::showCompilerDialog);
    connect(ui->pbt_exe, &QPushButton::clicked, this, &Widget::selectExecutableFile);
    connect(ui->pbt_output, &QPushButton::clicked, this, &Widget::setOutputPath);
    connect(ui->pbt_third, &QPushButton::clicked, this, &Widget::addThirdPartyLibraryFile);
    connect(ui->lst_third, &QListWidget::customContextMenuRequested, this, &Widget::showContextMenu);
}

void Widget::loadConfigIni() {
    settings_ = new QSettings(QCoreApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
    settings_->setIniCodec(QTextCodec::codecForName("utf-8")); // 设置编码方式

    if (!settings_->contains("Compiler/size")) {
        QMessageBox::information(this, "提示", "初次加载, 请添加编译器路径");
        return;
    } else {
        // 加载编译器名称
        int size = settings_->beginReadArray("Compiler");
        for (int i = 0; i < size; i++) {
            settings_->setArrayIndex(i);
            QString                   name = settings_->value("name").toString();
            CompilerDlg::CompilerInfo info(settings_->value("platform").toString(),
                                           settings_->value("path").toString());
            compilers_.insert(name, info);
            ui->cbx_compile->addItem(name);
        }
        settings_->endArray();
    }
}

void Widget::startPacking() {
    if (ui->lne_exe->text() == "") {
        QMessageBox::information(this, "提示", "可执行文件路径不能为空!");
        return;
    }

    if (ui->lne_output->text() == "") {
        QMessageBox::information(this, "提示", "输出路径不能为空!");
        return;
    }

    if (ui->cbx_compile->count() == 0) {
        QMessageBox::information(this, "提示", "请添加编译器信息!");
        return;
    }

    // 复制可执行文件到输出目录下
    ui->progressBar->setValue(20);
    QFileInfo info(ui->lne_exe->text());
    QString   dstPath = ui->lne_output->text() + "/" + info.fileName();
    if (!copyFile(ui->lne_exe->text(), dstPath, true)) {
        QMessageBox::critical(this, "提示", "拷贝文件时发生错误!");
        return;
    }

    // 开始打包可执行程序
    ui->progressBar->setValue(30);
    QString compilerPath = compilers_.value(ui->cbx_compile->currentText()).path;
    if (!QProcess::startDetached(compilerPath + " " + dstPath)) {
        QMessageBox::critical(this, "提示", "打包可执行程序时发生错误!");
        return;
    }
    ui->progressBar->setValue(60);

    // 复制第三方库文件
    int cnt = ui->lst_third->count();
    for (int i = 0; i < cnt; i++) {
        QListWidgetItem* curItem = ui->lst_third->item(i);
        info.setFile(curItem->text());
        dstPath = ui->lne_output->text() + "/" + info.fileName();
        if (!copyFile(ui->lne_exe->text(), dstPath, true)) {
            QMessageBox::critical(this, "提示", "拷贝 " + curItem->text() + " 时发生错误!");
        }
        ui->progressBar->setValue(60 + 30 * i / cnt);
    }

    ui->progressBar->setValue(90);

    // 选择解压路径
    if (ui->chb_compress->isChecked()) {
        QFileInfo info(ui->lne_exe->text());
        QString   filename = QFileDialog::getSaveFileName(this, "选择压缩文件路径", info.baseName() + ".zip", "压缩文件 (*.zip)");
        compresseDirectory(filename, ui->lne_output->text());
    }

    ui->progressBar->setValue(100);

    // 是否打开应用文件夹
    if (ui->chb_open->isChecked()) {
        QString output = ui->lne_output->text().replace("/", "\\");
        QProcess::startDetached("explorer " + output);
    }

    // 是否关闭当前应用
    if (ui->chb_close->isChecked()) {
        this->close();
    }
}

void Widget::showCompilerDialog() {
    compilerDlg_ = new CompilerDlg(this);
    compilerDlg_->setAttribute(Qt::WA_DeleteOnClose);
    // 编译器添加完成
    connect(compilerDlg_, &CompilerDlg::destroyed, this, [&]() {
        settings_->remove("Compiler");
        // 写入配置文件
        settings_->beginWriteArray("Compiler");
        int curIdx = 0;
        for (auto iter = compilers_.begin(); iter != compilers_.end(); ++iter) {
            settings_->setArrayIndex(curIdx++);
            settings_->setValue("name", iter.key());
            settings_->setValue("platform", iter.value().platform);
            settings_->setValue("path", iter.value().path);
        }
        settings_->endArray();
    });
    // 添加编译器信息
    connect(compilerDlg_, &CompilerDlg::addComiplerType,
            this, [&](const QString& name, const CompilerDlg::CompilerInfo& info) {
                if (!compilers_.contains(name)) {
                    compilers_.insert(name, info);
                    ui->cbx_compile->addItem(name);
                } else {
                    QMessageBox::critical(this, "错误", "该编译器名称已经存在!");
                }
            });
    compilerDlg_->exec();
}

void Widget::selectExecutableFile() {
    QString filename = QFileDialog::getOpenFileName(
        this,
        "选择Release文件",
        ".",
        "Executable File(*.exe)");
    if (filename != "") {
        ui->lne_exe->setText(filename);
    }
}

void Widget::setOutputPath() {
    QString pathname = QFileDialog::getExistingDirectory(
        this,
        "选择输出路径",
        ".",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (pathname != "") {
        ui->lne_output->setText(pathname);
    }
}

void Widget::addThirdPartyLibraryFile() {
    QStringList fileList = QFileDialog::getOpenFileNames(
        this,
        "选择第三方库文件",
        ".",
        "Static Library Files (*.lib);;Dynamic Library Files (*.dll);;All Files (*)");
    ui->lst_third->addItems(fileList);
}

void Widget::showContextMenu(const QPoint& pos) {
    QListWidgetItem* curItem = ui->lst_third->itemAt(pos);
    if (curItem == nullptr) {
        return;
    }

    QMenu*   popMenu      = new QMenu(this);
    QAction* actRemove    = new QAction("删除", popMenu);
    QAction* actRemoveAll = new QAction("删除所有", popMenu);
    popMenu->addAction(actRemove);
    popMenu->addAction(actRemoveAll);
    connect(actRemove, &QAction::triggered, this, [&]() {
        ui->lst_third->removeItemWidget(curItem);
        delete curItem;
    });
    connect(actRemoveAll, &QAction::triggered, this, [&]() {
        ui->lst_third->clear();
    });
    popMenu->exec(QCursor::pos());
    delete popMenu;
}

bool Widget::copyFile(const QString& srcPath, const QString& dstPath, bool coverFileIfExist) {
    if (srcPath == dstPath) {
        return true;
    }

    if (!QFile::exists(srcPath)) { //源文件不存在
        return false;
    }

    if (QFile::exists(dstPath)) {
        if (coverFileIfExist) {
            QFile::remove(dstPath);
        }
    }

    if (!QFile::copy(srcPath, dstPath)) {
        return false;
    }
    return true;
}

void Widget::compresseDirectory(const QString& dstPath, const QString& srcPath) {
    QZipWriter* writer = new QZipWriter(dstPath);
    writer->addDirectory(srcPath);
    writer->close();
    delete writer;
}
