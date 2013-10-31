#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	
	tripno.mass = 1.0;
	tripno.velocity = 0;
	tripno.forceElastic = 0;
	tripno.forceResistance = 0;
	tripno.position = ofPoint(0, 0);

	// init logs
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofLogVerbose() << "setup started";

	// init scene objects
	timeElapsed = 0;
	currentIndex = 1;

	minFreqLog = 100;
	maxFreqLog = 0;

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

	soundStream.setup(this, 0, 2, SAMPLE_RATE, AUDIO_BUFFER_SIZE, 4);
	fftOutput = new float[fft->getBinSize()];

	// seed random
	ofSeedRandom();

	ofLogVerbose() << "setup finished";
}

//--------------------------------------------------------------
void testApp::update(){
    unsigned long long now = ofGetElapsedTimeMillis();
	float dt = (now - timeElapsed)/1000.0f;
	timeElapsed = now;
	
	// update scene
	updateBackground();
	updateTripno(dt);
}

//--------------------------------------------------------------
void testApp::updateBackground() {

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
}

//--------------------------------------------------------------
void testApp::updateTripno(float dt) {

	const int signalAmp = 10000;

	float signal = 0;

	// pop max signal from the control data
	if (control.size()) {
		soundMutex.lock();
		signal = (*max_element(begin(control), end(control))) * signalAmp;
		control.clear();
		soundMutex.unlock();
	}

	double acceleration = (signal/* as control force */ + tripno.forceElastic + tripno.forceResistance) * tripno.mass;
	
	tripno.position.y +=  tripno.velocity * dt + acceleration * dt * dt;

	tripno.velocity += acceleration * dt;

	tripno.forceElastic = - ELASTIC_KOEFF * tripno.position.y;

	tripno.forceResistance = - tripno.velocity * RESISTANCE_KOEFF;
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
	drawScene();

	//plotFft();
	//plotSpectrum();
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
	ofCircle(viewPort.width * 0.3, viewPort.height * 0.5 - tripno.position.y, viewPort.width * 0.03);
}

//--------------------------------------------------------------
void testApp::plotFft() {
	ofRectangle area = ofRectangle(0,0, gameField.width, gameField.y);
	int count = fft->getBinSize();
	int lineHeight = 2;

	ofSetColor(221, 221, 227);
	ofSetLineWidth(lineHeight);

	float* buffer = new float[count];

	soundMutex.lock();
	memcpy(buffer, fftOutput, count);
	soundMutex.unlock();

	float maxAmp = 0.0f;
	int maxIndex = -1;
	for (int i = 0; i < count; i++)
	{
		if (maxAmp >= buffer[i])
			continue;

		maxAmp = buffer[i];
		maxIndex = i;
	}

	for (int i = 0; i < MAX_FBAND; i++)
	{
		float height = buffer[i] * 200;
		float x = i * lineHeight * 2 + lineHeight;
		ofLine(x, 0, x, height);
	}

	ofSetColor(255, 85, 84);
	ofDrawBitmapString(ofToString(ofGetFrameRate()) + " " + ofToString(maxAmp) + " at " + ofToString(maxIndex) + " bin size=" + ofToString(count) , 10, viewPort.height - 10);

	delete[] buffer;
}

//--------------------------------------------------------------
void testApp::plotSpectrum() {

	soundMutex.lock();
	vector < vector < float > > buffer(spectrum.size());
	for (int i = 0; i < spectrum.size(); i++)
	{
		vector<float> newLine(spectrum[i]);
		buffer[i] = newLine;
	}
	soundMutex.unlock();

	ofSetLineWidth(2);
	double maxLog = log(MAX_FBAND) / log(2);
	int maxHeight = viewPort.height / 2;

	for (int i = 0; i < buffer.size(); ++i)
	{
		vector<float> line = buffer[buffer.size() - i - 1];

		float maxVal = 0;
		int yFrom = 0, yTo = 0;
		float maxFreq = 0;
		int prevHeight = 0;
		for (int j = 0; j < line.size(); ++j)
		{
			float logFreq = log(j)/ log(2);
			int y = logFreq / maxLog * maxHeight;
			
			if(1 == line[j])
			{
				maxVal = line[j];
				yFrom = max(0, y);
				if (yFrom != 0) {
					yTo = prevHeight;
				}
				maxFreq = logFreq;
			}

			int color = 255-line[j] * 255;
			ofSetColor(color, color, color);
			ofLine(i, maxHeight - y, i, maxHeight - prevHeight);

			prevHeight = y;
		}

		ofSetColor(240, 84, 84);
		ofLine(i, maxHeight - yFrom, i, maxHeight - yTo);

		ofSetColor(184, 184, 184);
		//ofLine(i, viewPort.height, i, viewPort.height - maxFreq / maxLog * maxHeight);
	}

	const int controlBaseLine = viewPort.height - viewPort.height / 2;
	const int signalMultiplier = 40;
	for (int i = 0; i < pitches.size(); i++)
	{
		ofSetColor(184, 184, 184);
		ofLine(i, controlBaseLine, i, controlBaseLine - control[control.size() - i - 1] * signalMultiplier);
		ofLine(i, viewPort.height, i, viewPort.height- pitches[pitches.size() - i - 1] * signalMultiplier);
	}

	ofSetColor(184, 84, 84);
	int minFreqY = viewPort.height - minFreqLog * signalMultiplier;
	int maxFreqY = viewPort.height - maxFreqLog * signalMultiplier;
	ofLine(0, minFreqY, viewPort.width, minFreqY);
	ofLine(0, maxFreqY, viewPort.width, maxFreqY);
}

