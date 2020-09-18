#include <MIDI.h>
#include <ezButton.h> // need to install

// ## GE250 MIDI IN CC REFERENCE ##
// -------------------------------------------
// FUNCTION					CC#		VALUE
// -------------------------------------------
#define MIDI_BANK_SELECT 	0		// 0-1		NOT available (tested)
#define FXA 			 	10		// 0-127	*	(Comp or Wah)
#define OD_DS			 	11		// 0-127	**
#define AMP				 	12		// 0-127
#define TONE_CAPTURE	 	13		// 0-127
#define CAB				 	14		// 0-127
#define NS				 	15		// 0-127
#define EQ				 	16		// 0-127	*	(EQ or Vol Boost)
#define FX_LOOP			 	17		// 0-127
#define FXB				 	18		// 0-127	** (Chorus or Vol Boost)
#define DELAY			 	19		// 0-127	**
#define REVERB			 	20		// 0-127	*
#define TUNER				43		// 0-127	*
// -------------------------------------------

// ## MIDI Controller Switch Settings
#define FS_M 2	// Mode Selector Switch (Mode1, Mode2)
#define FS_1 3	// OD/DS (Mode1), EQ (Mode2) On/Off Switch
#define FS_2 4	// FXB (Mode1), FXA (Mode2) On/Off Switch
#define FS_3 5	// Delay (Mode1), Reverb (Mode2) On/Off Switch

static const uint8_t
	debounceTime = 200;	// unit (ms)

static uint8_t mPrevDebounceTimeM = 0;
static bool mStateFSM	  = HIGH;
static bool mStateFSMPrev = HIGH;
static uint8_t mPrevDebounceTime1 = 0;
static bool mStateFS1	  = HIGH;
static bool mStateFS1Prev = HIGH;
static uint8_t mPrevDebounceTime2 = 0;
static bool mStateFS2	  = HIGH;
static bool mStateFS2Prev = HIGH;
static uint8_t mPrevDebounceTime3 = 0;
static bool mStateFS3	  = HIGH;
static bool mStateFS3Prev = HIGH;

static bool controlMode	  = HIGH;	// HIGH : Mode1, LOW : Mode2
static bool editMode	  = LOW;	// edit switch mode (normal ON or OFF mode by each switch)

#if defined(ARDUINO_SAM_DUE) || defined(SAMD_SERIES)
   MIDI_CREATE_DEFAULT_INSTANCE();
#else
   #include <SoftwareSerial.h>
   using Transport = MIDI_NAMESPACE::SerialMIDI<SoftwareSerial>;
   int rxPin = 18;
   int txPin = 19;
   SoftwareSerial mySerial = SoftwareSerial(rxPin, txPin);
   Transport serialMIDI(mySerial);
   MIDI_NAMESPACE::MidiInterface<Transport> MIDI((Transport&)serialMIDI);
#endif

void setup() {
	pinMode(FS_M, INPUT_PULLUP);
	pinMode(FS_1, INPUT_PULLUP);
	pinMode(FS_2, INPUT_PULLUP);
	pinMode(FS_3, INPUT_PULLUP);

	MIDI.begin(MIDI_CHANNEL_OMNI); // Channel MUST be 'OMNI' (tested)
}

void loop() {
	// BUTTON MODE SELECTOR
	bool stateFSM = digitalRead(FS_M);
	if (stateFSM != mStateFSM) {
		if (millis() - mPrevDebounceTimeM > debounceTime) {
			mStateFSM = stateFSM;
			if (mStateFSM == LOW) {
				// FS_M OFF to ON
				controlMode = HIGH;
			} else {
				// FS_M ON to OFF
				controlMode = LOW;
			}
		}
	}
	if (stateFSM != mStateFSMPrev) {
		mPrevDebounceTimeM = millis();
		mStateFSMPrev = stateFSM;
	}

	// BUTTON SW1
	bool stateFS1 = digitalRead(FS_1);
	if (stateFS1 != mStateFS1) {
		if (millis() - mPrevDebounceTime1 > debounceTime) {
			mStateFS1 = stateFS1;
			if (mStateFS1 == LOW) {
				// FS1 OFF to ON
				if (controlMode == HIGH) {
					MIDI.sendControlChange(OD_DS, 127, 1);
				} else {
					MIDI.sendControlChange(EQ, 127, 1);
				}
			} else {
				// FS1 ON to OFF
				if (controlMode == HIGH) {
					MIDI.sendControlChange(OD_DS, 0, 1);
				} else {
					MIDI.sendControlChange(EQ, 0, 1);
				}
			}
		}
	}
	if (stateFS1 != mStateFS1Prev) {
		mPrevDebounceTime1 = millis();
		mStateFS1Prev = stateFS1;
	}

	// BUTTON SW2
	bool stateFS2 = digitalRead(FS_2);
	if (stateFS2 != mStateFS2) {
		if (millis() - mPrevDebounceTime2 > debounceTime) {
			mStateFS2 = stateFS2;
			if (mStateFS2 == LOW) {
				// FS2 OFF to ON
				if (controlMode == HIGH) {
					MIDI.sendControlChange(FXB, 127, 1);
				} else {
					MIDI.sendControlChange(FXA, 127, 1);
				}
			} else {
				// FS2 ON to OFF
				if (controlMode == HIGH) {
					MIDI.sendControlChange(FXB, 0, 1);
				} else {
					MIDI.sendControlChange(FXA, 0, 1);
				}
			}
		}
	}
	if (stateFS2 != mStateFS2Prev) {
		mPrevDebounceTime2 = millis();
		mStateFS2Prev = stateFS2;
	}

	// BUTTON SW3
	bool stateFS3 = digitalRead(FS_3);
	if (stateFS3 != mStateFS3) {
		if (millis() - mPrevDebounceTime3 > debounceTime) {
			mStateFS3 = stateFS3;
			if (mStateFS3 == LOW) {
				// FS3 OFF to ON
				if (controlMode == HIGH) {
					MIDI.sendControlChange(DELAY, 127, 1);
				} else {
					MIDI.sendControlChange(REVERB, 127, 1);
				}
			} else {
				// FS3 ON to OFF
				if (controlMode == HIGH) {
					MIDI.sendControlChange(DELAY, 0, 1);
				} else {
					MIDI.sendControlChange(REVERB, 0, 1);
				}
			}
		}
	}
	if (stateFS3 != mStateFS3Prev) {
		mPrevDebounceTime3 = millis();
		mStateFS3Prev = stateFS3;
	}
}
