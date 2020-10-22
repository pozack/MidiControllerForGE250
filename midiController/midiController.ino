#include <MIDI.h>
#include <ezButton.h>
#include <BlynkSimpleSerialBLE.h>	// need to install Blynk library

// ## GE250 MIDI IN CC REFERENCE ##
// -------------------------------------------
// FUNCTION               CC#     VALUE
// -------------------------------------------
#define MIDI_BANK_SELECT  0     // 0-1    NOT available
#define FXA               10    // 0-127  * (Comp or Wah)
#define OD_DS             11    // 0-127  **
#define AMP               12    // 0-127
#define TONE_CAPTURE      13    // 0-127
#define CAB               14    // 0-127
#define NS                15    // 0-127
#define EQ                16    // 0-127  *	(EQ or Vol Boost)
#define FX_LOOP           17    // 0-127
#define FXB               18    // 0-127  ** (Chorus or Vol Boost)
#define DELAY             19    // 0-127  **
#define REVERB            20    // 0-127  *
#define TUNER             22    // 0-127  *
// -------------------------------------------

// ## Midi controller switch settings
#define FS_1 3		// OD/DS (Mode1), EQ (Mode2) on/off switch + mode selector by long press
#define FS_2 4		// FXB (Mode1), FXA (Mode2) on/off switch
#define FS_3 5		// Delay (Mode1), Reverb (Mode2) on/off switch + tuner by long press
#define LED_M1 8	// FS1 LED-Green (Mode1)
#define LED_M2 9	// FS1 LED-Red (Mode2)
#define LED_S2 10	// FS2 LED (TBD) * Currently unused
#define LED_S3 11	// FS3 LED (Tuner)

#if defined(ARDUINO_SAM_DUE) || defined(SAMD_SERIES)
	MIDI_CREATE_DEFAULT_INSTANCE();
#else
	#include <SoftwareSerial.h>
	using Transport = MIDI_NAMESPACE::SerialMIDI<SoftwareSerial>;
	int rxMidi = 6;
	int txMidi = 7;
	SoftwareSerial midiSerial = SoftwareSerial(rxMidi, txMidi);
	Transport serialMIDI(midiSerial);
	MIDI_NAMESPACE::MidiInterface<Transport> MIDI((Transport&)serialMIDI);
#endif

// ## Blynk connection (w/ BT) settings
char auth[] = "-xXlC2gJviLVVsETY6DQMv1UJHZ9tNsk";	// Auth Token in Blynk App
int rxBlynk = 12;
int txBlynk = 13;
SoftwareSerial SwSerial(rxBlynk, txBlynk);

ezButton btnFS1(FS_1);
ezButton btnFS2(FS_2);
ezButton btnFS3(FS_3);

const int SHORT_PRESS_TIME = 500;
const int LONG_PRESS_TIME  = 500;

// ## Setting parameters for mode selector
unsigned long pressedTimeFs1	= 0;
unsigned long releasedTimeFs1	= 0;
unsigned long pressedTimeFs3	= 0;
unsigned long releasedTimeFs3	= 0;
bool isPressingFs1		= false;
bool isLongDetectedFs1	= false;
bool isPressingFs3		= false;
bool isLongDetectedFs3	= false;
bool controlMode		= true;    // true : Mode1, false : Mode2
bool tunerMode			= false;   // tuner on by long press

// setting default state of FS 1~3
bool mPrevStateFS1    = false;
bool mPrevStateFS2    = false;
bool mPrevStateFS3    = false;

void setup() {
	Serial.begin(9600);
	btnFS1.setDebounceTime(50);
	btnFS2.setDebounceTime(50);
	btnFS3.setDebounceTime(50);

	pinMode(LED_M1, OUTPUT);
	pinMode(LED_M2, OUTPUT);
	pinMode(LED_S2, OUTPUT);
	pinMode(LED_S3, OUTPUT);

	MIDI.begin(MIDI_CHANNEL_OMNI); // channel MUST be 'OMNI'

	SerialBLE.begin(9600);
	Blynk.begin(SerialBLE, auth);
}

