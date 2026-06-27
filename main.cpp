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
        setMinimumSize(450, 320);

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

        format_dropdown_ = new QComboBox();
        format_dropdown_->addItem("বুলেট পয়েন্ট (Point)");
        format_dropdown_->addItem("প্রধান শিরোনাম (Heading - Red)");
        format_dropdown_->addItem("উপ-শিরোনাম (Subheading - Blue)");

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
        QHBoxLayout *header_layout = new QHBoxLayout();
        QHBoxLayout *mode_layout = new QHBoxLayout();

        control_layout->addWidget(start_button_);
        control_layout->addWidget(stop_button_);
        control_layout->addWidget(add_image_button_);

        file_layout->addWidget(new QLabel("বিষয়:"));
        file_layout->addWidget(subject_dropdown_, 1);
        file_layout->addWidget(add_subject_button_);
        file_layout->addWidget(open_file_button_);

        header_layout->addWidget(new QLabel("ফরম্যাট:"));
        header_layout->addWidget(format_dropdown_, 1);

        mode_layout->addWidget(mode_label_);
        mode_layout->addWidget(mode_dropdown_, 1);

        main_layout->addWidget(status_label_);
        main_layout->addWidget(last_captured_label_);
        main_layout->addLayout(file_layout);
        main_layout->addLayout(header_layout);
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
        connect(clipboard_timer_, &QTimer::timeout, this, &ClipboardGrabber::check_clipboard);
        
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
            write_to_file(simplified_text);
        }
    }

    void on_subject_changed(const QString &text) {
        close_active_box();
        update_status_label();
        open_file_button_->setEnabled(!text.isEmpty());
        if (!text.isEmpty()) {
            update_toc_in_file(get_current_target_file());
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

private:
    void populate_subjects_from_disk() {
        // Populate dropdown from all .md files in the directory
        QDir directory(notes_dir_path_);
        QStringList md_files = directory.entryList(QStringList() << "*.md", QDir::Files);
        
        subject_dropdown_->clear();
        for (QString filename : md_files) {
            filename.chop(3); // Remove ".md"
            subject_dropdown_->addItem(filename);
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
        if (is_box_open_) {
            QString target_file = get_current_target_file();
            if (target_file != "নির্বাচিত নয়") {
                std::ofstream outfile;
                outfile.open(target_file.toStdString(), std::ios_base::app);
                if (outfile.is_open()) {
                    outfile << "\n</div>\n";
                    outfile.close();
                }
            }
            is_box_open_ = false;
        }
    }

    void write_image_to_file(const QString &image_filename) {
        QString target_file = get_current_target_file();
        if (target_file == "নির্বাচিত নয়") return;

        std::ofstream outfile;
        outfile.open(target_file.toStdString(), std::ios_base::app);
        
        if (outfile.is_open()) {
            QDateTime now = QDateTime::currentDateTime();
            QString current_date = now.toString("dd MMMM, yyyy");
            
            // Add date header if date changed or file is new
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

    void write_to_file(const QString &processed_text) {
        QString target_file = get_current_target_file();
        if (target_file == "নির্বাচিত নয়") return;

        std::ofstream outfile;
        outfile.open(target_file.toStdString(), std::ios_base::app);
        
        if (outfile.is_open()) {
            QDateTime now = QDateTime::currentDateTime();
            QString current_date = now.toString("dd MMMM, yyyy");
            
            // Add date header if date changed or file is new
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
                    outfile << "\n<h2 id=\"" << slug << "\" style=\"color: #e74c3c; font-weight: bold; font-style: italic; margin-bottom: 5px;\">" 
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
        };
        
        QList<HeadingInfo> headings;
        QString current_date = "";
        
        QRegularExpression date_regex("^###\\s*(?:\\*\\*\\*)?\\s*([0-9০-৯]{1,2}\\s+(?:January|February|March|April|May|June|July|August|September|October|November|December|জানুয়ারি|ফেব্রুয়ারি|মার্চ|এপ্রিল|মে|জুন|জুলাই|আগস্ট|সেপ্টেম্বর|অক্টোবর|নভেম্বর|ডিসেম্বর)[,\\s]+[0-9০-৯]{4})\\s*(?:\\*\\*\\*)?$", QRegularExpression::CaseInsensitiveOption);
        QRegularExpression h2_regex("<h2([^>]*)>(.*?)</h2>", QRegularExpression::CaseInsensitiveOption);
        QRegularExpression style_regex("style=\"([^\"]*)\"", QRegularExpression::CaseInsensitiveOption);
        QRegularExpression md_regex("^(#{1,3})\\s+(?!\\*\\*\\*)(.*?)$");

        QStringList processed_lines;
        int heading_counter = 0;

        for (int i = 0; i < lines.size(); ++i) {
            QString line = lines[i];
            QString trimmed_line = line.trimmed();

            QRegularExpressionMatch date_match = date_regex.match(trimmed_line);
            QRegularExpressionMatch h2_match = h2_regex.match(trimmed_line);
            QRegularExpressionMatch md_match = md_regex.match(trimmed_line);

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

                HeadingInfo info;
                info.index = heading_counter;
                info.title = title;
                info.slug = slug;
                info.date = current_date;
                info.is_html = true;
                info.style = style;
                info.level = 2;
                headings.append(info);

                // Rewrite the line to guarantee a valid id matching the slug
                QString rewritten_line = QString("<h2 id=\"%1\" style=\"%2\">%3</h2>").arg(slug, style, title);
                processed_lines.append(rewritten_line);
            } else if (md_match.hasMatch()) {
                heading_counter++;
                int level = md_match.captured(1).length();
                QString title = md_match.captured(2).trimmed();
                QString slug = QString::fromStdString(generate_slug(title));

                HeadingInfo info;
                info.index = heading_counter;
                info.title = title;
                info.slug = slug;
                info.date = current_date;
                info.is_html = false;
                info.style = "";
                info.level = level;
                headings.append(info);

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
            toc_block += "| পৃষ্ঠা (Page) | তারিখ (Date) | শিরোনাম (Chapter/Topic) |\n";
            toc_block += "| :---: | :---: | :--- |\n";
            
            for (const HeadingInfo &info : headings) {
                QString date_str = info.date.isEmpty() ? "---" : info.date;
                toc_block += QString("| **%1** | %2 | [%3](#%4) |\n")
                             .arg(QString::number(info.index), date_str, info.title, info.slug);
            }
            
            toc_block += "\n---\n";
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
