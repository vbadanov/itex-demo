
#include <aux/dsp.hpp>

namespace aux
{

/*===========================================================================*/
std::valarray<REAL> diff(const std::valarray<REAL> &a)
{
    unsigned long size = a.size();
    unsigned long size_diff = size - 1;
    std::valarray<REAL> res(size_diff);

    for(unsigned long i = 0; i < size_diff; i++)
    {
        res[i] = a[i+1] - a[i];
    }
    return res;
};


/*===========================================================================*/
std::valarray<REAL> cumsum(REAL first_val, const std::valarray<REAL> &a)
{
    unsigned long size = a.size();
    unsigned long size_cumsum = size + 1;
    std::valarray<REAL> res(size_cumsum);

    res[0] = first_val;
    for(unsigned long i = 1; i < size_cumsum; i++)
    {
        res[i] = res[i-1] + a[i-1];
    }
    return res;
};


/*===========================================================================*/
std::valarray<REAL> sgn(const std::valarray<REAL> &a, REAL max_value)
{
    unsigned long size = a.size();
    REAL buf = 0.0;
    std::valarray<REAL> res(size);

    for(unsigned long i = 0; i < size; i++)
    {
        buf = a[i];
        if(buf < 0)
        {
            res[i] = (-max_value);
        }
        else if (buf > 0)
        {
            res[i] = (+max_value);
        }
        else
        {
            res[i] = (0.0);
        }
    }
    return res;
};


/*===========================================================================*/
REAL mean(const std::valarray<REAL> &a)
{
    long size = a.size();
    REAL mean = 0;

    REAL inv_size = 1.0 / static_cast<REAL>(size);
    for(long i = 0; i < size; i++)
    {
        mean += a[i] * inv_size;
    };

    return mean;
};

/*===========================================================================*/
REAL stddev(const std::valarray<REAL> &a, REAL mean)
{
    long size = a.size();
    REAL sum = 0;

    REAL inv_size = 1.0 / static_cast<REAL>(size);
    for(long i = 0; i < size; i++)
    {
        sum += (a[i] - mean) * (a[i] - mean) * inv_size;
    };

    return sqrt(sum);
};

/*===========================================================================*/
REAL mse(const std::valarray<REAL> &a, const std::valarray<REAL> &b)
{
    long size = a.size();
    REAL sum = 0;

    REAL inv_size = 1.0 / static_cast<REAL>(size);
    for(long i = 0; i < size; i++)
    {
        sum += (a[i] - b[i]) * (a[i] - b[i]) * inv_size;
    };

    return sqrt(sum);
};


/*===========================================================================*/
REAL mae(const std::valarray<REAL> &a, const std::valarray<REAL> &b)
{
    long size = a.size();
    REAL sum = 0;

    REAL inv_size = 1.0 / static_cast<REAL>(size);
    for(long i = 0; i < size; i++)
    {
        sum += abs(a[i] - b[i]) * inv_size;
    };

    return sum;
};


/*===========================================================================*/
int compute_cosine_window(std::valarray<REAL> *window, bool half_window, REAL a0, REAL a1, REAL a2)
{
    unsigned long size = window->size();

    REAL shift = 0;
    REAL scale = 1.0;
    if(half_window)
    {
      shift = floor(static_cast<REAL>(size)/2.0);
      scale = 2.0;
    }
    REAL pi = 3.1415926535897932384626433832795;
    REAL cos_arg1 = 2.0*pi/(REAL)(size-1);
    REAL cos_arg2 = 2.0*cos_arg1;
    REAL x = 0.0;
    for(unsigned long i = 0; i < size; i++)
    {
        x = (static_cast<REAL>(i) / scale) - shift;
        (*window)[i] = a0 + a2*cos(x * cos_arg2) + a1*cos(x * cos_arg1);
    };
    return 0;
};


/*===========================================================================*/
int create_weight_window(std::valarray<REAL> *window, bool half_window, WindowType window_type)
{
    unsigned long size = window->size();
    if(size < 3) return -1; //use longer windows

    switch(window_type)
    {
        case WINDOW_TRIANGULAR:
        {
            /*
            ldiv_t div_res = div(size, (long)2);
            long win_size = size + (1 - div_res.rem); //if odd then +1 else +0; //((div_res.rem == 0) ? size : size+1);
            long half_win_size = (div(win_size, (long)2)).quot + 1;
            REAL div_2_by_win_size_minus_1 = 2.0/(REAL)(win_size-1);
            for(long i = 0; i <= half_win_size; i++)
            {
                (*window)[i] = ((REAL)i)*div_2_by_win_size_minus_1;
            };
            for(long i = half_win_size; i < size; i++)
            {
                (*window)[i] = 2.0-((REAL)i)*div_2_by_win_size_minus_1;
            };
            break;
            */
        };

        case WINDOW_HANNING:
        {
            REAL a0 = 0.5;
            REAL a1 = -0.5;
            REAL a2 = 0;
            compute_cosine_window(window, half_window, a0, a1, a2);
            break;
        };

        case WINDOW_HAMMING:
        {
            REAL a0 = 0.54;
            REAL a1 = -0.46;
            REAL a2 = 0;
            compute_cosine_window(window, half_window, a0, a1, a2);
            break;
        };

        case WINDOW_BLACKMAN_HARRIS:
        {
            REAL a0 = 0.42;
            REAL a1 = -0.5;
            REAL a2 = 0.08;
            compute_cosine_window(window, half_window, a0, a1, a2);
            break;
        };

        case WINDOW_RECTANGULAR:
        default:
        {
            for(unsigned long i = 0; i < size; i++)
            {
                (*window)[i] = 1;
            };
            break;
        };
    };

    /*
    //changing boundary values in order to avoid zeros on bounds of window (impossible to divide when doing ifft)
    if((*window)[0] == 0)
    {
        (*window)[0] = (*window)[1]*(*window)[1]/(*window)[2];
    };

    if((*window)[size-1] == 0)
    {
        (*window)[size-1] = (*window)[size-2]*(*window)[size-2]/(*window)[size-3];
    };
    */

    return 0;
};


/*===========================================================================*/
int fir_lowpass(std::valarray<REAL> *h, unsigned long length, REAL filter_frequency, WindowType win_type, bool min_phase)
{
    if(h->size() != length)
    {
        h->resize(length);
    }

    unsigned long shift = (min_phase ? 0 : (length / 2));

    // compute filter coeffs: sin(x)/x
    REAL pi = 3.1415926535897932384626433832795;
    REAL pi2f = 2 * pi * filter_frequency;
    REAL x = 0.0;
    REAL shift_dbl = static_cast<REAL>(shift);
    for(unsigned long i = 0; i < length; i++)
    {
        if(i == shift)
        {
            (*h)[i] = 2*filter_frequency;
        }
        else
        {
            x = static_cast<REAL>(i) - shift_dbl;
            (*h)[i] = sin(pi2f * x)/(pi * x);
        }
    }

    // compute window
    std::valarray<REAL> wnd(0.0, length);
    create_weight_window(&wnd, min_phase, win_type);

    //apply window and compute integral (sum)
    REAL sum = 0.0;
    for(unsigned long i = 0; i < length; i++)
    {
        (*h)[i] = (*h)[i] * wnd[i];
        sum += (*h)[i];
    }

    //normalize
    for(unsigned long i = 0; i < length; i++)
    {
        (*h)[i] = (*h)[i] / sum;
    }
    return 0;
}


/*===========================================================================*/
int convolution(const std::valarray<REAL> &data, const std::valarray<REAL> &h, std::valarray<REAL> *result)
{
    unsigned long datasize = data.size();
    unsigned long hsize = h.size();

    if(datasize < hsize) return -1;

    unsigned long resultsize = datasize - hsize + 1;

    result->resize(resultsize);
    for(unsigned long i = 0; i < resultsize; i++)
    {
        for(unsigned long m = 0; m < hsize; m++)
        {
            (*result)[i] += (h[m] * data[hsize - 1 + i - m]);
        }
    }
    return 0;
};

}
