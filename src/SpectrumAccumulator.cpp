#include <iostream>
#include <sstream>

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
		m_nfft(nfft), m_nAvg(100), m_minFreq(minFreq), m_maxFreq(maxFreq),
		m_sampleRate(sampleRate), m_curFreq(-1)
{
	m_newCenterFreqPort = pmt::mp("new_center_freq");
	message_port_register_out(m_newCenterFreqPort);

	updateFreqStepping();
}

SpectrumAccumulator::~SpectrumAccumulator()
{
}

void SpectrumAccumulator::updateFreqStepping(void)
{
	m_resolutionBandwidth = m_sampleRate / m_nfft;
	m_freqStep = m_sampleRate;

	m_avgVector.resize((m_maxFreq - m_minFreq) / m_resolutionBandwidth);
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

void SpectrumAccumulator::resetAccumulation(void)
{
	AverageInfo empty = {0, 1};

	gr::thread::scoped_lock lock(m_mutex);

	for(AverageVector::size_type i = 0; i < m_avgVector.size(); i++) {
		m_avgVector[i] = empty;
	}

	std::cout << "Accumulation reset." << std::endl;
}

int SpectrumAccumulator::work(int noutput_items,
                              gr_vector_const_void_star &input_items,
                              gr_vector_void_star &output_items)
{
	static size_t averageCounter = 1000000;
	static size_t delayCountdown = 10;

	double freqRange = m_maxFreq - m_minFreq;

	const float *in  = reinterpret_cast<const float*>(input_items[0]);

	std::vector<gr::tag_t> tags;

	// get the tags in the current window
	get_tags_in_window(tags, 0, 0, 1);

	for(gr::tag_t &tag : tags) {
		if(pmt::write_string(tag.key) == "center_freq") {
			m_curFreq = pmt::to_double(tag.value);
		}
	}

	if(m_curFreq < m_minFreq || m_curFreq > m_maxFreq) {
		// current frequency is out of range -> request change to minimum frequency
		message_port_pub(m_newCenterFreqPort, pmt::from_double((m_minFreq + m_maxFreq)/2));
		return noutput_items;
	}

	// when averaging is done for this frequency, hop to the next
	if(averageCounter >= m_nAvg) {
		averageCounter = 0;
		delayCountdown = 0.010 /* seconds */ * m_sampleRate/m_nfft; // countdown to allow settling

		double nextFreq = m_curFreq + m_sampleRate/3;

		// wrap around and reset
		if(nextFreq > m_maxFreq) {
			nextFreq = m_minFreq;

			for(size_t i = 0; (i < m_nfft) && (i < m_avgVector.size()); i++) {
				m_avgVector[i].value /= m_avgVector[i].count;
				m_avgVector[i].count = 1;
			}

			//resetAccumulation();
		}

		message_port_pub(m_newCenterFreqPort, pmt::from_double(nextFreq));
	}

	if(freqRange > m_sampleRate) {
		if(delayCountdown == 1) {
			int startIdx = freqToIndex(m_curFreq - m_sampleRate/2);

			gr::thread::scoped_lock lock(m_mutex);
			for(int i = m_nfft * 2 / 3; (i < m_nfft) && (startIdx+i < static_cast<int>(m_avgVector.size())); i++) {
				if(startIdx + i >= 0) {
					m_avgVector[startIdx + i].value /= m_avgVector[startIdx + i].count;
					m_avgVector[startIdx + i].count = 1;
				}
			}
		} else if(delayCountdown == 0) {
			int startIdx = freqToIndex(m_curFreq - m_sampleRate/2);

			gr::thread::scoped_lock lock(m_mutex);
			for(int i = 0; (i < m_nfft) && (startIdx+i < static_cast<int>(m_avgVector.size())); i++) {
				if(i >= m_nfft/2-2 && i <= m_nfft/2+1) {
					// skip the DC
					continue;
				} else if(startIdx + i >= 0) {
					m_avgVector[startIdx + i].value += in[i];
					m_avgVector[startIdx + i].count++;
				}
			}

			averageCounter++;
		}
	} else { // display width is smaller than a single window
		AverageVector::size_type startIdx = -freqToIndex(m_curFreq - m_sampleRate/2);

		if(averageCounter == m_nAvg) {
			gr::thread::scoped_lock lock(m_mutex);
			for(size_t i = 0; (i < m_nfft); i++) {
				m_avgVector[i].value /= m_avgVector[i].count;
				m_avgVector[i].count = 1;
			}
		} else {
			gr::thread::scoped_lock lock(m_mutex);
			for(size_t i = 0; (i < m_nfft); i++) {
				m_avgVector[i].value += in[startIdx + i];
				m_avgVector[i].count++;
			}
		}

		averageCounter++;
	}

	if(delayCountdown > 0) {
		delayCountdown--;
	}

	return noutput_items;
}

void SpectrumAccumulator::getCurrentResults(SpectrumAccumulator::AmplitudeVector *result)
{
	gr::thread::scoped_lock lock(m_mutex);

	result->resize(m_avgVector.size());

	for(AverageVector::size_type i = 0; i < m_avgVector.size(); i++) {
		if(m_avgVector[i].count != 0) {
			(*result)[i] = m_avgVector[i].value / m_avgVector[i].count / m_nfft;
		} else {
			(*result)[i] = 0;
		}
	}
}
