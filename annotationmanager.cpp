#include "annotationmanager.h"
#include <QDebug>

AnnotationManager::AnnotationManager(QGraphicsScene *scene, QObject *parent)
    : QObject(parent), scene(scene), currentLine(nullptr), selectedLine(nullptr),
    drawingMode(false), isDrawingActive(false)
{
    // Set up line appearance
    normalLinePen = QPen(Qt::red, 2, Qt::SolidLine);
    selectedLinePen = QPen(Qt::yellow, 3, Qt::SolidLine);
    drawingLinePen = QPen(Qt::blue, 2, Qt::DashLine);
}

void AnnotationManager::setDrawingMode(bool enabled)
{
    drawingMode = enabled;

    if (!enabled && isDrawingActive) {
        cancelCurrentLine();
    }

    qDebug() << "Drawing mode:" << (enabled ? "ON" : "OFF");
}

bool AnnotationManager::isDrawingMode() const
{
    return drawingMode;
}

void AnnotationManager::startLine(const QPointF &startPoint)
{
    if (!drawingMode) return;

    lineStartPoint = startPoint;

    // Create temporary line for visual feedback
    currentLine = scene->addLine(startPoint.x(), startPoint.y(),
                                 startPoint.x(), startPoint.y(),
                                 drawingLinePen);

    isDrawingActive = true;
    qDebug() << "Started line at:" << startPoint;
}

void AnnotationManager::updateLine(const QPointF &currentPoint)
{
    if (!drawingMode || !isDrawingActive || !currentLine) return;

    // Update the temporary line to show current mouse position
    currentLine->setLine(lineStartPoint.x(), lineStartPoint.y(),
                         currentPoint.x(), currentPoint.y());
}

void AnnotationManager::finishLine(const QPointF &endPoint)
{
    if (!drawingMode || !isDrawingActive || !currentLine) return;

    // Convert temporary line to permanent annotation
    currentLine->setPen(normalLinePen);
    annotationLines.append(currentLine);

    qDebug() << "Finished line from:" << lineStartPoint << "to:" << endPoint;

    // Reset state
    currentLine = nullptr;
    isDrawingActive = false;

    emit lineCountChanged(annotationLines.size());
}

void AnnotationManager::cancelCurrentLine()
{
    if (currentLine) {
        scene->removeItem(currentLine);
        delete currentLine;
        currentLine = nullptr;
    }
    isDrawingActive = false;
}

void AnnotationManager::clearAllLines()
{
    // Remove all lines from scene and delete them
    for (QGraphicsLineItem *line : annotationLines) {
        scene->removeItem(line);
        delete line;
    }
    annotationLines.clear();
    selectedLine = nullptr;

    qDebug() << "Cleared all annotation lines";
    emit lineCountChanged(0);
    emit lineSelected(false);
}

void AnnotationManager::deleteSelectedLine()
{
    if (selectedLine) {
        scene->removeItem(selectedLine);
        annotationLines.removeOne(selectedLine);
        delete selectedLine;
        selectedLine = nullptr;

        qDebug() << "Deleted selected line";
        emit lineCountChanged(annotationLines.size());
        emit lineSelected(false);
    }
}

void AnnotationManager::selectLine(QGraphicsLineItem *line)
{
    // Deselect previous line
    if (selectedLine) {
        selectedLine->setPen(normalLinePen);
    }

    // Select new line
    selectedLine = line;
    if (selectedLine) {
        selectedLine->setPen(selectedLinePen);
        emit lineSelected(true);
    } else {
        emit lineSelected(false);
    }
}

int AnnotationManager::getLineCount() const
{
    return annotationLines.size();
}

void AnnotationManager::setLineColor(const QColor &color)
{
    normalLinePen.setColor(color);
    updateLineAppearance();
}

void AnnotationManager::setLineWidth(int width)
{
    normalLinePen.setWidth(width);
    selectedLinePen.setWidth(width + 1);
    updateLineAppearance();
}

void AnnotationManager::updateLineAppearance()
{
    // Update all existing lines with new appearance
    for (QGraphicsLineItem *line : annotationLines) {
        if (line != selectedLine) {
            line->setPen(normalLinePen);
        }
    }

    if (selectedLine) {
        selectedLine->setPen(selectedLinePen);
    }
}
