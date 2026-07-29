#include "ClipboardGrabberUI.h"
#include "Utils.h"

void ClipboardGrabberUI::setupUi(QWidget *parent) {
    // --- Window Setup ---
    parent->setWindowTitle("ক্লিপবোর্ড গ্র্যাবার");
    parent->setMinimumSize(500, 620);
    parent->resize(540, 660);
    parent->setObjectName("MainWindow");

    // --- UI Widgets ---
    status_label = new QLabel("অবস্থা: বন্ধ");
    status_label->setObjectName("status_label");
    status_label->setAlignment(Qt::AlignCenter);

    last_captured_label = new QLabel("শেষ ক্যাপচার: (কিছুই না)");
    last_captured_label->setObjectName("last_captured_label");
    last_captured_label->setAlignment(Qt::AlignCenter);
    last_captured_label->setWordWrap(true);
    last_captured_label->setMinimumHeight(60);

    start_button = new QPushButton("শুরু (Start)");
    start_button->setStyleSheet("QPushButton { background-color: #44bd32; } QPushButton:hover { background-color: #44bd32; opacity: 0.9; }");
    start_button->setIcon(get_feather_icon(QChar(0xe9a8)));

    stop_button = new QPushButton("থামুন (Stop)");
    stop_button->setEnabled(false);
    stop_button->setStyleSheet("QPushButton { background-color: #e84118; } QPushButton:hover { background-color: #c23616; }");
    stop_button->setIcon(get_feather_icon(QChar(0xe9e4)));

    add_image_button = new QPushButton("ছবি যুক্ত করুন (Add Image)");
    add_image_button->setIcon(get_feather_icon(QChar(0xe978)));

    subject_dropdown = new QComboBox();
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
    select_heading_button->setStyleSheet("QPushButton { background-color: white; color: black; border: 1px solid #dcdde1; text-align: left; padding: 10px; font-weight: normal; border-radius: 4px; } QPushButton:hover { background-color: #f5f6fa; } QPushButton:disabled { background-color: #dcdde1; color: #7f8c8d; }");
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

    // --- Layout Panels ---
    // 1. Subject Management Panel (Card)
    QFrame *subject_card = new QFrame();
    subject_card->setObjectName("card");
    QVBoxLayout *subject_layout = new QVBoxLayout(subject_card);
    subject_layout->setContentsMargins(12, 12, 12, 12);
    subject_layout->setSpacing(8);
    QLabel *subject_title = new QLabel("নোট বিষয় নির্বাচন (Subject Selection):");
    subject_title->setStyleSheet("font-weight: bold; color: #192a56;");
    subject_layout->addWidget(subject_title);
    
    QHBoxLayout *file_layout = new QHBoxLayout();
    file_layout->addWidget(subject_dropdown, 1);
    file_layout->addWidget(toggle_subject_button);
    file_layout->addWidget(add_subject_button);
    file_layout->addWidget(add_folder_button);
    file_layout->addWidget(open_file_button);
    subject_layout->addLayout(file_layout);

    // 2. Capture Configuration Panel (Card)
    QFrame *capture_card = new QFrame();
    capture_card->setObjectName("card");
    QVBoxLayout *capture_layout = new QVBoxLayout(capture_card);
    capture_layout->setContentsMargins(12, 12, 12, 12);
    capture_layout->setSpacing(8);
    QLabel *capture_title = new QLabel("ক্যাপচার ও ইনপুট কনফিগারেশন (Capture Configuration):");
    capture_title->setStyleSheet("font-weight: bold; color: #192a56;");
    capture_layout->addWidget(capture_title);

    QHBoxLayout *options_layout = new QHBoxLayout();
    options_layout->addWidget(new QLabel("ফরম্যাট:"));
    options_layout->addWidget(format_dropdown, 1);
    options_layout->addWidget(mode_label);
    options_layout->addWidget(mode_dropdown, 1);
    capture_layout->addLayout(options_layout);

    QHBoxLayout *section_layout = new QHBoxLayout();
    section_layout->addWidget(section_label);
    section_layout->addWidget(section_dropdown, 1);
    section_layout->addWidget(add_section_button);
    capture_layout->addLayout(section_layout);

    QHBoxLayout *image_layout = new QHBoxLayout();
    image_layout->addWidget(add_image_button, 1);
    capture_layout->addLayout(image_layout);

    // 3. Heading Management Panel (Card)
    QFrame *heading_card = new QFrame();
    heading_card->setObjectName("card");
    QVBoxLayout *heading_card_layout = new QVBoxLayout(heading_card);
    heading_card_layout->setContentsMargins(12, 12, 12, 12);
    heading_card_layout->setSpacing(8);
    QLabel *heading_title = new QLabel("টার্গেট শিরোনাম নিয়ন্ত্রণ (Target Heading Control):");
    heading_title->setStyleSheet("font-weight: bold; color: #192a56;");
    heading_card_layout->addWidget(heading_title);

    QHBoxLayout *heading_layout = new QHBoxLayout();
    heading_layout->addWidget(heading_label);
    heading_layout->addWidget(select_heading_button, 1);
    heading_card_layout->addLayout(heading_layout);

    QHBoxLayout *heading_actions_layout = new QHBoxLayout();
    heading_actions_layout->addWidget(append_to_heading_button);
    heading_actions_layout->addWidget(inject_heading_button);
    heading_actions_layout->addWidget(shift_heading_button);
    heading_actions_layout->addWidget(delete_heading_button);
    heading_card_layout->addLayout(heading_actions_layout);

    // Main Layout
    QVBoxLayout *main_layout = new QVBoxLayout(parent);
    main_layout->setSpacing(10);
    main_layout->setContentsMargins(12, 12, 12, 12);

    main_layout->addWidget(status_label);
    main_layout->addWidget(last_captured_label);
    main_layout->addWidget(subject_card);
    main_layout->addWidget(capture_card);
    main_layout->addWidget(heading_card);

    settings_button = new QPushButton("সেটিংস");
    settings_button->setStyleSheet("QPushButton { background-color: #718093; } QPushButton:hover { background-color: #636e72; }");
    settings_button->setIcon(get_feather_icon(QChar(0xe9db)));

    QHBoxLayout *control_layout1 = new QHBoxLayout();
    control_layout1->addWidget(start_button);
    control_layout1->addWidget(stop_button);
    control_layout1->addWidget(settings_button);
    main_layout->addLayout(control_layout1);
}
