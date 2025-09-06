#ifndef DICOMLOADER_H
#define DICOMLOADER_H

#include <QString>
#include <QPixmap>
#include <QImage>
#include <vector>

#include "gdcmTag.h"
#include "gdcmDataSet.h"


class DicomLoader
{
public:
    DicomLoader();

    // Main functions for DICOM handling
    bool isDicomFile(const QString &fileName);
    QPixmap loadDicomImage(const QString &fileName);

    struct DicomMetadata {
        QString patientName;
        QString patientID;
        QString studyDate;
        QString modality;
        QString institutionName;
        QString studyDescription;
        int imageWidth;
        int imageHeight;
        int bitsStored;
        QString acquisitionDate;
    };

    DicomMetadata extractMetadata(const QString &fileName);



private:
    // Internal helper functions
    QPixmap convertDicomToPixmap(const QString &fileName);
    QImage convertToQImage(const std::vector<char> &buffer, unsigned int width,
                           unsigned int height, int bitsStored);
    QString extractTag(const gdcm::DataSet& dataset, const gdcm::Tag& tag);
};

#endif // DICOMLOADER_H
