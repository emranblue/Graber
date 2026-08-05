#ifndef GRABER_CONFIGPATHS_H
#define GRABER_CONFIGPATHS_H

#include <QString>

/**
 * Resolves project config JSON paths under ~/GraberNotes/config/.
 * Seeds missing files from embedded Qt resources (:/config/...).
 */
namespace ConfigPaths {

/** ~/GraberNotes */
QString notesRoot();

/** ~/GraberNotes/config */
QString configDir();

/** ~/GraberNotes/config/templates.json */
QString templatesJsonPath();

/** ~/GraberNotes/config/shortcut_templates.json */
QString shortcutTemplatesJsonPath();

/**
 * Ensure config dir exists and both JSON files are present.
 * Copies from :/config/* resources when the user file is missing.
 * Returns true if both user-facing paths are readable afterward.
 */
bool ensureUserConfigFiles();

/** Open a local file or folder with the system default app. */
bool openPathInSystem(const QString &path);

} // namespace ConfigPaths

#endif // GRABER_CONFIGPATHS_H
