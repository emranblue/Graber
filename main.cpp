#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QClipboard>
#include <QGuiApplication>
#include <QImage>
#include <QMimeData>
#include <QComboBox>
#include <QInputDialog>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFontDatabase>
#include <string>
#include <fstream>
#include <iostream>
#include <QDesktopServices>
#include <QUrl>
#include <QCloseEvent>
#include <QRegularExpression>
#include <QTextStream>
#include <cctype>

class ClipboardGrabber : public QWidget {
    Q_OBJECT

public:
    ClipboardGrabber(QWidget *parent = nullptr) : QWidget(parent) {
        // --- Setup ---
        setWindowTitle("ক্লিপবোর্ড গ্র্যাবার");
        setMinimumSize(480, 360);

        // --- State Variables ---
        is_running_ = false;
        is_box_open_ = false;
        last_simplified_text_ = "";
        last_date_ = "";
        
        // --- Scalability Fix: Use a dedicated notes directory ---
        notes_dir_path_ = QDir::homePath() + QDir::separator() + "GraberNotes";
        QDir dir(notes_dir_path_);
        if (!dir.exists()) {
            dir.mkpath(".");
        }

        // --- UI Styling ---
        this->setObjectName("MainWindow");

        // --- UI Widgets ---
        status_label_ = new QLabel("অবস্থা: বন্ধ");
        status_label_->setObjectName("status_label");
        status_label_->setAlignment(Qt::AlignCenter);

        last_captured_label_ = new QLabel("শেষ ক্যাপচার: (কিছুই না)");
        last_captured_label_->setObjectName("last_captured_label");
        last_captured_label_->setAlignment(Qt::AlignCenter);
        last_captured_label_->setWordWrap(true);
        last_captured_label_->setMinimumHeight(80);

        start_button_ = new QPushButton("শুরু (Start)");
        stop_button_ = new QPushButton("থামুন (Stop)");
        stop_button_->setEnabled(false);
        stop_button_->setStyleSheet("QPushButton { background-color: #e84118; } QPushButton:hover { background-color: #c23616; }");

        add_image_button_ = new QPushButton("ছবি যুক্ত করুন (Add Image)");
          subject_dropdown_ = new QComboBox();
        add_subject_button_ = new QPushButton("নতুন বিষয় (New Subject)");
        add_subject_button_->setStyleSheet("QPushButton { background-color: #44bd32; } QPushButton:hover { background-color: #44bd32; opacity: 0.9; }");

        open_file_button_ = new QPushButton("নোট খুলুন (Open Note)");
        open_file_button_->setStyleSheet("QPushButton { background-color: #0097e6; } QPushButton:hover { background-color: #00a8ff; }");
        open_file_button_->setEnabled(false);

        heading_label_ = new QLabel("শিরোনাম (Heading):");
        heading_dropdown_ = new QComboBox();
        heading_dropdown_->setEnabled(false);

        append_to_heading_button_ = new QPushButton("যুক্ত করুন (Append)");
        append_to_heading_button_->setStyleSheet("QPushButton { background-color: #f39c12; } QPushButton:hover { background-color: #e67e22; }");
        append_to_heading_button_->setEnabled(false);

        delete_heading_button_ = new QPushButton("মুছে ফেলুন (Delete)");
        delete_heading_button_->setStyleSheet("QPushButton { background-color: #c0392b; } QPushButton:hover { background-color: #ae2012; }");
        delete_heading_button_->setEnabled(false);

        format_dropdown_ = new QComboBox();
        format_dropdown_->addItem("বুলেট পয়েন্ট (Point)");
        format_dropdown_->addItem("প্রধান শিরোনাম (Heading - Red)");
        format_dropdown_->addItem("উপ-শিরোনাম (Subheading - Blue)");

        section_label_ = new QLabel("বিভাগ (Section):");
        section_dropdown_ = new QComboBox();
        section_dropdown_->addItem("পরিবেশ (Environment)", "environment");
        section_dropdown_->addItem("জ্বালানি (Energy)", "energy");
        section_dropdown_->addItem("অর্থনীতি (Economy)", "economy");
        section_dropdown_->addItem("সংস্কৃতি (Culture)", "culture");
        section_dropdown_->addItem("ভূগোল (Geography)", "geography");
        section_dropdown_->addItem("জনসংখ্যা (Population)", "population");
        section_dropdown_->addItem("আইন ও সংবিধান (Law-Constitution)", "law-constitution");
        section_dropdown_->addItem("রাজনীতি (Politics)", "politics");
        section_dropdown_->addItem("মুক্তিযুদ্ধ (Freedom-Fight)", "freedom-fight");
        section_dropdown_->addItem("অন্যান্য (Others)", "others");

        inject_heading_button_ = new QPushButton("শিরোনাম ইনজেক্ট করুন (Inject Heading)");
        inject_heading_button_->setStyleSheet("QPushButton { background-color: #8c7ae6; } QPushButton:hover { background-color: #9c88ff; }");
        inject_heading_button_->setEnabled(false);

        mode_label_ = new QLabel("মোড:");
        mode_dropdown_ = new QComboBox();
        mode_dropdown_->addItem("কপি মোড (Ctrl+C)");
        mode_dropdown_->addItem("সিলেক্ট মোড");


        // --- Layout ---
        QVBoxLayout *main_layout = new QVBoxLayout(this);
        main_layout->setSpacing(15);
        main_layout->setContentsMargins(20, 20, 20, 20);

        QHBoxLayout *control_layout = new QHBoxLayout();
        QHBoxLayout *file_layout = new QHBoxLayout();
        QHBoxLayout *heading_layout = new QHBoxLayout();
        QHBoxLayout *header_layout = new QHBoxLayout();
        QHBoxLayout *section_layout = new QHBoxLayout();
        QHBoxLayout *mode_layout = new QHBoxLayout();

        control_layout->addWidget(start_button_);
        control_layout->addWidget(stop_button_);
        control_layout->addWidget(add_image_button_);

        file_layout->addWidget(new QLabel("বিষয়:"));
        file_layout->addWidget(subject_dropdown_, 1);
        file_layout->addWidget(add_subject_button_);
        file_layout->addWidget(open_file_button_);

        heading_layout->addWidget(heading_label_);
        heading_layout->addWidget(heading_dropdown_, 1);
        heading_layout->addWidget(append_to_heading_button_);
        heading_layout->addWidget(delete_heading_button_);

        header_layout->addWidget(new QLabel("ফরম্যাট:"));
        header_layout->addWidget(format_dropdown_, 1);

        section_layout->addWidget(section_label_);
        section_layout->addWidget(section_dropdown_, 1);
        section_layout->addWidget(inject_heading_button_);

        mode_layout->addWidget(mode_label_);
        mode_layout->addWidget(mode_dropdown_, 1);

        main_layout->addWidget(status_label_);
        main_layout->addWidget(last_captured_label_);
        main_layout->addLayout(file_layout);
        main_layout->addLayout(heading_layout);
        main_layout->addLayout(header_layout);
        main_layout->addLayout(section_layout);
        main_layout->addLayout(mode_layout);
        main_layout->addLayout(control_layout);

        // --- Clipboard Timer ---
        clipboard_timer_ = new QTimer(this);
        clipboard_timer_->setInterval(1000); // Check every 1 second

        // --- Connections (Signals and Slots) ---
        connect(start_button_, &QPushButton::clicked, this, &ClipboardGrabber::start_monitoring);
        connect(stop_button_, &QPushButton::clicked, this, &ClipboardGrabber::stop_monitoring);
        connect(add_image_button_, &QPushButton::clicked, this, &ClipboardGrabber::add_clipboard_image);
        connect(add_subject_button_, &QPushButton::clicked, this, &ClipboardGrabber::add_subject);
        connect(open_file_button_, &QPushButton::clicked, this, &ClipboardGrabber::open_selected_file);
        connect(subject_dropdown_, &QComboBox::currentTextChanged, this, &ClipboardGrabber::on_subject_changed);
        connect(heading_dropdown_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ClipboardGrabber::update_button_states);
        connect(append_to_heading_button_, &QPushButton::clicked, this, &ClipboardGrabber::manual_append_to_heading);
        connect(delete_heading_button_, &QPushButton::clicked, this, &ClipboardGrabber::delete_selected_heading_section);
        connect(clipboard_timer_, &QTimer::timeout, this, &ClipboardGrabber::check_clipboard);
        connect(inject_heading_button_, &QPushButton::clicked, this, &ClipboardGrabber::inject_heading_from_clipboard);
        
        // --- Initial Population ---
        populate_subjects_from_disk();
        subject_dropdown_->setCurrentIndex(-1); // No initial selection
        update_status_label();
    }

protected:
    void closeEvent(QCloseEvent *event) override {
        close_active_box();
        QWidget::closeEvent(event);
    }

private slots:
    void start_monitoring() {
        if (subject_dropdown_->currentIndex() == -1) {
            status_label_->setText("অবস্থা: অনুগ্রহ করে প্রথমে একটি বিষয় নির্বাচন করুন!");
            return;
        }
        is_running_ = true;
        
        restore_state_from_file(get_current_target_file());
        update_toc_in_file(get_current_target_file());
        
        QClipboard::Mode mode = (mode_dropdown_->currentIndex() == 0) ? QClipboard::Clipboard : QClipboard::Selection;
        last_simplified_text_ = QGuiApplication::clipboard()->text(mode).simplified();
        
        clipboard_timer_->start();
        start_button_->setEnabled(false);
        stop_button_->setEnabled(true);
        subject_dropdown_->setEnabled(false);
        add_subject_button_->setEnabled(false);
        mode_dropdown_->setEnabled(false);
        update_status_label();
    }

