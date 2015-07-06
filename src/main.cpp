#include <iostream>
#include <sstream>

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
	size_t npoints = 1;
	SpectrumAccumulator::AmplitudeVector spectrum;

	sf::RenderWindow window(sf::VideoMode(1024, 768), "OsmoSDR Spectrum Analyzer");

	sf::VertexArray fftLine(sf::LinesStrip, npoints);
	sf::VertexArray cursorLine(sf::Lines, 2);
	cursorLine[0].color = sf::Color::White;
	cursorLine[1].color = sf::Color::White;

	sf::Font infoFont;
	infoFont.loadFromFile("/usr/share/fonts/dejavu/DejaVuSans.ttf");

	sf::Text infoText("InfoText", infoFont, 30);
	infoText.setPosition(10, 10);

	sf::Color botColor = sf::Color::Red;
	sf::Color topColor = sf::Color::Green;

	Receiver receiver("hackrf=0", 1024);


	receiver.setupFlowGraph();
	receiver.start();

	std::cout << "Hardware freq range: [" << receiver.getHardwareMinFreq() << ", " << receiver.getHardwareMaxFreq() << "]\n";
	std::cout << "Sweep freq range: [" << receiver.getSweepMinFreq() << ", " << receiver.getSweepMaxFreq() << "]\n";

	receiver.setSweepFreqRange(10e6, 1000e6);

	receiver.getCurrentResults(&spectrum);
	npoints = spectrum.size();
	fftLine.resize(npoints);

	for(size_t i = 0; i < npoints; i++) {
		fftLine[i].position.x = (200 / npoints) * i + (200 / npoints / 2);
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
				} else if(event.key.code == sf::Keyboard::Space) {
					receiver.resetData();
				}
			} else if(event.type == sf::Event::MouseMoved) {
				size_t x = event.mouseMove.x;
				size_t y = event.mouseMove.y;
				
				size_t w = window.getSize().x;
				size_t h = window.getSize().y;

				cursorLine[0].position.x = x;
				cursorLine[1].position.x = x;

				double freq = receiver.getSweepMinFreq()
				              + (receiver.getSweepMaxFreq() - receiver.getSweepMinFreq()) * x / w;

				std::ostringstream oss;
				oss << "Freq: " << (freq/1e6) << " MHz";
				infoText.setString(oss.str());
			} else if(event.type == sf::Event::Resized) {
				sf::FloatRect rect;
				rect.left   = 0;
				rect.top    = 0;
				rect.width  = window.getSize().x;
				rect.height = window.getSize().y;

				window.setView(sf::View(rect));

				cursorLine[0].position.y = 0;
				cursorLine[1].position.y = window.getSize().y;

				for(size_t i = 0; i < npoints; i++) {
					double w = window.getSize().x;

					fftLine[i].position.x = (w / npoints) * i + (w / npoints / 2);
				}
			}
		}

		double h = window.getSize().y;
		double scale = h / 30;
		double maxVal = 1024;

		receiver.getCurrentResults(&spectrum);

		for(size_t i = 0; i < npoints; i++) {
			double absVal = spectrum[i];

			double dB = 10 * std::log10(absVal / maxVal);

			fftLine[i].position.y = -dB * scale;

			double alpha = fftLine[i].position.y / h;

			if     (alpha < 0) alpha = 0;
			else if(alpha > 1) alpha = 1;

			fftLine[i].color.r = botColor.r * alpha + topColor.r * (1 - alpha);
			fftLine[i].color.g = botColor.g * alpha + topColor.g * (1 - alpha);
			fftLine[i].color.b = botColor.b * alpha + topColor.b * (1 - alpha);
		}

		window.clear();
		window.draw(cursorLine);
		window.draw(fftLine);
		window.draw(infoText);
		window.display();
	}

	receiver.stop();

	std::cout << "End of Program." << std::endl;
}
