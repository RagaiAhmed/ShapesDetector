#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include <qdebug.h>

QString colors[8]={"Black","Red","Green","Yellow","Blue","Purple","Cyan","White"};

void describe(QString path)
{
    // Read an image
    cv::Mat src = cv::imread(path.toStdString());
    //imshow( "src", src ); // Shows source image

    // Convert to grayscale
    cv::Mat src_gray;
    cvtColor( src, src_gray, cv::COLOR_BGR2GRAY );
    //imshow( "gray", src_gray );

    // Blur to remove noise (if any)
    cv::blur(src_gray,src_gray,cv::Size(3,3));

    // Preform canny eddge detection
    cv::Mat canny_output;
    cv::Canny(src_gray, canny_output, 1, 1 );
    //imshow( "detected edges", canny_output );

    // Detect lane direction
    QString lane = "";
    bool hasEdge[2]={};


    // Left edge
    for(int i=0;i<canny_output.size().height;i++)
    {
        hasEdge[0]|= canny_output.at<char>(i,0)!=0;
    }


    // Right edge
    for(int i=0;i<canny_output.size().height;i++)
    {
        hasEdge[1]|= canny_output.at<char>(i,canny_output.size().width-1)!=0;
    }

    if(!hasEdge[0] && !hasEdge[1]) lane = "Forward";
    if(hasEdge[0])lane += " Left";
    if(hasEdge[1])lane += " Right";
    qDebug()<<"Lane Direction: "<<lane<<"\n";


    // To store contours detected
    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;
    findContours( canny_output, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE );  // Detects only outer contours
    for( size_t i = 0; i< contours.size(); i++ )
    {
        double a = cv::contourArea(contours[i],1);


        a *=-1;  // Holes have negative areas, so we make them positive

        // If closed curve
        if(a>=100)
        {
            // Approx to nearest polygon
            cv::approxPolyDP(contours[i],contours[i],0.009 *  cv::arcLength(contours[i], 1),1);

            // Construct mask
            cv::Mat mask = cv::Mat::zeros( canny_output.size(), CV_8U );
            drawContours( mask, contours, i, 255,-1);
            //imshow( "mask", mask );

            cv::Scalar avg_BGR = cv::mean(src,mask);  // Compute average color

            QString shape = "Circle";  // By default it's shape is a circle
            switch (contours[i].size())
            {
            case 3:  // If has 3 points then it is a triangle
                shape = "Triangle";
                break;
            case 4:  // If has 4 points then it is a rectangle
                cv::Point2f c;
                float r;
                cv::minEnclosingCircle(contours[i],c,r);

                if(a>pow(r,2))  // Has side ratio bigger than tan(30)
                {
                    shape = "Square";
                }
                else
                {
                    shape = "Line";
                }
                break;

            }

            // Calculate moments of the image
            cv::Moments m = cv::moments(contours[i]);

            // Find the centroid
            int x = (int)(m.m10/m.m00);
            int y = (int)(m.m01/m.m00);

            // Calc its color index
            int color_ind = (avg_BGR[0]>128)<<2 | (avg_BGR[1]>128)<<1 | (avg_BGR[2]>128);


            // Prints some info
            qDebug()<<"Found: "<<shape;
            qDebug()<<"At: "<<"("<<x<<", "<<y<<")";
            qDebug()<<"Color: "<<colors[color_ind];
            qDebug()<<"\n";

            // Show black line around object
            cv::Mat show = src;
            cv::Scalar black_line  = cv::Scalar(0,0,0);
            drawContours( show, contours, i , black_line,10);
            imshow( "Found Object", show );

            // Wait for key press
            cv::waitKey(0);
        }
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    describe(ui->lineEdit->text());

}
