#include "ClipboardGrabberUI.h"
#include "Utils.h"

namespace {
// Soft elevation so cards lift subtly off the page instead of looking flat.
void applyCardShadow(QWidget *w) {
    auto *shadow = new QGraphicsDropShadowEffect(w);
    shadow->setBlurRadius(16);
    shadow->setXOffset(0);
    shadow->setYOffset(3);
    shadow->setColor(QColor(25, 42, 86, 30));
    w->setGraphicsEffect(shadow);
}

// A section header row: a small feather glyph in a tinted circle + bold title,
// used so each card reads at a glance instead of as a wall of controls.
QWidget *sectionHeader(const QChar &glyphCode, const QColor &accent, const QString &text) {
    QWidget *row = new QWidget();
    QHBoxLayout *l = new QHBoxLayout(row);
    l->setContentsMargins(0, 0, 0, 3);
    l->setSpacing(6);

    QLabel *badge = new QLabel();
    badge->setFixedSize(22, 22);
    badge->setAlignment(Qt::AlignCenter);
    badge->setStyleSheet(QString(
        "background-color: %1; border-radius: 11px;"
    ).arg(accent.lighter(175).name()));
    badge->setPixmap(get_feather_icon(glyphCode, accent.darker(110), 14).pixmap(14, 14));

    QLabel *title = new QLabel(text);
    title->setStyleSheet("font-weight: 700; font-size: 13px; color: #192a56;");

    l->addWidget(badge);
    l->addWidget(title, 1);
    return row;
}
}

