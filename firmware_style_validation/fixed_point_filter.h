#pragma once
#include <cstdint>
#include <vector>

namespace firmware_validation {

// Q8.8 fixed-point multiply.
int16_t q8_8_mul(int16_t a, int16_t b);

// One-pole fixed-point filter:
// y[n] = alpha*x[n] + (1-alpha)*y[n-1]
class FixedPointFilter {
public:
    explicit FixedPointFilter(int16_t alpha_q8_8);
    int16_t update(int16_t input_q8_8);
    int16_t state() const;

private:
    int16_t alpha_;
    int16_t state_;
};

std::vector<int16_t> run_filter_sequence(
    const std::vector<int16_t>& inputs_q8_8,
    int16_t alpha_q8_8
);

}  // namespace firmware_validation
