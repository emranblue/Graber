#include "InjectFromNoteDialog.h"
#include "MarkdownUtils.h"
#include "Utils.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDirIterator>
#include <QFileInfo>
#include <QSplitter>

InjectFromNoteDialog::InjectFromNoteDialog(const QString &notesDir,
                                           const QString &currentTargetFile,
                                           ParseFn parseFn,
                                           QWidget *parent)
    : QDialog(parent),
      notes_dir_(notesDir),
      current_target_(QFileInfo(currentTargetFile).canonicalFilePath()),
      parse_fn_(std::move(parseFn)) {
    setWindowTitle(QStringLiteral("ইনজেক্ট — অন্য নোট থেকে উপ-শিরোনাম"));
    setStyleSheet(
        "QDialog { background-color: #f5f5f7; font-family: 'SF Pro Text', 'Segoe UI', 'Kalpurush'; color: #1d1d1f; }"
        "QLabel { color: #1d1d1f; font-size: 13px; background: transparent; }"
        "QLineEdit { background: white; color: #1d1d1f; padding: 8px 10px; border: 1px solid rgba(0,0,0,0.12); "
        "border-radius: 8px; font-size: 13px; }"
        "QListWidget { background: white; border: 1px solid rgba(0,0,0,0.08); border-radius: 10px; "
        "padding: 4px; color: #1d1d1f; outline: none; font-size: 13px; }"
        "QListWidget::item { padding: 6px 8px; border-radius: 6px; }"
        "QListWidget::item:selected { background: #e8f0fe; color: #1d1d1f; }"
        "QPushButton { background-color: #5856d6; color: white; border-radius: 10px; "
        "padding: 10px 14px; font-weight: 600; border: none; min-width: 90px; }"
        "QPushButton:hover { background-color: #7d7aff; }"
        "QPushButton#cancelBtn { background-color: #718093; }"
        "QPushButton#cancelBtn:hover { background-color: #57606f; }"
        "QPushButton:disabled { background-color: #e5e5ea; color: #8e8e93; }");

    if (parent)
        resize(qMax(640, parent->width() * 4 / 5), qMax(520, parent->height() * 3 / 4));
    else
        resize(680, 560);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(14, 14, 14, 14);
    root->setSpacing(10);

    auto *hint = new QLabel(
        QStringLiteral("① GraberNotes থেকে অন্য .md ফাইল বাছুন  →  ② তার উপ-শিরোনাম বাছুন  →  ইনজেক্ট\n"
                       "ক্লিপবোর্ড ব্যবহার হয় না। ব্লক হুবহু কপি হয়ে বর্তমান নোটের নির্বাচিত স্থানে বসবে।"),
        this);
    hint->setWordWrap(true);
    hint->setStyleSheet("color: #57606f; font-size: 12px;");
    root->addWidget(hint);

    file_filter_ = new QLineEdit(this);
    file_filter_->setPlaceholderText(QStringLiteral("ফাইল খুঁজুন… (folder / name)"));
    connect(file_filter_, &QLineEdit::textChanged, this, &InjectFromNoteDialog::onFileFilterChanged);
    root->addWidget(file_filter_);

    auto *split = new QSplitter(Qt::Horizontal, this);

    // Left: files
    auto *left = new QWidget(split);
    auto *leftLay = new QVBoxLayout(left);
    leftLay->setContentsMargins(0, 0, 0, 0);
    leftLay->setSpacing(4);
    leftLay->addWidget(new QLabel(QStringLiteral("📁 নোট ফাইল (GraberNotes)"), left));
    file_list_ = new QListWidget(left);
    file_list_->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(file_list_, &QListWidget::currentRowChanged, this, &InjectFromNoteDialog::onFileSelected);
    leftLay->addWidget(file_list_, 1);
    split->addWidget(left);

    // Right: headings
    auto *right = new QWidget(split);
    auto *rightLay = new QVBoxLayout(right);
    rightLay->setContentsMargins(0, 0, 0, 0);
    rightLay->setSpacing(4);
    rightLay->addWidget(new QLabel(QStringLiteral("📑 শিরোনাম / উপ-শিরোনাম"), right));
    heading_list_ = new QListWidget(right);
    heading_list_->setSelectionMode(QAbstractItemView::SingleSelection);
    heading_list_->setWordWrap(true);
    connect(heading_list_, &QListWidget::itemDoubleClicked, this, &InjectFromNoteDialog::onHeadingDoubleClicked);
    rightLay->addWidget(heading_list_, 1);
    split->addWidget(right);

    split->setStretchFactor(0, 2);
    split->setStretchFactor(1, 3);
    root->addWidget(split, 1);

    status_ = new QLabel(QStringLiteral("একটি ফাইল নির্বাচন করুন…"), this);
    status_->setStyleSheet("color: #007aff; font-weight: 600;");
    root->addWidget(status_);

    auto *btns = new QHBoxLayout();
    btns->addStretch();
    auto *cancel = new QPushButton(QStringLiteral("বাতিল"), this);
    cancel->setObjectName(QStringLiteral("cancelBtn"));
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    auto *inject = new QPushButton(QStringLiteral("ইনজেক্ট"), this);
    connect(inject, &QPushButton::clicked, this, &InjectFromNoteDialog::onInjectClicked);
    btns->addWidget(cancel);
    btns->addWidget(inject);
    root->addLayout(btns);

    scanNoteFiles();
    populateFileList(QString());
}