void ClipboardGrabberUI::setupUi(QWidget *parent) {
    // --- Window Setup ---
    parent->setWindowTitle("Graber");
    parent->resize(620, 820); // placeholder, corrected right after layout by fit_window_to_content()
    parent->setObjectName("MainWindow");

    // --- UI Widgets ---
    status_label = new QLabel("অবস্থা: বন্ধ");
    status_label->setObjectName("status_label");
    status_label->setAlignment(Qt::AlignCenter);

    last_captured_label = new QLabel("শেষ ক্যাপচার: (কিছুই না)");
    last_captured_label->setObjectName("last_captured_label");
    last_captured_label->setAlignment(Qt::AlignCenter);
    last_captured_label->setWordWrap(true);
    last_captured_label->setMinimumHeight(46);

    start_button = new QPushButton("শুরু (Start)");
    start_button->setStyleSheet("QPushButton { background-color: #44bd32; } QPushButton:hover { background-color: #44bd32; opacity: 0.9; }");
    start_button->setIcon(get_feather_icon(QChar(0xe9a8)));

    stop_button = new QPushButton("থামুন (Stop)");
    stop_button->setEnabled(false);
    stop_button->setStyleSheet("QPushButton { background-color: #e84118; } QPushButton:hover { background-color: #c23616; }");
    stop_button->setIcon(get_feather_icon(QChar(0xe9e4)));

    add_image_button = new QPushButton("ছবি যুক্ত করুন (Add Image)");
    add_image_button->setIcon(get_feather_icon(QChar(0xe978)));

    folder_dropdown = new QComboBox();
    folder_dropdown->setIconSize(QSize(18, 18));
    subject_dropdown = new QComboBox();
    subject_dropdown->setIconSize(QSize(18, 18));

    toggle_subject_button = new QPushButton("বিষয় পরিবর্তন");
    toggle_subject_button->setStyleSheet("QPushButton { background-color: #9b59b6; } QPushButton:hover { background-color: #8e44ad; }");
    toggle_subject_button->setIcon(get_feather_icon(QChar(0xe9d0)));

    add_subject_button = new QPushButton("নতুন বিষয়");
    add_subject_button->setStyleSheet("QPushButton { background-color: #44bd32; } QPushButton:hover { background-color: #44bd32; opacity: 0.9; }");
    add_subject_button->setIcon(get_feather_icon(QChar(0xe9c9)));

    add_folder_button = new QPushButton("নতুন ফোল্ডার");
    add_folder_button->setStyleSheet("QPushButton { background-color: #e67e22; } QPushButton:hover { background-color: #d35400; }");
    add_folder_button->setIcon(get_feather_icon(QChar(0xe9c9)));

    open_file_button = new QPushButton("নোট খুলুন");
    open_file_button->setStyleSheet("QPushButton { background-color: #0097e6; } QPushButton:hover { background-color: #00a8ff; }");
    open_file_button->setEnabled(false);
    open_file_button->setIcon(get_feather_icon(QChar(0xe966)));

    heading_label = new QLabel("শিরোনাম (Heading):");
    select_heading_button = new QPushButton("(শেষে নতুন করে যোগ করুন / Append to End)");
    select_heading_button->setObjectName("select_heading_button");
    select_heading_button->setStyleSheet("QPushButton { background-color: white; color: black; border: 1px solid #dcdde1; text-align: left; padding: 7px; font-weight: normal; border-radius: 4px; } QPushButton:hover { background-color: #f5f6fa; } QPushButton:disabled { background-color: #dcdde1; color: #7f8c8d; }");
    select_heading_button->setEnabled(false);
    select_heading_button->setIcon(get_feather_icon(QChar(0xe90a), QColor("#2f3640")));

    append_to_heading_button = new QPushButton("যুক্ত করুন");
    append_to_heading_button->setStyleSheet("QPushButton { background-color: #f39c12; } QPushButton:hover { background-color: #e67e22; }");
    append_to_heading_button->setEnabled(false);
    append_to_heading_button->setIcon(get_feather_icon(QChar(0xe963)));

    shift_heading_button = new QPushButton("স্থানান্তর");
    shift_heading_button->setStyleSheet("QPushButton { background-color: #3498db; } QPushButton:hover { background-color: #2980b9; }");
    shift_heading_button->setEnabled(false);
    shift_heading_button->setIcon(get_feather_icon(QChar(0xe9bc)));

    delete_heading_button = new QPushButton("মুছে ফেলুন");
    delete_heading_button->setStyleSheet("QPushButton { background-color: #c0392b; } QPushButton:hover { background-color: #ae2012; }");
    delete_heading_button->setEnabled(false);
    delete_heading_button->setIcon(get_feather_icon(QChar(0xe9f6)));

    format_dropdown = new QComboBox();
    format_dropdown->addItem("বুলেট পয়েন্ট (Point)");
    format_dropdown->addItem("প্রধান শিরোনাম (Heading - Red)");
    format_dropdown->addItem("উপ-শিরোনাম (Subheading - Blue)");
    format_dropdown->addItem("মাইন্ড ম্যাপ (Timeline Mind Map)");
    format_dropdown->addItem("প্যারাগ্রাফ (Paragraph)");

    section_label = new QLabel("বিভাগ (Section):");
    section_dropdown = new QComboBox();

    add_section_button = new QPushButton("নতুন বিভাগ");
    add_section_button->setStyleSheet("QPushButton { background-color: #44bd32; } QPushButton:hover { background-color: #44bd32; opacity: 0.9; }");
    add_section_button->setIcon(get_feather_icon(QChar(0xe9c9)));

    inject_heading_button = new QPushButton("ইনজেক্ট করুন");
    inject_heading_button->setStyleSheet("QPushButton { background-color: #8c7ae6; } QPushButton:hover { background-color: #9c88ff; }");
    inject_heading_button->setEnabled(false);
    inject_heading_button->setIcon(get_feather_icon(QChar(0xe992)));

    mode_label = new QLabel("মোড:");
    mode_dropdown = new QComboBox();
    mode_dropdown->addItem("কপি মোড (Ctrl+C)");
    mode_dropdown->addItem("সিলেক্ট মোড");

    wizards_button = new QPushButton("উইজার্ড ও টুলস");
    wizards_button->setStyleSheet("QPushButton { background-color: #8e44ad; } QPushButton:hover { background-color: #9b59b6; }");
    wizards_button->setIcon(get_feather_icon(QChar(0xe9b8)));

    // --- Layout Panels ---
    // 1. Subject Management Panel (Card)
    subject_card = new QFrame();
    subject_card->setObjectName("card");
    QVBoxLayout *subject_layout = new QVBoxLayout(subject_card);
    subject_layout->setContentsMargins(12, 10, 12, 12);
    subject_layout->setSpacing(8);
    subject_layout->addWidget(sectionHeader(QChar(0xe9d0), QColor("#487eb0"), "ফোল্ডার ও বিষয়ের তালিকা (Folder & Subject Navigation)"));
    applyCardShadow(subject_card);

    // Row 1: Folder Selection
    QHBoxLayout *folder_layout = new QHBoxLayout();
    folder_layout->setSpacing(8);
    QLabel *lbl_folder = new QLabel("ফোল্ডার:");
    lbl_folder->setFixedWidth(48);
    folder_layout->addWidget(lbl_folder);
    folder_layout->addWidget(folder_dropdown, 1);
    folder_layout->addWidget(add_folder_button);
    subject_layout->addLayout(folder_layout);

    // Row 2: Subject Selection
    QHBoxLayout *file_layout = new QHBoxLayout();
    file_layout->setSpacing(8);
    QLabel *lbl_subject = new QLabel("বিষয়:");
    lbl_subject->setFixedWidth(48);
    file_layout->addWidget(lbl_subject);
    file_layout->addWidget(subject_dropdown, 1);
    file_layout->addWidget(toggle_subject_button);
    file_layout->addWidget(add_subject_button);
    file_layout->addWidget(open_file_button);
    subject_layout->addLayout(file_layout);

    // 2. Capture Configuration Panel (Card)
    capture_card = new QFrame();
    capture_card->setObjectName("card");
    QVBoxLayout *capture_layout = new QVBoxLayout(capture_card);
    capture_layout->setContentsMargins(12, 10, 12, 12);
    capture_layout->setSpacing(8);
    capture_layout->addWidget(sectionHeader(QChar(0xe9a8), QColor("#8c7ae6"), "ক্যাপচার ও ইনপুট কনফিগারেশন (Capture Configuration)"));
    applyCardShadow(capture_card);

    // Format + mode: this row stays visible even in the minimal "capturing"
    // view, since it's the one thing still useful to change mid-capture.
    QHBoxLayout *options_layout = new QHBoxLayout();
    options_layout->setSpacing(8);
    options_layout->addWidget(new QLabel("ফরম্যাট:"));
    options_layout->addWidget(format_dropdown, 1);
    options_layout->addWidget(mode_label);
    options_layout->addWidget(mode_dropdown, 1);
    capture_layout->addLayout(options_layout);

    // Section + image controls: bundled into one widget so it can be hidden
    // as a single unit while monitoring is running.
    capture_extra = new QWidget();
    QVBoxLayout *capture_extra_layout = new QVBoxLayout(capture_extra);
    capture_extra_layout->setContentsMargins(0, 0, 0, 0);
    capture_extra_layout->setSpacing(10);

    QHBoxLayout *section_layout = new QHBoxLayout();
    section_layout->setSpacing(8);
    section_layout->addWidget(section_label);
    section_layout->addWidget(section_dropdown, 1);
    section_layout->addWidget(add_section_button);
    capture_extra_layout->addLayout(section_layout);

    QHBoxLayout *image_layout = new QHBoxLayout();
    image_layout->addWidget(add_image_button, 1);
    capture_extra_layout->addLayout(image_layout);

    capture_layout->addWidget(capture_extra);

    // 3. Heading Management Panel (Card)
    heading_card = new QFrame();
    heading_card->setObjectName("card");
    QVBoxLayout *heading_card_layout = new QVBoxLayout(heading_card);
    heading_card_layout->setContentsMargins(12, 10, 12, 12);
    heading_card_layout->setSpacing(8);
    heading_card_layout->addWidget(sectionHeader(QChar(0xe90a), QColor("#e67e22"), "টার্গেট শিরোনাম নিয়ন্ত্রণ (Target Heading Control)"));
    applyCardShadow(heading_card);

    QHBoxLayout *heading_layout = new QHBoxLayout();
    heading_layout->setSpacing(8);
    heading_layout->addWidget(heading_label);
    heading_layout->addWidget(select_heading_button, 1);
    heading_card_layout->addLayout(heading_layout);

    QHBoxLayout *heading_actions_layout = new QHBoxLayout();
    heading_actions_layout->setSpacing(8);
    heading_actions_layout->addWidget(append_to_heading_button);
    heading_actions_layout->addWidget(inject_heading_button);
    heading_actions_layout->addWidget(shift_heading_button);
    heading_actions_layout->addWidget(delete_heading_button);
    heading_card_layout->addLayout(heading_actions_layout);

    // "Hero" card: live status + last captured preview — this is the "log"
    // that stays visible in both the full and minimal views.
    QFrame *hero_card = new QFrame();
    hero_card->setObjectName("heroCard");
    QVBoxLayout *hero_layout = new QVBoxLayout(hero_card);
    hero_layout->setContentsMargins(12, 12, 12, 12);
    hero_layout->setSpacing(8);
    hero_layout->addWidget(status_label);
    hero_layout->addWidget(last_captured_label);
    applyCardShadow(hero_card);

    // Scrollable body: guarantees cards never overlap or get clipped no
    // matter the window size — content scrolls instead of squeezing.
    QWidget *body = new QWidget();
    body->setObjectName("scrollBody");
    this->body = body;
    QVBoxLayout *body_layout = new QVBoxLayout(body);
    body_layout->setContentsMargins(10, 8, 10, 8);
    body_layout->setSpacing(10);

    body_layout->addWidget(hero_card);
    body_layout->addWidget(subject_card);
    body_layout->addWidget(capture_card);
    body_layout->addWidget(heading_card);
    body_layout->addStretch(1);

    QScrollArea *scroll_area = new QScrollArea();
    scroll_area->setObjectName("bodyScroll");
    scroll_area->setWidget(body);
    scroll_area->setWidgetResizable(true);
    scroll_area->setFrameShape(QFrame::NoFrame);
    scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll_area->setStyleSheet("QScrollArea#bodyScroll { background: transparent; border: none; } QWidget#scrollBody { background: transparent; }");

    // Primary controls: kept outside the scroll area so Start/Stop/Wizards/
    // Settings are always reachable regardless of scroll position or view.
    settings_button = new QPushButton("সেটিংস");
    settings_button->setObjectName("secondaryButton");
    settings_button->setIcon(get_feather_icon(QChar(0xe9db)));

    start_button->setObjectName("primaryActionButton");
    stop_button->setObjectName("primaryActionButton");

    QWidget *controls_bar = new QWidget();
    controls_bar->setObjectName("controlsBar");
    this->controls_bar = controls_bar;
    QHBoxLayout *control_layout1 = new QHBoxLayout(controls_bar);
    control_layout1->setContentsMargins(10, 8, 10, 8);
    control_layout1->setSpacing(6);
    control_layout1->addWidget(start_button, 1);
    control_layout1->addWidget(stop_button, 1);
    control_layout1->addWidget(wizards_button);
    control_layout1->addWidget(settings_button);

    // Main Layout
    QVBoxLayout *main_layout = new QVBoxLayout(parent);
    main_layout->setSpacing(0);
    main_layout->setContentsMargins(0, 0, 0, 0);

    main_layout->addWidget(scroll_area, 1);
    main_layout->addWidget(controls_bar);
}
