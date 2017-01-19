#include "characterization.hpp"
#include "face_changer.hpp"

// ******************
// Image blending
// ******************
// Clone where no blending is attempted around the edges of the copy.
void hard_clone(Mat source, Mat mask, Mat& target) {
    Mat composite = target.clone();
    source.copyTo(composite, mask);
    target = composite;
}

// rescale(rescale(rescale(L3) + L2) + L1) + L0 = collapsed
Mat collapse_pyramid(vector<Mat> pyramid) {
    assert(pyramid.size() > 0);
    Mat collapsed = pyramid[pyramid.size() - 1];
    Mat rescaled;
    for (int i = pyramid.size() - 2; i >= 0; --i) {
        pyrUp(collapsed, rescaled, pyramid[i].size());
        collapsed = rescaled + pyramid[i];
    }
    return collapsed;
}

vector<Mat> laplacian_pyramid(Mat source, int levels) {
    vector<Mat> gaussPyramid;
    buildPyramid(source, gaussPyramid, levels);

    Mat blurred;
    vector<Mat> laplacePyramid;
    int i;
    for (i = 0; i < levels - 1; ++i) {
        pyrUp(gaussPyramid[i+1], blurred, gaussPyramid[i].size());
        laplacePyramid.push_back(gaussPyramid[i] - blurred);
    }
    laplacePyramid.push_back(gaussPyramid[i]);
    return laplacePyramid;
}

// Clone with blending around the edges via Gaussian and Laplacian pyramids.
void blended_clone(Mat source, Mat mask, Mat& target) {
    int levels = 4;
    Mat buffer;

    vector<Mat> maskPyramid;
    Mat threeChannelMask, tmp;
    threshold(mask, threeChannelMask, 1, 1.0, THRESH_BINARY);
    cvtColor(threeChannelMask, tmp, CV_GRAY2BGR);
    threeChannelMask = tmp;
    buildPyramid(threeChannelMask, maskPyramid, levels);

    vector<Mat> laplaceSource, laplaceTarget;
    laplaceSource = laplacian_pyramid(source, levels);
    laplaceTarget = laplacian_pyramid(target, levels);

    Mat composite;
    vector<Mat> compositePyramid;
    for (int i = 0; i < levels; ++i) {
        composite = laplaceSource[i].mul(maskPyramid[i]) +
                    laplaceTarget[i].mul(Scalar(1.0, 1.0, 1.0) - maskPyramid[i]);
        compositePyramid.push_back(composite);
    }

    target = collapse_pyramid(compositePyramid);
}

// Poisson image blending clone.
void seamless_clone(Mat source, Mat mask, Mat& target) {
    Mat composite = target.clone();
    Point center = centroid(foreground2<uchar>(mask));
    seamlessClone(source, target, mask, center, composite, NORMAL_CLONE);
    target = composite;
}

// ******************
// Orientation alignment
// ******************
double distanceBetweenPoints(Point a, Point b) {
    return sqrt(pow(b.x - a.x, 2) + pow(b.y - a.y, 2));
}

Mat absoluteOrientationTransformationMatrix(std::vector<Point> face1, std::vector<Point> face2) {
    assert(face1.size() == face2.size());
    Point center1 = centroid(face1);   // original center
    Point center2 = centroid(face2);   // target center

    // Translate both coordinate systems to (0,0)
    int i;
    for (i = 0; i < face1.size(); ++i) {
        face1[i] = Point(face1[i].x - center1.x, face1[i].y - center1.y);
        face2[i] = Point(face2[i].x - center2.x, face2[i].y - center2.y);
    }

    // Find the scale
    double num = 0.0;
    double denom = 0.0;
    for (i = 0; i < face1.size(); ++i) {
        num += pow(distanceBetweenPoints(face2[i], Point(0,0)), 2);
        denom += pow(distanceBetweenPoints(face1[i], Point(0,0)), 2);
    }
    double scale = pow(num/denom, 0.5);
    //cout << "scale " << scale << endl;

    // Find the rotation
    num = 0.0;
    denom = 0.0;
    for (i = 0; i < face1.size(); ++i) {
        num += (face2[i].y*face1[i].x - face2[i].x*face1[i].y);
        denom += (face2[i].x*face1[i].x + face2[i].y*face1[i].y);
    }
    double theta = atan(num/denom);
    //cout << "rotation " << theta << endl;

    Mat trans(2, 3, CV_64F);
    trans.at<double>(0, 1) = scale * cos(theta);
    trans.at<double>(0, 2) = -scale * sin(theta);
    trans.at<double>(0, 3) = (-center1.x * scale * cos(theta)) -
                                (-center1.y * scale * sin(theta)) + center2.x;
    trans.at<double>(1, 1) = scale * sin(theta);
    trans.at<double>(1, 2) = scale * cos(theta);
    trans.at<double>(1, 3) = (-center1.x * scale * sin(theta)) -
                                (-center1.y * scale * cos(theta)) + center2.y;

    return trans;
}

