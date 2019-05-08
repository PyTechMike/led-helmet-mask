#include <FastLED.h>

FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    12
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
#define NUM_LEDS    42
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          70
#define FRAMES_PER_SECOND  120

#define qsuba(x, b)  ((x>b)?x-b:0) 


CRGBPalette16 currentPalette=LavaColors_p;
CRGBPalette16 targetPalette=OceanColors_p;
TBlendType    currentBlending;                                                               

int LED = 13;
String data;
boolean recieved;
boolean isTurnedOn = true;
static int16_t dist; 

void setup() {
	delay(1000); 
	FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
	FastLED.setBrightness(BRIGHTNESS);

	dist = random16(12345);
	    
	pinMode(LED, OUTPUT);
	digitalWrite(LED, LOW);
	Serial.begin(9600);
	Serial.setTimeout(100);   
}

typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = {xEyesPattern, confettiRed, smartRainbow, basicRainbow, roboCop, equalizer1, equalizer2, equalizer3, police}; 
DEFINE_GRADIENT_PALETTE (glitchGradientMapBlue) { 0, 44, 169, 133, 44, 7, 255, 136, 45, 0, 0, 0, 46, 7, 255, 136, 140, 44, 169, 133, 164, 7, 255, 136, 165, 0, 0, 0, 166, 7, 255, 136, 190, 44, 169, 133,	191, 0, 0, 0, 192, 44, 169, 133, 195, 7, 255, 136, 196, 0, 0, 0, 197, 7, 255, 136, 225, 7, 255, 136, 226, 0, 0, 0, 227, 7, 255, 136, 255, 44, 169, 133 };
                                       
uint16_t xscale = 30;                                       
uint16_t yscale = 30;        
uint8_t maxPaletteChanges = 24;                                    

int lastBrightness = 80; 
int lastSensitivity = 90; 
int equalizerSenitivity = 90;
int gCurrentPatternNumber = 2; 
int lastPatternNumber = 0; 
uint8_t gHue = 0;

uint8_t policeLightsCounter = 0;
uint8_t policeLightsModeCounter = 0;
uint8_t policeLightsPartsCounter = 0;

bool isEqualizer = false;
bool isAllModes = false;

int mic = A0;
const int sampleTime = 20; 
int micOut;

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))


void loop() {
	changeByTime();
	getData();     
	setMode(); 
	showMode();  
}

void changeByTime() {
	EVERY_N_MILLISECONDS( 3 ) {
		gHue++; 
		nblendPaletteTowardPalette(currentPalette, targetPalette, maxPaletteChanges);
	}
	EVERY_N_MILLISECONDS( 50 ) {
		policeLightsCounter++;
	}
	EVERY_N_MILLISECONDS( 500 ) {
		policeLightsPartsCounter++;
	}
	EVERY_N_SECONDS( 5 ) {
		policeLightsModeCounter++;
		targetPalette = CRGBPalette16(CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 192, random8(128,255)), CHSV(random8(), 255, random8(128,255)));
	}
	EVERY_N_SECONDS( 10 ) {
		nextMode();
	}
}

void getData() {
	if (Serial.available() > 0) {                 
		recieved = true;                   
		data = Serial.readString();           
	}
}

void setMode() {
	if (recieved) {
		recieved = false;
		if(!isTurnedOn) {
			if (data == "on") {
				isTurnedOn = true;
				gCurrentPatternNumber = lastPatternNumber;
			}
		} else {
			if (data == "off") {
				isTurnedOn = false;
				lastPatternNumber = gCurrentPatternNumber;
			}
			if (data == "td") { //roboCop
				gCurrentPatternNumber = 4;
			}
			if (data == "bm") { //bpm
				gCurrentPatternNumber = 3;
			}
			if (data == "sr") { //smartRainbow
				gCurrentPatternNumber = 2;
			}
			if (data == "e1") { //equalizer1
				gCurrentPatternNumber = 5;
			} 
			if (data == "e2") { //equalizer2
				gCurrentPatternNumber = 6;
			} 
			if (data == "e3") { //equalizer3
				gCurrentPatternNumber = 7;
			} 
			if (data == "cr") { //confettiRed
				gCurrentPatternNumber = 1;
			}
			if (data == "xp") { //xPattern
				gCurrentPatternNumber = 0;
			}
			if (data == "pe") { //police
				gCurrentPatternNumber = 8;
			}
			if (data == "am") { //allModes
				isAllModes = true;
				nextMode();
			}
			if (data != "am") { //allModes
				isAllModes = false;
			}
			if(data.startsWith("bt")) { //bright
				if(data.substring(2).toInt() != 0) {
					FastLED.setBrightness(data.substring(2).toInt());
				} else {
					FastLED.setBrightness(lastBrightness);
				}
				lastBrightness = data.substring(2).toInt();
			}
			if(data.startsWith("sn")) { //senitivity
				if(data.substring(2).toInt() != 0) {
					equalizerSenitivity = data.substring(2).toInt();
				} else {
					equalizerSenitivity = lastSensitivity;
				}
				lastSensitivity = data.substring(2).toInt();
			}
		}
		if (gCurrentPatternNumber == 5 || gCurrentPatternNumber == 6 || gCurrentPatternNumber == 7) { //special for equalizer
			isEqualizer = true;
		} else {
			isEqualizer = false;
		}
		
		Serial.print(data);
	}
}

