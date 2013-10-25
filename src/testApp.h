#pragma once

#include "ofMain.h"
#include "ofxFft.h"
#include "dywapitchtrack.h"

#define SEGMENTS_PER_VIEWPORT 20
#define SEGMENTS_STORED SEGMENTS_PER_VIEWPORT + 1
#define SEGMENT_MAX_HEIGHT_PART 0.2
#define MOVEMENT_SPEED 2 // Segments per second
#define VIEWPORT_ASPECT 1.77777778
#define SAMPLE_RATE 44100
#define AUDIO_BUFFER_SIZE 4096
#define MAX_FBAND 200
#define MIN_VOICE_FREQ 40
#define MAX_VOICE_FREQ 3000

class testApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		void audioIn(float * input, int bufferSize, int nChannels); 

		~testApp();
	
private:
		int ceilHeights[SEGMENTS_STORED], floorHeights[SEGMENTS_STORED];
		ofRectangle skyline[SEGMENTS_STORED], earthline[SEGMENTS_STORED];
		ofRectangle paddingTop, paddingBottom;
		ofRectangle gameField, viewPort;
		int currentIndex;
		double minFreqLog;
		double maxFreqLog;

		vector < vector < float > > spectrum;
		vector <float> pitches;
		vector <float> control;

		ofxFft* fft;
		float* fftOutput;
		
		unsigned long long timeElapsed;

		ofSoundStream soundStream;
		

		ofMutex soundMutex;

		void moveSegments(int index);
		void drawScene();
		void plotFft();
		void plotSpectrum();
		float smoothSignal(float rawVal, vector<float> signal);
};
