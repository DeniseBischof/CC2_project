#pragma once

#include "ofMain.h"
float getMaxFrequency();
void bassBoost();
void clearBoost();

class ofApp : public ofBaseApp {
	struct Color {
		Color(int red, int green, int blue) : r(red), g(green), b(blue) {};
		int r;
		int g;
		int b;
	};

	ofRectangle rectBlue;
	ofRectangle rectRed;
	ofRectangle rectPurple;

	bool blueIsClicked = false;
	bool redIsClicked = false;
	bool purpleIsClicked = false;

	vector <float> left;
	vector <float> right;
	vector <float> volHistory;

	int 	bufferCounter;
	int 	drawCounter;

	float smoothedVol;
	float scaledVol;

	vector<Color> pt_colors;
	ofSoundPlayer sound_player;
	ofImage forest;
	ofImage beach;
	ofImage volcano;
	double phase;


	ofSoundStream soundStream;

public:
	void setup();
	void update();
	void mousePressed(int x, int y, int button);
	void updatePoints();
	void connectPoints();
	void updateColors();
	void updateBars();
	void draw();
	void drawPoints();
	void drawBands();
	void audioIn(ofSoundBuffer& input);
};