//--------------------------------------------------------------
void testApp::audioIn(float * input, int bufferSize, int nChannels){	

	float* left = new float[bufferSize];
	float* right = new float[bufferSize];

	for (int i = 0; i < bufferSize; i++){
		left[i]		= input[i*2];
		right[i]	= input[i*2+1];
	}

	// Find average value of the signal
	double avgSample = 0;
	for (int i = 0; i < AUDIO_BUFFER_SIZE; i++)
	{
		avgSample += abs(left[i]);
	}
	avgSample /= AUDIO_BUFFER_SIZE;

	// Gate signal with half of average value
	for (int i = 0; i < AUDIO_BUFFER_SIZE; i++)
	{
		if (abs(left[i]) > avgSample * 0.5) {
			continue;
		}
		
		left[i] = 0;
	}

	//Get fft
	fft->setSignal(left);
	size_t count = fft->getBinSize();
	float* amplitudes = fft->getAmplitude();

	// Find average aplitude and clamp signal range
	const size_t minIndex = MIN_VOICE_FREQ * AUDIO_BUFFER_SIZE / SAMPLE_RATE;
	const size_t maxIndex = MAX_VOICE_FREQ * AUDIO_BUFFER_SIZE / SAMPLE_RATE;
	float averageAmp = 0;
	for (size_t i = 0; i < count; i++)
	{
		if (i < minIndex || i >= maxIndex)
		{
			amplitudes[i] = 0;
		}

		averageAmp += amplitudes[i];
	}
	averageAmp /= maxIndex - minIndex;

	// Gate amplitudes with average amp. And pow 2 the rest.
	for (size_t i = minIndex; i <= maxIndex; i++)
	{
		amplitudes[i] = amplitudes[i] > averageAmp 
			? amplitudes[i] * amplitudes[i]
			: 0;
	}

	// Get filtered signal with inverse fft.
	float* filteredSignal = new float[AUDIO_BUFFER_SIZE];
	memcpy(filteredSignal, fft->getSignal(), sizeof(float) * AUDIO_BUFFER_SIZE);

	// Convert signal to double array.
	double* samples = new double[AUDIO_BUFFER_SIZE];
	for (int i = 0; i < AUDIO_BUFFER_SIZE; i++)
	{
		samples[i] = left[i];
	}

	// Get pitch
	dywapitchtracker pitchtracker;
	dywapitch_inittracking(&pitchtracker);
	double freq =  max(0.0, dywapitch_computepitch(&pitchtracker, samples, 0, AUDIO_BUFFER_SIZE));

	// Get rid off the array
	delete[] filteredSignal;

	double freqLog = 0;
	double delta = 0;

	// Calculate delata (control signal)
	const double borderClampSpeed = 0.001;
	if (freq > 0)
	{
		freqLog = log(freq);

		if (freqLog < minFreqLog) {
			minFreqLog = freqLog;
		}
		else {
			minFreqLog *= 1.0 + borderClampSpeed;
		}

		if (freqLog > maxFreqLog) {
			maxFreqLog = freqLog;
		}
		else {
			maxFreqLog *= 1.0 - borderClampSpeed;
		}

		double centralFreqLog = (minFreqLog + maxFreqLog) /2;
		
		delta = freqLog - centralFreqLog;

		delta = smoothSignal(delta, control);
	}

	// Append data
	soundMutex.lock();
	control.push_back(delta);
	pitches.push_back(freqLog);
	soundMutex.unlock();
}

//--------------------------------------------------------------
float testApp::smoothSignal(float rawVal, vector<float> signal) {
	size_t count = signal.size();

	if (!count) {
		return rawVal;
	}

	return (signal[count - 1] + rawVal) / 2.0f;
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

//--------------------------------------------------------------
testApp::~testApp(){
	delete[] fftOutput;
	delete fft;
}
