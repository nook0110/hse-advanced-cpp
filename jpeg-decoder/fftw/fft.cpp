#include <fft.h>

#include <fftw3.h>
#include <cmath>
#include <cstddef>
#include <memory>
#include <stdexcept>

class DctCalculator::Impl {
public:
    Impl(size_t width, std::vector<double> *input, std::vector<double> *output)
        : length_(input->size() / width), width_(width), input_(input), output_(output) {
        if (input->size() != width * width || output->size() != width * width) {
            throw std::invalid_argument("bad");
        }
        plan_ = fftw_plan_r2r_2d(length_, width_, input_->data(), output_->data(),
                                 fftw_r2r_kind::FFTW_REDFT01, fftw_r2r_kind::FFTW_REDFT01,
                                 FFTW_ESTIMATE);
    };

    void Inverse() {
        for (int i = 0; i < width_; ++i) {
            for (int j = 0; j < width_; ++j) {
                auto idx = i * width_ + j;
                if (i == 0) {
                    (*input_)[idx] *= sqrt(2);
                }
                if (j == 0) {
                    (*input_)[idx] *= sqrt(2);
                }
            }
        }

        fftw_execute(plan_);

        for (auto &val : *output_) {
            val /= 16;
        }
    }

    ~Impl() {
        fftw_destroy_plan(plan_);
    }

private:
    int length_;
    int width_;
    std::vector<double> *input_;
    std::vector<double> *output_;
    fftw_plan plan_;
};

DctCalculator::DctCalculator(size_t width, std::vector<double> *input, std::vector<double> *output)
    : impl_(std::make_unique<Impl>(width, input, output)) {
}

void DctCalculator::Inverse() {
    impl_->Inverse();
}

DctCalculator::~DctCalculator() = default;