    void stop_monitoring() {
        is_running_ = false;
        clipboard_timer_->stop();
        close_active_box();
        update_toc_in_file(get_current_target_file());
        start_button_->setEnabled(true);
        stop_button_->setEnabled(false);
        subject_dropdown_->setEnabled(true);
        add_subject_button_->setEnabled(true);
        mode_dropdown_->setEnabled(true);
        update_status_label();
    }

    void add_subject() {
        bool ok;
        QString text = QInputDialog::getText(this, "বিষয় যোগ করুন",
                                             "নতুন বিষয়ের নাম:", QLineEdit::Normal,
                                             "", &ok);
        if (ok && !text.isEmpty()) {
            // Avoid adding duplicates
            if (subject_dropdown_->findText(text) == -1) {
                subject_dropdown_->addItem(text);
                
                // --- Scalability Fix: Create file immediately ---
                QString filename = notes_dir_path_ + QDir::separator() + text + ".md";
                if (!QFile::exists(filename)) {
                    QFile file(filename);
                    if (file.open(QIODevice::WriteOnly)) {
                        file.close();
                    }
                }
            }
            subject_dropdown_->setCurrentText(text);
        }
    }

    void check_clipboard() {
        QClipboard *clipboard = QGuiApplication::clipboard();
        QClipboard::Mode mode = (mode_dropdown_->currentIndex() == 0) ? QClipboard::Clipboard : QClipboard::Selection;
        QString current_text = clipboard->text(mode);
        QString simplified_text = current_text.simplified();

        if (!simplified_text.isEmpty() && simplified_text != last_simplified_text_) {
            last_simplified_text_ = simplified_text;
            last_captured_label_->setText("শেষ ক্যাপচার: " + simplified_text);
            QString current_section = section_dropdown_->currentData().toString();
            write_to_file(simplified_text, current_section);
        }
    }

    void on_subject_changed(const QString &text) {
        close_active_box();
        update_status_label();
        open_file_button_->setEnabled(!text.isEmpty());
        inject_heading_button_->setEnabled(!text.isEmpty());
        if (!text.isEmpty()) {
            QString target_file = get_current_target_file();
            normalize_markdown_file(target_file);
            update_toc_in_file(target_file);
            restore_state_from_file(target_file);
            populate_headings_from_file();
        } else {
            heading_dropdown_->clear();
            heading_dropdown_->setEnabled(false);
            delete_heading_button_->setEnabled(false);
            append_to_heading_button_->setEnabled(false);
        }
    }

    void inject_heading_from_clipboard() {
        if (subject_dropdown_->currentIndex() == -1) {
            status_label_->setText("অবস্থা: অনুগ্রহ করে প্রথমে একটি বিষয় নির্বাচন করুন!");
            return;
        }

        QClipboard *clipboard = QGuiApplication::clipboard();
        QClipboard::Mode mode = (mode_dropdown_->currentIndex() == 0) ? QClipboard::Clipboard : QClipboard::Selection;
        QString current_text = clipboard->text(mode);
        QString simplified_text = current_text.simplified();

        if (simplified_text.isEmpty()) {
            last_captured_label_->setText("শেষ ক্যাপচার: ক্লিপবোর্ডে কোনো লেখা পাওয়া যায়নি।");
            return;
        }

        last_simplified_text_ = simplified_text;
        last_captured_label_->setText("শেষ ক্যাপচার (শিরোনাম হিসেবে ইনজেক্ট করা হয়েছে): " + simplified_text);
        
        QString target_file = get_current_target_file();
        if (target_file == "নির্বাচিত নয়") return;

        restore_state_from_file(target_file);

        std::ofstream outfile;
        outfile.open(target_file.toStdString(), std::ios_base::app);
        
        if (outfile.is_open()) {
            QDateTime now = QDateTime::currentDateTime();
            QString current_date = now.toString("dd MMMM, yyyy");
            
            if (current_date != last_date_) {
                if (is_box_open_) {
                    outfile << "\n</div>\n";
                    is_box_open_ = false;
                }
                outfile << "\n### ***" << current_date.toStdString() << "***\n";
                last_date_ = current_date;
            }
            
            if (is_box_open_) {
                outfile << "\n</div>\n";
            }
            
            QString title = simplified_text.trimmed();
            std::string slug = generate_slug(title);
            QString section = section_dropdown_->currentData().toString();
            
            outfile << "\n<h2 id=\"" << slug << "\" data-section=\"" << section.toStdString() << "\" style=\"color: #e74c3c; font-weight: bold; font-style: italic; margin-bottom: 5px;\">" 
                    << title.toStdString() << "</h2>\n";
            outfile << "<div style=\"border: 2px solid #e2e8f0; border-radius: 8px; padding: 15px; margin: 10px 0; background-color: #f8fafc;\">\n";
            is_box_open_ = true;
            
            outfile.close();
            update_toc_in_file(target_file);
        } else {
            std::cerr << "Error: Could not open file " << target_file.toStdString() << std::endl;
            last_captured_label_->setText("ত্রুটি: ফাইলে লেখা যায়নি!");
        }
    }

    void open_selected_file() {
        QString target_file = get_current_target_file();
        if (target_file == "নির্বাচিত নয়" || !QFile::exists(target_file)) {
            status_label_->setText("অবস্থা: ফাইলটি খুঁজে পাওয়া যায়নি!");
            return;
        }

        // Close the active box before opening the file
        close_active_box();

        QDesktopServices::openUrl(QUrl::fromLocalFile(target_file));
    }

