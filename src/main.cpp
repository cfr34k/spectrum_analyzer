#include <iostream>
#include <sstream>

#include <SFML/Graphics.hpp>

#include "Receiver.h"
#include "version.h"

void zoomRange(float *lower, float *upper, float limLower, float limUpper, float steadyPos = 0.5, float factor = 1.1, bool in = true)
{
	float width = *upper - *lower;
	float lowerDelta = width * (factor - 1) * steadyPos;
	float upperDelta = width * (factor - 1) * (1 - steadyPos);

	std::cout << "Edges before: " << *lower << " .. " << *upper << std::endl;

	// move limits
	if(in) {
		(*lower) += lowerDelta;
		(*upper) -= upperDelta;
	} else {
		(*lower) -= lowerDelta;
		(*upper) += upperDelta;
	}

	// adjust to limits
	if(*lower < limLower) {
		*lower = limLower;
	}

	if(*upper > limUpper) {
		*upper = limUpper;
	}

	std::cout << "Edges after:  " << *lower << " .. " << *upper << std::endl;
}

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
	float h = windowSize.y;
	float scale = h / 50;

	for(size_t i = 0; i < vector.size(); i++) {
		float absVal = vector[i];

		float dB = 10 * std::log10(absVal);

		(*line)[i].position.y = -dB * scale;

		/*
			 float alpha = (*line)[i].position.y / h;

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
	size_t mouseX, mouseY, w, h;
	size_t nbins;
	float freq, lowerFreq, upperFreq;

	sf::FloatRect rect;
	std::ostringstream oss;

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

	while(window.isOpen())
	{
		sf::Event event;
		while(window.pollEvent(event))
		{
			switch(event.type) {
				case sf::Event::Closed:
					window.close();
					break;

				case sf::Event::KeyPressed:
					if(event.key.code == sf::Keyboard::Escape) {
						window.close();
					} else if(event.key.code == sf::Keyboard::Space) {
						receiver.resetData();
					}
					break;

				case sf::Event::MouseMoved:
					w = window.getSize().x;
					mouseX = event.mouseMove.x;

					cursorLine[0].position.x = mouseX;
					cursorLine[1].position.x = mouseX;

					freq = receiver.getSweepMinFreq()
						+ (receiver.getSweepMaxFreq() - receiver.getSweepMinFreq()) * mouseX / w;

					oss.str(""); // clear stream contents
					oss << "Freq: " << (freq/1e6) << " MHz";
					infoText.setString(oss.str());
					break;

				case sf::Event::MouseWheelMoved:
					w = window.getSize().x;

					lowerFreq = receiver.getSweepMinFreq();
					upperFreq = receiver.getSweepMaxFreq();

					if(event.mouseWheel.delta > 0) {
						zoomRange(&lowerFreq, &upperFreq,
						          receiver.getHardwareMinFreq(), receiver.getHardwareMaxFreq(),
						          static_cast<float>(mouseX) / w,
						          1.1, true); // zoom in
					} else {
						zoomRange(&lowerFreq, &upperFreq,
						          receiver.getHardwareMinFreq(), receiver.getHardwareMaxFreq(),
						          static_cast<float>(mouseX) / w,
						          1.1, false); // zoom out
					}

					receiver.setSweepFreqRange(lowerFreq, upperFreq);

					break;

				case sf::Event::Resized:
					w = window.getSize().x;
					h = window.getSize().y;

					rect.left   = 0;
					rect.top    = 0;
					rect.width  = w;
					rect.height = h;

					window.setView(sf::View(rect));

					cursorLine[0].position.y = 0;
					cursorLine[1].position.y = h;

					nbins = window.getSize().x;

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

				default:
					/* nothing special */
					break;
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
