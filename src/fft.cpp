#include "fft.h"

FFT::FFT(size_t len)
	: m_fftLen(len)
{
	m_fftwBuf = reinterpret_cast<fftw_complex*>(fftw_malloc(sizeof(fftw_complex) * len));

	m_fftwPlan = fftw_plan_dft_1d(len, m_fftwBuf, m_fftwBuf, FFTW_FORWARD, FFTW_ESTIMATE);
}

FFT::~FFT()
{
	fftw_free(m_fftwBuf);
}

void FFT::transform(const FFT::fft_complex *input, FFT::fft_complex *output)
{
	// copy the input to the FFTW buffer
	for(size_t i = 0; i < m_fftLen; i++) {
		m_fftwBuf[i][0] = input[i].real();
		m_fftwBuf[i][1] = input[i].imag();
	}

	// do the transform (in-place)
	fftw_execute(m_fftwPlan);

	// save the result in the result buffer
	for(size_t i = 0; i < m_fftLen; i++) {
		output[i] = fft_complex(m_fftwBuf[i][0], m_fftwBuf[i][1]);
	}
}
