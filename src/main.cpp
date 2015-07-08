#include <iostream>
#include <sstream>

#include <SFML/Graphics.hpp>

#include "Receiver.h"
#include "version.h"

void resultToScreenData(const SpectrumAccumulator::AmplitudeVector &spectrum,
                        size_t nbins,
                        SpectrumAccumulator::AmplitudeVector *noiseFloor,
                        SpectrumAccumulator::AmplitudeVector *avgPower,
                        SpectrumAccumulator::AmplitudeVector *peakPower)
{
	size_t curBin = 0;
	float startSample = 0;
	float samplesPerBin = static_cast<float>(spectrum.size()) / nbins;

	noiseFloor->resize(nbins);
	avgPower->resize(nbins);
	peakPower->resize(nbins);

	while(startSample < (spectrum.size() - samplesPerBin)) {
		float endSample = startSample + samplesPerBin;

		float min = 1e9, avg = 0, max = 0;
		size_t n = 0;
		for(size_t s = startSample; s < endSample; s++) {
			avg += spectrum[s];
			n++;

			if(spectrum[s] > max) {
				max = spectrum[s];
			}

			if(spectrum[s] < min) {
				min = spectrum[s];
			}
		}

		(*noiseFloor)[curBin] = min;
		(*peakPower)[curBin] = max;
		(*avgPower)[curBin] = avg / n;

		curBin++;
		startSample = endSample;
	}
}

void updateLineFromAmplitudeVector(const SpectrumAccumulator::AmplitudeVector &vector,
                                   const sf::Vector2u &windowSize,
                                   sf::VertexArray *line)
{
	double h = windowSize.y;
	double scale = h / 50;

	for(size_t i = 0; i < vector.size(); i++) {
		double absVal = vector[i];

		double dB = 10 * std::log10(absVal);

		(*line)[i].position.y = -dB * scale;

		/*
			 double alpha = (*line)[i].position.y / h;

			 if     (alpha < 0) alpha = 0;
			 else if(alpha > 1) alpha = 1;

			 (*line)[i].color.r = botColor.r * alpha + topColor.r * (1 - alpha);
			 (*line)[i].color.g = botColor.g * alpha + topColor.g * (1 - alpha);
			 (*line)[i].color.b = botColor.b * alpha + topColor.b * (1 - alpha);
			 */
	}
}

int main(void)
{
	SpectrumAccumulator::AmplitudeVector spectrum, noiseFloor, avgPower, peakPower;

	sf::RenderWindow window(sf::VideoMode(1024, 768), "OsmoSDR Spectrum Analyzer");
	window.setFramerateLimit(30);

	sf::VertexArray cursorLine(sf::Lines, 2);
	cursorLine[0].color = sf::Color::White;
	cursorLine[1].color = sf::Color::White;

	sf::VertexArray noiseFloorLine(sf::LinesStrip, 1024);
	sf::VertexArray avgPowerLine(sf::LinesStrip, 1024);
	sf::VertexArray peakPowerLine(sf::LinesStrip, 1024);

	sf::Font infoFont;
	infoFont.loadFromFile("/usr/share/fonts/dejavu/DejaVuSans.ttf");

	sf::Text infoText("InfoText", infoFont, 30);
	infoText.setPosition(10, 10);

	sf::Color noiseFloorColor = sf::Color::Blue;
	sf::Color avgPowerColor   = sf::Color::Green;
	sf::Color peakPowerColor  = sf::Color::Red;

	Receiver receiver("hackrf=0", 4096, 12.5e6);


	receiver.setupFlowGraph();
	receiver.start();

	std::cout << "Hardware freq range: [" << receiver.getHardwareMinFreq() << ", " << receiver.getHardwareMaxFreq() << "]\n";
	std::cout << "Sweep freq range: [" << receiver.getSweepMinFreq() << ", " << receiver.getSweepMaxFreq() << "]\n";

	receiver.setSweepFreqRange(2.4e9, 2.5e9);

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

				size_t nbins = window.getSize().x;

				noiseFloorLine.resize(nbins);
				avgPowerLine.resize(nbins);
				peakPowerLine.resize(nbins);

				for(size_t i = 0; i < nbins; i++) {
					noiseFloorLine[i].position.x = i;
					avgPowerLine[i].position.x   = i;
					peakPowerLine[i].position.x  = i;

					noiseFloorLine[i].color      = noiseFloorColor;
					avgPowerLine[i].color        = avgPowerColor;
					peakPowerLine[i].color       = peakPowerColor;
				}
			}
		}

		receiver.getCurrentResults(&spectrum);
		resultToScreenData(spectrum, window.getSize().x, &noiseFloor, &avgPower, &peakPower);
		updateLineFromAmplitudeVector(noiseFloor, window.getSize(), &noiseFloorLine);
		updateLineFromAmplitudeVector(avgPower,   window.getSize(), &avgPowerLine);
		updateLineFromAmplitudeVector(peakPower,  window.getSize(), &peakPowerLine);

		window.clear();
		window.draw(cursorLine);
		window.draw(noiseFloorLine);
		window.draw(avgPowerLine);
		window.draw(peakPowerLine);
		window.draw(infoText);
		window.display();
	}

	receiver.stop();

	std::cout << "End of Program." << std::endl;
}
