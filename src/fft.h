#ifndef FFT_H
#define FFT_H

#include <fftw3.h>

#include <complex>

class FFT {
	public:
		typedef std::complex<double> fft_complex;

	private:
		size_t    m_fftLen;
		fftw_plan m_fftwPlan;

		fftw_complex *m_fftwBuf;

	public:
		FFT(size_t len);

		~FFT();

		void transform(const fft_complex *input, fft_complex *output);
};

#endif // FFT_H
