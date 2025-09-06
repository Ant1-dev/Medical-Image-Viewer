#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QAction>
#include <QFileDialog>
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QSplitter>
#include "imageviewer.h"
#include "dicomloader.h"


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

    ImageViewer *imageView;
    QGraphicsScene *scene;
    DicomLoader *dicomLoader;

    // metadata display widgets
    QSplitter *mainSplitter;
    QTextEdit *metadataDisplay;
    QLabel *statusLabel;
};

#endif // MAINWINDOW_H
