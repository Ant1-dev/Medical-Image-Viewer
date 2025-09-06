#include "imageviewer.h"
#include <QtWidgets/qscrollbar.h>

ImageViewer::ImageViewer(QWidget *parent)
    : QGraphicsView(parent), isPanning(false)
{
    setDragMode(QGraphicsView::NoDrag);
}

void ImageViewer::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isPanning = true;
        lastPanPoint = event->pos();
        setCursor(Qt::ClosedHandCursor);
    }
    QGraphicsView::mousePressEvent(event);
}

void ImageViewer::mouseMoveEvent(QMouseEvent *event)
{
    if (isPanning) {
        QPoint delta = event->pos() - lastPanPoint;
        lastPanPoint = event->pos();

        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
    }
    QGraphicsView::mouseMoveEvent(event);
}

void ImageViewer::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isPanning = false;
        setCursor(Qt::ArrowCursor);
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
