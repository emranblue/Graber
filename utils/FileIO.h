#ifndef GRABER_FILEIO_H
#define GRABER_FILEIO_H

#include <QString>
#include "core/Result.h"

/**
 * Central, crash-resistant file helpers.
 * - Never throws
 * - Uses QSaveFile for atomic replaces (avoids truncated notes on power loss / crash)
 * - Path existence / readability checks before open
 */
namespace FileIO {

/** Read entire text file. Empty path or open failure → fail Result. */
Result<QString> readText(const QString &filePath);

/**
 * Atomically replace file contents.
 * Creates parent directories when possible.
 * On failure the original file (if any) is left intact.
 */
VoidResult writeTextAtomic(const QString &filePath, const QString &content);

/** Append text (read-modify-write via atomic write). */
VoidResult appendTextAtomic(const QString &filePath, const QString &chunk);

/** True if path is non-empty, exists, and is a regular readable file. */
bool isReadableFile(const QString &filePath);

/** Ensure parent directory of filePath exists. */
bool ensureParentDir(const QString &filePath);

} // namespace FileIO

#endif // GRABER_FILEIO_H
