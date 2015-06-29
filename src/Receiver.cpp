#include <gnuradio/fft/window.h>
#include <gnuradio/gr_complex.h>

#include "Receiver.h"

Receiver::Receiver(const std::string &device)
	: m_devString(device)
{
	m_topBlock = gr::make_top_block("I/Q Receiver");
}

Receiver::~Receiver()
{
}

void Receiver::setupFlowGraph(void)
{
	// create the blocks
	m_osmosdrSource = osmosdr::source::make(m_devString);

	m_osmosdrSource->set_bb_gain(20);
	m_osmosdrSource->set_if_gain(20);
	m_osmosdrSource->set_sample_rate(2000000);
	m_osmosdrSource->set_center_freq(100000000);

	m_fft = gr::fft::fft_vcc::make(NFFT,                        // length
	                               true,                        // forward
	                               gr::fft::window::hann(NFFT), // window
	                               true,                        // shift
	                               1);                          // threads

	m_streamToVec = gr::blocks::stream_to_vector::make(sizeof(gr_complex) * 1, NFFT);

	m_complexToMag = gr::blocks::complex_to_mag::make(NFFT);

	m_add = gr::blocks::add_cc::make(1);

	m_tagsStrobe = gr::blocks::tags_strobe::make(sizeof(gr_complex),
	                                             pmt::intern("100000000"),    // value
	                                             NFFT,                        // interval
	                                             pmt::intern("center_freq")); // key

	m_fileSink = gr::blocks::file_sink::make(NFFT * sizeof(float), "test.out", false);

	// set up connections
	m_topBlock->connect(m_osmosdrSource, 0, m_add, 0);
	m_topBlock->connect(m_tagsStrobe, 0, m_add, 1);

	m_topBlock->connect(m_add, 0, m_streamToVec, 0);

	m_topBlock->connect(m_streamToVec, 0, m_fft, 0);

	m_topBlock->connect(m_fft, 0, m_complexToMag, 0);

	m_topBlock->connect(m_complexToMag, 0, m_fileSink, 0);
}

void Receiver::start(void)
{
	m_topBlock->start();
}

void Receiver::stop(void)
{
	m_topBlock->stop();
}
