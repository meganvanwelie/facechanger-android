#ifndef __faceswap_H_INCLUDED__
#define __faceswap_H_INCLUDED__

//opencv libraries
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include <opencv2/photo/photo.hpp>
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

// face swap functions
void maskFace(Mat& mask, std::vector<Point> face);
Mat extractFace(Mat src, Mat mask);
void swapFaces(Mat src, Mat& output,
        std::vector<Point> f1, std::vector<Point> f2);

// blending functions
void hard_clone(Mat source, Mat mask, Mat& target);
void blended_clone(Mat source, Mat mask, Mat& target);
void seamless_clone(Mat source, Mat mask, Mat& target);

// orientation alignment
Mat getTransformationMatrix(
        std::vector<Point> face1, std::vector<Point> face2);

#endif
