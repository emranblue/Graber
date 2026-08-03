#ifndef SUBJECTPANEL_H
#define SUBJECTPANEL_H

#include <QFrame>
#include <QComboBox>
#include <QPushButton>

/**
 * Folder & subject navigation card.
 * Owns dropdowns and the four management buttons.
 */
class SubjectPanel : public QFrame {
    Q_OBJECT
public:
    explicit SubjectPanel(QWidget *parent = nullptr);

    QComboBox *folderDropdown() const { return folder_dropdown_; }
    QComboBox *subjectDropdown() const { return subject_dropdown_; }

    QPushButton *addFolderButton() const { return add_folder_button_; }
    QPushButton *addSubjectButton() const { return add_subject_button_; }
    QPushButton *toggleSubjectButton() const { return toggle_subject_button_; }
    QPushButton *openFileButton() const { return open_file_button_; }

private:
    QComboBox *folder_dropdown_ = nullptr;
    QComboBox *subject_dropdown_ = nullptr;
    QPushButton *add_folder_button_ = nullptr;
    QPushButton *add_subject_button_ = nullptr;
    QPushButton *toggle_subject_button_ = nullptr;
    QPushButton *open_file_button_ = nullptr;
};

#endif // SUBJECTPANEL_H
