#include "Arduino.h"

//Pin connected to ST_CP of 74HC595
int latchPin = 8;
//Pin connected to SH_CP of 74HC595
int clockPin = 12;
////Pin connected to DS of 74HC595
int dataPin = 11;

int ULbtnPin = 2;  //Upper-left button pin
int ULpreviousBtnValue = HIGH;
int URbtnPin = 3;  //Upper-left button pin
int URpreviousBtnValue = HIGH;
int LRbtnPin = 4;  //Lower-right button pin
int LRpreviousBtnValue = HIGH;
short LLbtnPin = 5;  //Lower-left button pin
int LLpreviousBtnValue = HIGH;

long lastDebounce = 0; // Last time the button was pressed
long debounceTime = 50; // Debounce delay
bool flashBool = false;
bool newRound = true;
int randomNumbers [4];
byte greenLEDState;  //Represents how many green LEDs are active

void flashGreenLEDs() {

	for(int i =0 ;i<5;i++) {

		delay(100);
		digitalWrite(latchPin, LOW);
		// shift out the bits:
		shiftOut(dataPin, clockPin, MSBFIRST, 15);
		//take the latch pin high so the LEDs will light up:
		digitalWrite(latchPin, HIGH);
		delay(100);
		digitalWrite(latchPin, LOW);
		// shift out the bits:
		shiftOut(dataPin, clockPin, MSBFIRST, 0);
		//take the latch pin high so the LEDs will light up:
		digitalWrite(latchPin, HIGH);
	}
}
void flashRedLEDs() {
	for(int i =0 ;i<5;i++) {

		delay(100);
		digitalWrite(latchPin, LOW);
		shiftOut(dataPin, clockPin, MSBFIRST, 240);
		digitalWrite(latchPin, HIGH);
		delay(100);
		digitalWrite(latchPin, LOW);
		shiftOut(dataPin, clockPin, MSBFIRST, 0);
		digitalWrite(latchPin, HIGH);
	}
}
void setSingleRedLEDhigh(String led) {

	/*
	 * Sets single red LED to high. UR = Upper Right, UL = Upper Left, LL = Lower Left, LR = Lower Right
	 */

	digitalWrite(latchPin, LOW);

	// shift out the bits:
	switch (led) {
		case "UR":
			shiftOut(dataPin, clockPin, MSBFIRST, greenLEDState^16);
			break;
		case "UL":
			shiftOut(dataPin, clockPin, MSBFIRST, greenLEDState^32);
			break;
		case "LR":
			shiftOut(dataPin, clockPin, MSBFIRST, 64);
			break;
		case "LL":
			shiftOut(dataPin, clockPin, MSBFIRST, greenLEDState^128);
			break;
		default:
			break;
	}
	//take the latch pin high so the LEDs will light up:
	digitalWrite(latchPin, HIGH);
	delay(500);
	digitalWrite(latchPin, LOW);
	shiftOut(dataPin, clockPin, MSBFIRST, greenLEDState);
	digitalWrite(latchPin, HIGH);

}

