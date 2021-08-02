#include "ofApp.h"

const int max_color_val = 255;
int bg_change_rate = 5;
int bg_color = 200;
double curr_time = 0;

const int num_pts = 550;
ofVec2f points[num_pts];
vector<double> y_off(num_pts), x_off(num_pts);
float pt_radius = 3;
float pt_velocity = .50;
float speed_multiplier = 1.1;
int cloud_radius = 625;

vector<bool> is_connected(num_pts, false);
int distance_threshold = 50;
float line_width = 2;


const int num_bands = 250;
vector<int> band_x_pos(num_bands);
const int band_width = 11;
const int max_band_height = 300;
const int min_band_height = 50;
bool reached_max_band_height = false;
float sound_spectrum[num_bands];
float sound_spectrum_smoothness = .92;
float curVol = 0.0;

int options = 1;

void ofApp::setup() {

	rectBlue.set(50, 100, 100, 40);
	rectRed.set(50, 200, 100, 40);
	rectPurple.set(50, 300, 100, 40);

	soundStream.printDeviceList();

	int bufferSize = 256;

	left.assign(bufferSize, 0.0);
	right.assign(bufferSize, 0.0);
	volHistory.assign(400, 0.0);

	bufferCounter = 0;
	drawCounter = 0;
	smoothedVol = 0.0;
	scaledVol = 0.0;


	ofSoundStreamSettings settings; // getting sound stream https://github.com/openframeworks/openFrameworks/tree/master/examples/sound/audioInputExample

	auto devices = soundStream.getMatchingDevices("default");
	if (!devices.empty()) {
		settings.setInDevice(devices[0]);
	}

	settings.setInListener(this);
	settings.sampleRate = 44100;
	settings.numOutputChannels = 0;
	settings.numInputChannels = 2;
	settings.bufferSize = bufferSize;
	soundStream.setup(settings);

	// Loading image to use as background https://www.youtube.com/watch?v=eXx5aJCmbz0
	forest.load("../../images/forest.jpg");
	beach.load("../../images/forest.jpg");
	volcano.load("../../images/forest.jpg");

	for (size_t i = 0; i < num_pts; i++) {
		x_off[i] = ofRandom(0, 1000);
		y_off[i] = ofRandom(0, 1000);
		pt_colors.push_back(*(new Color(0, 0, 0)));
	}
	for (size_t i = 0; i < num_bands; i++) {
		sound_spectrum[i] = 0;
		band_x_pos[i] = (band_width + 1) * i;
	}
}

void ofApp::update() {
	ofSoundUpdate();
	updatePoints();
	updateBars();

	scaledVol = ofMap(smoothedVol, 0.0, 0.17, 0.0, 1.0, true);

	//lets record the volume into an array
	volHistory.push_back(scaledVol);

	//if we are bigger the the size we want to record - lets drop the oldest value
	if (volHistory.size() >= 400) {
		volHistory.erase(volHistory.begin(), volHistory.begin() + 1);
	}

}

void ofApp::mousePressed(int x, int y, int button) {
	blueIsClicked = rectBlue.inside(x, y);
	redIsClicked = rectRed.inside(x, y);
	purpleIsClicked = rectPurple.inside(x, y);
}

void ofApp::audioIn(ofSoundBuffer& input) {

	int numCounted = 0;

	for (size_t i = 0; i < input.getNumFrames(); i++) {
		left[i] = input[i * 2] * 0.5;
		right[i] = input[i * 2 + 1] * 0.5;

		curVol += left[i] * left[i];
		curVol += right[i] * right[i];
		numCounted += 2;
	}

	curVol /= (float)numCounted;

	curVol = sqrt(curVol);

	smoothedVol *= 0.93;
	smoothedVol += 0.07 * curVol;

	bufferCounter++;

}



void ofApp::updateColors() {
	for (size_t i = 0; i < is_connected.size(); i++) {
		if (!is_connected[i]) {
			if (options == 1) {
				pt_colors[i].r = 50;
				pt_colors[i].g = ofRandom(100, 150);
				pt_colors[i].b = ofRandom(50, 125);
			}
			else if (options == 2) {
				pt_colors[i].r = ofRandom(125, 175);
				pt_colors[i].g = 50;
				pt_colors[i].b = ofRandom(50, 125);
			}
			else {
				pt_colors[i].r = ofRandom(125, 175);
				pt_colors[i].g = 50;
				pt_colors[i].b = ofRandom(125, 175);
			}
		}
	}
}


void ofApp::updatePoints() {
	double elapsed_time = ofGetElapsedTimef();

	double dt = elapsed_time - curr_time;
	curr_time = elapsed_time;

	for (size_t i = 0; i < num_pts; i++) {

		y_off[i] += pt_velocity * dt * getMaxFrequency() * speed_multiplier;
		x_off[i] += pt_velocity * dt * getMaxFrequency() * speed_multiplier;

		// Update position using Perlin Noise 
		// https://en.wikipedia.org/wiki/Perlin_noise
		points[i].x = ofSignedNoise(x_off[i]) * cloud_radius;
		points[i].y = ofSignedNoise(y_off[i]) * cloud_radius - 50;
	}
}


