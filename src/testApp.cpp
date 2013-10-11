#include "testApp.h"

#define SEGMENTS_PER_VIEWPORT 20
#define SEGMENT_MAX_HEIGHT_PART 0.2
#define MOVEMENT_SPEED 100 // Pixels per second
#define VIEWPORT_ASPECT 1.77777778

ofPoint ceilSegments[SEGMENTS_PER_VIEWPORT+1], floorSegments[SEGMENTS_PER_VIEWPORT+1];

//--------------------------------------------------------------
void testApp::setup(){
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofLogVerbose() << "setup started";

	ofResetElapsedTimeCounter();

	ofRectangle viewPort = ofGetCurrentViewport();
	float segmentWidth = ceil(viewPort.width / SEGMENTS_PER_VIEWPORT);
	float maxSegmentHeight = viewPort.height * SEGMENT_MAX_HEIGHT_PART;
	float minSegmentHeight = maxSegmentHeight / 2;

	ofSeedRandom();

	for (int i = 0; i < SEGMENTS_PER_VIEWPORT + 1; ++i) {
		ceilSegments[i] = ofPoint(i * segmentWidth, ofRandom(minSegmentHeight, maxSegmentHeight));

		float floorSermentHeight = ofRandom(minSegmentHeight, maxSegmentHeight);
		floorSegments[i] = ofPoint(i * segmentWidth, floorSermentHeight);
	}

	ofLogVerbose() << "setup finished";
}

void moveSegments(int index) {
	for (int i = index + 1; i < SEGMENTS_PER_VIEWPORT + 1; ++i) {
		ceilSegments[i-1] = ceilSegments[i];
		floorSegments[i-1] = floorSegments[i];
	}

	ofRectangle viewPort = ofGetCurrentViewport();
	float segmentWidth = ceil(viewPort.width / SEGMENTS_PER_VIEWPORT);
	float maxSegmentHeight = viewPort.height * SEGMENT_MAX_HEIGHT_PART;
	float minSegmentHeight = maxSegmentHeight / 2;

	ceilSegments[SEGMENTS_PER_VIEWPORT].x += segmentWidth;
	ceilSegments[SEGMENTS_PER_VIEWPORT].y = ofRandom(minSegmentHeight, maxSegmentHeight);

	floorSegments[SEGMENTS_PER_VIEWPORT].x += segmentWidth;
	float floorSermentHeight = ofRandom(minSegmentHeight, maxSegmentHeight);
	floorSegments[SEGMENTS_PER_VIEWPORT].y = floorSermentHeight;
}

//--------------------------------------------------------------
unsigned long long timeElapsed = 0;
void testApp::update(){
    unsigned long long now = ofGetElapsedTimeMillis();
	float dt = (now - timeElapsed) / 1000.f;
	timeElapsed = now;

	float offset = MOVEMENT_SPEED * dt;

	ofRectangle viewPort = ofGetCurrentViewport();
	ofRectangle gameField = viewPort;
	gameField.height = viewPort.width / VIEWPORT_ASPECT;

	float segmentWidth = ceil(viewPort.width / SEGMENTS_PER_VIEWPORT);

	for (int i = 0; i < SEGMENTS_PER_VIEWPORT + 1; ++i) {
		ceilSegments[i].x -= offset;
		floorSegments[i].x -= offset;

		if (ceilSegments[i].x < -segmentWidth) {
			moveSegments(i);
		}
	}

	ofLogVerbose() << "update finished, dt=" << dt;
}

//--------------------------------------------------------------
void testApp::draw(){
	ofBackground(47, 52, 64);
	ofSetColor(63, 83, 140);
    ofFill();

	ofRectangle viewPort = ofGetCurrentViewport();
	float segmentWidth = ceil(viewPort.width / SEGMENTS_PER_VIEWPORT);

	for (int i = 0; i < SEGMENTS_PER_VIEWPORT + 1; ++i) {
		ofRect(ceilSegments[i].x, 0, segmentWidth, ceilSegments[i].y);
		ofRect(floorSegments[i].x, viewPort.height, segmentWidth, -ceilSegments[i].y);
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
