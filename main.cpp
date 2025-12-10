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
#include <string>
#include <fstream>
#include <iostream>

class ClipboardGrabber : public QWidget {
    Q_OBJECT

public:
    ClipboardGrabber(QWidget *parent = nullptr) : QWidget(parent) {
        // --- Setup ---
        setWindowTitle("Clipboard Graber");
        setMinimumSize(450, 220);

        // --- State Variables ---
        is_running_ = false;
        last_clipboard_text_ = "";

        // --- UI Widgets ---
        status_label_ = new QLabel("Status: Stopped");
        status_label_->setAlignment(Qt::AlignCenter);

        last_captured_label_ = new QLabel("Last captured: (nothing)");
        last_captured_label_->setAlignment(Qt::AlignCenter);
        last_captured_label_->setWordWrap(true);

        start_button_ = new QPushButton("Start");
        stop_button_ = new QPushButton("Stop");
        stop_button_->setEnabled(false);

        subject_dropdown_ = new QComboBox();
        add_subject_button_ = new QPushButton("Add Subject");

        // --- Layout ---
        QVBoxLayout *main_layout = new QVBoxLayout(this);
        QHBoxLayout *control_layout = new QHBoxLayout();
        QHBoxLayout *file_layout = new QHBoxLayout();

        control_layout->addWidget(start_button_);
        control_layout->addWidget(stop_button_);

        file_layout->addWidget(subject_dropdown_);
        file_layout->addWidget(add_subject_button_);

        main_layout->addWidget(status_label_);
        main_layout->addWidget(last_captured_label_);
        main_layout->addLayout(file_layout);
        main_layout->addLayout(control_layout);

        // --- Clipboard Timer ---
        clipboard_timer_ = new QTimer(this);
        clipboard_timer_->setInterval(1000); // Check every 1 second

        // --- Connections (Signals and Slots) ---
        connect(start_button_, &QPushButton::clicked, this, &ClipboardGrabber::start_monitoring);
        connect(stop_button_, &QPushButton::clicked, this, &ClipboardGrabber::stop_monitoring);
        connect(add_subject_button_, &QPushButton::clicked, this, &ClipboardGrabber::add_subject);
        connect(clipboard_timer_, &QTimer::timeout, this, &ClipboardGrabber::check_clipboard);
        
        // --- Initial Population ---
        populate_subjects_from_disk();
        subject_dropdown_->setCurrentIndex(-1); // No initial selection
        update_status_label();
    }

private slots:
    void start_monitoring() {
        if (subject_dropdown_->currentIndex() == -1) {
            status_label_->setText("Status: Please select a subject first!");
            return;
        }
        is_running_ = true;
        // FIX: Prime the clipboard text to ignore anything copied before starting.
        last_clipboard_text_ = QGuiApplication::clipboard()->text();
        clipboard_timer_->start();
        start_button_->setEnabled(false);
        stop_button_->setEnabled(true);
        subject_dropdown_->setEnabled(false);
        add_subject_button_->setEnabled(false);
        update_status_label();
    }

    void stop_monitoring() {
        is_running_ = false;
        clipboard_timer_->stop();
        start_button_->setEnabled(true);
        stop_button_->setEnabled(false);
        subject_dropdown_->setEnabled(true);
        add_subject_button_->setEnabled(true);
        update_status_label();
    }

    void add_subject() {
        bool ok;
        QString text = QInputDialog::getText(this, "Add Subject",
                                             "New subject name:", QLineEdit::Normal,
                                             "", &ok);
        if (ok && !text.isEmpty()) {
            // Avoid adding duplicates
            if (subject_dropdown_->findText(text) == -1) {
                subject_dropdown_->addItem(text);
            }
            subject_dropdown_->setCurrentText(text);
        }
    }

    void check_clipboard() {
        QClipboard *clipboard = QGuiApplication::clipboard();
        QString current_text = clipboard->text();

        if (!current_text.isEmpty() && current_text != last_clipboard_text_) {
            last_clipboard_text_ = current_text;
            last_captured_label_->setText("Last captured: " + current_text);
            write_to_file(current_text);
        }
    }

private:
    void populate_subjects_from_disk() {
        // 1. Ensure default files exist to provide a starting point
        QStringList default_subjects = {"Bangladesh", "International"};
        for (const QString& subject : default_subjects) {
            QString filename = subject + ".md";
            if (!QFile::exists(filename)) {
                QFile file(filename);
                if (file.open(QIODevice::WriteOnly)) {
                    file.close();
                }
            }
        }

        // 2. Populate dropdown from all .md files in the directory
        QDir directory(".");
        QStringList md_files = directory.entryList(QStringList() << "*.md", QDir::Files);
        
        subject_dropdown_->clear();
        for (QString filename : md_files) {
            filename.chop(3); // Remove ".md"
            subject_dropdown_->addItem(filename);
        }
    }

    QString get_current_target_file() {
        if (subject_dropdown_->currentIndex() == -1) {
            return "Not Selected";
        }
        return subject_dropdown_->currentText() + ".md";
    }

    void update_status_label() {
        QString status_text = is_running_ ? "Running" : "Stopped";
        QString target_file = get_current_target_file();
        status_label_->setText(QString("Status: %1 | Target: %2").arg(status_text, target_file));
    }

    void write_to_file(const QString &text) {
        QString target_file = get_current_target_file();
        if (target_file == "Not Selected") return;

        std::ofstream outfile;
        outfile.open(target_file.toStdString(), std::ios_base::app);
        
        if (outfile.is_open()) {
            QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            
            // MORE ROBUST FIX: Use simplified() to replace any newline sequence and other whitespace with a single space.
            QString processed_text = text.simplified();

            outfile << "---\n";
            outfile << "**Captured on:** " << timestamp.toStdString() << "\n\n";
            if (!processed_text.isEmpty()) {
                outfile << "> " << processed_text.toStdString() << "\n\n";
            }
            outfile << "---\n\n";
            
            outfile.close();
        } else {
            std::cerr << "Error: Could not open file " << target_file.toStdString() << std::endl;
            last_captured_label_->setText("Error: Could not write to file!");
        }
    }

    // --- State Variables ---
    bool is_running_;
    QString last_clipboard_text_;

    // --- UI Pointers ---
    QLabel *status_label_;
    QLabel *last_captured_label_;
    QPushButton *start_button_;
    QPushButton *stop_button_;
    QComboBox *subject_dropdown_;
    QPushButton *add_subject_button_;
    QTimer *clipboard_timer_;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    ClipboardGrabber window;
    window.show();

    return app.exec();
}

#include "main.moc"
