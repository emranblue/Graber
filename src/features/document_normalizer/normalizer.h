#ifndef DOCUMENT_NORMALIZER_H
#define DOCUMENT_NORMALIZER_H

#include <QString>
#include "../../core/application_state.h"

class DocumentNormalizer {
public:
    static void normalizeMarkdownFile(const QString &file_path);
    static void restoreStateFromFile(const QString &file_path);
    
private:
    static QString normalizeHeading(const QString &title, const QString &section);
};

#endif // DOCUMENT_NORMALIZER_H
