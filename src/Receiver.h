#ifndef RECEIVER_H
#define RECEIVER_H

#include <string>

#include <gnuradio/top_block.h>
#include <gnuradio/fft/fft_vcc.h>
#include <gnuradio/blocks/add_cc.h>
#include <gnuradio/blocks/stream_to_vector.h>
#include <gnuradio/blocks/complex_to_mag.h>
#include <gnuradio/blocks/tags_strobe.h>
#include <gnuradio/blocks/file_sink.h>
#include <osmosdr/source.h>

#include "MessageHandler.h"
#include "SpectrumAccumulator.h"

class Receiver
{
	private:
		size_t m_nfft;

		double m_sampleRate;

		std::string m_devString;

		// blocks
		gr::top_block_sptr                 m_topBlock;

		osmosdr::source::sptr              m_source;
		gr::fft::fft_vcc::sptr             m_fft;
		gr::blocks::stream_to_vector::sptr m_streamToVec;
		gr::blocks::add_cc::sptr           m_add;
		gr::blocks::complex_to_mag::sptr   m_complexToMag;
		gr::blocks::tags_strobe::sptr      m_tagsStrobe;
		gr::blocks::file_sink::sptr        m_fileSink;
		SpectrumAccumulator::sptr          m_spectrumAccumulator;

		MessageHandler::sptr               m_messageHandler;

	public:
		Receiver(const std::string &device, size_t nfft);
		~Receiver();

		void setupFlowGraph(void);

		void start(void) { m_topBlock->start(); };
		void stop(void) { m_topBlock->stop(); };

		void getCurrentResults(SpectrumAccumulator::AmplitudeVector *result);

		bool setSweepFreqRange(double minFreq, double maxFreq);

		double getHardwareMinFreq(void);
		double getHardwareMaxFreq(void);

		double getSweepMinFreq(void);
		double getSweepMaxFreq(void);

		void resetData(void);
};

#endif // RECEIVER_H
