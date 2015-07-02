#include <iostream>

#include "MessageHandler.h"

MessageHandler::sptr MessageHandler::make(osmosdr::source::sptr osmosdrSource, gr::blocks::tags_strobe::sptr tagsStrobe)
{
	return MessageHandler::sptr(new MessageHandler(osmosdrSource, tagsStrobe));
}

MessageHandler::MessageHandler(osmosdr::source::sptr osmosdrSource, gr::blocks::tags_strobe::sptr tagsStrobe)
	: gr::block("Message handler",
	            gr::io_signature::make(0, 0, 0),
	            gr::io_signature::make(0, 0, 0)),
	  m_osmosdrSource(osmosdrSource), m_tagsStrobe(tagsStrobe)
{
	m_setCenterFreqPort = pmt::mp("set_center_freq");

	message_port_register_in(m_setCenterFreqPort);
	set_msg_handler(m_setCenterFreqPort, boost::bind(&MessageHandler::handle_set_center_freq, this, _1));
}

MessageHandler::~MessageHandler()
{
}

void MessageHandler::handle_set_center_freq(pmt::pmt_t msg)
{
	double newFreq = pmt::to_double(msg);
	double realNewFreq = m_osmosdrSource->set_center_freq(newFreq);

	m_tagsStrobe->set_value(pmt::from_double(realNewFreq));

	std::cout << "Frequency adjustment requested: " << newFreq << " Hz; got " << realNewFreq << " Hz" << std::endl;
}

