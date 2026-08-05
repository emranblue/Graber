#ifndef CAPTUREPANEL_H
#define CAPTUREPANEL_H

#include <QFrame>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QWidget>

// Shared constant — single source of truth for the Diagram format entry.
inline const QString kDiagramFormatLabel = QStringLiteral("ডায়াগ্রাম (Diagram)");

/**
 * Capture configuration card:
 * format / mode / section / image / diagram quick-insert.
 */
class CapturePanel : public QFrame {
    Q_OBJECT
public:
    explicit CapturePanel(QWidget *parent = nullptr);

    QComboBox *formatDropdown() const { return format_dropdown_; }
    QComboBox *modeDropdown() const { return mode_dropdown_; }
    QComboBox *sectionDropdown() const { return section_dropdown_; }
    QComboBox *diagramDropdown() const { return diagram_dropdown_; }

    void populateFormatDropdown();

    QPushButton *addImageButton() const { return add_image_button_; }
    QPushButton *addSectionButton() const { return add_section_button_; }
    QPushButton *insertDiagramButton() const { return insert_diagram_button_; }

    QWidget *captureExtra() const { return capture_extra_; }
    QWidget *diagramQuickRow() const { return diagram_quick_row_; }

    QLabel *sectionLabel() const { return section_label_; }
    QLabel *modeLabel() const { return mode_label_; }

    bool isDiagramFormatSelected() const;
    void setDiagramDropdownEnabled(bool enabled);
    void setExtraVisible(bool visible);
    void setDiagramRowVisible(bool visible);

private:
    QComboBox *format_dropdown_ = nullptr;
    QComboBox *mode_dropdown_ = nullptr;
    QComboBox *section_dropdown_ = nullptr;
    QComboBox *diagram_dropdown_ = nullptr;

    QPushButton *add_image_button_ = nullptr;
    QPushButton *add_section_button_ = nullptr;
    QPushButton *insert_diagram_button_ = nullptr;

    QWidget *capture_extra_ = nullptr;
    QWidget *diagram_quick_row_ = nullptr;

    QLabel *section_label_ = nullptr;
    QLabel *mode_label_ = nullptr;
};

#endif // CAPTUREPANEL_H
