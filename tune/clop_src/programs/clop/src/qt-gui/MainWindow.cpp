#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "AboutDialog.h"
#include "CRAIICursor.h"
#include "CExperimentSettings.h"
#include "CExperimentFromSettings.h"
#include "CCLOPSyntaxHighlighter.h"
#include "CParameter.h"
#include "CMESampleMean.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QSettings>
#include <QLabel>
#include <QCloseEvent>
#include <QScrollBar>
#include <QPainter>
#include <QProgressDialog>

#include <iostream>
#include <sstream>

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
MainWindow::MainWindow(QWidget *parent):
 QMainWindow(parent),
 ui(new Ui::MainWindow),
 pes(0),
 pexperiment(0)
{
 ui->setupUi(this);
 ui->plotWidget->setMainWindow(this);

 //
 // Create actions for recent files
 //
 separatorAct = ui->menuFile->addSeparator();
 for (int i = 0; i < MaxRecentFiles; ++i)
 {
  recentFileActs[i] = new QAction(this);
  recentFileActs[i]->setVisible(false);
  connect(recentFileActs[i], SIGNAL(triggered()), SLOT(onOpenRecentFile()));
  ui->menuFile->addAction(recentFileActs[i]);
 }
 ui->menuFile->addSeparator();
 updateRecentFileActions();

 //
 // Setup syntax highlighter for QLR files
 //
 new CCLOPSyntaxHighlighter(ui->textEdit);

 //
 // Counter label
 //
 counterLabel = new QLabel();
 ui->statusBar->addPermanentWidget(counterLabel);

 //
 // Set initial state
 //
 statusLabel = new QLabel();
 ui->statusBar->addPermanentWidget(statusLabel);
 setState(Opening);
}

/////////////////////////////////////////////////////////////////////////////
MainWindow::~MainWindow()
/////////////////////////////////////////////////////////////////////////////
{
 deleteExperiment();
 delete ui;
}

/////////////////////////////////////////////////////////////////////////////
void MainWindow::deleteExperiment()
/////////////////////////////////////////////////////////////////////////////
{
 if (pexperiment)
  delete pexperiment;
 if (pes)
  delete pes;
 pes = 0;
 pexperiment = 0;
 setState(Opening);
}

