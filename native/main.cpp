// dlib libraries
#include <dlib/opencv.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
// opencv libraries
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/imgcodecs.hpp>
// c++ standard libraries
#include <iostream>
#include <vector>

#include "face_changer.hpp"
#include "characterization.hpp"

using namespace dlib;
using namespace cv;
using namespace std;

// Using Dlib, get vector of facial landmark points for all faces in the frame
std::vector<std::vector<Point>> faceLandmarks (
        Mat frame, frontal_face_detector detector, shape_predictor sp) {
    std::vector<std::vector<Point>> landmarks;

    cv_image<bgr_pixel> img(frame); // convert openCV Mat to dlib format

    std::vector<dlib::rectangle> faces = detector(img);
    dlib::rectangle face;
    dlib::point p;
    Point pcv;
    for (auto face : faces) {
        // save landmarks for current face
        std::vector<Point> facial_landmarks;
        full_object_detection shape = sp(img, face);
        // extract non-edge-of-face landmarks only (start at 17)
        for (int j = 0; j < shape.num_parts(); ++j) {
            p = shape.part(j);
            pcv = Point(p.x(), p.y());
            facial_landmarks.push_back(pcv);
        }
        landmarks.push_back(facial_landmarks);
    }
    return landmarks;
}

int main(int argc, char** argv)
{
	Mat frame;
    VideoCapture cap(0);

    String dat_file = "../data/shape_predictor_68_face_landmarks.dat";
    bool image_only = false;
    if (argc >= 2) {
        image_only = true;
        Mat img = imread(argv[1], CV_LOAD_IMAGE_COLOR);
        if (!img.data) {
            cout << "Could not open file " << argv[1] << endl;
            return -1;
        }
        resize(img, frame, Size(img.cols*0.5, img.rows*0.5), 0, 0);
    } else {
        // initialize video stream, if not successful then exit program
        if (!cap.isOpened() || !cap.read(frame)) {
            cout << "Cannot open or read from the video camera" << endl;
            return -1;
        }
    }

    // initialize facial landmark detection library
	frontal_face_detector detector =
        get_frontal_face_detector(); // used to detect bounding boxes
	shape_predictor sp;
	deserialize(dat_file) >> sp;     // load face shape model for prediction

    int i, j = 0;
    Mat output, outputtmp;
    std::vector<std::vector<Point>> faces;
    std::vector<Point> face;

    Rect search_area;
    int skip = 0;
	while (1) {         // continuously read in video frames
        if (!image_only) {
            if (!cap.read(frame)) {
                cout << "Could not read frame from video stream" << endl;
                break;
            }
        }
        output = frame.clone();

        // detect faces using Dlib
        if (!skip) {
            faces = faceLandmarks(frame, detector, sp);
        }

        // swap faces
        int n = faces.size();
        if ( n > 1 ) {
            for (i = 0; i < n; i+=2) {
                std::vector<Point> source(faces[i]); // copy of face
                std::vector<Point> target(faces[(i + 1) % n]); // copy of face
                swapFaces(frame, output, source, target);
            }
        }

        imshow("FaceSwap", output);
        ++skip;
        skip = skip % 2;

        if (image_only) {
            waitKey(0);
            break;
        } else {
            waitKey(30);
        }
    }
}