void generateRandomNumbers()  {
	//Create 4 random numbers
	//flash Red LEDs in order
	delay(1500);
	for(int i = 0; i<4; i++) {
		randomNumbers[i] =  random(0, 4);
		Serial.println(randomNumbers[i]);
		switch (randomNumbers[i]) {
			case 0:
				setSingleRedLEDhigh("UL");
				break;
			case 1:
				setSingleRedLEDhigh("UR");
				break;
			case 2:
				setSingleRedLEDhigh("LL");
				break;
			case 3:
				setSingleRedLEDhigh("LR");
				break;
			default:
				break;
		}

		delay(1000);
	}


	newRound = false;
}
void updateGreenLEDs() {
	digitalWrite(latchPin, LOW);
	// shift out the bits:
	shiftOut(dataPin, clockPin, MSBFIRST, greenLEDState);
	//take the latch pin high so the LEDs will light up:
	digitalWrite(latchPin, HIGH);
}
int getUserInput() {

	int pressedButton = -1; //Stays -1 if no button was pressed

	int buttonValueUL = digitalRead(ULbtnPin);
	int buttonValueUR = digitalRead(URbtnPin);
	int buttonValueLR = digitalRead(LRbtnPin);
	int buttonValueLL = digitalRead(LLbtnPin);

	//Check if UL Button was pressed
	if(buttonValueUL != ULpreviousBtnValue && millis() - lastDebounce >= debounceTime) {
			if(buttonValueUL == LOW && ULpreviousBtnValue == HIGH) {
				Serial.println("UL Button pressed!");
				setSingleRedLEDhigh("UL");
				pressedButton = 0;
			}
			lastDebounce = millis();
			// Change to the latest button state
			ULpreviousBtnValue = buttonValueUL;
		}
		//Check if UR Button was pressed
		if(buttonValueUR != URpreviousBtnValue && millis() - lastDebounce >= debounceTime) {
			if(buttonValueUR == LOW && URpreviousBtnValue == HIGH) {
				setSingleRedLEDhigh("UR");
				Serial.println("UR Button pressed!");
				pressedButton = 1;
			}
			lastDebounce = millis();
			// Change to the latest button state
			URpreviousBtnValue = buttonValueUR;
		}
		//Check if LR Button was pressed
		if(buttonValueLR != LRpreviousBtnValue && millis() - lastDebounce >= debounceTime) {
			if(buttonValueLR == LOW && LRpreviousBtnValue == HIGH) {
				setSingleRedLEDhigh("LR");
				Serial.println("LR Button pressed!");
				pressedButton = 3;
			}
			lastDebounce = millis();
			// Change to the latest button state
			LRpreviousBtnValue = buttonValueLR;
		}
		//Check if LL Button was pressed
		if(buttonValueLL != LLpreviousBtnValue && millis() - lastDebounce >= debounceTime) {
				if(buttonValueLL == LOW && LLpreviousBtnValue == HIGH) {
					setSingleRedLEDhigh("LL");
					Serial.println("LL Button pressed!");
					pressedButton = 2;
				}
				lastDebounce = millis();
				// Change to the latest button state
				LLpreviousBtnValue = buttonValueLL;
		}
		// Delays the execution to allow time for the serial transmission
		delay(75);
		return pressedButton;
}
bool evalAnswer(int userInput) {
	/*
	 * Checks if user's answer is correct.
	 * Green LEDs state will be used to determine what value to compare to.
	 * When 0 Greend LEDs are lit: first random number must be guessed
	 * When 1 (first) green LED is lit: second random number must be guessed
	 * When 2 (first+second) LED are lit: third random number must be guessed
	 * ...
	 */
	switch (greenLEDState) {
		case 0: // 0 LEDs on
			if(userInput == randomNumbers[0])
				return true;
			break;
		case 8: //1 LED on
			if(userInput == randomNumbers[1])
				return true;
			break;
		case 12: //2 LEDs on
			if(userInput == randomNumbers[2])
				return true;
			break;
		case 14: //3 LEDs on
			if(userInput == randomNumbers[3])
				return true;
			break;
		default:
		break;
	}
	return false;
}
void incrementGreenLEDs() {
	/* Lights up one green LED after another when guess was right*/
	greenLEDState = greenLEDState>>1;
	greenLEDState = greenLEDState^8;
}
bool gameIsWon() { //Checks if everything was guessed right
	if(greenLEDState == 15) {
		flashGreenLEDs();
		greenLEDState = 0;
		return true;
	}
	else
		return false;

}
void setup() {

  randomSeed(analogRead(0));

  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  pinMode(ULbtnPin, INPUT_PULLUP);
  pinMode(URbtnPin, INPUT_PULLUP);
  pinMode(LRbtnPin, INPUT_PULLUP);
  pinMode(LLbtnPin, INPUT_PULLUP);

  Serial.begin(9600);

}

void loop() {

	if(flashBool == false) {
		flashGreenLEDs();
		flashBool = true;
	}

	if(newRound)
		generateRandomNumbers();

	int userInput = getUserInput();
	if(userInput!=-1) {  //Button was pressed {
		Serial.println(userInput); //Check if pressed button macthes first value of random generated 4 values
		if(evalAnswer(userInput)) {
			//add 1 green LED
			incrementGreenLEDs();
			//Check if all 4 green LEDs are on
			Serial.println("Correct!");
			updateGreenLEDs();
			if(gameIsWon())
				newRound = true;
		}
		else {
			flashRedLEDs();
			newRound = true;
			greenLEDState = 0;
		}
	}

}
