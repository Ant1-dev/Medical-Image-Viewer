#ifndef ANNOTATIONMANAGER_H
#define ANNOTATIONMANAGER_H

#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QPointF>
#include <QColor>
#include <QPen>
#include <QList>
#include <QMouseEvent>

class AnnotationManager : public QObject
{
    Q_OBJECT

public:
    explicit AnnotationManager(QGraphicsScene *scene, QObject *parent = nullptr);

    // Mode management
    void setDrawingMode(bool enabled);
    bool isDrawingMode() const;

    //Line drawing
    void startLine(const QPointF &startPoint);
    void updateLine(const QPointF &currentPoint);
    void finishLine(const QPointF &endPoint);
    void cancelCurrentLine();

    // Line management
    void clearAllLines();
    void deleteSelectedLine();
    void selectLine(QGraphicsLineItem *line);
    int getLineCount() const;

    //Line appearance
    void setLineColor(const QColor &color);
    void setLineWidth(int width);

signals:
    void lineCountChanged(int count);
    void lineSelected(bool hasSelection);

private:
    QGraphicsScene *scene;
    QList<QGraphicsLineItem*> annotationLines;
    QGraphicsLineItem *currentLine;
    QGraphicsLineItem *selectedLine;

    bool drawingMode;
    bool isDrawingActive;
    QPointF lineStartPoint;

    // appearance settings
    QPen normalLinePen;
    QPen selectedLinePen;
    QPen drawingLinePen;

    void updateLineAppearance();
};


#endif // ANNOTATIONMANAGER_H