void showMode() {
	if(isTurnedOn) { 
		gPatterns[gCurrentPatternNumber]();
	}
	if(!isEqualizer) {
		fadeToBlackBy(leds, NUM_LEDS, 16);
	}
	if(isEqualizer) {
		fadeToBlackBy(leds, NUM_LEDS, 60); 
	}
	FastLED.show();
	FastLED.delay(1000 / FRAMES_PER_SECOND);
}

void nextMode() {
	if(isAllModes) {
  		gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns); 
		if (gCurrentPatternNumber == 5 || gCurrentPatternNumber == 6 || gCurrentPatternNumber == 7) { //special for equalizer
			isEqualizer = false;
			gCurrentPatternNumber = 8;
		}
	}
}

int xEyesMatrix[NUM_LEDS] = { 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 0, 1, 0, 0, 1, 0, 1, 0,
	0, 0, 1, 0, 0, 0, 0, 1, 0, 0,
	0, 1, 0, 1,       1, 0, 1, 0,
	   0, 0,             0, 0
};

void xEyesPattern() {
	matrixPatternCreater(xEyesMatrix);
}

void matrixPatternCreater(int matrixPattern[NUM_LEDS]) {
	CRGBPalette16 glitchPallete = glitchGradientMapBlue;
	for ( int i = 0; i < NUM_LEDS; i++ ) {
		if(matrixPattern[i] > 0) {
			leds[i] = ColorFromPalette(glitchPallete, gHue);
		}
	}
}

void smartRainbow() {
	int thisPhase = beatsin8(6, -64, 64);                           
	int thatPhase = beatsin8(7, -64, 64);

	for (int i = 0; i < NUM_LEDS; i++) {
		int colorIndex = cubicwave8((i * 23) + thisPhase) / 2 + cos8((i * 15)+thatPhase) / 2;           
		int thisBright = qsuba(colorIndex, beatsin8( 7, 0, 96));                                 
		leds[i] = ColorFromPalette(currentPalette, 255, thisBright, currentBlending);  
	}                                              
}

void confettiRed() {
	fadeToBlackBy( leds, NUM_LEDS, 10);
	int pos = random16(NUM_LEDS);
	leds[pos] = CHSV( 0, 200, 255);
}

void roboCop() {
	uint8_t fadeval = 240;
	uint8_t bpm = 50;

	uint8_t dot = beatsin8(bpm, 10, 19);
	leds[dot] = CHSV(gHue, 200, 255);
	leds[39 - dot] = CHSV(gHue, 200, 255);

	nscale8(leds, NUM_LEDS, fadeval);
}

int policeMatrix[NUM_LEDS] = { 
	0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
	1, 1, 1, 1,       0, 0, 0, 0,
	   0, 0,             1, 1
};

void police() {
	int isRedPart = true;  
	if(policeLightsPartsCounter % 2 == 0) {
		isRedPart = true;
	} else {
		isRedPart = false;
	}

	for ( int i = 0; i < NUM_LEDS; i++) { 
		if(policeLightsCounter % 2 == 0) { // turn on
			if(policeLightsModeCounter % 2 == 0) { // 1 mode
				if(policeMatrix[i] == 0 && isRedPart) {
					leds[i] = CHSV(0, 240, 255);
				} else if(policeMatrix[i] == 1 && !isRedPart) {
					leds[i] = CHSV(160, 240, 255);
				}
			} else {
				if(i < 20 && isRedPart) {
					leds[i] = CHSV(0, 240, 255);
				} else if(i > 19 && !isRedPart) {
					leds[i] = CHSV(160, 240, 255);
				}
			}
		} else {
			leds[i] = CHSV(0, 0, 0);
		} 
	}
}

