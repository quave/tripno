#include "testApp.h"

#define SEGMENTS_PER_VIEWPORT 20
#define SEGMENTS_STORED SEGMENTS_PER_VIEWPORT + 1
#define SEGMENT_MAX_HEIGHT_PART 0.2
#define MOVEMENT_SPEED 100 // Pixels per second
#define VIEWPORT_ASPECT 1.77777778

int ceilHeights[SEGMENTS_STORED], floorHeights[SEGMENTS_STORED];
ofRectangle skyline[SEGMENTS_STORED], earthline[SEGMENTS_STORED];

//--------------------------------------------------------------
void testApp::setup(){
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofLogVerbose() << "setup started";

	ofSeedRandom();

    for (int i = 0; i < SEGMENTS_STORED; ++i) {
        ceilHeights[i] = floorHeights[i] = 0;
    }

	ofLogVerbose() << "setup finished";
}

void moveSegments(int index) {
	for (int i = index + 1; i < SEGMENTS_STORED; ++i) {
		ceilHeights[i-1] = ceilHeights[i];
		floorHeights[i-1] = floorHeights[i];

		skyline[i-1] = skyline[i];
		earthline[i-1] = earthline[i];
	}

	ceilHeights[SEGMENTS_STORED - 1] = 0;
	floorHeights[SEGMENTS_STORED - 1] = 0;
}

//--------------------------------------------------------------
unsigned long long timeElapsed = 0;
void testApp::update(){
    unsigned long long now = ofGetElapsedTimeMillis();
	float dt = (now - timeElapsed) / 1000.f;
	timeElapsed = now;

	float offset = MOVEMENT_SPEED * dt;

	ofRectangle viewPort = ofGetCurrentViewport();

	if (viewPort.width == 0 || viewPort.height == 0) {
		return;
	}

	ofRectangle gameField = viewPort;

	gameField.height = viewPort.width / VIEWPORT_ASPECT;
	gameField.y = (viewPort.height - gameField.height) / 2;

	float segmentWidth = ceil(gameField.width / SEGMENTS_PER_VIEWPORT);
	float maxSegmentHeight = gameField.height * SEGMENT_MAX_HEIGHT_PART;
	float minSegmentHeight = maxSegmentHeight / 2;

	for (int i = 0; i < SEGMENTS_STORED; ++i) {
	    if (ceilHeights[i] == 0) {// generate
            float startX = i==0 ? 0 : skyline[i-1].x + segmentWidth;

            ceilHeights[i] = ofRandom(minSegmentHeight, maxSegmentHeight);
            skyline[i] = ofRectangle(startX, gameField.y, segmentWidth, ceilHeights[i]);

            floorHeights[i] = ofRandom(minSegmentHeight, maxSegmentHeight);
            earthline[i] = ofRectangle(startX, gameField.y + gameField.height - floorHeights[i],
                                       segmentWidth, floorHeights[i]);
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
	ofBackground(47, 52, 64);
	ofSetColor(63, 83, 140);
    ofFill();

	ofRectangle viewPort = ofGetCurrentViewport();
	float segmentWidth = ceil(viewPort.width / SEGMENTS_PER_VIEWPORT);

	for (int i = 0; i < SEGMENTS_STORED; ++i) {
		ofRect(skyline[i]);
		ofRect(earthline[i]);
	}

	ofSetColor(255, 85, 84);
    ofFill();
	ofCircle(viewPort.width * 0.3, viewPort.height * 0.5, viewPort.width * 0.03);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

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

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){

}
