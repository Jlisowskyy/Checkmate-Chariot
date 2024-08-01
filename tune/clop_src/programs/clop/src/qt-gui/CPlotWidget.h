#ifndef CPLOTWIDGET_H
#define CPLOTWIDGET_H

#include <QWidget>
#include <QColor>

#include "COutcome.h"

class MainWindow;

class CPlotWidget : public QWidget
{
Q_OBJECT

    const MainWindow *mw;
    QColor backgroundColor;
    QColor outcomeColor[COutcome::AllOutcomes];

public:
    explicit CPlotWidget(QWidget *parent = 0);
    void setMainWindow(const MainWindow *mwNew) {mw = mwNew;}

protected:
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent( QMouseEvent *event);
};

#endif // CPLOTWIDGET_H
