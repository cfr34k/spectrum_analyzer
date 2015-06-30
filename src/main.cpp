#include <iostream>

#include <SFML/Graphics.hpp>

#include "Receiver.h"
#include "version.h"

/*
void readFFTData(size_t N, FFT::fft_complex *data)
{
	for(size_t i = 0; i < N; i++) {
		uint8_t val;

		std::cin.read(reinterpret_cast<char*>(&val), sizeof(val));
		data[i].real(val / 128.);

		std::cin.read(reinterpret_cast<char*>(&val), sizeof(val));
		data[i].imag(val / 128.);
	}
}
*/

int main(void)
{
	const size_t FFT_POINTS = 1024;

	Receiver receiver("hackrf=0", FFT_POINTS);

	sf::RenderWindow window(sf::VideoMode(1024, 768), "OsmoSDR Spectrum Analyzer");

	sf::VertexArray fftLine(sf::LinesStrip, FFT_POINTS);

	sf::Color botColor = sf::Color::Red;
	sf::Color topColor = sf::Color::Green;

	receiver.setupFlowGraph();
	receiver.start();

	for(size_t i = 0; i < FFT_POINTS; i++) {
		fftLine[i].position.x = (200 / FFT_POINTS) * i + (200 / FFT_POINTS / 2);
		fftLine[i].position.y = 100;
	}

	while(window.isOpen())
	{
		sf::Event event;
		while(window.pollEvent(event))
		{
			if(event.type == sf::Event::Closed) {
				window.close();
			} else if(event.type == sf::Event::KeyPressed) {
				if(event.key.code == sf::Keyboard::Escape) {
					window.close();
				}
			} else if(event.type == sf::Event::Resized) {
				sf::FloatRect rect;
				rect.left   = 0;
				rect.top    = 0;
				rect.width  = window.getSize().x;
				rect.height = window.getSize().y;

				window.setView(sf::View(rect));

				for(size_t i = 0; i < FFT_POINTS; i++) {
					double w = window.getSize().x;

					fftLine[i].position.x = (w / FFT_POINTS) * i + (w / FFT_POINTS / 2);
				}
			}
		}

		double h = window.getSize().y;
		double scale = h / 30;
		double maxVal = FFT_POINTS;

		/*
		for(size_t i = 0; i < FFT_POINTS; i++) {
			double absVal = std::abs(fftOutput[i]);

			double dB = 10 * std::log10(absVal / maxVal);

			fftLine[i].position.y = -dB * scale;

			double alpha = fftLine[i].position.y / h;

			if     (alpha < 0) alpha = 0;
			else if(alpha > 1) alpha = 1;

			fftLine[i].color.r = botColor.r * alpha + topColor.r * (1 - alpha);
			fftLine[i].color.g = botColor.g * alpha + topColor.g * (1 - alpha);
			fftLine[i].color.b = botColor.b * alpha + topColor.b * (1 - alpha);
		}
		*/

		window.clear();
		window.draw(fftLine);
		window.display();
	}

	receiver.stop();

	std::cout << "End of Program." << std::endl;
}
