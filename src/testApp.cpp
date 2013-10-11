#include "testApp.h"

#define SEGMENTS_PER_VIEWPORT 20
#define SEGMENTS_STORED SEGMENTS_PER_VIEWPORT + 1
#define SEGMENT_MAX_HEIGHT_PART 0.2
#define MOVEMENT_SPEED 100 // Pixels per second
#define VIEWPORT_ASPECT 1.77777778

int ceilHeights[SEGMENTS_STORED], floorHeights[SEGMENTS_STORED];
ofRectangle skyline[SEGMENTS_STORED], earthline[SEGMENTS_STORED];
ofRectangle paddingTop, paddingBottom;

//--------------------------------------------------------------
void testApp::setup(){
	
	// init logs
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofLogVerbose() << "setup started";

	// init vertical sync and some graphics
	ofSetVerticalSync(true);
	ofSetCircleResolution(80);
	ofBackground(47, 52, 64);

	// init audio
	// 0 output channels, 
	// 2 input channels
	// 44100 samples per second
	// 256 samples per buffer
	// 4 num buffers (latency)
	
	soundStream.listDevices();
	int bufferSize = 256;

	left.assign(bufferSize, 0.0);
	right.assign(bufferSize, 0.0);
	
	bufferCounter	= 0;
	drawCounter		= 0;

	soundStream.setup(this, 0, 2, 44100, bufferSize, 4);

	// seed random
	ofSeedRandom();

	// init scene objects
    for (int i = 0; i < SEGMENTS_STORED; ++i) {
        ceilHeights[i] = floorHeights[i] = 0;
		earthline[i] = skyline[i] = ofRectangle(0,0,0,0);
    }

	paddingTop = paddingBottom = ofRectangle(0,0,0,0);

	ofLogVerbose() << "setup finished";
}

void moveSegments(int index) {
	for (int i = index + 1; i < SEGMENTS_STORED; ++i) {
		ceilHeights[i-1] = ceilHeights[i];
		floorHeights[i-1] = floorHeights[i];

		skyline[i-1] = skyline[i];
		earthline[i-1] = earthline[i];
	}

	floorHeights[SEGMENTS_STORED - 1] = ceilHeights[SEGMENTS_STORED - 1] = 0;
	earthline[SEGMENTS_STORED - 1] = skyline[SEGMENTS_STORED - 1] = ofRectangle(0,0,0,0);
}

//--------------------------------------------------------------
unsigned long long timeElapsed = 0;
void testApp::update(){
    unsigned long long now = ofGetElapsedTimeMillis();
	float dt = (now - timeElapsed) / 1000.f;
	timeElapsed = now;

	// update scene
	float offset = MOVEMENT_SPEED * dt;

	ofRectangle viewPort = ofGetCurrentViewport();

	if (viewPort.width == 0 || viewPort.height == 0) {
		return;
	}

	ofRectangle gameField = viewPort;


	gameField.height = viewPort.width / VIEWPORT_ASPECT;
	gameField.y = (viewPort.height - gameField.height) / 2;

	if (paddingTop.width == 0) {
		paddingTop = ofRectangle(0, 0, viewPort.width, gameField.y);
	}
	if (paddingBottom.width == 0) {
		paddingBottom = ofRectangle(0, viewPort.height - gameField.y, viewPort.width, gameField.y);
	}

	float segmentWidth = ceil(gameField.width / SEGMENTS_PER_VIEWPORT);
	float maxSegmentHeight = gameField.height * SEGMENT_MAX_HEIGHT_PART;
	float minSegmentHeight = maxSegmentHeight / 2;

	for (int i = 0; i < SEGMENTS_STORED; ++i) {
	    if (ceilHeights[i] == 0) {// generate hights
            ceilHeights[i] = ofRandom(minSegmentHeight, maxSegmentHeight);
            floorHeights[i] = ofRandom(minSegmentHeight, maxSegmentHeight);
	    }

		if (skyline[i].width == 0) { // generate rectangles
            float startX = i==0 ? 0 : skyline[i-1].x + segmentWidth;

            skyline[i] = ofRectangle(startX, gameField.y, segmentWidth, ceilHeights[i]);
            earthline[i] = ofRectangle(startX, gameField.y + gameField.height - floorHeights[i],
                                       segmentWidth, floorHeights[i]);
			
			//ofLogVerbose() << "Gen rect viewPort="<< viewPort;
		}
	    else {
            skyline[i].x -= offset;
            earthline[i].x -= offset;

            if (skyline[i].x < -segmentWidth) {
                moveSegments(i);
            }
	    }
	}

}

//--------------------------------------------------------------
void testApp::draw(){
	ofSetColor(63, 83, 140);
    ofFill();

	ofRectangle viewPort = ofGetCurrentViewport();

	ofRect(paddingTop);
	ofRect(paddingBottom);

	for (int i = 0; i < SEGMENTS_STORED; ++i) {
		ofRect(skyline[i]);
		ofRect(earthline[i]);
	}

	ofSetColor(255, 85, 84);
    ofFill();
	ofCircle(viewPort.width * 0.3, viewPort.height * 0.5, viewPort.width * 0.03);
}

//--------------------------------------------------------------
void testApp::audioIn(float * input, int bufferSize, int nChannels){	
	
	// samples are "interleaved"
	int numCounted = bufferSize * nChannels;	

	//lets go through each sample and calculate the root mean square which is a rough way to calculate volume	
	for (int i = 0; i < bufferSize; i++){
		left[i]		= input[i*2]*0.5;
		right[i]	= input[i*2+1]*0.5;

		ofLogVerbose() << left[i] << " " << right[i];
	}

	bufferCounter++;	
}

//--------------------------------------------------------------
void testApp::keyPressed  (int key){ 
	if( key == 's' ){
		soundStream.start();
	}
	
	if( key == 'e' ){
		soundStream.stop();
	}
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
	for (int i = 0; i < SEGMENTS_STORED; ++i) {
		skyline[i] = ofRectangle(0,0,0,0);
		earthline[i] = ofRectangle(0,0,0,0);
	}

	paddingTop = paddingBottom = ofRectangle(0,0,0,0);
	
	ofLogVerbose() << "Resized w=" << w << ", h=" << h;
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){

}
