#ifndef IMAGE_CAPTURER_H
#define IMAGE_CAPTURER_H

#include <QString>

class ImageCapturer {
public:
    explicit ImageCapturer(const QString &notesDir);
    
    bool captureAndSave(const QString &subject, QString &outputFilename);
    bool appendImageToFile(const QString &filePath, const QString &imageFilename);
    
private:
    QString notes_dir_path_;
    QString getImageDirectory() const;
};

#endif // IMAGE_CAPTURER_H
