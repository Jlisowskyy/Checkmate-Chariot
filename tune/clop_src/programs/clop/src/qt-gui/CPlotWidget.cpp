#include "CPlotWidget.h"
#include "MainWindow.h"
#if 1
#include "ui_MainWindow.h"
#endif
#include "CExperimentFromSettings.h"
#include "CParameter.h"

#include <QPainter>
#include <QMouseEvent>
#include <sstream>

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
CPlotWidget::CPlotWidget(QWidget *parent):
 QWidget(parent),
 mw(0),
 backgroundColor(192, 192, 255)
{
 setAttribute(Qt::WA_OpaquePaintEvent, true);
 outcomeColor[COutcome::Loss] = Qt::black;
 outcomeColor[COutcome::Win] = Qt::white;
 outcomeColor[COutcome::Draw] = QColor(128, 128, 128);
 outcomeColor[COutcome::Unknown] = Qt::yellow;
 outcomeColor[COutcome::InProgress] = Qt::red;
}

/////////////////////////////////////////////////////////////////////////////
// Paint
/////////////////////////////////////////////////////////////////////////////
void CPlotWidget::paintEvent(QPaintEvent *event)
{
 QPainter painter(this);
 painter.fillRect(0, 0, width(), height(), backgroundColor);

 if (mw && mw->pexperiment)
 {
  const CResults &results = mw->pexperiment->results;

  int ix = mw->ui->comboX->currentIndex() - 1;
  int iy = mw->ui->comboY->currentIndex() - 1;

  int PointSize = mw->ui->pointSizeSlider->value();

  unsigned first = mw->ui->firstSlider->value();
  unsigned last = mw->ui->lastSlider->value();

  //
  // Loop over samples
  //
  for (unsigned i = first; i < last; i++)
  {
   //
   // Merge replications
   //
   bool fMerge = false;
   if (mw->ui->mergeReplicationsCheckBox->isChecked() &&
       results.GetOutcome(i) != COutcome::Unknown &&
       results.GetOutcome(i) != COutcome::InProgress)
   {
    while (i < last - 1 &&
           results.GetOutcome(i + 1) != COutcome::Unknown &&
           results.GetOutcome(i + 1) != COutcome::InProgress &&
           mw->pexperiment->reg.GetSampleData(i + 1).Index != int(i + 1))
    {
     i++;
     fMerge = true;
    }
   }

   //
   // Color of this point
   //
   QColor col;

   if (fMerge)
   {
    const CSampleData &sdi = mw->pexperiment->reg.GetSampleData(i);
    const CSampleData &sd = mw->pexperiment->reg.GetSampleData(sdi.Index);
    const int c = int(255.0 * (sd.tCount[COutcome::Win] +
                         0.5 * sd.tCount[COutcome::Draw]) / sd.GetCount());
    col = QColor(c, c, c);
   }
   else
    col = outcomeColor[results.GetOutcome(i)];

   //
   // Optional weight
   //
   const double *v = results.GetSample(i);
   if (mw->ui->sampleWeightCheckBox->isChecked() &&
       results.GetOutcome(i) != COutcome::InProgress)
   {
    double x = mw->pexperiment->reg.GetSampleData(mw->pexperiment->reg.GetSampleData(i).Index).Weight;
    if (x > 1.0)
     x = 1.0;
    int Weight = x * 256;
    if (Weight == 0)
     continue;

    int r, g, b, a;
    col.getRgb(&r, &g, &b, &a);

    int r0, g0, b0, a0;
    backgroundColor.getRgb(&r0, &g0, &b0, &a0);

    col.setRgb((r * Weight + r0 * (256 - Weight)) >> 8,
               (g * Weight + g0 * (256 - Weight)) >> 8,
               (b * Weight + b0 * (256 - Weight)) >> 8,
               a0);
   }

   //
   // Point coordinates
   //
   int xx;
   if (ix >= 0)
    xx = int((v[ix] + 1.0) * 0.5 * width());
   else
    xx = (width() * (i - first + 0.5)) / (last - first);

   int yy;
   if (iy >= 0)
    yy = int((-v[iy] + 1.0) * 0.5 * height());
   else
    yy = height() - (height() * (i - first + 0.5)) / (last - first);


   //
   // Plot point
   //
   painter.fillRect(xx - PointSize / 2,
                    yy - PointSize / 2,
                    PointSize,
                    PointSize,
                    col);
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
// Display cursor coordinates in status bar
/////////////////////////////////////////////////////////////////////////////
void CPlotWidget::mouseMoveEvent(QMouseEvent *event)
{
 if (mw && mw->pexperiment)
 {
  const CParameterCollection &paramcol = mw->pexperiment->paramcol;

  int xx = event->x();
  int yy = event->y();

  int first = mw->ui->firstSlider->value();
  int last = mw->ui->lastSlider->value();
  if (first > last)
   last = first;

  std::ostringstream oss;

  int ix = mw->ui->comboX->currentIndex() - 1;
  if (ix >= 0)
  {
   double x = (2.0 * double(xx) / double(width())) - 1.0;
   x = paramcol.GetParam(ix).TransformFromQLR(x);
   oss << paramcol.GetParam(ix).GetName() << " = " << x;
  }
  else
   oss << "N = " << first + (xx * (last - first)) / width();

  oss << "; ";

  int iy = mw->ui->comboY->currentIndex() - 1;
  if (iy >= 0)
  {
   double y = 1.0 - (2.0 * double(yy) / double(height()));
   y = paramcol.GetParam(iy).TransformFromQLR(y);
   oss << paramcol.GetParam(iy).GetName() << " = " << y;
  }
  else
   oss << "N = " << last + (yy * (first - last)) / height();

  mw->ui->statusBar->showMessage(oss.str().c_str());
 }
}
