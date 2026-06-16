#include "fixed_point_filter.h"

namespace firmware_validation {

int16_t q8_8_mul(int16_t a, int16_t b) {
    const int32_t product = static_cast<int32_t>(a) * static_cast<int32_t>(b);
    return static_cast<int16_t>(product >> 8);
}

FixedPointFilter::FixedPointFilter(int16_t alpha_q8_8)
    : alpha_(alpha_q8_8), state_(0) {}

int16_t FixedPointFilter::update(int16_t input_q8_8) {
    constexpr int16_t one_q8_8 = 256;
    const int16_t inverse_alpha = static_cast<int16_t>(one_q8_8 - alpha_);

    const int16_t input_term = q8_8_mul(alpha_, input_q8_8);
    const int16_t state_term = q8_8_mul(inverse_alpha, state_);
    state_ = static_cast<int16_t>(input_term + state_term);
    return state_;
}

int16_t FixedPointFilter::state() const {
    return state_;
}

std::vector<int16_t> run_filter_sequence(
    const std::vector<int16_t>& inputs_q8_8,
    int16_t alpha_q8_8
) {
    FixedPointFilter filter(alpha_q8_8);
    std::vector<int16_t> outputs;
    outputs.reserve(inputs_q8_8.size());

    for (int16_t sample : inputs_q8_8) {
        outputs.push_back(filter.update(sample));
    }

    return outputs;
}

}  // namespace firmware_validation
