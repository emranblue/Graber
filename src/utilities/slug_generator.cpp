#include "slug_generator.h"
#include <cctype>

std::string SlugGenerator::generate(const QString &text) {
    std::string result;
    bool last_was_hyphen = true;
    
    std::string utf8_str = text.trimmed().toStdString();
    for (size_t i = 0; i < utf8_str.length(); ) {
        unsigned char c = utf8_str[i];
        if (c < 128) {
            if (std::isalnum(c)) {
                result += std::tolower(c);
                last_was_hyphen = false;
                i++;
            } else {
                if (!last_was_hyphen) {
                    result += '-';
                    last_was_hyphen = true;
                }
                i++;
            }
        } else {
            size_t len = 1;
            if ((c & 0xE0) == 0xC0) len = 2;
            else if ((c & 0xF0) == 0xE0) len = 3;
            else if ((c & 0xF8) == 0xF0) len = 4;
            
            if (i + len <= utf8_str.length()) {
                result.append(utf8_str.c_str() + i, len);
                last_was_hyphen = false;
                i += len;
            } else {
                i++;
            }
        }
    }
    
    while (!result.empty() && result.back() == '-') {
        result.pop_back();
    }
    return result;
}
