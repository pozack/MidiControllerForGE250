#include <MIDI.h>
#include <ezButton.h>

// ## GE250 MIDI IN CC REFERENCE ##
// -------------------------------------------
// FUNCTION					CC#		VALUE
// -------------------------------------------
#define MIDI_BANK_SELECT  0		  // 0-1		NOT available
#define FXA 			 	      10		// 0-127	*	(Comp or Wah)
#define OD_DS			 	      11		// 0-127	**
#define AMP				 	      12		// 0-127
#define TONE_CAPTURE	 	  13		// 0-127
#define CAB				 	      14		// 0-127
#define NS				 	      15		// 0-127
#define EQ				 	      16		// 0-127	*	(EQ or Vol Boost)
#define FX_LOOP			 	    17		// 0-127
#define FXB				 	      18		// 0-127	** (Chorus or Vol Boost)
#define DELAY			 	      19		// 0-127	**
#define REVERB			 	    20		// 0-127	*
#define TUNER				      22		// 0-127	*
// -------------------------------------------

// ## MIDI Controller Switch Settings
#define FS_M 2	// mode selector (Mode1, Mode2) by shor press, tuner on by long press
#define FS_1 3	// OD/DS (Mode1), EQ (Mode2) on/off switch
#define FS_2 4	// FXB (Mode1), FXA (Mode2) on/off switch
#define FS_3 5	// Delay (Mode1), Reverb (Mode2) on/off switch

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

ezButton btnFSM(FS_M);
ezButton btnFS1(FS_1);
ezButton btnFS2(FS_2);
ezButton btnFS3(FS_3);

const int SHORT_PRESS_TIME = 500;
const int LONG_PRESS_TIME = 500;

// setting parameters for mode selector
unsigned long pressedTime   = 0;
unsigned long releasedTime  = 0;
bool isPressing     = false;
bool isLongDetected = false;
bool controlMode    = true;    // true : Mode1, false : Mode2
bool tunerMode      = false;   // tuner on by long press

// setting default state of FS 1~3
bool mPrevStateFS1    = false;
bool mPrevStateFS2    = false;
bool mPrevStateFS3    = false;

void setup() {
  Serial.begin(9600);
  btnFSM.setDebounceTime(50);
  btnFS1.setDebounceTime(50);
  btnFS2.setDebounceTime(50);
  btnFS3.setDebounceTime(50);
  
	MIDI.begin(MIDI_CHANNEL_OMNI); // channel MUST be 'OMNI'
}

void loop() {

  btnFSM.loop();
  btnFS1.loop();
  btnFS2.loop();
  btnFS3.loop();

  // ### 1. BUTTON MODE SELECTOR ###
  if (btnFSM.isPressed()) {
    pressedTime = millis();
    isPressing = true;
    isLongDetected = false;
  }

  if (btnFSM.isReleased()) {
    releasedTime = millis();
    isPressing = false;

    long pressDuration = releasedTime - pressedTime;

    if (pressDuration < SHORT_PRESS_TIME) {
      // Switch Control Mode (Mode1 <--> Mode2)
      Serial.println("A short press is detected!");

      tunerMode = false;

      if (controlMode == true)
        controlMode = false;  // Switch Mode1 to Mode2
        // FS_M LED change red
      else
        controlMode = true;   // Switch Mode2 to Mode1
        // FS_M LED change green
    }
  }

  if (isPressing == true && isLongDetected == false) {
    long pressDuration = millis() - pressedTime;

    if (pressDuration > LONG_PRESS_TIME) {
      Serial.println("A long press is detected!");
      isLongDetected = true;
      tunerMode = true;
    }
  }

  if (tunerMode == true)
    // tuner on
    MIDI.sendControlChange(TUNER, 127, 1);
    // FS_M LED neen to blink on
  else
    // tuner off
    MIDI.sendControlChange(TUNER, 0, 1);
    // FS_M LED need to blink


	// ### 2. BUTTON SW1 ###
  if (btnFS1.isPressed() && tunerMode == false) {
    Serial.println("The FS_1 is pressed!");
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

  if (btnFS1.isReleased()) {
    Serial.println("The FS_1 is released!");
  }


  // ### 3. BUTTON SW2 ###
  if (btnFS2.isPressed() && tunerMode == false) {
    Serial.println("The FS_2 is pressed!");
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

  if (btnFS2.isReleased()) {
    Serial.println("The FS_2 is released!");
  }


  // ### 4. BUTTON SW3 ###
  if (btnFS3.isPressed() && tunerMode == false) {
    Serial.println("The FS_3 is pressed!");
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

  if (btnFS3.isReleased()) {
    Serial.println("The FS_3 is released!");
  }
}
