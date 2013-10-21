#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	
	// init logs
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofLogVerbose() << "setup started";

	// init scene objects
	timeElapsed = 0;
	currentIndex = 1;

    for (int i = 0; i < SEGMENTS_STORED; ++i) {
        ceilHeights[i] = floorHeights[i] = 0;
		earthline[i] = skyline[i] = ofRectangle(0,0,0,0);
    }

	gameField = paddingTop = paddingBottom = 
		ofRectangle(0,0,0,0);

	viewPort = ofGetCurrentViewport();

	// init vertical sync and some graphics
	ofSetVerticalSync(true);
	ofSetCircleResolution(6);
	ofBackground(47, 52, 64);

	// init audio
	soundStream.listDevices();

	fft = ofxFft::create(AUDIO_BUFFER_SIZE, OF_FFT_WINDOW_RECTANGULAR);

	left.assign(AUDIO_BUFFER_SIZE, 0.0);
	right.assign(AUDIO_BUFFER_SIZE, 0.0);
	
	soundStream.setup(this, 0, 2, 44100, AUDIO_BUFFER_SIZE, 4);
	audioInput = new float[AUDIO_BUFFER_SIZE];
	fftOutput = new float[fft->getBinSize()];
	ifftOutput = new float[AUDIO_BUFFER_SIZE];

	// seed random
	ofSeedRandom();

	ofLogVerbose() << "setup finished";
}

//--------------------------------------------------------------
void testApp::update(){
    unsigned long long now = ofGetElapsedTimeMillis();
	float dt = (now - timeElapsed) / 1000.0f;
	timeElapsed = now;

	// update scene

	//offset in segments
	float beginOffset = MOVEMENT_SPEED * timeElapsed / 1000.0f;

	if (viewPort.width == 0 || viewPort.height == 0) {
		return;
	}

	gameField = viewPort;

	gameField.height = viewPort.width / VIEWPORT_ASPECT;
	gameField.y = (viewPort.height - gameField.height) / 2;
	float segmentWidth = ceil(gameField.width / SEGMENTS_PER_VIEWPORT);

	if (paddingTop.width == 0) {
		paddingTop = ofRectangle(0, 0, viewPort.width, gameField.y);
	}
	if (paddingBottom.width == 0) {
		paddingBottom = ofRectangle(0, viewPort.height - gameField.y, viewPort.width, gameField.y);
	}

	float maxSegmentHeight = gameField.height * SEGMENT_MAX_HEIGHT_PART;
	float minSegmentHeight = maxSegmentHeight / 2;

	// current offset in segments
	float offset = beginOffset - currentIndex;

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
		}
	    else {
			earthline[i].x = skyline[i].x = (i - offset) * segmentWidth ;

            if (skyline[i].x < -segmentWidth) {
                moveSegments(i);
            }
	    }
	}

	moveSegments(floor(beginOffset) - currentIndex);
	currentIndex = floor(beginOffset);
    
    
    // update signal
    
	int count = fft->getBinSize();
	float maxAmp = 0.0f;
	int maxIndex = -1;
	for (int i = 0; i < count; i++)
	{
		if (maxAmp >= fftOutput[i])
			continue;
        
		maxAmp = fftOutput[i];
		maxIndex = i;
	}

    for (int i = 0; i < count; ++i) {
        fftOutput[i] /= maxAmp;
    }
}

//--------------------------------------------------------------
void testApp::moveSegments(int count) {

	if (count <= 0) {
		return;
	}

	for (int i = 0; i < SEGMENTS_STORED - count; ++i) {
		ceilHeights[i] = ceilHeights[i + count];
		floorHeights[i] = floorHeights[i + count];

		skyline[i] = skyline[i + count];
		earthline[i] = earthline[i + count];
	}

	for (int i = SEGMENTS_STORED - count; i < SEGMENTS_STORED; i++)
	{
		floorHeights[i] = ceilHeights[i] = 0;
		earthline[i] = skyline[i] = ofRectangle(0,0,0,0);
	}
}

//--------------------------------------------------------------
void testApp::draw(){
	//drawScene();

	//plotFft();
	plotSpectrum();
}

//--------------------------------------------------------------
void testApp::drawScene() {

	ofSetColor(63, 83, 140);
    ofFill();


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
void testApp::plotFft() {
	ofRectangle area = ofRectangle(0,0, gameField.width, gameField.y);
	int count = fft->getBinSize();
	int lineHeight = 2;

	ofSetColor(221, 221, 227);
	ofSetLineWidth(lineHeight);

	float maxAmp = 0.0f;
	int maxIndex = -1;
	for (int i = 0; i < count; i++)
	{
		if (maxAmp >= fftOutput[i])
			continue;

		maxAmp = fftOutput[i];
		maxIndex = i;
	}

	for (int i = 0; i < MAX_FBAND; i++)
	{
		//float gated = max(0.0f, fftOutput[i] - maxAmp * 0.2f);
		float height = fftOutput[i] * 200;
		float x = i * lineHeight * 2 + lineHeight;
		ofLine(x, 0, x, height);
	}

	ofSetColor(255, 85, 84);
	ofDrawBitmapString(ofToString(ofGetFrameRate()) + " " + ofToString(maxAmp) + " at " + ofToString(maxIndex) + " bin size=" + ofToString(count) , 10, viewPort.height - 10);
}

//--------------------------------------------------------------
void testApp::plotSpectrum() {

	ofSetLineWidth(2);
	double maxHeight = log(MAX_FBAND) / log(2);

	for (int i = 0; i < spectrum.size(); ++i)
	{
		vector<float> line = spectrum[spectrum.size() - i - 1];

		float maxVal = 0;
		double maxIndex = 0;
		float prevHeight = 0;
		int drawPosition = 0;
		for (int j = 0; j < line.size(); ++j)
		{
			float heightValue = (log(j)/ log(2)) / maxHeight * viewPort.height;
			int delta = min(1, (int)abs(prevHeight - heightValue));
			prevHeight = heightValue;
			drawPosition += delta * 2;
			int y = drawPosition;
			
			if(maxVal < line[j])
			{
				maxVal = line[j];
				maxIndex = y;
			}

			int color = 255-line[j] * 255;
			ofSetColor(color, color, color);
			
			ofLine(i, y, i+2, y+2);
		}

		ofSetColor(240, 84, 84);
		ofLine(i, maxIndex, i+2, maxIndex+2);

		ofSetColor(184, 184, 184);
		ofLine(i, viewPort.height, i, viewPort.height - maxIndex);
	}

}

//--------------------------------------------------------------
void testApp::audioIn(float * input, int bufferSize, int nChannels){	

	for (int i = 0; i < bufferSize; i++){
		left[i]		= input[i*2];
		right[i]	= input[i*2+1];
	}

	memcpy(audioInput, left.data(), sizeof(float) * bufferSize);

	fft->setSignal(audioInput);
	memcpy(fftOutput, fft->getAmplitude(), sizeof(float) * fft->getBinSize());

	vector<float> line(fftOutput, fftOutput + MAX_FBAND);

	float maxVal;
	for (int i=0; i<line.size(); ++i)
	{
		maxVal = max(maxVal, line[i]);
	}

	for (int i=0; i<line.size(); ++i)
	{
		line[i] /= maxVal;
	}

	spectrum.push_back(line);

	fft->clampSignal();
	memcpy(ifftOutput, fft->getSignal(), sizeof(float) * fft->getSignalSize());
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
	
	viewPort.width = w;
	viewPort.height = h;
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){

}
