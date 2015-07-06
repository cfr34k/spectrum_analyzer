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
		struct AverageInfo {
			double value; // accumulated values
			size_t count; // number of accumulated values
		};

		typedef std::vector<AverageInfo> AverageVector;

		size_t m_nfft;
		size_t m_nAvg;
		double m_minFreq;
		double m_maxFreq;
		double m_sampleRate;

		double m_freqStep;
		double m_resolutionBandwidth;

		AverageVector m_avgVector;
		double        m_curFreq;

		pmt::pmt_t    m_newCenterFreqPort;

		gr::thread::mutex m_mutex;

		void updateFreqStepping(void);

	public:
		~SpectrumAccumulator();

		void setMinFreq(double minFreq);
		void setMaxFreq(double maxFreq);
		void setSampleRate(double sampleRate);

		void resetAccumulation(void);

		int work(int noutput_items,
		         gr_vector_const_void_star &input_items,
		         gr_vector_void_star &output_items);

		void getCurrentResults(AmplitudeVector *result);

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

		double getMinFreq(void)
		{
			return m_minFreq;
		}

		double getMaxFreq(void)
		{
			return m_maxFreq;
		}
};

#endif // SPECTRUM_ACCUMULATOR_H
