#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QGraphicsView>
#include <QMouseEvent>

class AnnotationManager;

class ImageViewer : public QGraphicsView
{
    Q_OBJECT

public:
    explicit ImageViewer(QWidget *parent = nullptr);
    bool loadImage(const QString &fileName);
    void setDrawingMode(bool enabled);
    void setAnnotationManager(AnnotationManager *manager);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void setupScene();
    void fitImageInView();

    QGraphicsScene *scene;
    QGraphicsPixmapItem *pixmapItem;

    // panning
    bool isPanning;
    QPoint lastPanPoint;

    // drawing
    bool drawingMode;
    AnnotationManager *annotationManager;
};

#endif // IMAGEVIEWER_H
