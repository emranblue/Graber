#include "section_definitions.h

QList<SectionDef> SectionDefinitions::getDefaultSections() {
    return {
        {"environment", "পরিবেশ", "Environment"},
        {"energy", "জ্বালানি", "Energy"},
        {"economy", "অর্থনীতি", "Economy"},
        {"culture", "সংস্কৃতি", "Culture"},
        {"geography", "ভূগোল", "Geography"},
        {"population", "জনসংখ্যা", "Population"},
        {"law-constitution", "আইন ও সংবিধান", "Law-Constitution"},
        {"politics", "রাজনীতি", "Politics"},
        {"freedom-fight", "মুক্তিযুদ্ধ", "Freedom-Fight"},
        {"agriculture", "কৃষি", "Agriculture"},
        {"history", "ইতিহাস", "History"},
        {"education", "শিক্ষা", "Education"},
        {"health", "স্বাস্থ্য", "Health"},
        {"science-tech", "বিজ্ঞান ও প্রযুক্তি", "Science-Tech"},
        {"foreign-policy", "পররাষ্ট্রনীতি", "Foreign-Policy"},
        {"administration", "প্রশাসন", "Administration"},
        {"others", "অন্যান্য", "Others"}
    };
}

QString SectionDefinitions::detectSectionFromTitle(const QString &title) {
    QString lower_title = title.toLower().trimmed();
    
    struct MapEntry {
        QString keyword;
        QString section;
    };
    
    QList<MapEntry> mappings = {
        {"environment", "environment"}, {"পরিবেশ", "environment"},
        {"energy", "energy"}, {"জ্বালানি", "energy"},
        {"economy", "economy"}, {"অর্থনীতি", "economy"},
        {"culture", "culture"}, {"সংস্কৃতি", "culture"},
        {"geography", "geography"}, {"ভূগোল", "geography"},
        {"population", "population"}, {"জনসংখ্যা", "population"},
        {"law-constitution", "law-constitution"}, {"আইন ও সংবিধান", "law-constitution"},
        {"politics", "politics"}, {"রাজনীতি", "politics"},
        {"freedom-fight", "freedom-fight"}, {"মুক্তিযুদ্ধ", "freedom-fight"},
        {"agriculture", "agriculture"}, {"কৃষি", "agriculture"},
        {"history", "history"}, {"ইতিহাস", "history"},
        {"education", "education"}, {"শিক্ষা", "education"},
        {"health", "health"}, {"স্বাস্থ্য", "health"},
        {"science-tech", "science-tech"}, {"বিজ্ঞান ও প্রযুক্তি", "science-tech"}, {"বিজ্ঞান", "science-tech"}, {"প্রযুক্তি", "science-tech"},
        {"foreign-policy", "foreign-policy"}, {"পররাষ্ট্রনীতি", "foreign-policy"},
        {"administration", "administration"}, {"প্রশাসন", "administration"}
    };
    
    for (const auto &entry : mappings) {
        if (lower_title.contains(entry.keyword)) {
            return entry.section;
        }
    }
    return "others";
}
