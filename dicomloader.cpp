#include "dicomloader.h"
#include <QDebug>
#include <QFileInfo>
#include <QImage>
#include <vector>


#include "gdcmImageReader.h"
#include "gdcmImage.h"
#include "gdcmPhotometricInterpretation.h"
#include "gdcmPixelFormat.h"

DicomLoader::DicomLoader()
{
    qDebug() << "DicomLoader intialized";

}

bool DicomLoader::isDicomFile(const QString &fileName)
{

    QString lowerFileName = fileName.toLower();
    if (lowerFileName.endsWith(".dcm") || lowerFileName.endsWith(".dicom")) {
        return true;
    }
    //TODO: Add DICOM header validation later
    return false;
}

QPixmap DicomLoader::loadDicomImage(const QString &fileName)
{
    qDebug() << "Attempting to load DICOM:" << fileName;

    if (!QFileInfo::exists(fileName)) {
        qDebug() << "File does not exist:" << fileName;
        return QPixmap();
    }
    return convertDicomToPixmap(fileName);
}

QPixmap DicomLoader::convertDicomToPixmap(const QString &fileName)
{
    qDebug() << "=== DICOM Loading Debug ===";
    qDebug() << "File:" << fileName;

    gdcm::ImageReader reader;
    reader.SetFileName(fileName.toStdString().c_str());

    qDebug() << "Attempting to read DICOM file...";
    if (!reader.Read()) {
        return QPixmap();
    }

    qDebug() << "GDCM successfully read the file!";

    const gdcm::Image &image = reader.GetImage();

    // Get image dimensions
    const unsigned int *dims = image.GetDimensions();
    unsigned int width = dims[0];
    unsigned int height = dims[1];

    qDebug() << "DICOM dimensions:" << width << "x" << height;

    // Get pixel format info
    gdcm::PixelFormat pixelFormat = image.GetPixelFormat();
    qDebug() << "Bits stored:" << pixelFormat.GetBitsStored();
    qDebug() << "Samples per pixel:" << pixelFormat.GetSamplesPerPixel();

    // Check for reasonable dimensions
    if (width == 0 || height == 0 || width > 10000 || height > 10000) {
        qDebug() << "ERROR: Invalid dimensions detected";
        return QPixmap();
    }

    // Allocate buffer for pixel data
    std::vector<char> buffer;
    unsigned long bufferLength = image.GetBufferLength();
    qDebug() << "Buffer length needed:" << bufferLength;

    buffer.resize(bufferLength);

    qDebug() << "Attempting to extract pixel buffer...";
    if (!image.GetBuffer(buffer.data())) {
        qDebug() << "ERROR: Failed to get pixel buffer from DICOM";
        return QPixmap();
    }

    // Convert to QImage first, then QPixmap
    QImage qimage = convertToQImage(buffer, width, height, pixelFormat.GetBitsStored());

    if (qimage.isNull()) {
        qDebug() << "ERROR: Failed to convert to QImage";
        return QPixmap();
    }

    return QPixmap::fromImage(qimage);
}

