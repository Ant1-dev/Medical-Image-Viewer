#include "imageviewer.h"
#include "annotationmanager.h"
#include <QtWidgets/qscrollbar.h>

ImageViewer::ImageViewer(QWidget *parent)
    : QGraphicsView(parent)
    , scene(nullptr)
    , pixmapItem(nullptr)
    , isPanning(false)
    , drawingMode(false)
    , annotationManager(nullptr)
{
    setupScene();

    setDragMode(QGraphicsView::NoDrag);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
}

void ImageViewer::setupScene()
{
    if (scene) {
        delete scene;  // Clean up existing scene
    }

    scene = new QGraphicsScene(this);
    setScene(scene);
    scene->setBackgroundBrush(QBrush(Qt::darkGray));

    qDebug() << "Scene created and set:" << (scene != nullptr);
}

void ImageViewer::setAnnotationManager(AnnotationManager *manager)
{
    annotationManager = manager;
}

void ImageViewer::setDrawingMode(bool enabled)
{
    drawingMode = enabled;

    if (enabled) {
        setCursor(Qt::CrossCursor);
    } else {
        setCursor(Qt::ArrowCursor);
    }
}

void ImageViewer::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete && annotationManager) {
        annotationManager->deleteSelectedLine();
        qDebug() << "Deleted selected line via keyboard";
    }

    QGraphicsView::keyPressEvent(event);
}

void ImageViewer::mousePressEvent(QMouseEvent *event)
{
    if (!annotationManager) {
        QGraphicsView::mousePressEvent(event);
        return;
    }

    if (drawingMode && event->button() == Qt::LeftButton) {
        // Drawing mode start a new line
        QPointF scenePos = mapToScene(event->pos());
        annotationManager->startLine(scenePos);
    } else if (!drawingMode && event->button() == Qt::LeftButton) {
        // View mode check for line selection or start panning
        QPointF scenePos = mapToScene(event->pos());
        QGraphicsScene *actualScene = QGraphicsView::scene();

        if (actualScene) {
            // Check for line items at click position
            QRectF clickArea(scenePos.x() - 3, scenePos.y() - 3, 6, 6);
            QList<QGraphicsItem*> items = actualScene->items(clickArea);

            QGraphicsLineItem *clickedLine = nullptr;
            for (QGraphicsItem *item : items) {
                if (QGraphicsLineItem *lineItem = qgraphicsitem_cast<QGraphicsLineItem*>(item)) {
                    clickedLine = lineItem;
                    break;
                }
            }

            if (clickedLine) {
                // Line clicked select it
                annotationManager->selectLine(clickedLine);
            } else {
                // Empty space clicked deselect and start panning
                annotationManager->selectLine(nullptr);
                isPanning = true;
                lastPanPoint = event->pos();
                setCursor(Qt::ClosedHandCursor);
            }
        }
    }

    QGraphicsView::mousePressEvent(event);
}

void ImageViewer::mouseMoveEvent(QMouseEvent *event)
{
    if (drawingMode && annotationManager) {
        // Drawing mode
        QPointF scenePos = mapToScene(event->pos());
        annotationManager->updateLine(scenePos);
    } else if (isPanning && (event->buttons() & Qt::LeftButton)) {
        // View mode
        QPoint delta = event->pos() - lastPanPoint;
        lastPanPoint = event->pos();

        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
    }

    QGraphicsView::mouseMoveEvent(event);
}

void ImageViewer::mouseReleaseEvent(QMouseEvent *event)
{
    if (drawingMode && annotationManager && event->button() == Qt::LeftButton) {
        // Drawing mode finish the line
        QPointF scenePos = mapToScene(event->pos());
        annotationManager->finishLine(scenePos);
        qDebug() << "Finished line at scene position:" << scenePos;
    } else if (event->button() == Qt::LeftButton) {
        // View mode stop panning
        isPanning = false;
        setCursor(drawingMode ? Qt::CrossCursor : Qt::ArrowCursor);
    }

    QGraphicsView::mouseReleaseEvent(event);
}

void ImageViewer::wheelEvent(QWheelEvent *event)
{
    const double scaleFactor = 1.15;

    if (event->angleDelta().y() > 0) {
        if (transform().m11() < 10.0) {
            scale(scaleFactor, scaleFactor);
        }
    } else {
        if (transform().m11() > 0.1) {
            scale(1.0 / scaleFactor, 1.0 / scaleFactor);
        }
    }
    event->accept();
}
