#include "characterization.hpp"

/* Normalize values in vector between a and b. In-place. */
void normalize(vector<double>& vec, double a, double b) {
    double minv, maxv;
    minMaxLoc(vec, &minv, &maxv);
    transform(vec.begin(), vec.end(), vec.begin(),
              [=](double d){return a + ((d - minv) * (b - a)  / (maxv - minv));});
}

int area(vector<Point> object) {
    return object.size();
}

Point centroid(vector<Point> object) {
    Point p;
    double xbar = 0.0;
    double ybar = 0.0;
    double area = 0.0;
    for ( int i = 0; i < object.size(); ++i ) {
        p = object[i];
        xbar += p.x;
        ybar += p.y;
        area += 1;
    }
    xbar = xbar / area;
    ybar = ybar / area;
    return Point(xbar, ybar);
}

double circularity(vector<Point> object) {
    Point center = centroid(object);
    double a = 0.0;
    double b = 0.0;
    double c = 0.0;
    Point p;
    for ( int i = 0; i < object.size(); ++i ) {
        p = object[i];
        a += pow((p.x - center.x), 2);
        b += (p.x - center.x) * (p.y - center.y);
        c += pow((p.y - center.y), 2);
    }
    b = 2 * b;
    double circularity = -1;
    if ( b != 0 && a != c ) {
        double h = sqrt(((a-c)*(a-c)) + (b*b));
        double emin = ((a+c)/2) - (((a-c)/2)*((a-c)/h)) - ((b/2)*(b/h));
        double emax = ((a+c)/2) + (((a-c)/2)*((a-c)/h)) + ((b/2)*(b/h));
        circularity = emin / emax;
    }
    return circularity;
}

double orientation(vector<Point> object) {
    Point center = centroid(object);
    double a = 0.0;
    double b = 0.0;
    double c = 0.0;
    Point p;
    for ( int i = 0; i < object.size(); ++i ) {
        p = object[i];
        a += pow((p.x - center.x), 2);
        b += (p.x - center.x) * (p.y - center.y);
        c += pow((p.y - center.y), 2);
    }
    //double theta = (b != 0 && a != c) ? (0.5 * atan( 2*b / (a - c) )) : -1;
    double theta = (b != 0 && a != c) ?
                    (0.5 * acos( (a-c) / sqrt(pow(b, 2) + pow(a-c, 2)))) : 0;

    return theta;
}

double curvature(Point l, Point m, Point n) {
    double dxi = (m.x - l.x);
    double dxi1 = (n.x - m.x);
    double dyi = (m.y - l.y);
    double dyi1 = (n.y - m.y);
    double dsi = sqrt(pow(dxi,2) + pow(dyi,2));
    double dsi1 = sqrt(pow(dxi1,2) + pow(dyi1,2));
    double ds = (dsi + dsi1) / 2.0;

    double c = (1/ds) * sqrt(pow((dxi/dsi) - (dxi1/dsi1), 2) +
                         pow((dyi/dsi) - (dyi1/dsi1), 2));
    c = pow(c, 2);
    return c;
}

vector<double> curvature(vector<Point> contour) {
    vector<double> cs;
    // adjust contour to handle start and end positions
    contour.insert(contour.begin(), contour[contour.size() - 1]);
    contour.push_back(contour[1]);
    // assign contour calculation for all points in original contour
    int i;
    for ( i = 1; i < contour.size() - 1; ++i ) {
        cs.push_back(curvature(contour[i-1], contour[i], contour[i+1]));
    }

    normalize(cs);

    return cs;
}

