#ifndef SLUG_GENERATOR_H
#define SLUG_GENERATOR_H

#include <QString>
#include <string>

class SlugGenerator {
public:
    static std::string generate(const QString &text);
};

#endif // SLUG_GENERATOR_H