Mat getTransformationMatrix(std::vector<Point> face1, std::vector<Point> face2) {
    //Point center1 = centroid(f1);   // original center
    //Point center2 = centroid(f2);   // target center
    //double theta = rotationAngle(f1, f2);
    Point2f face1_keypoints[3];
    Point2f face2_keypoints[3];
    face1_keypoints[0] = face1[8];     // chin
    face1_keypoints[1] = face1[36];    // eye
    face1_keypoints[2] = face1[45];    // eye
    face2_keypoints[0] = face2[8];     // chin
    face2_keypoints[1] = face2[36];    // eye
    face2_keypoints[2] = face2[45];    // eye

    Mat trans = getAffineTransform(face1_keypoints, face2_keypoints);
    return trans;
}

// ******************
// Face swapping
// ******************
void swapFaces(Mat src, Mat& output, std::vector<Point> f1, std::vector<Point> f2) {
    Mat trans_f2_to_f1;
    Mat trans_f1_to_f2 = getTransformationMatrix(f1, f2);
    invertAffineTransform(trans_f1_to_f2, trans_f2_to_f1);

    // create masks and extract faces for swapping
    Mat face1, face2;
    Mat mask1 = Mat::zeros(src.size(), CV_8UC1);
    Mat mask2 = Mat::zeros(src.size(), CV_8UC1);
    maskFace(mask1, f1);
    face1 = extractFace(src, mask1);
    maskFace(mask2, f2);
    face2 = extractFace(src, mask2);

    // warp masks
    Mat warped_mask1;
    Mat warped_mask2;
    warpAffine(mask1, warped_mask1, trans_f1_to_f2, mask1.size(),
                                INTER_NEAREST, BORDER_CONSTANT, Scalar(0,0,0));
    warpAffine(mask2, warped_mask2, trans_f2_to_f1, mask2.size(),
                                INTER_NEAREST, BORDER_CONSTANT, Scalar(0,0,0));

    // warp faces and copy to final frame with final  mask
    Mat warped_face1;
    Mat warped_face2;
    warpAffine(face1, warped_face1, trans_f1_to_f2, face1.size(),
                                INTER_NEAREST, BORDER_CONSTANT, Scalar(0,0,0));
    warpAffine(face2, warped_face2, trans_f2_to_f1, face2.size(),
                                INTER_NEAREST, BORDER_CONSTANT, Scalar(0,0,0));

    // Image blend the faces onto the output image
    //seamless_clone(warped_face1, warped_mask1, output);
    //seamless_clone(warped_face2, warped_mask2, output);
    //hard_clone(warped_face1, warped_mask1, output);
    //hard_clone(warped_face2, warped_mask2, output);
    blended_clone(warped_face1, warped_mask1, output);
    blended_clone(warped_face2, warped_mask2, output);
}

// Creates a mask of a face region given a vector of facial landmarks.
void maskFace(Mat& mask, std::vector<Point> face) {
    // create mask
    std::vector<std::vector<Point>> hull(1);
    convexHull(Mat(face), hull[0], false);
    drawContours(mask, hull, 0, Scalar(255,255,255), -1);
}

// creates a mask such that all pixels belonging to the face area are
// white (255, 255, 255) and all remaining pixels are black (0, 0, 0)
Mat extractFace(Mat src, Mat mask) {
    Mat dst = Mat::zeros(src.size(), src.type());
    src.copyTo(dst, mask);
    return dst;
}

