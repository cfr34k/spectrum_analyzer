#ifndef SPECTRUM_ACCUMULATOR_H
#define SPECTRUM_ACCUMULATOR_H

#include <gnuradio/sync_block.h>
#include <vector>

class SpectrumAccumulator : public gr::sync_block
{
	public:
		// typedefs
		typedef boost::shared_ptr<SpectrumAccumulator> sptr;

		typedef std::vector<float> AmplitudeVector;

		static sptr make(size_t nfft, double minFreq, double maxFreq, double sampleRate);

	protected:
		SpectrumAccumulator(size_t nfft, double minFreq, double maxFreq, double sampleRate);

	private:
		size_t m_nfft;
		double m_minFreq;
		double m_maxFreq;
		double m_sampleRate;

		double m_freqStep;
		double m_resolutionBandwidth;

		void updateFreqStepping(void);

	public:
		~SpectrumAccumulator();

		void setMinFreq(double minFreq);
		void setMaxFreq(double maxFreq);
		void setSampleRate(double sampleRate);

		int work(int noutput_items,
		         gr_vector_const_void_star &input_items,
		         gr_vector_void_star &output_items);

		/*
		 * Hopefully inlined functions
		 */

		AmplitudeVector::size_type freqToIndex(double freq)
		{
			return (freq - m_minFreq) / m_resolutionBandwidth;
		}

		double getResolutionBandwidth(void)
		{
			return m_resolutionBandwidth;
		}

};

#endif // SPECTRUM_ACCUMULATOR_H
