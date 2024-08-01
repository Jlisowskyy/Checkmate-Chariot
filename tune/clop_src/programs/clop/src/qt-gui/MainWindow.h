#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <deque>

class QLabel;
class CExperimentSettings;
class CExperimentFromSettings;
class CPlotWidget;

namespace Ui {
    class MainWindow;
}

class MainWindow: public QMainWindow
{
    Q_OBJECT

    friend class CPlotWidget;

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *);

private:
    Ui::MainWindow *ui;

    enum { MaxRecentFiles = 10 };
    QAction *recentFileActs[MaxRecentFiles];
    QAction *separatorAct;

    QLabel *statusLabel;
    enum State {Opening, Ready, Running, Waiting} state;
    QLabel *counterLabel;

    CExperimentSettings *pes;
    CExperimentFromSettings *pexperiment;
    void deleteExperiment();

    std::deque<std::string> messageBuffer;
    QStringList slNames;

    void loadFile(const QString &fileName);
    void updateRecentFileActions();
    void setState(State stateNew);
    bool checkRunning();

    void setMaxTable(int i, int j, double x);
    void showMaxVector(int col, const double *v);
    void updateWinRate(int Column, double W, double D, double L);
    void addTableItem(int i, int j, double x);

private slots:
    void on_action_Open_triggered();
    void on_action_Close_triggered();

    void on_action_Start_triggered();
    void on_action_SoftStop_triggered();
    void on_action_HardStop_triggered();

    void on_action_About_Qt_triggered();
    void on_action_About_triggered();

    void onOpenRecentFile();

    void onRexpReady() {if (pexperiment) setState(Ready);}
    void onRexpRunning() {setState(Running);}
    void onRexpMessage(std::string sMessage);

    void updateConnections();
    void updateAll();

    void updateCounter();
    void updateStatistics();
    void updatePlot();
    void displayHessian();
    void displayEigen();
    void displayMax();
};

#endif // MAINWINDOW_H
