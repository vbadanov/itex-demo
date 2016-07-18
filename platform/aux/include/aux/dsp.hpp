
#ifndef DSP_H_
#define DSP_H_

#include <valarray>
#include <aux/real-number.hpp>
#include <aux/windows-types.hpp>

namespace aux
{

/*===========================================================================*/
std::valarray<REAL> diff(const std::valarray<REAL> &a);
std::valarray<REAL> cumsum(REAL first_val, const std::valarray<REAL> &a);
std::valarray<REAL> sgn(const std::valarray<REAL> &a, REAL max_value);
REAL mean(const std::valarray<REAL> &a);
REAL stddev(const std::valarray<REAL> &a, REAL mean);
REAL mse(const std::valarray<REAL> &a, const std::valarray<REAL> &b);
REAL mae(const std::valarray<REAL> &a, const std::valarray<REAL> &b);
int compute_cosine_window(std::valarray<REAL> *window, bool half_window, REAL a0, REAL a1, REAL a2);
int create_weight_window(std::valarray<REAL> *window, bool half_window, WindowType window_type);
int fir_lowpass(std::valarray<REAL> *h, unsigned long length, REAL filter_frequency, WindowType win_type, bool min_phase);
int convolution(const std::valarray<REAL> &data, const std::valarray<REAL> &h, std::valarray<REAL> *result);

}

#endif // DSP_H_