float getMaxFrequency() {
	float max = 0.0F;
	for (size_t i = 0; i < num_bands; i++) {
		if (max < sound_spectrum[i] * curVol) {
			max = sound_spectrum[i];
		}
	}
	return max;
}


void ofApp::updateBars() {
	// Audio reactive shape help from this tutorial: https://www.youtube.com/watch?v=IiTsE7P-GDs
	float* val = ofSoundGetSpectrum(num_bands);
	for (int i = 0; i < num_bands; i++) {
		left[i] *= right[i];
		sound_spectrum[i] = max((right[i] * left[i]) * ofRandom(1, 10), val[i]);
	}
}

void ofApp::draw() {

	if (reached_max_band_height && bg_color <= max_color_val - bg_change_rate) {
		bg_color += bg_change_rate;
	}
	else if (bg_color >= 0) {
		bg_color -= bg_change_rate;
	}
	ofSetColor(bg_color);
	forest.draw(0, 0, ofGetWidth(), ofGetHeight());
	drawPoints();
	drawBands();

	ofDrawBitmapString("Choose your color theme", 32, 32);

	if (blueIsClicked) {
		ofSetColor(ofColor::gray);
		ofDrawRectangle(rectBlue);
		options = 1;
	}
	else {
		ofSetColor(ofColor::blue);
		ofDrawRectangle(rectBlue);
	}

	if (redIsClicked) {
		ofSetColor(ofColor::gray);
		ofDrawRectangle(rectRed);
		options = 2;
	}
	else {
		ofSetColor(ofColor::red);
		ofDrawRectangle(rectRed);
	}
	if (purpleIsClicked) {
		ofSetColor(ofColor::gray);
		ofDrawRectangle(rectPurple);
		options = 3;
	}
	else {
		ofSetColor(ofColor::purple);
		ofDrawRectangle(rectPurple);
	}




}


void ofApp::drawBands() {
	reached_max_band_height = false;
	for (int i = 0; i < num_bands; i++) {
		float band_height = left[i] * right[i] * max_band_height;

		if (options == 1) {
			ofSetColor(5, 20, 40);
		}
		else if (options == 2) {
			ofSetColor(50, 5, 5);
		}
		else {
			ofSetColor(50, 5, 50);
		}

		float band_height_threshold = max_band_height - 280;

		if (band_height >= band_height_threshold) {
			reached_max_band_height = true;

			if (options == 1) {
				ofSetColor(50, ofRandom(50, 100), ofRandom(50, 125));
			}
			else if (options == 2) {
				ofSetColor(ofRandom(125, 175), 50, ofRandom(50, 125));
			}
			else {
				ofSetColor(ofRandom(125, 175), 50, ofRandom(125, 175));
			}
		}
		ofRect(ofGetWidth() - band_x_pos[i], 0, band_width, (band_height * .6) + min_band_height);
		ofRect(band_x_pos[i], ofGetHeight(), band_width, -(band_height + min_band_height));
	}
}

void ofApp::drawPoints() {
	if (reached_max_band_height) {
		bassBoost();
	}
	else {
		clearBoost();
	}
	// Center points
	// https://stackoverflow.com/questions/12516550/openframeworks-rotate-an-image-from-its-center-through-opengl-calls
	ofPushMatrix();
	ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2);

	for (size_t i = 0; i < num_pts; i++) {
		ofSetColor(0);
		ofDrawCircle(points[i], pt_radius + 1);

		if (options == 1) {
			ofSetColor(50, ofRandom(50, 100), ofRandom(75, 150));
		}
		else if (options == 2) {
			ofSetColor(ofRandom(125, 175), 50, ofRandom(50, 125));
		}
		else {
			ofSetColor(ofRandom(125, 175), 50, ofRandom(125, 175));
		}

		ofDrawCircle(points[i], pt_radius);
	}
	connectPoints();
	ofPopMatrix();
}


void bassBoost() {
	pt_radius = 6;
	line_width = 4;
	speed_multiplier = 1.1;
}

void clearBoost() {
	pt_radius = 3;
	line_width = 2;
	speed_multiplier = .8;
}

void ofApp::connectPoints() {
	for (size_t i = 0; i < num_pts; i++) {
		is_connected[i] = false;
		for (size_t j = i + 1; j < num_pts; j++) {
			double dist = ofDist(points[i].x, points[i].y, points[j].x, points[j].y);
			if (dist < distance_threshold) {
				is_connected[i] = true;
				ofSetColor(pt_colors[i].r, pt_colors[i].g, pt_colors[i].b);
				ofSetLineWidth(line_width);
				ofDrawLine(points[i], points[j]);
			}
		}
	}
	updateColors();
}