void loop() {
	btnFS1.loop();
	btnFS2.loop();
	btnFS3.loop();

	Blynk.run();

	// ### 1. BUTTON FS-1 / MODE SELECTOR ###
	if (btnFS1.isPressed()) {
		pressedTimeFs1 = millis();
		isPressingFs1 = true;
		isLongDetectedFs1 = false;
	}

	if (btnFS1.isReleased()) {
		releasedTimeFs1 = millis();
		isPressingFs1 = false;
		long pressDuration = releasedTimeFs1 - pressedTimeFs1;
		if (pressDuration < SHORT_PRESS_TIME) {
			// When detecting short press in FS-1, OD/DS (Mode1) & EQ (Mode2) on/off switch
			Serial.println("FS1 short press is detected!");
			tunerMode = false;
			if (mPrevStateFS1 == false) {
				if (controlMode == true) {
					MIDI.sendControlChange(OD_DS, 127, 1);
				} else {
					MIDI.sendControlChange(EQ, 127, 1);
				}
				mPrevStateFS1 = true;
			} else {
				if (controlMode == true) {
					MIDI.sendControlChange(OD_DS, 0, 1);
				} else {
					MIDI.sendControlChange(EQ, 0, 1);
				}
				mPrevStateFS1 = false;
			}
		}
	}

	if (isPressingFs1 == true && isLongDetectedFs1 == false) {
		long pressDuration = millis() - pressedTimeFs1;
		if (pressDuration > LONG_PRESS_TIME) {
			// When detecting long press in FS-1, switching selector mode
			Serial.println("FS1 long press is detected!");
			isLongDetectedFs1 = true;
			if (controlMode == true) {
				controlMode = false;  // Switch Mode1 to Mode2
				// FS1 LED change red
				digitalWrite(LED_M1, LOW);
				digitalWrite(LED_M2, HIGH);
			} else {
				controlMode = true;   // Switch Mode2 to Mode1
				// FS1 LED change green
				digitalWrite(LED_M2, LOW);
				digitalWrite(LED_M1, HIGH);
			}
		}
	}

	// ### 2. BUTTON FS-2 ###
	if (btnFS2.isPressed()) {
		Serial.println("FS2 is pressed!");
	}

	if (btnFS2.isReleased()) {
		Serial.println("FS2 is released!");
		tunerMode = false;
		if (mPrevStateFS2 == false) {
			if (controlMode == true) {
				MIDI.sendControlChange(FXB, 127, 1);
			} else {
				MIDI.sendControlChange(FXA, 127, 1);
			}
			mPrevStateFS2 = true;
		} else {
			if (controlMode == true) {
				MIDI.sendControlChange(FXB, 0, 1);
			} else {
				MIDI.sendControlChange(FXA, 0, 1);
			}
			mPrevStateFS2 = false;
		}
	}

	// ### 3. BUTTON FS-3 / TUNER ###
	if (btnFS3.isPressed()) {
		pressedTimeFs3 = millis();
		isPressingFs3 = true;
		isLongDetectedFs3 = false;
	}

	if (btnFS3.isReleased()) {
		releasedTimeFs3 = millis();
		isPressingFs3 = false;
		long pressDuration = releasedTimeFs3 - pressedTimeFs3;
		if (pressDuration < SHORT_PRESS_TIME) {
			// When detecting short press in FS-3, Delay (Mode1) & Reverb (Mode2) on/off switch
			Serial.println("FS3 short press is detected!");
			tunerMode = false;
			if (mPrevStateFS3 == false) {
				if (controlMode == true) {
					MIDI.sendControlChange(DELAY, 127, 1);
				} else {
					MIDI.sendControlChange(REVERB, 127, 1);
				}
				mPrevStateFS3 = true;
			} else {
				if (controlMode == true) {
					MIDI.sendControlChange(DELAY, 0, 1);
				} else {
					MIDI.sendControlChange(REVERB, 0, 1);
				}
				mPrevStateFS3 = false;
			}
		}
	}

	if (isPressingFs3 == true && isLongDetectedFs3 == false) {
		long pressDuration = millis() - pressedTimeFs3;
		if (pressDuration > LONG_PRESS_TIME) {
			// When detecting long press in FS-3, operating tuner
			Serial.println("FS3 long press is detected!");
			isLongDetectedFs3 = true;
			tunerMode = true;
		}
	}

	// Operating TUNER by tunerMode value
	if (tunerMode == true) {
		MIDI.sendControlChange(TUNER, 127, 1);
		// FS3 LED need to blink on
		digitalWrite(LED_S3, HIGH);
		delay(500);
		digitalWrite(LED_S3, LOW);
		delay(500);
	} else {
		MIDI.sendControlChange(TUNER, 0, 1);
		// FS3 LED need to blink off
		digitalWrite(LED_S3, LOW);
	}
}