void basicRainbow() {
	uint8_t beatA = beatsin8(20, 0, 255);                  
	uint8_t beatB = beatsin8(13, 0, 255);
	fill_rainbow(leds, NUM_LEDS, (beatA+beatB)/2, 1); 
}

	// 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	// 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	// 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	// 0, 0, 0, 0,       0, 0, 0, 0,
	//    0, 0,             0, 0

int equalizerOXOMatrix[NUM_LEDS] = { 
	9, 3, 3, 2, 9, 9, 2, 3, 3, 9,
	3, 4, 4, 3, 2, 2, 3, 4, 4, 3,
	3, 4, 4, 3, 1, 1, 3, 4, 4, 3,
	0, 3, 3, 1,       1, 3, 3, 0,
	   0, 0,             0, 0
};

void equalizer1() {
	equalizerSingle(equalizerOXOMatrix, 5);
}


int equalizerXXXMatrix[NUM_LEDS] = { 
	2, 9, 9, 5, 3, 3, 5, 9, 9, 2,
	4, 2, 5, 3, 1, 1, 3, 5, 2, 4,
	4, 2, 5, 1, 0, 0, 1, 5, 2, 4,
	4, 2, 5, 0,       0, 5, 2, 4,
	   2, 5,             5, 2
} ;
void equalizer2() {
	equalizerBreath(equalizerXXXMatrix, 6);
}


int equalizerBreathMatrix[NUM_LEDS] = { 
	4, 3, 2, 1, 0, 0, 1, 2, 3, 4,
	4, 3, 2, 1, 0, 0, 1, 2, 3, 4,
	4, 3, 2, 1, 0, 0, 1, 2, 3, 4,
	4, 3, 2, 1,       1, 2, 3, 4,
	   3, 2,             2, 3
};
void equalizer3() {
	equalizerBreath(equalizerBreathMatrix, 5);
}

void equalizerBreath(int equalizerMatrix[NUM_LEDS], int equalizerBands) {
	unsigned long startTime = millis(); 
	unsigned int amp = 0; 
	unsigned int maxAmp = 0;
	unsigned int minAmp = 1023;

	equalizerSenitivity = 100 - equalizerSenitivity;
	if(equalizerSenitivity == 0) {
		equalizerSenitivity = 5;
	} else if(equalizerSenitivity > 100) {
		equalizerSenitivity = 100;
	}

	while(millis() - startTime < sampleTime) {
	micOut = analogRead(mic);
	if( micOut < 1023) {
		if (micOut > maxAmp) {
			maxAmp = micOut; 
		}
		else if (micOut < minAmp) {
			minAmp = micOut; 
		}
	}
	}
	amp = maxAmp - minAmp;

	int fill = map(amp, 10, 10 * equalizerSenitivity, 0, equalizerBands);
	if(fill > equalizerBands) {
		fill = equalizerBands;
	} else if (fill < 0) {
		fill = 0;
	}
	fill = fill - 1;
	for(int i = 0; i < NUM_LEDS; i++) {
		if(equalizerMatrix[i] <= fill) {
			leds[i] = CHSV(gHue + equalizerMatrix[i] * 20, 255, 255);
		}
	}
}

void equalizerSingle(int equalizerMatrix[NUM_LEDS], int equalizerBands) {
	unsigned long startTime = millis(); 
	unsigned int amp = 0; 
	unsigned int maxAmp = 0;
	unsigned int minAmp = 1023;

	equalizerSenitivity = 100 - equalizerSenitivity;
	if(equalizerSenitivity == 0) {
		equalizerSenitivity = 5;
	} else if(equalizerSenitivity > 100) {
		equalizerSenitivity = 100;
	}

	while(millis() - startTime < sampleTime) {
	micOut = analogRead(mic);
	if( micOut < 1023) {
		if (micOut > maxAmp) {
			maxAmp = micOut; 
		} else if (micOut < minAmp) {
			minAmp = micOut; 
		}
	}
	}
	amp = maxAmp - minAmp;

	int fill = map(amp, 5, 10 * equalizerSenitivity, 0, equalizerBands);
	if(fill > equalizerBands) {
		fill = equalizerBands;
	} else if (fill < 0) {
		fill = 0;
	}
	fill = fill - 1;
	for(int i = 0; i < NUM_LEDS; i++) {
		if(equalizerMatrix[i] == fill) {
			leds[i] = CHSV(gHue + equalizerMatrix[i] * 20, 255, 255);
		}
	}
}