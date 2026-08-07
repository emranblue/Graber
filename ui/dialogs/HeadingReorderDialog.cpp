#include "HeadingReorderDialog.h"
#include "MarkdownUtils.h"
#include "Utils.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QMimeData>
#include <QMessageBox>
#include <QApplication>
#include <QPainter>
#include <QDrag>
#include <functional>

namespace {

constexpr int RoleSlug = Qt::UserRole;
constexpr int RoleType = Qt::UserRole + 1; // "heading" | "subheading"

class ReorderTree : public QTreeWidget {
public:
    using QTreeWidget::QTreeWidget;

    std::function<void(const QString &src, const QString &tgt)> onMove;

protected:
    void startDrag(Qt::DropActions /*supportedActions*/) override {
        QTreeWidgetItem *item = currentItem();
        if (!item)
            return;
        // Only subheadings may be dragged
        if (item->data(0, RoleType).toString() != QLatin1String("subheading"))
            return;

        QMimeData *mime = new QMimeData;
        mime->setText(item->data(0, RoleSlug).toString());
        mime->setData(QStringLiteral("application/x-graber-heading-slug"),
                      item->data(0, RoleSlug).toString().toUtf8());

        QDrag *drag = new QDrag(this);
        drag->setMimeData(mime);
        // Simple text pixmap for feedback
        QPixmap pix(220, 28);
        pix.fill(QColor(0, 122, 255, 40));
        QPainter p(&pix);
        p.setPen(QColor("#1d1d1f"));
        p.drawText(pix.rect().adjusted(8, 0, -8, 0), Qt::AlignVCenter | Qt::AlignLeft,
                   item->text(0));
        p.end();
        drag->setPixmap(pix);
        drag->setHotSpot(QPoint(16, 14));
        drag->exec(Qt::MoveAction);
    }

    void dragEnterEvent(QDragEnterEvent *event) override {
        if (event->mimeData()->hasFormat(QStringLiteral("application/x-graber-heading-slug")))
            event->acceptProposedAction();
        else
            event->ignore();
    }

    void dragMoveEvent(QDragMoveEvent *event) override {
        if (!event->mimeData()->hasFormat(QStringLiteral("application/x-graber-heading-slug"))) {
            event->ignore();
            return;
        }
        QTreeWidgetItem *over = itemAt(event->position().toPoint());
        if (!over) {
            event->ignore();
            return;
        }
        event->acceptProposedAction();
    }

    void dropEvent(QDropEvent *event) override {
        if (!event->mimeData()->hasFormat(QStringLiteral("application/x-graber-heading-slug"))) {
            event->ignore();
            return;
        }
        const QString sourceSlug =
            QString::fromUtf8(event->mimeData()->data(QStringLiteral("application/x-graber-heading-slug")));
        if (sourceSlug.isEmpty()) {
            event->ignore();
            return;
        }

        QTreeWidgetItem *over = itemAt(event->position().toPoint());
        if (!over) {
            event->ignore();
            return;
        }

        const QString targetSlug = over->data(0, RoleSlug).toString();
        if (targetSlug.isEmpty() || targetSlug == sourceSlug) {
            event->ignore();
            return;
        }

        // Drop indicator: if dropping on upper half of a subheading, insert *before*
        // that item → target becomes previous sibling (or parent heading).
        QString insertAfterSlug = targetSlug;
        const QRect r = visualItemRect(over);
        const int y = event->position().toPoint().y();
        const bool upperHalf = (y < r.center().y());

        if (upperHalf && over->data(0, RoleType).toString() == QLatin1String("subheading")) {
            QTreeWidgetItem *parent = over->parent();
            if (parent) {
                const int idx = parent->indexOfChild(over);
                if (idx <= 0) {
                    // Before first sub → insert after the parent main heading
                    insertAfterSlug = parent->data(0, RoleSlug).toString();
                } else {
                    insertAfterSlug = parent->child(idx - 1)->data(0, RoleSlug).toString();
                }
            }
        }

        if (insertAfterSlug == sourceSlug) {
            event->ignore();
            return;
        }

        event->acceptProposedAction();
        if (onMove)
            onMove(sourceSlug, insertAfterSlug);
    }
};

} // namespace

