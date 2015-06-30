#include <iostream>

#include <gnuradio/io_signature.h>

#include "SpectrumAccumulator.h"

SpectrumAccumulator::sptr SpectrumAccumulator::make(size_t nfft, double minFreq, double maxFreq, double sampleRate)
{
	return SpectrumAccumulator::sptr(new SpectrumAccumulator(nfft, minFreq, maxFreq, sampleRate));
}

SpectrumAccumulator::SpectrumAccumulator(size_t nfft, double minFreq, double maxFreq, double sampleRate)
	: gr::sync_block("SpectrumAccumulator",
	                 gr::io_signature::make(1, 1, nfft * sizeof(float)),
	                 gr::io_signature::make(0, 0, sizeof(float))),
	  m_nfft(nfft), m_minFreq(minFreq), m_maxFreq(maxFreq), m_sampleRate(sampleRate)
{
	updateFreqStepping();
}

SpectrumAccumulator::~SpectrumAccumulator()
{
}

void SpectrumAccumulator::updateFreqStepping(void)
{
	m_resolutionBandwidth = m_sampleRate / m_nfft;
	m_freqStep = m_sampleRate;
}

void SpectrumAccumulator::setMinFreq(double minFreq)
{
	m_minFreq = minFreq;
	updateFreqStepping();
}
void SpectrumAccumulator::setMaxFreq(double maxFreq)
{
	m_maxFreq = maxFreq;
	updateFreqStepping();
}
void SpectrumAccumulator::setSampleRate(double sampleRate)
{
	m_sampleRate = sampleRate;
	updateFreqStepping();
}

int SpectrumAccumulator::work(int noutput_items,
                              gr_vector_const_void_star &input_items,
                              gr_vector_void_star &output_items)
{
	const float *in  = reinterpret_cast<const float*>(input_items[0]);

	std::vector<gr::tag_t> tags;

	// get the tags in the current window
	get_tags_in_window(tags, 0, 0, 1);

	for(gr::tag_t &tag : tags) {
		std::cout << "Tag found: " << tag.key << " = " << tag.value << std::endl;
	}

	return noutput_items;
}
