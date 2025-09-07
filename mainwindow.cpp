#include "mainwindow.h"
#include <QMenuBar>
#include <QAction>
#include <QFileDialog>
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), isCurrentImageDicom(false), isDrawingMode(false)
{
    // Create graphics components
    scene = new QGraphicsScene(this);
    imageView = new ImageViewer(this);
    imageView->setScene(scene);

    // Create DICOM loader
    dicomLoader = new DicomLoader();

    // Create annotation manager
    annotationManager = new AnnotationManager(scene, this);

    // Connect annotation manager to image viewer
    imageView->setAnnotationManager(annotationManager);

    // Connect annotation manager signals
    connect(annotationManager, &AnnotationManager::lineCountChanged,
            this, &MainWindow::updateAnnotationStatus);
    connect(annotationManager, &AnnotationManager::lineSelected,
            this, &MainWindow::updateSelectionStatus);

    // Create metadata display
    metadataDisplay = new QTextEdit(this);
    metadataDisplay->setMaximumWidth(250);
    metadataDisplay->setReadOnly(true);
    metadataDisplay->setFont(QFont("Arial", 13));

    // Create annotation controls
    createAnnotationControls();

    // Create right panel with metadata and controls
    QWidget *rightPanel = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->addWidget(metadataDisplay);
    rightLayout->addWidget(annotationGroup);
    rightLayout->addStretch();

    // Create splitter
    mainSplitter = new QSplitter(Qt::Horizontal, this);
    mainSplitter->addWidget(imageView);
    mainSplitter->addWidget(rightPanel);
    mainSplitter->setSizes({750, 250});

    setCentralWidget(mainSplitter);
    createMenuBar();

    setWindowTitle("Medical Image Annotation Tool");
    resize(1000, 700);

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

void MainWindow::createAnnotationControls()
{
    annotationGroup = new QGroupBox("Annotation Tools", this);
    QVBoxLayout *layout = new QVBoxLayout(annotationGroup);

    // mode selection
    QLabel *modeLabel = new QLabel("Mode:", this);
    layout->addWidget(modeLabel);

    QHBoxLayout *modeLayout = new QHBoxLayout();
    viewModeBtn = new QPushButton("View Mode", this);
    viewModeBtn->setCheckable(true);
    viewModeBtn->setChecked(true);

    drawModeBtn = new QPushButton("Draw Mode", this);
    drawModeBtn->setCheckable(true);

    modeLayout->addWidget(viewModeBtn);
    modeLayout->addWidget(drawModeBtn);
    layout->addLayout(modeLayout);

    // Current mode display
    currentModeLabel = new QLabel("Current: View Mode", this);
    currentModeLabel->setStyleSheet("QLabel { font-weight: bold; color: blue; }");
    layout->addWidget(currentModeLabel);

    // Line count display
    lineCountLabel = new QLabel("Lines: 0", this);
    lineCountLabel->setStyleSheet("QLabel { font-weight: bold; }");
    layout->addWidget(lineCountLabel);

    // Instructions
    instructionsLabel = new QLabel(
        "View Mode:\n"
        "• Pan & Zoom\n"
        "• Click line to select\n"
        "• Delete key to remove\n\n"
        "Draw Mode:\n"
        "• Click & drag to draw lines", this);
    instructionsLabel->setWordWrap(true);
    instructionsLabel->setStyleSheet("QLabel { font-size: 10px; color: gray; }");
    layout->addWidget(instructionsLabel);

    // Action buttons
    layout->addWidget(new QLabel("Actions:", this));

    clearAllBtn = new QPushButton("Clear All Lines", this);
    clearAllBtn->setEnabled(false);  // Disabled until lines exist
    layout->addWidget(clearAllBtn);

    deleteSelectedBtn = new QPushButton("Delete Selected", this);
    deleteSelectedBtn->setEnabled(false);  // Disabled until line selected
    layout->addWidget(deleteSelectedBtn);

    // Connect
    connect(viewModeBtn, &QPushButton::clicked, this, &MainWindow::toggleDrawingMode);
    connect(drawModeBtn, &QPushButton::clicked, this, &MainWindow::toggleDrawingMode);
    connect(clearAllBtn, &QPushButton::clicked, this, &MainWindow::clearAllAnnotations);
    connect(deleteSelectedBtn, &QPushButton::clicked, this, &MainWindow::deleteSelectedAnnotation);
}

void MainWindow::toggleDrawingMode()
{

    if (viewModeBtn->isChecked()) {
        isDrawingMode = false;
        drawModeBtn->setChecked(false);
        currentModeLabel->setText("Current: View Mode");
        currentModeLabel->setStyleSheet("QLabel { font-weight: bold; color: blue; }");
        // Tell annotation manager and image view
        annotationManager->setDrawingMode(false);
        imageView->setDrawingMode(false);
    } else {
        isDrawingMode = true;
        viewModeBtn->setChecked(false);
        currentModeLabel->setText("Current: Draw Mode");
        currentModeLabel->setStyleSheet("QLabel { font-weight: bold; color: red; }");
        // Tell annotation manager and image view
        annotationManager->setDrawingMode(true);
        imageView->setDrawingMode(true);
    }
}

void MainWindow::clearAllAnnotations()
{
    annotationManager->clearAllLines();
}

void MainWindow::deleteSelectedAnnotation()
{
    annotationManager->deleteSelectedLine();
}

void MainWindow::updateAnnotationStatus(int lineCount)
{
    lineCountLabel->setText(QString("Lines: %1").arg(lineCount));
    clearAllBtn->setEnabled(lineCount > 0);
}

void MainWindow::updateSelectionStatus(bool hasSelection)
{
    deleteSelectedBtn->setEnabled(hasSelection);
}

