#include "image_capturer.h"
#include "../../utilities/file_handler.h"
#include "../../utilities/logger.h"
#include <QClipboard>
#include <QGuiApplication>
#include <QImage>
#include <QDateTime>
#include <QDir>
#include <fstream>

ImageCapturer::ImageCapturer(const QString &notesDir)
    : notes_dir_path_(notesDir) {
}

QString ImageCapturer::getImageDirectory() const {
    return notes_dir_path_ + QDir::separator() + "images";
}

bool ImageCapturer::captureAndSave(const QString &subject, QString &outputFilename) {
    QClipboard *clipboard = QGuiApplication::clipboard();
    QImage image = clipboard->image();
    
    if (image.isNull()) {
        Logger::error("No image in clipboard");
        return false;
    }
    
    // Ensure images directory exists
    QString images_dir = getImageDirectory();
    if (!FileHandler::ensureDirectoryExists(images_dir)) {
        Logger::error("Failed to create images directory");
        return false;
    }
    
    QDateTime now = QDateTime::currentDateTime();
    QString timestamp = now.toString("yyyyMMdd_hhmmss_zzz");
    outputFilename = QString("img_%1.png").arg(timestamp);
    QString filepath = images_dir + QDir::separator() + outputFilename;
    
    if (image.save(filepath, "PNG")) {
        Logger::info(QString("Image saved: %1").arg(filepath));
        return true;
    } else {
        Logger::error("Failed to save image");
        return false;
    }
}

bool ImageCapturer::appendImageToFile(const QString &filePath, const QString &imageFilename) {
    std::ofstream outfile;
    outfile.open(filePath.toStdString(), std::ios_base::app);
    
    if (!outfile.is_open()) {
        Logger::error("Failed to open file for appending image");
        return false;
    }
    
    outfile << "\n![Image](images/" << imageFilename.toStdString() << ")\n\n";
    outfile.close();
    
    Logger::info(QString("Image appended to file: %1").arg(filePath));
    return true;
}
