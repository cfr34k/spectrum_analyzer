#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <osmosdr/source.h>
#include <gnuradio/block.h>
#include <gnuradio/blocks/tags_strobe.h>

class MessageHandler : public gr::block
{
	public:
		typedef boost::shared_ptr<MessageHandler> sptr;

		static sptr make(osmosdr::source::sptr osmosdrSource, gr::blocks::tags_strobe::sptr tagsStrobe);

	private:
		pmt::pmt_t m_setCenterFreqPort;

		osmosdr::source::sptr         m_osmosdrSource;
		gr::blocks::tags_strobe::sptr m_tagsStrobe;

		void handle_set_center_freq(pmt::pmt_t msg);

	protected:
		MessageHandler(osmosdr::source::sptr osmosdrSource, gr::blocks::tags_strobe::sptr tagsStrobe);

	public:
		~MessageHandler();

		osmosdr::source::sptr getOsmoSDRSource(void) { return m_osmosdrSource; }
};

#endif // MESSAGE_HANDLER_H
