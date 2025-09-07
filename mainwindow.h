#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QAction>
#include <QFileDialog>
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QSplitter>
#include <QSlider>
#include <QPushButton>
#include <QGroupBox>
#include <QGridLayout>
#include "imageviewer.h"
#include "dicomloader.h"
#include "annotationmanager.h"


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void createMenuBar();
    void openImage();
    void updateMetadataDisplay(const QString &fileName);
    void clearMetadataDisplay();
    void createAnnotationControls();
    void toggleDrawingMode();
    void clearAllAnnotations();
    void deleteSelectedAnnotation();
    void updateAnnotationStatus(int lineCount);
    void updateSelectionStatus(bool hasSelection);

    // image on screen
    ImageViewer *imageView;
    QGraphicsScene *scene;
    DicomLoader *dicomLoader;
    AnnotationManager *annotationManager;

    // display split and metadata
    QSplitter *mainSplitter;
    QTextEdit *metadataDisplay;

    // Annotation controls
    QGroupBox *annotationGroup;
    QPushButton *drawModeBtn;
    QPushButton *viewModeBtn;
    QPushButton *clearAllBtn;
    QPushButton *deleteSelectedBtn;
    QLabel *currentModeLabel;
    QLabel *instructionsLabel;
    QLabel *lineCountLabel;

    // Current image data
    QString currentFileName;
    bool isCurrentImageDicom;
    bool isDrawingMode;

};
#endif // MAINWINDOW_H
