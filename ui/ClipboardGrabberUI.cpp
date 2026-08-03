#include "ClipboardGrabberUI.h"
#include <QVBoxLayout>
#include <QScrollArea>
#include <QFrame>

void ClipboardGrabberUI::setupUi(QWidget *parent) {
    parent->setWindowTitle("ক্লিপবোর্ড গ্র্যাবার");
    parent->resize(620, 820); // placeholder; corrected by fit_window_to_content()
    parent->setObjectName("MainWindow");

    // ---- Build modular panels ----
    status_panel   = new StatusPanel(parent);
    subject_panel  = new SubjectPanel(parent);
    capture_panel  = new CapturePanel(parent);
    heading_panel  = new HeadingPanel(parent);
    controls_bar_w = new ControlsBar(parent);

    // ---- Wire flat aliases (controller keeps using ui_.xxx) ----
    status_label         = status_panel->statusLabel();
    last_captured_label  = status_panel->lastCapturedLabel();

    subject_card         = subject_panel;
    folder_dropdown      = subject_panel->folderDropdown();
    subject_dropdown     = subject_panel->subjectDropdown();
    add_folder_button    = subject_panel->addFolderButton();
    add_subject_button   = subject_panel->addSubjectButton();
    toggle_subject_button= subject_panel->toggleSubjectButton();
    open_file_button     = subject_panel->openFileButton();

    capture_card         = capture_panel;
    format_dropdown      = capture_panel->formatDropdown();
    mode_dropdown        = capture_panel->modeDropdown();
    section_dropdown     = capture_panel->sectionDropdown();
    diagram_dropdown     = capture_panel->diagramDropdown();
    add_image_button     = capture_panel->addImageButton();
    add_section_button   = capture_panel->addSectionButton();
    insert_diagram_button= capture_panel->insertDiagramButton();
    capture_extra        = capture_panel->captureExtra();
    diagram_quick_row    = capture_panel->diagramQuickRow();
    section_label        = capture_panel->sectionLabel();
    mode_label           = capture_panel->modeLabel();

    heading_card         = heading_panel;
    select_heading_button      = heading_panel->selectHeadingButton();
    append_to_heading_button   = heading_panel->appendButton();
    inject_heading_button      = heading_panel->injectButton();
    shift_heading_button       = heading_panel->shiftButton();
    delete_heading_button      = heading_panel->deleteButton();
    heading_label              = heading_panel->headingLabel();

    controls_bar         = controls_bar_w;
    start_button         = controls_bar_w->startButton();
    stop_button          = controls_bar_w->stopButton();
    wizards_button       = controls_bar_w->wizardsButton();
    settings_button      = controls_bar_w->settingsButton();

    // ---- Assemble scrollable body ----
    body = new QWidget();
    body->setObjectName("scrollBody");
    auto *body_layout = new QVBoxLayout(body);
    body_layout->setContentsMargins(10, 8, 10, 8);
    body_layout->setSpacing(10);
    body_layout->addWidget(status_panel);
    body_layout->addWidget(subject_panel);
    body_layout->addWidget(capture_panel);
    body_layout->addWidget(heading_panel);
    body_layout->addStretch(1);

    auto *scroll_area = new QScrollArea();
    scroll_area->setObjectName("bodyScroll");
    scroll_area->setWidget(body);
    scroll_area->setWidgetResizable(true);
    scroll_area->setFrameShape(QFrame::NoFrame);
    scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll_area->setStyleSheet(
        "QScrollArea#bodyScroll { background: transparent; border: none; }"
        "QWidget#scrollBody { background: transparent; }");

    // ---- Main window layout ----
    auto *main_layout = new QVBoxLayout(parent);
    main_layout->setSpacing(0);
    main_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->addWidget(scroll_area, 1);
    main_layout->addWidget(controls_bar_w);
}
