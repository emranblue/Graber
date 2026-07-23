#include "application_state.h"
#include <QDir>

ApplicationState* ApplicationState::instance_ = nullptr;

ApplicationState& ApplicationState::instance() {
    if (!instance_) {
        instance_ = new ApplicationState();
    }
    return *instance_;
}

ApplicationState::ApplicationState()
    : is_running_(false), last_date_(""), last_simplified_text_("") {
    notes_dir_path_ = QDir::homePath() + QDir::separator() + "GraberNotes";
    QDir dir(notes_dir_path_);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
}

ApplicationState::~ApplicationState() {
}
