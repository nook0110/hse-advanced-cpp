#include "ugly.h"
#include <memory>
#include <vector>
#include "bad.h"

struct SplineImpl {
    SplineImpl(std::vector<double> x, std::vector<double> y, double a, double b)
        : x(std::move(x)), y(std::move(y)) {
        y2.resize(this->x.size());
        mySplineSnd(this->x.data(), this->y.data(), this->x.size(), a, b, y2.data());
    }

    double Interpolate(double x) {
        double answer;
        mySplintCube(this->x.data(), y.data(), y2.data(), this->x.size(), x, &answer);
        return answer;
    }

    std::vector<double> x;
    std::vector<double> y;
    std::vector<double> y2;
};

Spline::Spline(const std::vector<double>& x, const std::vector<double>& y, double a, double b)
    : impl_(std::make_unique<SplineImpl>(x, y, a, b)) {
}

double Spline::Interpolate(double x) {
    return impl_->Interpolate(x);
}