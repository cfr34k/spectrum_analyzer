#ifndef RECEIVER_H
#define RECEIVER_H

#include <string>

#include <osmosdr/source.h>
#include <gnuradio/top_block.h>
#include <gnuradio/fft/fft_vcc.h>
#include <gnuradio/blocks/add_cc.h>
#include <gnuradio/blocks/stream_to_vector.h>
#include <gnuradio/blocks/complex_to_mag.h>
#include <gnuradio/blocks/tags_strobe.h>
#include <gnuradio/blocks/file_sink.h>

#include "SpectrumAccumulator.h"

class Receiver
{
	private:
		size_t m_nfft;

		std::string m_devString;

		// top block
		gr::top_block_sptr     m_topBlock;

		// blocks
		osmosdr::source::sptr              m_osmosdrSource;
		gr::fft::fft_vcc::sptr             m_fft;
		gr::blocks::stream_to_vector::sptr m_streamToVec;
		gr::blocks::add_cc::sptr           m_add;
		gr::blocks::complex_to_mag::sptr   m_complexToMag;
		gr::blocks::tags_strobe::sptr      m_tagsStrobe;
		gr::blocks::file_sink::sptr        m_fileSink;
		SpectrumAccumulator::sptr          m_spectrumAccumulator;

	public:
		Receiver(const std::string &device, size_t nfft);
		~Receiver();

		void setupFlowGraph(void);
		void start(void);
		void stop(void);
};

#endif // RECEIVER_H
