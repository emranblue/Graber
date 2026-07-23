#ifndef APPLICATION_STATE_H
#define APPLICATION_STATE_H

#include <QString>
#include <QList>

struct NoteItem {
    QString title;
    QString slug;
    QString type; // "heading" or "subheading"
    QString section;
    QString parent_slug;
};

class ApplicationState {
public:
    static ApplicationState& instance();
    
    // Getters and setters
    QString getNotesDirectory() const { return notes_dir_path_; }
    void setNotesDirectory(const QString &path) { notes_dir_path_ = path; }
    
    bool isRunning() const { return is_running_; }
    void setRunning(bool running) { is_running_ = running; }
    
    QString getLastDate() const { return last_date_; }
    void setLastDate(const QString &date) { last_date_ = date; }
    
    QString getLastSimplifiedText() const { return last_simplified_text_; }
    void setLastSimplifiedText(const QString &text) { last_simplified_text_ = text; }
    
    QList<NoteItem> getAllHeadings() const { return all_headings_; }
    void setAllHeadings(const QList<NoteItem> &headings) { all_headings_ = headings; }
    
private:
    ApplicationState();
    ~ApplicationState();
    
    static ApplicationState* instance_;
    
    QString notes_dir_path_;
    bool is_running_;
    QString last_date_;
    QString last_simplified_text_;
    QList<NoteItem> all_headings_;
};

#endif // APPLICATION_STATE_H
