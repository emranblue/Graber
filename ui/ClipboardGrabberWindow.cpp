#include "ClipboardGrabberWindow.h"
#include "ClipboardGrabberUI.h"
#include "MarkdownDocumentFormatter.h"
#include "DiagramTemplates.h"
#include "Utils.h"

#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QDebug>

ClipboardGrabberWindow::ClipboardGrabberWindow(QWidget *parent)
    : QWidget(parent)
    , ui(std::make_unique<ClipboardGrabberUI>())
{
    // 1. Initialize UI Elements
    ui->setupUi(this);

    // 2. Adjust initial layout size
    fitWindowToContent();

    // 3. Setup Signal/Slot Connections
    setupConnections();

    debugLog("ClipboardGrabberWindow initialized successfully.");
}

ClipboardGrabberWindow::~ClipboardGrabberWindow() = default;

void ClipboardGrabberWindow::setupConnections() {
    // Start / Stop capture signals
    connect(ui->start_button, &QPushButton::clicked, this, &ClipboardGrabberWindow::onStartButtonClicked);
    connect(ui->stop_button, &QPushButton::clicked, this, &ClipboardGrabberWindow::onStopButtonClicked);

    // Diagram Insertion Button Connection
    connect(ui->insert_diagram_button, &QPushButton::clicked, this, &ClipboardGrabberWindow::onInsertDiagramClicked);
}

void ClipboardGrabberWindow::setMonitoringActive(bool active) {
    is_monitoring_ = active;

    ui->start_button->setEnabled(!active);
    ui->stop_button->setEnabled(active);

    // Hide heavy administrative panels mid-capture to keep UI minimal
    ui->subject_card->setVisible(!active);
    ui->heading_card->setVisible(!active);
    ui->capture_extra->setVisible(!active);

    if (active) {
        ui->status_label->setText("অবস্থা: রানিং (Monitoring...)");
        ui->status_label->setStyleSheet("color: #ffffff; font-weight: bold; background-color: #2ecc71; border-radius: 4px; padding: 4px;");
    } else {
        ui->status_label->setText("অবস্থা: বন্ধ (Stopped)");
        ui->status_label->setStyleSheet("color: #7f8c8d; font-weight: bold; background-color: transparent;");
    }

    fitWindowToContent();
}

void ClipboardGrabberWindow::onStartButtonClicked() {
    setMonitoringActive(true);
    debugLog("Monitoring started.");
}

void ClipboardGrabberWindow::onStopButtonClicked() {
    setMonitoringActive(false);
    debugLog("Monitoring stopped.");
}

void ClipboardGrabberWindow::onInsertDiagramClicked() {
    // Fetch active choice from dropdown
    QString selectedType = ui->diagram_dropdown->currentData().toString();
    
    // Grab text currently on system clipboard
    QString rawContent = QApplication::clipboard()->text();

    if (rawContent.trimmed().isEmpty()) {
        rawContent = "Sample Diagram Step";
    }

    // Format diagram via MarkdownDocumentFormatter
    MarkdownDocumentFormatter formatter;
    QString formattedDiagram = formatter.formatDiagram(rawContent, selectedType);

    // Update display label to preview result
    ui->last_captured_label->setText(QString("ডায়াগ্রাম তৈরি হয়েছে:\n%1").arg(selectedType));

    // Copy formatted Mermaid code back to clipboard
    QApplication::clipboard()->setText(formattedDiagram);

    debugLog(QString("Inserted diagram template: %1").arg(selectedType));
}

void ClipboardGrabberWindow::fitWindowToContent() {
    this->adjustSize();
    this->setMinimumWidth(550);
    this->setMaximumWidth(700);
}