/////////////////////////////////////////////////////////////////////////////
// Load a file
/////////////////////////////////////////////////////////////////////////////
void MainWindow::loadFile(const QString &fileName)
{
 if (!checkRunning())
  return;

 CRAIICursor cursor(Qt::WaitCursor);

 //
 // Remove from the recent file list
 //
 QSettings settings;
 QStringList files = settings.value("recentFileList").toStringList();
 files.removeAll(fileName);
 settings.setValue("recentFileList", files);
 updateRecentFileActions();

 //
 // Try to open file
 //
 QFile file(fileName);
 if (!file.open(QFile::ReadOnly | QFile::Text))
 {
  QMessageBox::warning(this,
                       tr("Error"),
                       tr("Cannot read file %1:\n%2.")
                       .arg(fileName)
                       .arg(file.errorString()));
  return;
 }
 setWindowTitle(QFileInfo(fileName).fileName());

 //
 // Read the file into the text Field
 //
 QTextStream in(&file);
 ui->textEdit->setPlainText(in.readAll());

 //
 // Store at the front of the recent file list
 //
 files.prepend(fileName);
 while (files.size() > MaxRecentFiles)
  files.removeLast();
 settings.setValue("recentFileList", files);
 updateRecentFileActions();

 //
 // Build experiment based on file name
 //
 std::ifstream ifs(fileName.toStdString().c_str());
 deleteExperiment();
 QDir::setCurrent(QFileInfo(fileName).absolutePath());
 pes = new CExperimentSettings(ifs);
 pexperiment = new CExperimentFromSettings(*pes);

 //
 // Connections for state management
 //
 connect(&pexperiment->rexp,
         SIGNAL(Finished()),
         SLOT(onRexpReady()));
 connect(&pexperiment->rexp,
         SIGNAL(Started()),
         SLOT(onRexpRunning()));

 //
 // Connections for GUI update
 //
 updateConnections();
 connect(&pexperiment->rexp,
         SIGNAL(Sample()),
         SLOT(updateCounter()));

 //
 // Progress dialog
 //
 QProgressDialog progress("Loading data...", 0, 0, 0, this);
 progress.setWindowModality(Qt::WindowModal);
 connect(&pexperiment->rexp, SIGNAL(StartLoading(int)),
         &progress,SLOT(setMaximum(int)));
 connect(&pexperiment->rexp, SIGNAL(LoadingProgress(int)),
         &progress, SLOT(setValue(int)));
 //connect(&progress, SIGNAL(canceled()),
 //        &pexperiment->rexp, SLOT(AbortLoading()));

 //
 // Build list of parameter names
 //
 CParameterCollection &paramcol = pexperiment->paramcol;

 slNames.clear();
 for (int i = 0; i < paramcol.GetSize(); i++)
  slNames << QString(paramcol.GetParam(i).GetName().c_str());

 //
 // Start logs (and load data)
 //
 pexperiment->QuickLoad();

 //
 // Fill combo boxes with parameter names
 //
 ui->comboX->clear();
 ui->comboY->clear();
 ui->comboX->addItem("Time");
 ui->comboY->addItem("Time");
 ui->comboX->addItems(slNames);
 ui->comboY->addItems(slNames);

 ui->comboX->setCurrentIndex(1);
 if (paramcol.GetSize() > 1)
  ui->comboY->setCurrentIndex(2);
 else
  ui->comboY->setCurrentIndex(0);

 //
 // Setup Hessian display
 //
 ui->matrixTable->setRowCount(paramcol.GetSize());
 ui->matrixTable->setColumnCount(paramcol.GetSize());
 ui->matrixTable->setHorizontalHeaderLabels(slNames);
 ui->matrixTable->setVerticalHeaderLabels(slNames); 
 ui->matrixTable->resizeRowsToContents();
 ui->matrixTable->verticalHeader()->setResizeMode(QHeaderView::Fixed);
 ui->matrixTable->horizontalHeader()->setResizeMode(QHeaderView::Fixed);

 //
 // Setup eigenvectors display
 //
 ui->eigenTable->setRowCount(paramcol.GetSize() + 2);
 ui->eigenTable->setColumnCount(paramcol.GetSize());
 ui->eigenTable->setVerticalHeaderLabels(QStringList() << "Eigenvalue" << "" << slNames);
 ui->eigenTable->resizeRowsToContents();
 ui->eigenTable->verticalHeader()->setResizeMode(QHeaderView::Fixed);
 ui->eigenTable->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
 for (int i = paramcol.GetSize(); --i >= 0;)
 {
  QTableWidgetItem *item = new QTableWidgetItem();
  item->setFlags(0);
  ui->eigenTable->setItem(1, i, item);
 }

 //
 // Setup max display
 //
 ui->maxTable->clear();
 ui->maxTable->setRowCount(paramcol.GetSize() + 8);
 ui->maxTable->setColumnCount(2);
 {
  QStringList sl;
  sl << "Mean";
  sl << "Max";
  ui->maxTable->setHorizontalHeaderLabels(sl);
 }
 ui->maxTable->setVerticalHeaderLabels(QStringList() <<
  slNames << " " << "95% UCB" << "Elo" << "95% LCB" <<
             " " << "95% UCB" << "Win Rate" << "95% LCB");
 ui->maxTable->resizeRowsToContents();

 //
 // Setup win rate display
 //
 ui->winRateTable->clear();
 ui->winRateTable->setRowCount(12);
 ui->winRateTable->setColumnCount(3);
 ui->winRateTable->setHorizontalHeaderLabels(QStringList() <<
  "All" << "Weighted" << "Central");
 ui->winRateTable->setVerticalHeaderLabels(QStringList() <<
  "Wins" << "Draws" << "Losses" << "Games" <<
  " " << "95% UCB" << "Elo" << "95% LCB" <<
  " " << "95% UCB" << "Win Rate" << "95% LCB");
 ui->winRateTable->resizeRowsToContents();

 //
 // Set range for first and last sample, in plot
 //
 {
  int N = pexperiment->results.GetSamples();
  ui->lastSlider->setMaximum(N);
  ui->lastSpinBox->setMaximum(N);
  ui->firstSlider->setMaximum(N);
  ui->firstSpinBox->setMaximum(N);
  ui->firstSlider->setValue(0);
  ui->firstSpinBox->setValue(0);
  ui->lastSlider->setValue(N);
  ui->lastSpinBox->setValue(N);
 }

 //
 // Get ready
 //
 //ui->tabWidget->setCurrentIndex(0);
 setState(Ready);
 updateAll();
}