    void add_clipboard_image() {
        if (subject_dropdown_->currentIndex() == -1) {
            status_label_->setText("অবস্থা: অনুগ্রহ করে প্রথমে একটি বিষয় নির্বাচন করুন!");
            return;
        }

        QClipboard *clipboard = QGuiApplication::clipboard();
        QImage image = clipboard->image();

        if (image.isNull()) {
            last_captured_label_->setText("শেষ ক্যাপচার: ক্লিপবোর্ডে কোনো ছবি পাওয়া যায়নি।");
            return;
        }

        // Ensure images directory exists
        QString images_dir_path = notes_dir_path_ + QDir::separator() + "images";
        QDir img_dir(images_dir_path);
        if (!img_dir.exists()) {
            img_dir.mkpath(".");
        }

        QDateTime now = QDateTime::currentDateTime();
        QString timestamp = now.toString("yyyyMMdd_hhmmss_zzz");
        QString filename = QString("img_%1.png").arg(timestamp);
        QString filepath = images_dir_path + QDir::separator() + filename;

        if (image.save(filepath, "PNG")) {
            write_image_to_file(filename);
            last_captured_label_->setText("শেষ ক্যাপচার: ছবি যোগ করা হয়েছে (" + filename + ")");
        } else {
            last_captured_label_->setText("ত্রুটি: ছবি সংরক্ষণ করা যায়নি!");
        }
    }

    void update_button_states() {
        bool has_subject = (subject_dropdown_->currentIndex() != -1);
        bool has_selected_heading = (heading_dropdown_->currentIndex() > 0);
        
        delete_heading_button_->setEnabled(has_subject && has_selected_heading);
        append_to_heading_button_->setEnabled(has_subject && has_selected_heading);
    }

    void manual_append_to_heading() {
        if (subject_dropdown_->currentIndex() == -1) return;
        if (heading_dropdown_->currentIndex() <= 0) return;
        
        QClipboard *clipboard = QGuiApplication::clipboard();
        QClipboard::Mode mode = (mode_dropdown_->currentIndex() == 0) ? QClipboard::Clipboard : QClipboard::Selection;
        QString current_text = clipboard->text(mode);
        QString simplified_text = current_text.simplified();

        if (simplified_text.isEmpty()) {
            last_captured_label_->setText("শেষ ক্যাপচার: ক্লিপবোর্ডে কোনো লেখা পাওয়া যায়নি।");
            return;
        }

        QString target_file = get_current_target_file();
        QString slug = heading_dropdown_->currentData().toString();
        
        if (append_content_to_heading(target_file, slug, simplified_text)) {
            update_toc_in_file(target_file);
            last_captured_label_->setText("শেষ ক্যাপচার (নির্বাচিত শিরোনামে ম্যানুয়ালি যুক্ত করা হয়েছে): " + simplified_text);
        } else {
            last_captured_label_->setText("ত্রুটি: নির্বাচিত শিরোনামে যুক্ত করা যায়নি!");
        }
    }

    void delete_selected_heading_section() {
        if (subject_dropdown_->currentIndex() == -1) return;
        int heading_idx = heading_dropdown_->currentIndex();
        if (heading_idx <= 0) return; // index 0 is "(শেষে নতুন করে যোগ করুন)"
        
        QString slug = heading_dropdown_->currentData().toString();
        QString target_file = get_current_target_file();
        
        QFile file(target_file);
        if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
            last_captured_label_->setText("ত্রুটি: ফাইলটি খোলা যায়নি!");
            return;
        }
        
        QTextStream in(&file);
        QString content = in.readAll();
        file.close();
        
        int start_pos = -1;
        int end_pos = -1;
        bool is_html = false;
        bool found = false;
        
        if (get_heading_bounds(content, slug, start_pos, end_pos, is_html)) {
            found = true;
        } else if (get_subheading_bounds(content, slug, start_pos, end_pos)) {
            found = true;
        }
        
        if (found) {
            QString deleted_chunk = content.mid(start_pos, end_pos - start_pos);
            
            // Remove the chunk from content
            content.remove(start_pos, end_pos - start_pos);
            
            // Write the remaining content back to file
            if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
                QTextStream out(&file);
                out << content;
                file.close();
            }
            
            // Save the deleted chunk to another folder in a txt file
            QString del_filename = QString("%1_%2_%3.txt")
                                   .arg(subject_dropdown_->currentText())
                                   .arg(slug)
                                   .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
            
            // Ensure deleted directory exists
            QString deleted_dir_path = notes_dir_path_ + QDir::separator() + "deleted";
            QDir del_dir(deleted_dir_path);
            if (!del_dir.exists()) {
                del_dir.mkpath(".");
            }
            
            QString del_filepath = deleted_dir_path + QDir::separator() + del_filename;
            QFile del_file(del_filepath);
            if (del_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream del_out(&del_file);
                del_out << deleted_chunk;
                del_file.close();
                last_captured_label_->setText(QString("মুছে ফেলা হয়েছে এবং ব্যাকআপ রাখা হয়েছে: %1").arg(del_filename));
            } else {
                last_captured_label_->setText("মুছে ফেলা হয়েছে কিন্তু ব্যাকআপ রাখা যায়নি!");
            }
            
            // Refresh headings dropdown & TOC
            update_toc_in_file(target_file);
            populate_headings_from_file();
        } else {
            last_captured_label_->setText("ত্রুটি: শিরোনাম বা উপ-শিরোনামটি খুঁজে পাওয়া যায়নি!");
        }
    }

