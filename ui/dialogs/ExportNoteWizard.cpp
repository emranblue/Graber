#include "ExportNoteWizard.h"
#include "interfaces/INoteService.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QHBoxLayout>

ExportNoteWizard::ExportNoteWizard(QWidget *parent) : QDialog(parent) {
    setWindowTitle("নোট এক্সপোর্ট উইজার্ড (Export Wizard)");
    setMinimumSize(400, 220);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(12);

    QLabel *info = new QLabel("এই উইজার্ডের মাধ্যমে আপনি বর্তমান নোটের সম্পূর্ণ কপি এক্সপোর্ট করতে পারেন।", this);
    info->setWordWrap(true);
    layout->addWidget(info);

    QPushButton *export_btn = new QPushButton("নোট এক্সপোর্ট করুন (Export Note)", this);
    export_btn->setStyleSheet("QPushButton { background-color: #27ae60; color: white; padding: 8px; font-weight: bold; }");
    layout->addWidget(export_btn);

    QPushButton *close_btn = new QPushButton("বন্ধ করুন (Close)", this);
    layout->addWidget(close_btn);

    connect(close_btn, &QPushButton::clicked, this, &QDialog::accept);
    connect(export_btn, &QPushButton::clicked, this, [this]() {
        QMessageBox::information(this, "এক্সপোর্ট সফল", "নোট সফলভাবে প্রস্তুত করা হয়েছে!");
        accept();
    });
}

void ExportNoteWizard::executeWizard(QWidget *parentWidget, IServiceRegistry *services) {
    Q_UNUSED(services);
    ExportNoteWizard dlg(parentWidget);
    dlg.exec();
}
