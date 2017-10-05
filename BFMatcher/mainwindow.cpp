#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <QTime>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>

#include "opencv2/core.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/xfeatures2d.hpp"
#include "opencv2/opencv_modules.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace std;
using namespace cv;
using namespace cv::xfeatures2d;

int number_of_matches;
float algorithm_error, total;
std::vector<KeyPoint> keypoints_1, keypoints_2;
std::vector<DMatch> first_matches, good_matches;
std::vector<std::vector<DMatch>> matches;
std::vector<Point2f> object, scene;
//QString img1_path = "D:/Samples/Panorama_query.jpg", img2_path = "D:/Samples/Panorama_train.jpg";
QString img1_path = "D:/Samples/Box.png", img2_path = "D:/Samples/Box_in_scene.png";
Mat img_1, img_2, descriptors_1, descriptors_2, img_matches;
BFMatcher bf;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Učitavamo sample slike (radi lakšeg testiranja)
    ui->pathImg1->setText(img1_path);
    ui->pathImg2->setText(img2_path);
    img_1 = imread(img1_path.toStdString(), 0);
    img_2 = imread(img2_path.toStdString(), 0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_img1Button_clicked()
{
    img1_path = QFileDialog::getOpenFileName(this, tr("Browse for query image"), "/home",
                                                   tr("Image Files (*.png *.jpg *.jpeg *.bmp)"));
    if(img1_path!=nullptr)
    {
        ui->pathImg1->setText(img1_path);
        img_1 = imread(img1_path.toStdString(), 0);
        ui->statusBar->showMessage("Uspješno učitavanje prve slike.");
    }
}

void MainWindow::on_img2Button_clicked()
{
    img2_path = QFileDialog::getOpenFileName(this, tr("Browse for train image"), "/home",
                                                   tr("Image Files (*.png *.jpg *.jpeg *.bmp)"));
    if(img2_path!=nullptr)
    {
        ui->pathImg2->setText(img2_path);
        img_2 = imread(img2_path.toStdString(), 0);
        ui->statusBar->showMessage("Uspješno učitavanje druge slike.");
    }
}

void MainWindow::on_poveziButton_clicked()
{
    int msec = 0;
    QTime timer;
    Ptr<cv::FeatureDetector> detector;

    if(!ui->orbButton->isChecked() && !ui->siftButton->isChecked() && !ui->surfButton->isChecked() && !ui->briskButton->isChecked())
    {
        ui->statusBar->showMessage("Odaberite alogritam!");
    }
    else
    {
        ui->statusBar->showMessage("U toku je izvršavanje programa...");
        if(ui->orbButton->isChecked())
        {
            // ORB (Oriented FAST and Rotated BRIEF) algoritam
            timer.start();
            bf = BFMatcher(NORM_HAMMING, false);
            detector = cv::ORB::create(50000,1.2f,8,ui->orbEdgeThrBox->value(),0,2,ORB::HARRIS_SCORE,
                                       31,ui->orbFastThrBox->value());
        }
        else if(ui->briskButton->isChecked())
        {
            // BRISK (Binary Robust Invariant Scalable Keypoints) algoritam
            timer.start();
            bf = BFMatcher(NORM_HAMMING, false);
            detector = cv::BRISK::create(ui->briskThrBox->value(),3,1.0f);
        }
        else if(ui->siftButton->isChecked())
        {
            // SIFT (Scale-Invariant Feature Transform) algoritam
            timer.start();
            bf = BFMatcher(NORM_L2, false);
            detector = xfeatures2d::SiftFeatureDetector::create(0,3,ui->siftContrastThrBox->value(),
                                                                ui->siftEdgeThrBox->value(),1.6);
        }
        else
        {
            // SURF (Speeded-Up Robust Features) algoritam
            timer.start();
            bf = BFMatcher(NORM_L2, false);
            detector = xfeatures2d::SurfFeatureDetector::create(ui->surfHessianThrBox->value(),4,3,false,false);
        }
        detector->detectAndCompute(img_1, Mat(), keypoints_1, descriptors_1);
        detector->detectAndCompute(img_2, Mat(), keypoints_2, descriptors_2);
        /* Ili pojedinačno koristeći i ekstraktor (npr. za SURF) -> daje iste rezultate:
        detector->detect(img_1, keypoints_1);
        detector->detect(img_2, keypoints_2);
        Ptr<cv::xfeatures2d::SurfDescriptorExtractor> extractor;
        extractor = xfeatures2d::SurfDescriptorExtractor::create();
        extractor->compute(img_1, keypoints_1, descriptors_1);
        extractor->compute(img_2, keypoints_2, descriptors_2);*/
        //msec = timer.elapsed();
        //ui->statusBar->showMessage((QString)"Vrijeme potrebno za detekciju značajki i određivanje deskriptora: " + QString::number(msec/1000.00) + " s.");
        number_of_matches = ui->featuresBox->value();
        first_matches.clear();
        good_matches.clear();
        matches.clear();
        object.clear();
        scene.clear();
        algorithm_error = 0;
        total = 0;
        BFMatching();
        msec = timer.elapsed();
        ui->statusBar->showMessage((QString)"Ukupno vrijeme trajanja algoritma: " + QString::number(msec/1000.00) + " s.");

        // Postavljanje defaultnih vrijednosti
        ui->maxBrojBox->setChecked(false);
        ui->orbEdgeThrBox->setValue(31);
        ui->orbFastThrBox->setValue(20),
        ui->briskThrBox->setValue(30);
        ui->siftContrastThrBox->setValue(0.04);
        ui->siftEdgeThrBox->setValue(10);
        ui->surfHessianThrBox->setValue(100);
    }
}

void MainWindow::BFMatching()
{
    // Formiranje parova značajki (matches) pomoću BFMatcher-a i kNN algoritma (k=2)
       // (inače: bf.match(descriptors_1, descriptors_2, matches);)
    bf.knnMatch(descriptors_1, descriptors_2, matches, 2);

    // Ratio test (Lowe)
    for (unsigned int i = 0; i < matches.size(); i++)
    {
        const float ratio = 0.8;
        if (matches[i][0].distance/matches[i][1].distance < ratio)
        {
            good_matches.push_back(matches[i][0]);
        }
    }

    // Sortiranje parova značajki po udaljenosti
    sort(good_matches.begin(), good_matches.end());
    if((int)good_matches.size()!=0)
    {
        if(!ui->maxBrojBox->isChecked())
        {
            if(number_of_matches<=(int)good_matches.size())
                DrawingMatchesAndObjectDetection(number_of_matches);
            else
            {
                QMessageBox mssg_box;
                mssg_box.setText(QString("Ne postoji ") + QString::number(number_of_matches) + " parova značajki. Bit će prikazan maksimalan broj pronađenih parova značajki: " + QString::number((int)good_matches.size()));
                mssg_box.setIcon(QMessageBox::Warning);
                mssg_box.setWindowTitle(QString("Poruka"));
                mssg_box.exec();
                DrawingMatchesAndObjectDetection((int)good_matches.size());
            }
        }
        else
            DrawingMatchesAndObjectDetection((int)good_matches.size());
    }
    else
    {
        QMessageBox mssg_box;
        mssg_box.setText(QString("Nisu pronađeni parovi značajki na ovim slikama!"));
        mssg_box.setIcon(QMessageBox::Warning);
        mssg_box.setWindowTitle(QString("Poruka"));
        mssg_box.exec();
    }
}

void MainWindow::DrawingMatchesAndObjectDetection(int n)
{
    QString Message = nullptr;

    // Izdvajanje prvih n parova značajki
    for(int i = 0; i < n; i++)
    {
            first_matches.push_back(good_matches[i]);
    }
    // Crtanje prvih n parova
    drawMatches(img_1, keypoints_1, img_2, keypoints_2, first_matches, img_matches,
                Scalar::all(-1), Scalar::all(-1),
                vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

    if(!img_matches.empty())
    {
        for(int i=0; i<(int)first_matches.size(); i++)
        {
            total = total + first_matches[i].distance;
            Message = Message + QString::number(i+1) + ". match -> keypoint 1: " + QString::number(first_matches[i].queryIdx)
                                                     + ", keypoint 2: " + QString::number(first_matches[i].trainIdx)
                                                     + ", distance: " + QString::number(first_matches[i].distance) + " \n";
        }
        algorithm_error = total/((float)first_matches.size());
        Message = Message + "Error: " + QString::number(algorithm_error) + " \n";
    }
    else
        Message = "Nisu pronađeni parovi značajki na ovim slikama!";

    ObjectDetection();
    ShowMatches = new Matches(this, Message);
    imshow("Parovi pronadjenih znacajki i detektovan objekat", img_matches);
    ShowMatches->show();
    this->move(12,30);
}

void MainWindow::ObjectDetection()
{
    // Koristimo maksimalan broj dobrih parova značajki za detekciju objekta, zbog bolje preciznosti detekcije
    for(int i=0; i<(int)good_matches.size(); i++)
    {
        // Izdvajanje značajki na slici objekta i slici scene, pomoću indeksa query i train deskriptora
        object.push_back(keypoints_1[good_matches[i].queryIdx].pt);
        scene.push_back(keypoints_2[good_matches[i].trainIdx].pt);
    }
    Mat H = findHomography(object, scene, CV_RANSAC);
    if(!H.empty())
    {
        // Pronalaženje uglova objekta (odnosno slike objekta) koji detektujemo
        std::vector<Point2f> objCorners(4), scCorners(4);
        objCorners[0] = cvPoint(0, 0);
        objCorners[1] = cvPoint(img_1.cols, 0);
        objCorners[2] = cvPoint(img_1.cols, img_1.rows);
        objCorners[3] = cvPoint(0, img_1.rows);
        // Pronalaženje uglova objekta (slike objekta) na slici scene
        perspectiveTransform(objCorners, scCorners, H);
        // Oivičenje objekta na slici 'scene'
        Point2f trans = objCorners[1]; // faktor transliranja za dužinu slike objekta
        line(img_matches, scCorners[0] + trans, scCorners[1] + trans, Scalar(0, 0, 255), 3);
        line(img_matches, scCorners[1] + trans, scCorners[2] + trans, Scalar(0, 0, 255), 3);
        line(img_matches, scCorners[2] + trans, scCorners[3] + trans, Scalar(0, 0, 255), 3);
        line(img_matches, scCorners[3] + trans, scCorners[0] + trans, Scalar(0, 0, 255), 3);
    }
    else
    {
        QMessageBox mssg_box;
        mssg_box.setText(QString("Pronađen nedovoljan broj parova značajki na slikama!"));
        mssg_box.setIcon(QMessageBox::Warning);
        mssg_box.setWindowTitle(QString("Nemoguće detektovati objekt"));
        mssg_box.exec();
    }
}

void MainWindow::on_orbButton_clicked()
{
    ui->statusBar->clearMessage();
    ui->orbEdgeThrBox->setEnabled(true);
    ui->orbFastThrBox->setEnabled(true);
    ui->siftContrastThrBox->setEnabled(false);
    ui->siftEdgeThrBox->setEnabled(false);
    ui->surfHessianThrBox->setEnabled(false);
    ui->briskThrBox->setEnabled(false);
}

void MainWindow::on_siftButton_clicked()
{
    ui->statusBar->clearMessage();
    ui->siftContrastThrBox->setEnabled(true);
    ui->siftEdgeThrBox->setEnabled(true);
    ui->orbEdgeThrBox->setEnabled(false);
    ui->orbFastThrBox->setEnabled(false);
    ui->surfHessianThrBox->setEnabled(false);
    ui->briskThrBox->setEnabled(false);
}

void MainWindow::on_surfButton_clicked()
{
    ui->statusBar->clearMessage();
    ui->surfHessianThrBox->setEnabled(true);
    ui->siftContrastThrBox->setEnabled(false);
    ui->siftEdgeThrBox->setEnabled(false);
    ui->orbEdgeThrBox->setEnabled(false);
    ui->orbFastThrBox->setEnabled(false);
    ui->briskThrBox->setEnabled(false);
}

void MainWindow::on_briskButton_clicked()
{
    ui->statusBar->clearMessage();
    ui->briskThrBox->setEnabled(true);
    ui->surfHessianThrBox->setEnabled(false);
    ui->siftContrastThrBox->setEnabled(false);
    ui->siftEdgeThrBox->setEnabled(false);
    ui->orbEdgeThrBox->setEnabled(false);
    ui->orbFastThrBox->setEnabled(false);
}
