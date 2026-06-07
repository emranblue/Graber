#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QClipboard>
#include <QGuiApplication>
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

class ClipboardGrabber : public QWidget {
    Q_OBJECT

public:
    ClipboardGrabber(QWidget *parent = nullptr) : QWidget(parent) {
        // --- Setup ---
        setWindowTitle("ক্লিপবোর্ড গ্র্যাবার");
        setMinimumSize(450, 320);

        // --- State Variables ---
        is_running_ = false;
        is_header_mode_ = false;
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

        subject_dropdown_ = new QComboBox();
        add_subject_button_ = new QPushButton("নতুন বিষয় (New Subject)");
        add_subject_button_->setStyleSheet("QPushButton { background-color: #44bd32; } QPushButton:hover { background-color: #44bd32; opacity: 0.9; }");

        header_toggle_button_ = new QPushButton("টপিক হেডার: বন্ধ (OFF)");
        header_toggle_button_->setCheckable(true);
        header_toggle_button_->setStyleSheet(
            "QPushButton { background-color: #7f8c8d; color: white; }"
            "QPushButton:checked { background-color: #f1c40f; color: #2f3640; }"
        );

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

        file_layout->addWidget(new QLabel("বিষয়:"));
        file_layout->addWidget(subject_dropdown_, 1);
        file_layout->addWidget(add_subject_button_);

        header_layout->addWidget(new QLabel("ফরম্যাট:"));
        header_layout->addWidget(header_toggle_button_, 1);

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
        connect(add_subject_button_, &QPushButton::clicked, this, &ClipboardGrabber::add_subject);
        connect(header_toggle_button_, &QPushButton::toggled, this, &ClipboardGrabber::toggle_header_mode);
        connect(clipboard_timer_, &QTimer::timeout, this, &ClipboardGrabber::check_clipboard);
        
        // --- Initial Population ---
        populate_subjects_from_disk();
        subject_dropdown_->setCurrentIndex(-1); // No initial selection
        update_status_label();
    }

private slots:
    void start_monitoring() {
        if (subject_dropdown_->currentIndex() == -1) {
            status_label_->setText("অবস্থা: অনুগ্রহ করে প্রথমে একটি বিষয় নির্বাচন করুন!");
            return;
        }
        is_running_ = true;
        
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

    void toggle_header_mode(bool checked) {
        is_header_mode_ = checked;
        if (is_header_mode_) {
            header_toggle_button_->setText("টপিক হেডার: চালু (ON)");
        } else {
            header_toggle_button_->setText("টপিক হেডার: বন্ধ (OFF)");
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
                outfile << "\n### " << current_date.toStdString() << "\n";
                last_date_ = current_date;
            }
            
            // Add pointwise text or header
            if (!processed_text.isEmpty()) {
                if (is_header_mode_) {
                    outfile << "\n# " << processed_text.trimmed().toStdString() << "\n\n";
                } else {
                    outfile << "- " << processed_text.trimmed().toStdString() << "\n";
                }
            }
            
            outfile.close();
        } else {
            std::cerr << "Error: Could not open file " << target_file.toStdString() << std::endl;
            last_captured_label_->setText("ত্রুটি: ফাইলে লেখা যায়নি!");
        }
    }

    // --- State Variables ---
    bool is_running_;
    bool is_header_mode_;
    QString last_simplified_text_;
    QString last_date_;
    QString notes_dir_path_;

    // --- UI Pointers ---
    QLabel *status_label_;
    QLabel *last_captured_label_;
    QPushButton *start_button_;
    QPushButton *stop_button_;
    QComboBox *subject_dropdown_;
    QPushButton *add_subject_button_;
    QPushButton *header_toggle_button_;
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
