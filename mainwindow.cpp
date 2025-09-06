#include "mainwindow.h"
#include <QMenuBar>
#include <QAction>
#include <QFileDialog>
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Window properties
    setWindowTitle("Medical Image Viewer");
    resize(1000, 700);

    // graphics image components
    scene = new QGraphicsScene(this);
    imageView = new ImageViewer(this);
    imageView->setScene(scene);

    // DICOM loader
    dicomLoader = new DicomLoader();

    // metadata display
    metadataDisplay = new QTextEdit(this);
    metadataDisplay->setMaximumWidth(300);
    metadataDisplay->setReadOnly(true);
    metadataDisplay->setFont(QFont("Courier", 15));

    // splitter to divide image and data
    mainSplitter = new QSplitter(Qt::Horizontal, this);
    mainSplitter->addWidget(imageView);
    mainSplitter->addWidget(metadataDisplay);
    mainSplitter->setSizes({600, 400});


    setCentralWidget(mainSplitter);

    createMenuBar();

    clearMetadataDisplay();
}

MainWindow::~MainWindow()
{

}

void MainWindow::createMenuBar()
{
    QMenu *fileMenu = menuBar()->addMenu("File");
    QAction *openAction = new QAction("Open Image...", this);

    connect(openAction, &QAction::triggered, this, &MainWindow::openImage);

    fileMenu->addAction(openAction);
}

void MainWindow::openImage()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Open Medical Image",
        "",
        "Image Files (*.png *.PNG *.jpg *.JPG *.jpeg *.JPEG *.bmp *.BMP *.dcm *.DCM *.dicom *.DICOM)"
    );

    if (!fileName.isEmpty()) {
        QPixmap pixmap;

        if (dicomLoader->isDicomFile(fileName)) {
            qDebug() << "Loading DICOM file:" << fileName;
            pixmap = dicomLoader->loadDicomImage(fileName);
        } else {
            qDebug() << "Loading standard image:" << fileName;
            pixmap = QPixmap(fileName);
        }

        if (!pixmap.isNull()) {
            scene->clear();
            scene->addPixmap(pixmap);
            imageView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);

            updateMetadataDisplay(fileName);

            qDebug() << "Loaded image:" << fileName;
        } else {
            QMessageBox::warning(this, "Error", "Failed to load image: " + fileName);
            clearMetadataDisplay();
            qDebug() << "Failed to load image:" << fileName;
        }
    }
}

void MainWindow::updateMetadataDisplay(const QString &fileName)
{
    if (dicomLoader->isDicomFile(fileName)) {
        DicomLoader::DicomMetadata metadata = dicomLoader->extractMetadata(fileName);

        QString displayText;
        displayText += "=== DICOM METADATA ===\n\n";
        displayText += QString("Patient Name: %1\n").arg(metadata.patientName);
        displayText += QString("Patient ID: %1\n").arg(metadata.patientID);
        displayText += QString("Study Date: %1\n").arg(metadata.studyDate);
        displayText += QString("Acquisition Date: %1\n").arg(metadata.acquisitionDate);
        displayText += QString("Modality: %1\n").arg(metadata.modality);
        displayText += QString("Institution: %1\n").arg(metadata.institutionName);
        displayText += QString("Study Description: %1\n\n").arg(metadata.studyDescription);

        displayText += "=== IMAGE PROPERTIES ===\n\n";
        displayText += QString("Dimensions: %1 x %2\n").arg(metadata.imageWidth).arg(metadata.imageHeight);
        displayText += QString("Bits Stored: %1\n").arg(metadata.bitsStored);
        displayText += QString("Total Pixels: %1\n").arg(metadata.imageWidth * metadata.imageHeight);

        displayText += "\n=== FILE INFO ===\n\n";
        displayText += QString("File Name: %1\n").arg(QFileInfo(fileName).fileName());
        displayText += QString("File Size: %1 KB\n").arg(QFileInfo(fileName).size() / 1024);

        metadataDisplay->setPlainText(displayText);
    } else {
        QPixmap pixmap(fileName);
        QString displayText;
        displayText += "=== STANDARD IMAGE ===\n\n";
        displayText += QString("Dimensions: %1 x %2\n").arg(pixmap.width()).arg(pixmap.height());
        displayText += QString("Format: %1\n").arg(QFileInfo(fileName).suffix().toUpper());
        displayText += QString("File Name: %1\n").arg(QFileInfo(fileName).fileName());
        displayText += QString("File Size: %1 KB\n").arg(QFileInfo(fileName).size() / 1024);

        metadataDisplay->setPlainText(displayText);
    }
}

void MainWindow::clearMetadataDisplay() {
    QString welcomeText;
    welcomeText += "=== MEDICAL IMAGE VIEWER ===\n\n";
    welcomeText += "DICOM Support: Enabled\n";
    welcomeText += "Supported Formats:\n";
    welcomeText += "• DICOM (.dcm, .dicom)\n";
    welcomeText += "• PNG, JPEG, BMP\n\n";
    welcomeText += "Features:\n";
    welcomeText += "• Pan: Click & Drag\n";
    welcomeText += "• Zoom: Mouse Wheel\n";
    welcomeText += "• Metadata: Auto-displayed\n\n";
    welcomeText += "Load an image to begin...";

    metadataDisplay->setPlainText(welcomeText);
}