/////////////////////////////////////////////////////////////////////////////
// Set automatic GUI update depending on menu option
/////////////////////////////////////////////////////////////////////////////
void MainWindow::updateConnections()
{
 if (pexperiment)
 {
  if (ui->action_Automatic_GUI_update->isChecked())
  {
   connect(&pexperiment->rexp,
           SIGNAL(Message(std::string)),
           SLOT(onRexpMessage(std::string)));
   connect(&pexperiment->rexp,
           SIGNAL(Sample()),
           SLOT(updatePlot()));
   connect(&pexperiment->rexp,
           SIGNAL(Result()),
           SLOT(updateAll()));
  }
  else
  {
   disconnect(&pexperiment->rexp,
             SIGNAL(Message(std::string)),
             this,
             SLOT(onRexpMessage(std::string)));
   disconnect(&pexperiment->rexp,
              SIGNAL(Sample()),
              this,
              SLOT(updatePlot()));
   disconnect(&pexperiment->rexp,
              SIGNAL(Result()),
              this,
              SLOT(updateAll()));
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
// Update all GUI elements
/////////////////////////////////////////////////////////////////////////////
void MainWindow::updateAll()
{
 updateCounter();
 updateStatistics();
 updatePlot();
 displayHessian();
 displayEigen();
 displayMax();
}

/////////////////////////////////////////////////////////////////////////////
// Update counter
/////////////////////////////////////////////////////////////////////////////
void MainWindow::updateCounter()
{
 if (pexperiment)
  counterLabel->setText(QString::number(pexperiment->results.GetSamples()));
 else
  counterLabel->setText("N/A");
}

/////////////////////////////////////////////////////////////////////////////
// Update menu with recent files
/////////////////////////////////////////////////////////////////////////////
void MainWindow::updateRecentFileActions()
{
 QSettings settings;
 QStringList files = settings.value("recentFileList").toStringList();

 int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

 for (int i = 0; i < numRecentFiles; ++i)
 {
  QString text = tr("&%1 %2").arg(i + 1).arg(files[i]);
  recentFileActs[i]->setText(text);
  recentFileActs[i]->setData(files[i]);
  recentFileActs[i]->setVisible(true);
 }

 for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
  recentFileActs[j]->setVisible(false);

 separatorAct->setVisible(numRecentFiles > 0);
}

/////////////////////////////////////////////////////////////////////////////
// State
/////////////////////////////////////////////////////////////////////////////
void MainWindow::setState(State stateNew)
{
 switch(stateNew)
 {
  case Opening: /////////////////////////////////////////////////////////////
   statusLabel->setText("No experiment loaded");
   updateCounter();
   ui->tabWidget->setVisible(false);
   ui->action_Close->setDisabled(true);
   ui->action_Start->setDisabled(true);
   ui->action_SoftStop->setDisabled(true);
   ui->action_HardStop->setDisabled(true);
  break;

  case Ready: ///////////////////////////////////////////////////////////////
   statusLabel->setText("Ready");
   ui->tabWidget->setVisible(true);
   ui->listWidget->scrollToBottom();
   ui->action_Close->setDisabled(false);
   ui->action_Start->setDisabled(false);
   ui->action_SoftStop->setDisabled(true);
   ui->action_HardStop->setDisabled(true);
  break;

  case Running: /////////////////////////////////////////////////////////////
   statusLabel->setText("Running");   
   ui->action_Start->setDisabled(true);
   ui->action_SoftStop->setDisabled(false);
   ui->action_HardStop->setDisabled(false);
  break;

  case Waiting: /////////////////////////////////////////////////////////////
   statusLabel->setText("Waiting");
   ui->action_Start->setDisabled(true);
   ui->action_SoftStop->setDisabled(true);
   ui->action_HardStop->setDisabled(false);
  break;
 }

 state = stateNew;
}

/////////////////////////////////////////////////////////////////////////////
// Ask for confirmation of any operation that would require a hard stop
/////////////////////////////////////////////////////////////////////////////
bool MainWindow::checkRunning()
{
 if (state == Running || state == Waiting)
  return
   (QMessageBox::warning
    (
     this,
     "Warning",
     "Experiment is running. Do you really wish to stop it now?",
     QMessageBox::Yes | QMessageBox::No
    ) == QMessageBox::Yes
   );
 else
  return true;
}

/////////////////////////////////////////////////////////////////////////////
// Slot for recent files
/////////////////////////////////////////////////////////////////////////////
void MainWindow::onOpenRecentFile()
{
 QAction *action = qobject_cast<QAction *>(sender());
 if (action)
  loadFile(action->data().toString());
}

/////////////////////////////////////////////////////////////////////////////
void MainWindow::on_action_About_triggered()
/////////////////////////////////////////////////////////////////////////////
{
 AboutDialog *about = new AboutDialog(this);
 about->adjustSize();
 about->show();
}

/////////////////////////////////////////////////////////////////////////////
void MainWindow::closeEvent(QCloseEvent * event)
/////////////////////////////////////////////////////////////////////////////
{
 if (checkRunning())
 {
  deleteExperiment();
  event->accept();
 }
 else
  event->ignore();
}

/////////////////////////////////////////////////////////////////////////////
void MainWindow::on_action_Open_triggered()
/////////////////////////////////////////////////////////////////////////////
{
 if (!checkRunning())
  return;

 QString fileName = QFileDialog::getOpenFileName
 (
  this,
  "Open Experiment",
  0,
  "CLOP Files (*.clop *.qlr)"
 );
 if (!fileName.isNull())
 {
  deleteExperiment();
  loadFile(fileName);
 }
}

/////////////////////////////////////////////////////////////////////////////
void MainWindow::on_action_Close_triggered()
/////////////////////////////////////////////////////////////////////////////
{
 if (checkRunning())
  deleteExperiment();
}

/////////////////////////////////////////////////////////////////////////////
void MainWindow::on_action_Start_triggered()
/////////////////////////////////////////////////////////////////////////////
{
 if (pexperiment)
  pexperiment->rexp.Run();
}

/////////////////////////////////////////////////////////////////////////////
void MainWindow::on_action_SoftStop_triggered()
/////////////////////////////////////////////////////////////////////////////
{
 if (pexperiment)
 {
  pexperiment->rexp.SoftStop();
  setState(Waiting);
 }
}

/////////////////////////////////////////////////////////////////////////////
void MainWindow::on_action_HardStop_triggered()
/////////////////////////////////////////////////////////////////////////////
{
 if (pexperiment)
 {
  pexperiment->rexp.HardStop();
  setState(Waiting);
 }
}

/////////////////////////////////////////////////////////////////////////////
// Add line to log
/////////////////////////////////////////////////////////////////////////////
void MainWindow::onRexpMessage(std::string sMessage)
{
 messageBuffer.push_back(sMessage);

 if (!ui->listWidget->verticalScrollBar()->isSliderDown())
 {
  while (!messageBuffer.empty())
  {
   ui->listWidget->addItem(messageBuffer.front().c_str());
   messageBuffer.pop_front();
  }

  const int MaxLogLines = 100;
  while (ui->listWidget->count() > MaxLogLines)
   delete ui->listWidget->item(0);

  ui->listWidget->scrollToBottom();
 }
}

/////////////////////////////////////////////////////////////////////////////
// Update plot
/////////////////////////////////////////////////////////////////////////////
void MainWindow::updatePlot()
{
 if (pexperiment)
 {
  unsigned N = pexperiment->results.GetSamples();
  bool fAutoLast = (ui->lastSlider->value() == ui->lastSlider->maximum());

  ui->lastSlider->setMaximum(N);
  ui->lastSpinBox->setMaximum(N);
  ui->firstSlider->setMaximum(N);
  ui->firstSpinBox->setMaximum(N);
  if (fAutoLast)
   ui->lastSlider->setValue(N);
  ui->plotWidget->update();
 }
}

/////////////////////////////////////////////////////////////////////////////
void MainWindow::addTableItem(int i, int j, double x)
/////////////////////////////////////////////////////////////////////////////
{
 QTableWidgetItem *item = new QTableWidgetItem(QString::number(x));
 item->setFlags(Qt::ItemIsEnabled);
 ui->winRateTable->setItem(i, j, item);
}

/////////////////////////////////////////////////////////////////////////////
void MainWindow::updateWinRate(int Column, double W, double D, double L)
/////////////////////////////////////////////////////////////////////////////
{
 double Total = W + D + L;
 double Score = W + 0.5 * D;
 double Rate = Score / Total;

 double TotalVariance = W * (1.0 - Rate) * (1.0 - Rate) +
                        D * (0.5 - Rate) * (0.5 - Rate) +
                        L * (0.0 - Rate) * (0.0 - Rate);
 double Margin = 1.96 * std::sqrt(TotalVariance) / Total;

 const double EloMul = 400.0 / std::log(10.0);

 addTableItem( 0, Column, W);
 addTableItem( 1, Column, D);
 addTableItem( 2, Column, L);
 addTableItem( 3, Column, Total);
 addTableItem( 5, Column, EloMul * pexperiment->reg.Rating(Rate + Margin));
 addTableItem( 6, Column, EloMul * pexperiment->reg.Rating(Rate));
 addTableItem( 7, Column, EloMul * pexperiment->reg.Rating(Rate - Margin));
 addTableItem( 9, Column, Rate + Margin);
 addTableItem(10, Column, Rate);
 addTableItem(11, Column, Rate - Margin);
}

/////////////////////////////////////////////////////////////////////////////
// Update statistics
/////////////////////////////////////////////////////////////////////////////
void MainWindow::updateStatistics()
{
 if (pexperiment)
 {
  pexperiment->results.Refresh();

  //
  // Wins, Losses, and winning rate
  //
  updateWinRate(0,
   pexperiment->reg.GetCount(COutcome::Win),
   pexperiment->reg.GetCount(COutcome::Draw),
   pexperiment->reg.GetCount(COutcome::Loss));

  //
  // Weighted winning rate
  //
  const CResults &results = pexperiment->results;
  double tCount[3] = {0.0, 0.0, 0.0};
  double tCentralCount[3] = {0.0, 0.0, 0.0};
  for (int i = results.GetSamples(); --i >= 0;)
   if (results.GetOutcome(i) < 3)
   {
    const double *v = results.GetSample(i);
    double Weight = pexperiment->reg.GetWeight(v);
    if (Weight == 1.0)
     tCentralCount[results.GetOutcome(i)] += Weight;
    tCount[results.GetOutcome(i)] += Weight;
   }
  updateWinRate(1, tCount[COutcome::Win],
                   tCount[COutcome::Draw],
                   tCount[COutcome::Loss]);
  updateWinRate(2, tCentralCount[COutcome::Win],
                   tCentralCount[COutcome::Draw],
                   tCentralCount[COutcome::Loss]);
 }
}

/////////////////////////////////////////////////////////////////////////////
// About Qt
/////////////////////////////////////////////////////////////////////////////
void MainWindow::on_action_About_Qt_triggered()
{
 QMessageBox::aboutQt(this);
}

/////////////////////////////////////////////////////////////////////////////
// Compute matrix color
/////////////////////////////////////////////////////////////////////////////
static QColor MakeColor(double v, double max)
{
 int rTarget = 96, gTarget = 96, bTarget = 96;
 if (v < 0)
 {
  bTarget = 255;
  v = -v;
 }
 else
  rTarget = 255;

 int x = int(256 * v / max);
 if (x > 256)
  x = 256;
 if (x < 0)
  x = 0;

 int r = (255 * (256 - x) + rTarget * x) >> 8;
 int g = (255 * (256 - x) + gTarget * x) >> 8;
 int b = (255 * (256 - x) + bTarget * x) >> 8;

 return QColor(r, g, b);
}

/////////////////////////////////////////////////////////////////////////////
// Display Hessian matrix in matrixTable
/////////////////////////////////////////////////////////////////////////////
void MainWindow::displayHessian()
{
 if (pexperiment && &pexperiment->reg.GetPF() == &pexperiment->pfq)
 {
  const int n = pexperiment->paramcol.GetSize();

  std::vector<double> vHessian(n * n);
  pexperiment->pfq.GetHessian(pexperiment->reg.MAP(), &vHessian[0]);

  for (int p = n + 1, i = n; --i >= 0;)
   for (int j = i + 1; --j >= 0; p++)
   {
    double h_ij = vHessian[i * n + j];
    QTableWidgetItem *item = new QTableWidgetItem(QString::number(h_ij));
    item->setFlags(Qt::ItemIsEnabled);
    double positivity = pexperiment->reg.GetParamPositivity(p);    
    const double maxPositivity = 8.0;
    item->setBackgroundColor(MakeColor(positivity, maxPositivity));
    ui->matrixTable->setItem(i, j, item);
    ui->matrixTable->setItem(j, i, new QTableWidgetItem(*item));
   }
 }
}

/////////////////////////////////////////////////////////////////////////////
// Eigenvectors and eigenvalues of the Hessian matrix
/////////////////////////////////////////////////////////////////////////////
void MainWindow::displayEigen()
{
 if (pexperiment && &pexperiment->reg.GetPF() == &pexperiment->pfq)
 {
  const int n = pexperiment->paramcol.GetSize();

  //
  // Jacobi decomposition
  //
  pexperiment->eigen.Update();
  const double *vEigenValues = pexperiment->eigen.GetValues();
  const double *vEigenVectors = pexperiment->eigen.GetVectors();
  const int *vOrder = pexperiment->eigen.GetOrder();

  //
  // Eigenvectors
  //
  for (int i = n; --i >= 0;)
   for (int j = n; --j >= 0;)
   {
    const int k = vOrder[j];
    double x = vEigenVectors[i * n + k];
    QTableWidgetItem *item = new QTableWidgetItem(QString::number(x));
    item->setFlags(Qt::ItemIsEnabled);
    item->setBackgroundColor(MakeColor(x, 1.0));

    ui->eigenTable->setItem(i + 2, j, item);
   }

  //
  // Eigenvalues
  //
  {
   double min = vEigenValues[vOrder[0] * (n + 1)];
   double max = vEigenValues[vOrder[n - 1] * (n + 1)];
   double Range = -min > max ? -min : max;
   for (int j = n; --j >= 0;)
   {
    const int k = vOrder[j];
    double x = vEigenValues[k * (n + 1)];
    QTableWidgetItem *item = new QTableWidgetItem(QString::number(x));
    item->setFlags(Qt::ItemIsEnabled);
    item->setBackgroundColor(MakeColor(x, Range));
    ui->eigenTable->setItem(0, j, item);
   }
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
// Set a value in the max table
/////////////////////////////////////////////////////////////////////////////
void MainWindow::setMaxTable(int i, int j, double x)
{
 QTableWidgetItem *item = new QTableWidgetItem(QString::number(x));
 item->setFlags(Qt::ItemIsEnabled);
 ui->maxTable->setItem(i, j, item);
}

/////////////////////////////////////////////////////////////////////////////
// Show a max vector
/////////////////////////////////////////////////////////////////////////////
void MainWindow::showMaxVector(int col, const double *v)
{
 const int n = pexperiment->paramcol.GetSize();

 for (int j = 0; j < n; j++)
 {
  const CParameter &param = pexperiment->paramcol.GetParam(j);
  double x = param.TransformFromQLR(v[j]);
  setMaxTable(j, col, x);
 }

 double Rating;
 double Variance;
 pexperiment->reg.GetPosteriorInfo(v, Rating, Variance);
 const double c = 1.96;

 double Deviation = std::sqrt(Variance);
 double LowerRating = Rating - c * Deviation;
 double UpperRating = Rating + c * Deviation;
 double Elo = Rating * 400.0 / std::log(10.0);
 double LowerElo = LowerRating * 400.0 / std::log(10.0);
 double UpperElo = UpperRating * 400.0 / std::log(10.0);

 setMaxTable(n + 1, col, UpperElo);
 setMaxTable(n + 2, col, Elo);
 setMaxTable(n + 3, col, LowerElo);

 setMaxTable(n + 5, col, pexperiment->reg.WinRate(Rating + c * Deviation));
 setMaxTable(n + 6, col, pexperiment->reg.WinRate(Rating));
 setMaxTable(n + 7, col, pexperiment->reg.WinRate(Rating - c * Deviation));
}

/////////////////////////////////////////////////////////////////////////////
// Maximum estimation
/////////////////////////////////////////////////////////////////////////////
void MainWindow::displayMax()
{
 if (pexperiment)
 {
  ui->maxTable->clearContents();
  const int n = pexperiment->paramcol.GetSize();
  std::vector<double> v(n);

  CMESampleMean mesm(pexperiment->reg);
  mesm.MaxParameter(&v[0]);
  showMaxVector(0, &v[0]);

  if (pexperiment->reg.GetPF().GetMax(&pexperiment->reg.MAP()[0], &v[0]))
   showMaxVector(1, &v[0]);
 }
}
