#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "matches.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    Matches *ShowMatches;

private slots:
    void on_img1Button_clicked();

    void on_img2Button_clicked();

    void on_poveziButton_clicked();

    void on_orbButton_clicked();

    void on_siftButton_clicked();

    void on_surfButton_clicked();

    void DrawingMatchesAndObjectDetection(int n);

    void ObjectDetection();

    void BFMatching();

    void on_briskButton_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
