#ifndef __characterization_H_INCLUDED__
#define __characterization_H_INCLUDED__

//opencv libraries
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
//C++ standard libraries
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <glob.h>
#include <math.h>

using namespace cv;
using namespace std;

template <typename T>
vector<Point> foreground(Mat src, int obj_id=255) {
    assert(src.channels() == 1);
    vector<Point> foreground;
    int row, col = 0;
    for ( row = 0; row < src.rows; ++row ) {
        for ( col = 0; col < src.cols; ++col ) {
            if (src.at<T>(row, col) > 0) {
                Point p = Point(col, row);
                foreground.push_back(p);
            }
        }
    }
    return foreground;
}
template <typename T>
vector<Point> foreground2(Mat I, int obj_id=255) {
    assert(src.channels() == 1);
    int row,col;
    vector<Point> foreground;
    for ( row = 0; row < I.rows; ++row ) {
        for ( col = 0; col < I.cols; ++col ) {
            if (I.at<T>(row, col) > 0) {
                Point p = Point(col, row);
                foreground.push_back(p);
            }
        }
    }
    return foreground;
}

void normalize(vector<double>& vec, double a=0.0, double b=1.0);
Point centroid(vector<Point> object);
int area(vector<Point> object);
double circularity(vector<Point> object);
double orientation(vector<Point> object);
double curvature(Point l, Point m, Point n);
vector<double> curvature(vector<Point> contour);

#endif