HeadingReorderDialog::HeadingReorderDialog(const QList<NoteItem> &all_headings,
                                           const QString &current_slug,
                                           QWidget *parent)
    : QDialog(parent), headings_(all_headings) {
    setWindowTitle(QStringLiteral("শাফল — উপ-শিরোনাম পুনর্বিন্যাস (Shuffle / Drag-Reorder)"));
    setStyleSheet(
        "QDialog { background-color: #f5f5f7; font-family: 'SF Pro Text', 'Segoe UI', 'Kalpurush'; color: #1d1d1f; }"
        "QLabel { color: #1d1d1f; font-size: 13px; background: transparent; }"
        "QTreeWidget { background: white; border: 1px solid rgba(0,0,0,0.08); border-radius: 12px; "
        "padding: 6px; color: #1d1d1f; outline: none; font-size: 13px; }"
        "QTreeWidget::item { padding: 6px 4px; border-radius: 6px; }"
        "QTreeWidget::item:selected { background: #e8f0fe; color: #1d1d1f; }"
        "QTreeWidget::item:hover { background: #f0f4f8; }"
        "QPushButton { background-color: #007aff; color: white; border-radius: 10px; "
        "padding: 10px 14px; font-weight: 600; border: none; min-width: 80px; }"
        "QPushButton:hover { background-color: #0066d6; }"
        "QPushButton#closeBtn { background-color: #718093; }"
        "QPushButton#closeBtn:hover { background-color: #57606f; }");

    if (parent)
        resize(qMax(520, parent->width() * 3 / 4), qMax(560, parent->height() * 3 / 4));
    else
        resize(560, 640);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->setSpacing(10);

    hint_label_ = new QLabel(
        QStringLiteral("মাউস দিয়ে উপ-শিরোনাম টেনে অন্য শিরোনাম/উপ-শিরোনামের উপর ছাড়ুন।\n"
                       "ড্রপ = সেই আইটেমের পরে স্থানান্তর। উপরের অর্ধে ড্রপ = আগে স্থানান্তর।\n"
                       "স্লাগ (id) অপরিবর্তিত থাকে → TOC লিংক ও নেভিগেশন ঠিক থাকে।"),
        this);
    hint_label_->setWordWrap(true);
    hint_label_->setStyleSheet("color: #57606f; font-size: 12px;");
    layout->addWidget(hint_label_);

    auto *rtree = new ReorderTree(this);
    tree_ = rtree;
    tree_->setColumnCount(1);
    tree_->setHeaderHidden(true);
    tree_->setAnimated(true);
    tree_->setIndentation(22);
    tree_->setSelectionMode(QAbstractItemView::SingleSelection);
    tree_->setDragEnabled(true);
    tree_->setAcceptDrops(true);
    tree_->setDropIndicatorShown(true);
    tree_->setDragDropMode(QAbstractItemView::DragDrop);
    tree_->setDefaultDropAction(Qt::MoveAction);
    tree_->viewport()->setAcceptDrops(true);
    layout->addWidget(tree_, 1);

    rtree->onMove = [this](const QString &src, const QString &tgt) {
        status_label_->setText(QStringLiteral("স্থানান্তর হচ্ছে…"));
        emit moveRequested(src, tgt);
    };

    status_label_ = new QLabel(QStringLiteral("টেনে আনুন ও ছাড়ুন (Drag & drop)"), this);
    status_label_->setStyleSheet("color: #007aff; font-weight: 600;");
    layout->addWidget(status_label_);

    auto *btn_row = new QHBoxLayout();
    btn_row->addStretch();
    auto *close_btn = new QPushButton(QStringLiteral("বন্ধ (Close)"), this);
    close_btn->setObjectName(QStringLiteral("closeBtn"));
    connect(close_btn, &QPushButton::clicked, this, &QDialog::accept);
    btn_row->addWidget(close_btn);
    layout->addLayout(btn_row);

    rebuildTree(current_slug);
}

void HeadingReorderDialog::reload(const QList<NoteItem> &all_headings, const QString &select_slug) {
    headings_ = all_headings;
    rebuildTree(select_slug);
    status_label_->setText(QStringLiteral("✓ স্থানান্তর সম্পন্ন — আবার টেনে সাজাতে পারেন"));
    status_label_->setStyleSheet("color: #27ae60; font-weight: 600;");
}

void HeadingReorderDialog::rebuildTree(const QString &select_slug) {
    tree_->clear();
    const QStringList display_ids = MarkdownUtils::compute_display_ids(headings_);

    QTreeWidgetItem *current_main = nullptr;
    QTreeWidgetItem *to_select = nullptr;

    for (int i = 0; i < headings_.size(); ++i) {
        const NoteItem &it = headings_.at(i);
        const QString did = (i < display_ids.size()) ? display_ids.at(i) : QString();

        if (it.type == QLatin1String("heading")) {
            auto *item = new QTreeWidgetItem(tree_);
            item->setText(0, QStringLiteral("%1  %2").arg(did, it.title));
            item->setData(0, RoleSlug, it.slug);
            item->setData(0, RoleType, QStringLiteral("heading"));
            item->setFlags((item->flags() | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled | Qt::ItemIsSelectable)
                           & ~Qt::ItemIsDragEnabled);
            QFont f = item->font(0);
            f.setBold(true);
            item->setFont(0, f);
            item->setForeground(0, QColor("#c0392b"));
            item->setExpanded(true);
            current_main = item;
            if (it.slug == select_slug)
                to_select = item;
        } else {
            QTreeWidgetItem *parent = current_main ? current_main : tree_->invisibleRootItem();
            auto *item = new QTreeWidgetItem(parent);
            item->setText(0, QStringLiteral("↳ %1  %2").arg(did, it.title));
            item->setData(0, RoleSlug, it.slug);
            item->setData(0, RoleType, QStringLiteral("subheading"));
            item->setFlags(item->flags() | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled
                           | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            item->setForeground(0, QColor("#2980b9"));
            if (it.slug == select_slug)
                to_select = item;
        }
    }

    if (to_select) {
        tree_->setCurrentItem(to_select);
        tree_->scrollToItem(to_select);
    }
}

QTreeWidgetItem *HeadingReorderDialog::findItemBySlug(const QString &slug) const {
    const auto find_rec = [&](auto &&self, QTreeWidgetItem *node) -> QTreeWidgetItem * {
        if (!node)
            return nullptr;
        if (node->data(0, RoleSlug).toString() == slug)
            return node;
        for (int i = 0; i < node->childCount(); ++i) {
            if (auto *f = self(self, node->child(i)))
                return f;
        }
        return nullptr;
    };
    for (int i = 0; i < tree_->topLevelItemCount(); ++i) {
        if (auto *f = find_rec(find_rec, tree_->topLevelItem(i)))
            return f;
    }
    return nullptr;
}