QImage DicomLoader::convertToQImage(const std::vector<char> &buffer, unsigned int width,
                                    unsigned int height, int bitsStored)
{
    // Check if 8 bit
    if (bitsStored == 8) {
        QImage image(width, height, QImage::Format_Grayscale8);

        for (unsigned int y = 0; y < height; ++y) {
            for (unsigned int x = 0; x < width; ++x) {
                unsigned int index = y * width + x;
                if (index < buffer.size()) {
                    unsigned char pixel = static_cast<unsigned char>(buffer[index]);
                    image.setPixel(x, y, qRgb(pixel, pixel, pixel));
                }
            }
        }
        return image;
    }
    else if (bitsStored == 12) {
        // 12-bit grayscale (common for medical X-rays, CR, DR)
        QImage image(width, height, QImage::Format_Grayscale8);

        const uint16_t *pixelData = reinterpret_cast<const uint16_t*>(buffer.data());

        for (unsigned int y = 0; y < height; ++y) {
            for (unsigned int x = 0; x < width; ++x) {
                unsigned int index = y * width + x;
                if (index * 2 < buffer.size()) {
                    // Get 12-bit pixel (stored in 16-bit container)
                    uint16_t pixel12 = pixelData[index];

                    // Scale 12-bit (0-4095) to 8-bit (0-255)
                    uint8_t pixel8 = pixel12 >> 4;  // Divide by 16 (2^4)

                    image.setPixel(x, y, qRgb(pixel8, pixel8, pixel8));
                }
            }
        }
        return image;
    }
    else if (bitsStored == 16) {
        // 16-bit grayscale
        QImage image(width, height, QImage::Format_Grayscale8);

        const uint16_t *pixelData = reinterpret_cast<const uint16_t*>(buffer.data());

        for (unsigned int y = 0; y < height; ++y) {
            for (unsigned int x = 0; x < width; ++x) {
                unsigned int index = y * width + x;
                if (index * 2 < buffer.size()) {
                    // Scale 16-bit to 8-bit for display
                    uint16_t pixel16 = pixelData[index];
                    uint8_t pixel8 = pixel16 >> 8;  // Divide by 256
                    image.setPixel(x, y, qRgb(pixel8, pixel8, pixel8));
                }
            }
        }
        return image;
    }

    qDebug() << "Unsupported pixel format:" << bitsStored << "bits";
    return QImage();
}

QString DicomLoader::extractTag(const gdcm::DataSet& dataset, const gdcm::Tag& tag)
{
    if (dataset.FindDataElement(tag)) {
        const gdcm::DataElement& element = dataset.GetDataElement(tag);
        if (!element.IsEmpty()) {
            std::string value = std::string(element.GetByteValue()->GetPointer(),
                                            element.GetByteValue()->GetLength());
            // remove null terminators and whitespace
            value.erase(std::find(value.begin(), value.end(), '\0'), value.end());
            return QString::fromStdString(value).trimmed();
        }
    }
    return "N/A";
}


DicomLoader::DicomMetadata DicomLoader::extractMetadata(const QString &fileName)
{
    DicomMetadata metadata;

    // Default values
    metadata.patientName = "Unknown";
    metadata.patientID = "Unknown";
    metadata.studyDate = "Unknown";
    metadata.modality = "Unknown";
    metadata.institutionName = "Unknown";
    metadata.studyDescription = "Unknown";
    metadata.imageWidth = 0;
    metadata.imageHeight = 0;
    metadata.bitsStored = 0;
    metadata.acquisitionDate = "Unknown";

    if (!isDicomFile(fileName)) {
        qDebug() << "Not a DICOM file, returning empty metadata";
        return metadata;
    }
    gdcm::ImageReader reader;
    reader.SetFileName(fileName.toStdString().c_str());

    if (!reader.Read()) {
        qDebug() << "Failed to read DICOM for metadata extraction";
        return metadata;
    }
    const gdcm::DataSet& dataset = reader.GetFile().GetDataSet();

    // Extract standard DICOM tags
    metadata.patientName = extractTag(dataset, gdcm::Tag(0x0010, 0x0010));
    metadata.patientID = extractTag(dataset, gdcm::Tag(0x0010, 0x0020));
    metadata.studyDate = extractTag(dataset, gdcm::Tag(0x0008, 0x0020));
    metadata.modality = extractTag(dataset, gdcm::Tag(0x0008, 0x0060));
    metadata.institutionName = extractTag(dataset, gdcm::Tag(0x0008, 0x0080));
    metadata.studyDescription = extractTag(dataset, gdcm::Tag(0x0008, 0x1030));
    metadata.acquisitionDate = extractTag(dataset, gdcm::Tag(0x0008, 0x0022));

    // Get image dimensions
    const gdcm::Image &image = reader.GetImage();
    const unsigned int *dims = image.GetDimensions();
    metadata.imageWidth = dims[0];
    metadata.imageHeight = dims[1];
    metadata.bitsStored = image.GetPixelFormat().GetBitsStored();

    qDebug() << "Extracted metadata for:" << metadata.patientName;
    return metadata;
}