void InjectFromNoteDialog::scanNoteFiles() {
    all_rel_paths_.clear();
    all_abs_paths_.clear();

    QDir root(notes_dir_);
    if (!root.exists())
        return;

    QDirIterator it(notes_dir_, QStringList{QStringLiteral("*.md")},
                    QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        const QFileInfo fi = it.fileInfo();
        const QString abs = fi.canonicalFilePath();
        if (abs.isEmpty())
            continue;
        // skip current target note
        if (!current_target_.isEmpty() && abs == current_target_)
            continue;
        // skip obvious system/backup paths
        const QString rel = root.relativeFilePath(fi.absoluteFilePath());
        if (rel.startsWith(QStringLiteral("backup/")) || rel.startsWith(QStringLiteral("deleted/"))
            || rel.startsWith(QStringLiteral(".git/")) || rel.startsWith(QStringLiteral("config/")))
            continue;

        all_rel_paths_ << rel;
        all_abs_paths_ << abs;
    }
}

void InjectFromNoteDialog::populateFileList(const QString &filter) {
    file_list_->clear();
    heading_list_->clear();
    source_file_.clear();
    source_slug_.clear();
    source_title_.clear();

    const QString f = filter.trimmed().toLower();
    for (int i = 0; i < all_rel_paths_.size(); ++i) {
        if (!f.isEmpty() && !all_rel_paths_.at(i).toLower().contains(f))
            continue;
        auto *item = new QListWidgetItem(all_rel_paths_.at(i), file_list_);
        item->setData(Qt::UserRole, all_abs_paths_.at(i));
        item->setToolTip(all_abs_paths_.at(i));
    }

    status_->setText(QStringLiteral("%1 টি নোট ফাইল").arg(file_list_->count()));
}

void InjectFromNoteDialog::onFileFilterChanged(const QString &text) {
    populateFileList(text);
}

void InjectFromNoteDialog::onFileSelected() {
    QListWidgetItem *item = file_list_->currentItem();
    if (!item) {
        heading_list_->clear();
        return;
    }
    loadHeadingsForFile(item->data(Qt::UserRole).toString());
}

void InjectFromNoteDialog::loadHeadingsForFile(const QString &absPath) {
    heading_list_->clear();
    source_file_ = absPath;
    source_slug_.clear();
    source_title_.clear();

    if (absPath.isEmpty() || !parse_fn_) {
        status_->setText(QStringLiteral("পার্স করা যায়নি।"));
        return;
    }

    const QList<NoteItem> items = parse_fn_(absPath);
    if (items.isEmpty()) {
        status_->setText(QStringLiteral("এই ফাইলে কোনো শিরোনাম নেই: %1")
                             .arg(QFileInfo(absPath).fileName()));
        return;
    }

    const QStringList ids = MarkdownUtils::compute_display_ids(items);
    for (int i = 0; i < items.size(); ++i) {
        const NoteItem &it = items.at(i);
        const QString did = (i < ids.size()) ? ids.at(i) : QString();
        QString label;
        if (it.type == QLatin1String("heading")) {
            label = QStringLiteral("%1  %2").arg(did, it.title);
        } else {
            label = QStringLiteral("  ↳ %1  %2").arg(did, it.title);
        }
        auto *row = new QListWidgetItem(label, heading_list_);
        row->setData(Qt::UserRole, it.slug);
        row->setData(Qt::UserRole + 1, it.title);
        row->setData(Qt::UserRole + 2, it.type);
        if (it.type == QLatin1String("heading")) {
            QFont f = row->font();
            f.setBold(true);
            row->setFont(f);
            row->setForeground(QColor("#c0392b"));
        } else {
            row->setForeground(QColor("#2980b9"));
        }
    }

    status_->setText(QStringLiteral("%1 — %2 টি শিরোনাম")
                         .arg(QFileInfo(absPath).fileName())
                         .arg(items.size()));
}

void InjectFromNoteDialog::onHeadingDoubleClicked(QListWidgetItem *item) {
    if (!item)
        return;
    source_slug_ = item->data(Qt::UserRole).toString();
    source_title_ = item->data(Qt::UserRole + 1).toString();
    if (!source_file_.isEmpty() && !source_slug_.isEmpty())
        accept();
}

void InjectFromNoteDialog::onInjectClicked() {
    QListWidgetItem *item = heading_list_->currentItem();
    if (!item) {
        status_->setText(QStringLiteral("আগে একটি শিরোনাম/উপ-শিরোনাম নির্বাচন করুন।"));
        status_->setStyleSheet("color: #e74c3c; font-weight: 600;");
        return;
    }
    if (source_file_.isEmpty()) {
        status_->setText(QStringLiteral("আগে একটি ফাইল নির্বাচন করুন।"));
        status_->setStyleSheet("color: #e74c3c; font-weight: 600;");
        return;
    }
    source_slug_ = item->data(Qt::UserRole).toString();
    source_title_ = item->data(Qt::UserRole + 1).toString();
    if (source_slug_.isEmpty())
        return;
    accept();
}