private:
    void populate_subjects_from_disk() {
        // Populate dropdown from all .md files in the directory
        QDir directory(notes_dir_path_);
        QStringList md_files = directory.entryList(QStringList() << "*.md", QDir::Files);
        
        subject_dropdown_->clear();
        for (QString filename : md_files) {
            QString full_path = notes_dir_path_ + QDir::separator() + filename;
            normalize_markdown_file(full_path);
            update_toc_in_file(full_path);
            
            filename.chop(3); // Remove ".md"
            subject_dropdown_->addItem(filename);
        }
    }

    QString detect_section_from_title(const QString &title) {
        QString lower_title = title.toLower().trimmed();
        
        struct MapEntry {
            QString keyword;
            QString section;
        };
        QList<MapEntry> mappings = {
            {"environment", "environment"}, {"পরিবেশ", "environment"},
            {"energy", "energy"}, {"জ্বালানি", "energy"},
            {"economy", "economy"}, {"অর্থনীতি", "economy"},
            {"culture", "culture"}, {"সংস্কৃতি", "culture"},
            {"geography", "geography"}, {"ভূগোল", "geography"},
            {"population", "population"}, {"জনসংখ্যা", "population"},
            {"law-constitution", "law-constitution"}, {"আইন ও সংবিধান", "law-constitution"},
            {"politics", "politics"}, {"রাজনীতি", "politics"},
            {"freedom-fight", "freedom-fight"}, {"মুক্তিযুদ্ধ", "freedom-fight"}
        };
        
        for (const auto &entry : mappings) {
            if (lower_title.contains(entry.keyword)) {
                return entry.section;
            }
        }
        return "others";
    }

    void normalize_markdown_file(const QString &file_path) {
        QFile file(file_path);
        if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
            return;
        }

        QTextStream in(&file);
        QString content = in.readAll();
        file.close();

        // 1. Remove existing TOC block if present
        int toc_start = content.indexOf("<!-- TOC_START -->");
        int toc_end = content.indexOf("<!-- TOC_END -->");
        QString clean_content = content;
        if (toc_start != -1 && toc_end != -1) {
            QString pre_toc = content.left(toc_start);
            QString post_toc = content.mid(toc_end + QString("<!-- TOC_END -->").length());
            clean_content = pre_toc + post_toc;
        }

        QStringList lines = clean_content.split('\n');
        QStringList output_lines;
        bool inside_div = false;

        QRegularExpression date_regex("^###\\s*(?:\\*\\*\\*)?\\s*([0-9০-৯]{1,2}\\s+(?:January|February|March|April|May|June|July|August|September|October|November|December|জানুয়ারি|ফেব্রুয়ারি|মার্চ|এপ্রিল|মে|জুন|জুলাই|আগস্ট|সেপ্টেম্বর|অক্টোবর|নভেম্বর|ডিসেম্বর)[,\\s]+[0-9০-৯]{4})\\s*(?:\\*\\*\\*)?$", QRegularExpression::CaseInsensitiveOption);
        QRegularExpression h2_regex("<h2([^>]*)>(.*?)</h2>", QRegularExpression::CaseInsensitiveOption);
        QRegularExpression md_regex("^(#{1,3})\\s+(?!\\*\\*\\*)(.*?)$");
        QRegularExpression md_section_regex("<!--\\s*section:([\\w-]+)\\s*-->");
        QRegularExpression section_attr_regex("data-section=\"([^\"]*)\"", QRegularExpression::CaseInsensitiveOption);
        QRegularExpression style_regex("style=\"([^\"]*)\"", QRegularExpression::CaseInsensitiveOption);

        for (int i = 0; i < lines.size(); ++i) {
            QString line = lines[i];
            QString trimmed_line = line.trimmed();

            QRegularExpressionMatch date_match = date_regex.match(trimmed_line);
            QRegularExpressionMatch h2_match = h2_regex.match(trimmed_line);
            QRegularExpressionMatch md_match = md_regex.match(trimmed_line);

            if (date_match.hasMatch()) {
                if (inside_div) {
                    output_lines.append("</div>");
                    inside_div = false;
                }
                output_lines.append(line);
            } else if (h2_match.hasMatch()) {
                if (inside_div) {
                    output_lines.append("</div>");
                    inside_div = false;
                }
                
                QString attributes = h2_match.captured(1);
                QString title = h2_match.captured(2).trimmed();
                QString slug = QString::fromStdString(generate_slug(title));

                QString style = "color: #e74c3c; font-weight: bold; font-style: italic; margin-bottom: 5px;"; // default
                QRegularExpressionMatch style_match = style_regex.match(attributes);
                if (style_match.hasMatch()) {
                    style = style_match.captured(1);
                }

                QString section = detect_section_from_title(title);
                QRegularExpressionMatch section_match = section_attr_regex.match(attributes);
                if (section_match.hasMatch()) {
                    section = section_match.captured(1);
                }

                QString rewritten_line = QString("<h2 id=\"%1\" data-section=\"%2\" style=\"%3\">%4</h2>")
                                         .arg(slug, section, style, title);
                output_lines.append(rewritten_line);
            } else if (md_match.hasMatch()) {
                int level = md_match.captured(1).length();
                if (level == 3 && trimmed_line.contains("***")) {
                    output_lines.append(line);
                    continue;
                }

                QString rest = md_match.captured(2).trimmed();
                QString section = detect_section_from_title(rest);
                QRegularExpressionMatch section_match = md_section_regex.match(rest);
                QString title = rest;
                if (section_match.hasMatch()) {
                    section = section_match.captured(1);
                    title = rest.left(section_match.capturedStart()).trimmed();
                }

                QString slug = QString::fromStdString(generate_slug(title));

                if (level == 2) {
                    if (inside_div) {
                        output_lines.append("</div>");
                        inside_div = false;
                    }
                    QString style = "color: #e74c3c; font-weight: bold; font-style: italic; margin-bottom: 5px;";
                    QString html_heading = QString("<h2 id=\"%1\" data-section=\"%2\" style=\"%3\">%4</h2>")
                                           .arg(slug, section, style, title);
                    output_lines.append(html_heading);
                    output_lines.append("<div style=\"border: 2px solid #e2e8f0; border-radius: 8px; padding: 15px; margin: 10px 0; background-color: #f8fafc;\">");
                    inside_div = true;
                } else if (level == 3) {
                    QString style = "color: #2980b9; font-weight: bold; font-style: italic; margin-top: 10px; margin-bottom: 5px;";
                    QString html_subheading = QString("<h3 id=\"%1\" style=\"%2\">%3</h3>")
                                              .arg(slug, style, title);
                    output_lines.append(html_subheading);
                }
            } else if (trimmed_line.contains("<div") && trimmed_line.contains("border")) {
                inside_div = true;
                output_lines.append(line);
            } else if (trimmed_line.startsWith("</div>")) {
                inside_div = false;
                output_lines.append(line);
            } else {
                output_lines.append(line);
            }
        }

        if (inside_div) {
            output_lines.append("</div>");
        }

        QString final_content = output_lines.join('\n');

        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            QTextStream out(&file);
            out << final_content << "\n";
            file.close();
        }
    }

    QString get_current_target_file() {
        if (subject_dropdown_->currentIndex() == -1) {
            return "নির্বাচিত নয়";
        }
        return notes_dir_path_ + QDir::separator() + subject_dropdown_->currentText() + ".md";
    }

    void update_status_label() {
        QString status_text = is_running_ ? "চলছে" : "বন্ধ";
        QString target_file = get_current_target_file();
        if (target_file != "নির্বাচিত নয়") {
            QFileInfo fileInfo(target_file);
            target_file = fileInfo.fileName();
        }
        status_label_->setText(QString("অবস্থা: %1 | লক্ষ্য: %2").arg(status_text, target_file));
    }

    void close_active_box() {
        QString target_file = get_current_target_file();
        if (target_file != "নির্বাচিত নয়" && QFile::exists(target_file)) {
            restore_state_from_file(target_file);
            if (is_box_open_) {
                std::ofstream outfile;
                outfile.open(target_file.toStdString(), std::ios_base::app);
                if (outfile.is_open()) {
                    outfile << "\n</div>\n";
                    outfile.close();
                }
                is_box_open_ = false;
            }
        }
    }

    void write_image_to_file(const QString &image_filename) {
        QString target_file = get_current_target_file();
        if (target_file == "নির্বাচিত নয়") return;

        restore_state_from_file(target_file);

        std::ofstream outfile;
        outfile.open(target_file.toStdString(), std::ios_base::app);
        
        if (outfile.is_open()) {
            QDateTime now = QDateTime::currentDateTime();
            QString current_date = now.toString("dd MMMM, yyyy");
            
            if (current_date != last_date_) {
                if (is_box_open_) {
                    outfile << "\n</div>\n";
                    is_box_open_ = false;
                }
                outfile << "\n### ***" << current_date.toStdString() << "***\n";
                last_date_ = current_date;
            }
            
            outfile << "\n![Image](images/" << image_filename.toStdString() << ")\n\n";
            outfile.close();
            update_toc_in_file(target_file);
        } else {
            std::cerr << "Error: Could not open file " << target_file.toStdString() << std::endl;
            last_captured_label_->setText("ত্রুটি: ফাইলে লেখা যায়নি!");
        }
    }

    void write_to_file(const QString &processed_text, const QString &section = "others") {
        QString target_file = get_current_target_file();
        if (target_file == "নির্বাচিত নয়") return;

        // Check if we have a selected heading in the dropdown
        if (heading_dropdown_->currentIndex() > 0) {
            QString slug = heading_dropdown_->currentData().toString();
            if (append_content_to_heading(target_file, slug, processed_text)) {
                update_toc_in_file(target_file);
                last_captured_label_->setText("শেষ ক্যাপচার (নির্বাচিত শিরোনামে যুক্ত করা হয়েছে): " + processed_text);
                return;
            } else {
                last_captured_label_->setText("ত্রুটি: নির্বাচিত শিরোনামে যুক্ত করা যায়নি!");
            }
        }

        restore_state_from_file(target_file);

        std::ofstream outfile;
        outfile.open(target_file.toStdString(), std::ios_base::app);
        
        if (outfile.is_open()) {
            QDateTime now = QDateTime::currentDateTime();
            QString current_date = now.toString("dd MMMM, yyyy");
            
            if (current_date != last_date_) {
                if (is_box_open_) {
                    outfile << "\n</div>\n";
                    is_box_open_ = false;
                }
                outfile << "\n### ***" << current_date.toStdString() << "***\n";
                last_date_ = current_date;
            }
            
            int format_index = format_dropdown_->currentIndex();
            if (!processed_text.isEmpty()) {
                if (format_index == 1) { // Heading Mode
                    if (is_box_open_) {
                        outfile << "\n</div>\n";
                    }
                    QString title = processed_text.trimmed();
                    std::string slug = generate_slug(title);
                    outfile << "\n<h2 id=\"" << slug << "\" data-section=\"" << section.toStdString() << "\" style=\"color: #e74c3c; font-weight: bold; font-style: italic; margin-bottom: 5px;\">" 
                            << title.toStdString() << "</h2>\n";
                    outfile << "<div style=\"border: 2px solid #e2e8f0; border-radius: 8px; padding: 15px; margin: 10px 0; background-color: #f8fafc;\">\n";
                    is_box_open_ = true;
                } else if (format_index == 2) { // Subheading Mode
                    if (!is_box_open_) {
                        outfile << "<div style=\"border: 2px solid #e2e8f0; border-radius: 8px; padding: 15px; margin: 10px 0; background-color: #f8fafc;\">\n";
                        is_box_open_ = true;
                    }
                    outfile << "\n<h3 style=\"color: #2980b9; font-weight: bold; font-style: italic; margin-top: 10px; margin-bottom: 5px;\">" 
                            << processed_text.trimmed().toStdString() << "</h3>\n";
                } else { // Point Mode
                    outfile << "- " << processed_text.trimmed().toStdString() << "\n";
                }
            }
            
            outfile.close();
            update_toc_in_file(target_file);
            if (format_index == 1) {
                populate_headings_from_file();
            }
        } else {
            std::cerr << "Error: Could not open file " << target_file.toStdString() << std::endl;
            last_captured_label_->setText("ত্রুটি: ফাইলে লেখা যায়নি!");
        }
    }

    std::string generate_slug(const QString &text) {
        std::string result;
        bool last_was_hyphen = true; // start with true to avoid leading hyphen
        
        std::string utf8_str = text.trimmed().toStdString();
        for (size_t i = 0; i < utf8_str.length(); ) {
            unsigned char c = utf8_str[i];
            if (c < 128) {
                // ASCII character
                if (std::isalnum(c)) {
                    result += std::tolower(c);
                    last_was_hyphen = false;
                    i++;
                } else {
                    if (!last_was_hyphen) {
                        result += '-';
                        last_was_hyphen = true;
                    }
                    i++;
                }
            } else {
                // UTF-8 multi-byte character (keep it as is)
                size_t len = 1;
                if ((c & 0xE0) == 0xC0) len = 2;
                else if ((c & 0xF0) == 0xE0) len = 3;
                else if ((c & 0xF8) == 0xF0) len = 4;
                
                if (i + len <= utf8_str.length()) {
                    result.append(utf8_str.c_str() + i, len);
                    last_was_hyphen = false;
                    i += len;
                } else {
                    i++; // invalid UTF-8 byte
                }
            }
        }
        
        // Trim trailing hyphen
        while (!result.empty() && result.back() == '-') {
            result.pop_back();
        }
        return result;
    }

    void restore_state_from_file(const QString &file_path) {
        last_date_ = "";
        is_box_open_ = false;
        
        QFile file(file_path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return;
        }
        
        QTextStream in(&file);
        QRegularExpression date_regex("^###\\s*(?:\\*\\*\\*)?\\s*([0-9০-৯]{1,2}\\s+(?:January|February|March|April|May|June|July|August|September|October|November|December|জানুয়ারি|ফেব্রুয়ারি|মার্চ|এপ্রিল|মে|জুন|জুলাই|আগস্ট|সেপ্টেম্বর|অক্টোবর|নভেম্বর|ডিসেম্বর)[,\\s]+[0-9০-৯]{4})\\s*(?:\\*\\*\\*)?$", QRegularExpression::CaseInsensitiveOption);
        
        QString last_found_date = "";
        bool box_currently_open = false;
        
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            QRegularExpressionMatch match = date_regex.match(line);
            if (match.hasMatch()) {
                last_found_date = match.captured(1).trimmed();
            }
            if (line.contains("<div") && line.contains("border")) {
                box_currently_open = true;
            }
            if (line.contains("</div>")) {
                box_currently_open = false;
            }
        }
        file.close();
        
        last_date_ = last_found_date;
        is_box_open_ = box_currently_open;
    }

    void update_toc_in_file(const QString &file_path) {
        QFile file(file_path);
        if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
            return;
        }

        QTextStream in(&file);
        QString content = in.readAll();
        file.close();

        // 1. Remove existing TOC block if present
        int toc_start = content.indexOf("<!-- TOC_START -->");
        int toc_end = content.indexOf("<!-- TOC_END -->");
        QString clean_content = content;
        if (toc_start != -1 && toc_end != -1) {
            QString pre_toc = content.left(toc_start);
            QString post_toc = content.mid(toc_end + QString("<!-- TOC_END -->").length());
            clean_content = pre_toc + post_toc;
        }

        // 2. Parse lines to find headings and dates
        QStringList lines = clean_content.split('\n');
        
        struct HeadingInfo {
            int index;
            QString title;
            QString slug;
            QString date;
            bool is_html;
            QString style;
            int level;
            QString section;
        };
        
        QList<HeadingInfo> headings;
        QString current_date = "";
        
        QRegularExpression date_regex("^###\\s*(?:\\*\\*\\*)?\\s*([0-9০-৯]{1,2}\\s+(?:January|February|March|April|May|June|July|August|September|October|November|December|জানুয়ারি|ফেব্রুয়ারি|মার্চ|এপ্রিল|মে|জুন|জুলাই|আগস্ট|সেপ্টেম্বর|অক্টোবর|নভেম্বর|ডিসেম্বর)[,\\s]+[0-9০-৯]{4})\\s*(?:\\*\\*\\*)?$", QRegularExpression::CaseInsensitiveOption);
        QRegularExpression h2_regex("<h2([^>]*)>(.*?)</h2>", QRegularExpression::CaseInsensitiveOption);
        QRegularExpression h3_regex("<h3([^>]*)>(.*?)</h3>", QRegularExpression::CaseInsensitiveOption);
        QRegularExpression style_regex("style=\"([^\"]*)\"", QRegularExpression::CaseInsensitiveOption);
        QRegularExpression section_attr_regex("data-section=\"([^\"]*)\"", QRegularExpression::CaseInsensitiveOption);
        QRegularExpression md_regex("^(#{2})\\s+(.*?)$");
        QRegularExpression md_sub_regex("^(#{3})\\s+(?!\\*\\*\\*)(.*?)$");
        QRegularExpression md_section_regex("<!--\\s*section:([\\w-]+)\\s*-->");

        QStringList processed_lines;
        int heading_counter = 0;

        for (int i = 0; i < lines.size(); ++i) {
            QString line = lines[i];
            QString trimmed_line = line.trimmed();

            QRegularExpressionMatch date_match = date_regex.match(trimmed_line);
            QRegularExpressionMatch h2_match = h2_regex.match(trimmed_line);
            QRegularExpressionMatch h3_match = h3_regex.match(trimmed_line);
            QRegularExpressionMatch md_match = md_regex.match(trimmed_line);
            QRegularExpressionMatch md_sub_match = md_sub_regex.match(trimmed_line);

            if (date_match.hasMatch()) {
                current_date = date_match.captured(1).trimmed();
                processed_lines.append(line);
            } else if (h2_match.hasMatch()) {
                heading_counter++;
                QString attributes = h2_match.captured(1);
                QString title = h2_match.captured(2).trimmed();
                QString slug = QString::fromStdString(generate_slug(title));

                // Extract existing style
                QString style = "color: #e74c3c; font-weight: bold; font-style: italic; margin-bottom: 5px;"; // default
                QRegularExpressionMatch style_match = style_regex.match(attributes);
                if (style_match.hasMatch()) {
                    style = style_match.captured(1);
                }

                // Extract existing data-section
                QString section = "others";
                QRegularExpressionMatch section_match = section_attr_regex.match(attributes);
                if (section_match.hasMatch()) {
                    section = section_match.captured(1);
                }

                // Validate section
                QStringList valid_sections = {"environment", "energy", "economy", "culture", "geography", "population", "law-constitution", "politics", "freedom-fight", "others"};
                if (!valid_sections.contains(section)) {
                    section = "others";
                }

                HeadingInfo info;
                info.index = heading_counter;
                info.title = title;
                info.slug = slug;
                info.date = current_date;
                info.is_html = true;
                info.style = style;
                info.level = 2;
                info.section = section;
                headings.append(info);

                // Rewrite the line to guarantee a valid id matching the slug and preserve attributes
                QString rewritten_line = QString("<h2 id=\"%1\" data-section=\"%2\" style=\"%3\">%4</h2>")
                                         .arg(slug, section, style, title);
                processed_lines.append(rewritten_line);
            } else if (h3_match.hasMatch()) {
                QString attributes = h3_match.captured(1);
                QString title = h3_match.captured(2).trimmed();
                QString slug = QString::fromStdString(generate_slug(title));
                QString style = "color: #2980b9; font-weight: bold; font-style: italic; margin-top: 10px; margin-bottom: 5px;";
                QRegularExpressionMatch style_match = style_regex.match(attributes);
                if (style_match.hasMatch()) {
                    style = style_match.captured(1);
                }
                
                QString rewritten_line = QString("<h3 id=\"%1\" style=\"%2\">%3</h3>")
                                         .arg(slug, style, title);
                processed_lines.append(rewritten_line);
            } else if (md_match.hasMatch()) {
                heading_counter++;
                QString rest = md_match.captured(2).trimmed();

                QString section = "others";
                QRegularExpressionMatch section_match = md_section_regex.match(rest);
                QString title = rest;
                if (section_match.hasMatch()) {
                    section = section_match.captured(1);
                    title = rest.left(section_match.capturedStart()).trimmed();
                }

                // Validate section
                QStringList valid_sections = {"environment", "energy", "economy", "culture", "geography", "population", "law-constitution", "politics", "freedom-fight", "others"};
                if (!valid_sections.contains(section)) {
                    section = "others";
                }

                QString slug = QString::fromStdString(generate_slug(title));

                HeadingInfo info;
                info.index = heading_counter;
                info.title = title;
                info.slug = slug;
                info.date = current_date;
                info.is_html = false;
                info.style = "";
                info.level = 2;
                info.section = section;
                headings.append(info);

                processed_lines.append(line);
            } else if (md_sub_match.hasMatch()) {
                processed_lines.append(line);
            } else {
                processed_lines.append(line);
            }
        }

        // 3. Generate the TOC content
        QString toc_block = "";
        if (!headings.isEmpty()) {
            toc_block += "<!-- TOC_START -->\n";
            toc_block += "## সূচিপত্র (Table of Contents)\n\n";

            struct SectionDef {
                QString key;
                QString bangla;
                QString english;
            };

            QList<SectionDef> sections_list = {
                {"environment", "পরিবেশ", "Environment"},
                {"energy", "জ্বালানি", "Energy"},
                {"economy", "অর্থনীতি", "Economy"},
                {"culture", "সংস্কৃতি", "Culture"},
                {"geography", "ভূগোল", "Geography"},
                {"population", "জনসংখ্যা", "Population"},
                {"law-constitution", "আইন ও সংবিধান", "Law-Constitution"},
                {"politics", "রাজনীতি", "Politics"},
                {"freedom-fight", "মুক্তিযুদ্ধ", "Freedom-Fight"},
                {"others", "অন্যান্য", "Others"}
            };

            for (const SectionDef &sec : sections_list) {
                QList<HeadingInfo> sec_headings;
                for (const HeadingInfo &info : headings) {
                    if (info.section == sec.key) {
                        sec_headings.append(info);
                    }
                }

                if (!sec_headings.isEmpty()) {
                    toc_block += QString("### %1 (%2)\n").arg(sec.bangla, sec.english);
                    toc_block += "| পৃষ্ঠা (Page) | তারিখ (Date) | আইডি (ID) | শিরোনাম (Chapter/Topic) |\n";
                    toc_block += "| :---: | :---: | :---: | :--- |\n";

                    for (const HeadingInfo &info : sec_headings) {
                        QString date_str = info.date.isEmpty() ? "---" : info.date;
                        toc_block += QString("| **%1** | %2 | `%3` | [%4](#%5) |\n")
                                     .arg(QString::number(info.index), date_str, info.slug, info.title, info.slug);
                    }
                    toc_block += "\n";
                }
            }

            toc_block += "---\n";
            toc_block += "<!-- TOC_END -->\n";
        }

        // 4. Combine TOC and processed lines
        QString final_content = "";
        QString body_content = processed_lines.join('\n').trimmed();
        
        if (!toc_block.isEmpty()) {
            final_content = toc_block + "\n\n" + body_content;
        } else {
            final_content = body_content;
        }
        
        // 5. Write back to file
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            QTextStream out(&file);
            out << final_content << "\n";
            file.close();
        }
    }

    struct NoteItem {
        QString title;
        QString slug;
        QString type; // "heading" or "subheading"
        QString section;
        QString parent_slug;
    };

    void parse_note_structure(const QString &file_path, QList<NoteItem> &items) {
        items.clear();
        QFile file(file_path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return;
        }

        QTextStream in(&file);
        QString content = in.readAll();
        file.close();

        // Strip TOC
        int toc_start = content.indexOf("<!-- TOC_START -->");
        int toc_end = content.indexOf("<!-- TOC_END -->");
        QString clean_content = content;
        if (toc_start != -1 && toc_end != -1) {
            QString pre_toc = content.left(toc_start);
            QString post_toc = content.mid(toc_end + QString("<!-- TOC_END -->").length());
            clean_content = pre_toc + post_toc;
        }

        QStringList lines = clean_content.split('\n');
        
        QRegularExpression h2_regex("<h2([^>]*)>(.*?)</h2>", QRegularExpression::CaseInsensitiveOption);
        QRegularExpression h3_regex("<h3([^>]*)>(.*?)</h3>", QRegularExpression::CaseInsensitiveOption);
        QRegularExpression md_regex("^(#{2})\\s+(.*?)$"); // ## heading
        QRegularExpression md_sub_regex("^(#{3})\\s+(?!\\*\\*\\*)(.*?)$"); // ### subheading (excluding date)
        QRegularExpression section_attr_regex("data-section=\"([^\"]*)\"", QRegularExpression::CaseInsensitiveOption);

        QString current_h2_slug = "";

        for (int i = 0; i < lines.size(); ++i) {
            QString line = lines[i];
            QString trimmed_line = line.trimmed();

            QRegularExpressionMatch h2_match = h2_regex.match(trimmed_line);
            QRegularExpressionMatch h3_match = h3_regex.match(trimmed_line);
            QRegularExpressionMatch md_match = md_regex.match(trimmed_line);
            QRegularExpressionMatch md_sub_match = md_sub_regex.match(trimmed_line);

            if (h2_match.hasMatch()) {
                QString attributes = h2_match.captured(1);
                QString title = h2_match.captured(2).trimmed();
                QString slug = QString::fromStdString(generate_slug(title));
                
                QString section = "others";
                QRegularExpressionMatch section_match = section_attr_regex.match(attributes);
                if (section_match.hasMatch()) {
                    section = section_match.captured(1);
                }

                NoteItem item = {title, slug, "heading", section, ""};
                items.append(item);
                current_h2_slug = slug;
            } else if (md_match.hasMatch()) {
                QString title = md_match.captured(2).trimmed();
                QString slug = QString::fromStdString(generate_slug(title));
                QString section = detect_section_from_title(title);

                NoteItem item = {title, slug, "heading", section, ""};
                items.append(item);
                current_h2_slug = slug;
            } else if (h3_match.hasMatch()) {
                QString title = h3_match.captured(2).trimmed();
                QString slug = QString::fromStdString(generate_slug(title));

                NoteItem item = {title, slug, "subheading", "others", current_h2_slug};
                items.append(item);
            } else if (md_sub_match.hasMatch()) {
                QString title = md_sub_match.captured(2).trimmed();
                QString slug = QString::fromStdString(generate_slug(title));

                NoteItem item = {title, slug, "subheading", "others", current_h2_slug};
                items.append(item);
            }
        }
    }

    void save_tree_file(const QString &file_path, const QList<NoteItem> &items) {
        QString tree_path = file_path;
        tree_path.replace(".md", ".tree");
        
        QFile file(tree_path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return;
        }

        QTextStream out(&file);
        out << "Note Structure Tree\n";
        out << "===================\n\n";

        for (const NoteItem &item : items) {
            if (item.type == "heading") {
                out << QString("- [%1] %2 (id: %3)\n").arg(item.section.toUpper(), item.title, item.slug);
            } else if (item.type == "subheading") {
                out << QString("  └── %1 (id: %2)\n").arg(item.title, item.slug);
            }
        }
        file.close();
    }

    void populate_headings_from_file() {
        QString current_selected_slug = "";
        if (heading_dropdown_->currentIndex() > 0) {
            current_selected_slug = heading_dropdown_->currentData().toString();
        }

        heading_dropdown_->clear();
        heading_dropdown_->addItem("(শেষে নতুন করে যোগ করুন / Append to End)", "");

        QString target_file = get_current_target_file();
        if (target_file == "নির্বাচিত নয়" || !QFile::exists(target_file)) {
            heading_dropdown_->setEnabled(false);
            delete_heading_button_->setEnabled(false);
            append_to_heading_button_->setEnabled(false);
            return;
        }

        QList<NoteItem> items;
        parse_note_structure(target_file, items);
        
        // Save outline tree file
        save_tree_file(target_file, items);

        for (const NoteItem &item : items) {
            if (item.type == "heading") {
                heading_dropdown_->addItem(QString("%1 (id: %2) [%3]").arg(item.title, item.slug, item.section), item.slug);
            } else if (item.type == "subheading") {
                heading_dropdown_->addItem(QString("  ↳ %1 (id: %2)").arg(item.title, item.slug), item.slug);
            }
        }

        heading_dropdown_->setEnabled(true);
        
        if (!current_selected_slug.isEmpty()) {
            int index = heading_dropdown_->findData(current_selected_slug);
            if (index != -1) {
                heading_dropdown_->setCurrentIndex(index);
            }
        }
        
        update_button_states();
    }

    bool get_heading_bounds(const QString &content, const QString &slug, int &start_pos, int &end_pos, bool &is_html) {
        QString html_pattern = QString("<h2[^>]*id=\"%1\"[^>]*>").arg(QRegularExpression::escape(slug));
        QRegularExpression html_rx(html_pattern, QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch html_match = html_rx.match(content);
        
        if (html_match.hasMatch()) {
            is_html = true;
            start_pos = html_match.capturedStart();
            
            int div_close_pos = content.indexOf("</div>", html_match.capturedEnd());
            if (div_close_pos != -1) {
                end_pos = div_close_pos + 6;
                while (end_pos < content.length() && (content[end_pos] == '\n' || content[end_pos] == '\r')) {
                    end_pos++;
                }
                return true;
            } else {
                int next_h = content.indexOf("<h2", html_match.capturedEnd());
                QRegularExpression next_md_rx("^#{1,3}\\s+", QRegularExpression::MultilineOption);
                QRegularExpressionMatch next_md_match = next_md_rx.match(content, html_match.capturedEnd());
                int next_md = next_md_match.hasMatch() ? next_md_match.capturedStart() : -1;
                
                int next_pos = -1;
                if (next_h != -1 && next_md != -1) next_pos = qMin(next_h, next_md);
                else if (next_h != -1) next_pos = next_h;
                else next_pos = next_md;
                
                if (next_pos != -1) {
                    end_pos = next_pos;
                } else {
                    end_pos = content.length();
                }
                return true;
            }
        }
        
        QStringList lines = content.split('\n');
        int char_counter = 0;
        QRegularExpression md_rx("^(#{2})\\s+(.*?)$");
        
        for (int i = 0; i < lines.size(); ++i) {
            QString line = lines[i];
            int line_len_with_nl = line.length() + 1;
            QRegularExpressionMatch md_match = md_rx.match(line.trimmed());
            
            if (md_match.hasMatch()) {
                QString title = md_match.captured(2).trimmed();
                QString lslug = QString::fromStdString(generate_slug(title));
                if (lslug == slug) {
                    is_html = false;
                    start_pos = char_counter;
                    
                    int next_heading_pos = -1;
                    int current_char_pos = char_counter + line_len_with_nl;
                    for (int j = i + 1; j < lines.size(); ++j) {
                        QString next_line = lines[j];
                        if (md_rx.match(next_line.trimmed()).hasMatch() || next_line.trimmed().startsWith("<h2")) {
                            next_heading_pos = current_char_pos;
                            break;
                        }
                        current_char_pos += next_line.length() + 1;
                    }
                    
                    if (next_heading_pos != -1) {
                        end_pos = next_heading_pos;
                    } else {
                        end_pos = content.length();
                    }
                    return true;
                }
            }
            char_counter += line_len_with_nl;
        }
        
        return false;
    }

    bool get_subheading_insert_pos(const QString &content, const QString &slug, int &insert_pos) {
        QString html_pattern = QString("<h3[^>]*id=\"%1\"[^>]*>").arg(QRegularExpression::escape(slug));
        QRegularExpression html_rx(html_pattern, QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch html_match = html_rx.match(content);
        
        int start_search = -1;
        if (html_match.hasMatch()) {
            start_search = html_match.capturedEnd();
        } else {
            QStringList lines = content.split('\n');
            int char_counter = 0;
            QRegularExpression md_sub_rx("^(#{3})\\s+(?!\\*\\*\\*)(.*?)$");
            for (int i = 0; i < lines.size(); ++i) {
                QString line = lines[i];
                QRegularExpressionMatch md_match = md_sub_rx.match(line.trimmed());
                if (md_match.hasMatch()) {
                    QString title = md_match.captured(2).trimmed();
                    QString lslug = QString::fromStdString(generate_slug(title));
                    if (lslug == slug) {
                        start_search = char_counter + line.length() + 1;
                        break;
                    }
                }
                char_counter += line.length() + 1;
            }
        }

        if (start_search == -1) {
            return false;
        }

        int next_h3 = content.indexOf("<h3", start_search, Qt::CaseInsensitive);
        int next_h2 = content.indexOf("<h2", start_search, Qt::CaseInsensitive);
        int next_div_close = content.indexOf("</div>", start_search);
        
        QRegularExpression next_md_rx("^#{2,3}\\s+", QRegularExpression::MultilineOption);
        QRegularExpressionMatch next_md_match = next_md_rx.match(content, start_search);
        int next_md = next_md_match.hasMatch() ? next_md_match.capturedStart() : -1;

        int min_pos = content.length();
        if (next_h3 != -1 && next_h3 < min_pos) min_pos = next_h3;
        if (next_h2 != -1 && next_h2 < min_pos) min_pos = next_h2;
        if (next_div_close != -1 && next_div_close < min_pos) min_pos = next_div_close;
        if (next_md != -1 && next_md < min_pos) min_pos = next_md;

        insert_pos = min_pos;
        return true;
    }

    bool get_subheading_bounds(const QString &content, const QString &slug, int &start_pos, int &end_pos) {
        QString html_pattern = QString("<h3[^>]*id=\"%1\"[^>]*>").arg(QRegularExpression::escape(slug));
        QRegularExpression html_rx(html_pattern, QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch html_match = html_rx.match(content);
        
        if (html_match.hasMatch()) {
            start_pos = html_match.capturedStart();
        } else {
            QStringList lines = content.split('\n');
            int char_counter = 0;
            QRegularExpression md_sub_rx("^(#{3})\\s+(?!\\*\\*\\*)(.*?)$");
            bool found = false;
            for (int i = 0; i < lines.size(); ++i) {
                QString line = lines[i];
                QRegularExpressionMatch md_match = md_sub_rx.match(line.trimmed());
                if (md_match.hasMatch()) {
                    QString title = md_match.captured(2).trimmed();
                    QString lslug = QString::fromStdString(generate_slug(title));
                    if (lslug == slug) {
                        start_pos = char_counter;
                        found = true;
                        break;
                    }
                }
                char_counter += line.length() + 1;
            }
            if (!found) return false;
        }

        return get_subheading_insert_pos(content, slug, end_pos);
    }

    bool append_content_to_heading(const QString &file_path, const QString &slug, const QString &processed_text) {
        QFile file(file_path);
        if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
            return false;
        }
        
        QTextStream in(&file);
        QString content = in.readAll();
        file.close();
        
        int start_pos = -1;
        int end_pos = -1;
        bool is_html = false;
        
        if (get_heading_bounds(content, slug, start_pos, end_pos, is_html)) {
            if (is_html) {
                int div_close_pos = content.lastIndexOf("</div>", end_pos);
                if (div_close_pos != -1 && div_close_pos >= start_pos) {
                    QString to_append = "";
                    int format_index = format_dropdown_->currentIndex();
                    if (format_index == 2) {
                        to_append = QString("\n<h3 style=\"color: #2980b9; font-weight: bold; font-style: italic; margin-top: 10px; margin-bottom: 5px;\">%1</h3>\n")
                                    .arg(processed_text.trimmed());
                    } else if (format_index == 1) {
                        to_append = QString("\n<h3 style=\"color: #e74c3c; font-weight: bold; font-style: italic; margin-top: 10px; margin-bottom: 5px;\">%1</h3>\n")
                                    .arg(processed_text.trimmed());
                    } else {
                        to_append = QString("- %1\n").arg(processed_text.trimmed());
                    }
                    
                    content.insert(div_close_pos, to_append);
                    
                    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
                        QTextStream out(&file);
                        out << content;
                        file.close();
                        return true;
                    }
                }
            } else {
                QString to_append = "";
                int format_index = format_dropdown_->currentIndex();
                if (format_index == 2) {
                    to_append = QString("\n### %1\n").arg(processed_text.trimmed());
                } else if (format_index == 1) {
                    to_append = QString("\n## %1\n").arg(processed_text.trimmed());
                } else {
                    to_append = QString("- %1\n").arg(processed_text.trimmed());
                }
                
                content.insert(end_pos, to_append);
                
                if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
                    QTextStream out(&file);
                    out << content;
                    file.close();
                    return true;
                }
            }
        } else {
            int insert_pos = -1;
            if (get_subheading_insert_pos(content, slug, insert_pos)) {
                QString to_append = "";
                int format_index = format_dropdown_->currentIndex();
                if (format_index == 2) {
                    to_append = QString("\n<h3 style=\"color: #2980b9; font-weight: bold; font-style: italic; margin-top: 10px; margin-bottom: 5px;\">%1</h3>\n")
                                .arg(processed_text.trimmed());
                } else if (format_index == 1) {
                    to_append = QString("\n<h3 style=\"color: #e74c3c; font-weight: bold; font-style: italic; margin-top: 10px; margin-bottom: 5px;\">%1</h3>\n")
                                .arg(processed_text.trimmed());
                } else {
                    to_append = QString("- %1\n").arg(processed_text.trimmed());
                }
                
                content.insert(insert_pos, to_append);
                
                if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
                    QTextStream out(&file);
                    out << content;
                    file.close();
                    return true;
                }
            }
        }
        return false;
    }

    // --- State Variables ---
    bool is_running_;
    bool is_box_open_;
    QString last_simplified_text_;
    QString last_date_;
    QString notes_dir_path_;

    // --- UI Pointers ---
    QLabel *status_label_;
    QLabel *last_captured_label_;
    QPushButton *start_button_;
    QPushButton *stop_button_;
    QPushButton *add_image_button_;
    QComboBox *subject_dropdown_;
    QPushButton *add_subject_button_;
    QPushButton *open_file_button_;
    QComboBox *format_dropdown_;
    QTimer *clipboard_timer_;
    QLabel *mode_label_;
    QComboBox *mode_dropdown_;
    QLabel *section_label_;
    QComboBox *section_dropdown_;
    QPushButton *inject_heading_button_;
    QLabel *heading_label_;
    QComboBox *heading_dropdown_;
    QPushButton *append_to_heading_button_;
    QPushButton *delete_heading_button_;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Global UI Styling to ensure black text on light backgrounds
    app.setStyleSheet(
        "QWidget { background-color: #f5f6fa; font-family: 'Segoe UI', 'Kalpurush'; color: #2f3640; }"
        "QLabel { color: #2f3640; font-size: 14px; }"
        "QPushButton { background-color: #487eb0; color: white; border-radius: 5px; padding: 10px; font-weight: bold; border: none; min-width: 80px; }"
        "QPushButton:hover { background-color: #40739e; }"
        "QPushButton:disabled { background-color: #dcdde1; color: #7f8c8d; }"
        "QComboBox { border: 1px solid #dcdde1; border-radius: 4px; padding: 5px; background: white; color: black; min-height: 30px; }"
        "QComboBox QAbstractItemView { background: white; color: black; selection-background-color: #487eb0; selection-color: white; }"
        "QLineEdit { background: white; color: black; padding: 5px; border: 1px solid #dcdde1; border-radius: 4px; }"
        "#status_label { font-size: 15px; font-weight: bold; color: #192a56; padding: 5px; background: transparent; }"
        "#last_captured_label { background-color: white; border: 1px solid #dcdde1; border-radius: 5px; padding: 12px; color: #2f3640; font-style: italic; border-left: 4px solid #487eb0; }"
    );

    // Add the Kalpurush font
    int fontId = QFontDatabase::addApplicationFont(":/Kalpurush.ttf");
    if (fontId != -1) {
        QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
        if (!fontFamilies.isEmpty()) {
            QFont font(fontFamilies.at(0));
            app.setFont(font);
        }
    }

    ClipboardGrabber window;
    window.show();

    return app.exec();
}

#include "main.moc"
