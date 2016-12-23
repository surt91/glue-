#include "autocorrelation.hpp"

/** Calculates the autocorrelation time \f$\tau\f$ of the given timeseries.
 *
 *  It uses a FFT to do it fast.
 *
 *  \f[ \tau = \int \mathrm{d}t S(t) \f]
 *  With the autocorrelation \f$S(t)\f$, defined as the convolution of
 *  the timeseries \f$x\f$ with itself. This can be calculated in \f$N\log N\f$
 *
 *  \f[ X = \mathrm{FFT}(x) \f] TODO
 *
 */
double autocorrelationTime(const std::vector<double> &timeseries)
{
    const size_t N = timeseries.size();
    const double m = mean(timeseries);

    kiss_fft_cfg cfg = kiss_fft_alloc(2*N-1, false, NULL, 0);

    kiss_fft_cpx *numbers = new kiss_fft_cpx[2*N-1];
    for(size_t i=0; i<N; ++i)
    {
        numbers[i].r = timeseries[i] - m;
        numbers[i].i = 0;
    }
    // zero padding
    for(size_t i=N; i<2*N-1; ++i)
    {
        numbers[i].r = 0;
        numbers[i].i = 0;
    }

    // FFT
    kiss_fft(cfg, numbers, numbers);
    free(cfg);

    // calculate pointwise numbers*conjugate(numbers) (inplace)
    for(size_t i=0; i<2*N-1; ++i)
    {
        const double tmp_r = numbers[i].r;
        const double tmp_i = numbers[i].i;
        numbers[i].r = tmp_r*tmp_r + tmp_i*tmp_i;
        numbers[i].i = 0;
    }

    // iFFT
    cfg = kiss_fft_alloc(2*N-1, true, NULL, 0);
    kiss_fft(cfg, numbers, numbers);
    free(cfg);

    // the autocorrelation is the real part
    // integrate (simple sum is sufficient) to get the autocorrelationtime
    // (just go to the first zero crossing for an upper bound and less noise)
    double tau = 0;
    for(size_t i=0; i<N; ++i)
    {
        // also normalize such that the first value is 1
        if(numbers[i].r < 0)
            break;

        //~ std::cout << numbers[i].r/numbers[0].r << "\n";

        tau += numbers[i].r/numbers[0].r;
    }

    delete numbers;

    return tau;
}
