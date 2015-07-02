#include <iostream>

#include <gnuradio/fft/window.h>
#include <gnuradio/gr_complex.h>

#include <gnuradio/blocks/message_debug.h>

#include "Receiver.h"

Receiver::Receiver(const std::string &device, size_t nfft)
	: m_nfft(nfft), m_devString(device)
{
}

Receiver::~Receiver()
{
}

void Receiver::setupFlowGraph(void)
{
	// create the blocks
	m_topBlock = gr::make_top_block("Spectrum Analyzer");

	m_source = osmosdr::source::make(m_devString);

	m_fft = gr::fft::fft_vcc::make(m_nfft,                        // length
	                               true,                          // forward
	                               gr::fft::window::hann(m_nfft), // window
	                               true,                          // shift
	                               1);                            // threads

	m_streamToVec = gr::blocks::stream_to_vector::make(sizeof(gr_complex) * 1, m_nfft);

	m_complexToMag = gr::blocks::complex_to_mag::make(m_nfft);

	m_add = gr::blocks::add_cc::make(1);

	m_tagsStrobe = gr::blocks::tags_strobe::make(sizeof(gr_complex),
	                                             pmt::from_double(1e8),    // value
	                                             m_nfft,                      // interval
	                                             pmt::intern("center_freq")); // key

	m_messageHandler = MessageHandler::make(m_source, m_tagsStrobe);

	m_source->set_bb_gain(20);
	m_source->set_if_gain(20);
	m_source->set_sample_rate(10000000);
	m_source->set_center_freq(100000000);

	//m_fileSink = gr::blocks::file_sink::make(sizeof(gr_complex), "test.out", false);

	osmosdr::freq_range_t freq_range = m_source->get_freq_range(0);

	m_spectrumAccumulator = SpectrumAccumulator::make(m_nfft, freq_range.start(), freq_range.stop(), 10000000);

	// set up connections
	m_topBlock->connect(m_source, 0, m_add, 0);
	m_topBlock->connect(m_tagsStrobe, 0, m_add, 1);

	m_topBlock->connect(m_add, 0, m_streamToVec, 0);

	m_topBlock->connect(m_streamToVec, 0, m_fft, 0);

	m_topBlock->connect(m_fft, 0, m_complexToMag, 0);

	//m_topBlock->connect(m_complexToMag, 0, m_fileSink, 0);
	m_topBlock->connect(m_complexToMag, 0, m_spectrumAccumulator, 0);

	m_topBlock->msg_connect(m_spectrumAccumulator, "new_center_freq",
	                        m_messageHandler, "set_center_freq");
}
