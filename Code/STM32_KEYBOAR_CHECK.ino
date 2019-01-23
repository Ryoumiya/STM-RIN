//STM32duino
#include <USBComposite.h> //USB Keyboard & Mouse Library
	USBHID HID;					//HID 
	HIDKeyboard Keyboard(HID);	//Keyboard
	HIDMouse Mouse(HID); 		//Mouse

	


//GLOBAL VARIABLES
uint32_t Mil;
const uint16_t DelayBetweenRuns[2] = {100,30};	// 0 = Keyboard at 100milis; 1 = Mouse at 30milis
uint32_t WorkingDelayTime[2];	//The value that get change based on milis()
int8_t XKeycoords[3],Ykeycoords[3]; //XY of the keys
int16_t PointerLoct[2]; 		// 0 = X , 1 = Y
uint8_t ModVAL; 				//Sets the modifier Pins
/*
	0	NC 
	1	SHIFT
	2	CAPS 
	3	Alternate Key 1
	4	Alternate Key 2
	5	SHIFT COUNTER
	6	NC
	7	XY OVER 3 COUNTER
*/


//Resets the list of pressed pin coordinates
void Reset_XYcoordCount(){
	for(uint8_t i = 0;i < 3;i++){
		XKeycoords[i] = -1;
		Ykeycoords[i] = -1;
	}
}

//If the bit is set then activate the Pin 
void setDigit(uint8_t BytIN){
  //PIN 0
  if(bitRead(BytIN,0)){
    digitalWrite(PB12,HIGH);
  }else {
    digitalWrite(PB12,LOW);
  }

  //PIN 1
  if(bitRead(BytIN,1)){
    digitalWrite(PB13,HIGH);
  }else {
    digitalWrite(PB13,LOW);
  }

  //PIN 2
  if(bitRead(BytIN,2)){
    digitalWrite(PB14,HIGH);
  }else {
    digitalWrite(PB14,LOW);
  }

  //PIN 3
  if(bitRead(BytIN,3)){
    digitalWrite(PB5,HIGH);
  }else {
    digitalWrite(PB5,LOW);
  }

  //PIN 4
  if(bitRead(BytIN,4)){
    digitalWrite(PB6,HIGH);
  }else {
    digitalWrite(PB6,LOW);
  }

  //PIN 5
  if(bitRead(BytIN,5)){
    digitalWrite(PB7,HIGH);
  }else {
    digitalWrite(PB7,LOW);
  }

  //PIN 6
  if(bitRead(BytIN,6)){
    digitalWrite(PB8,HIGH);
  }else {
    digitalWrite(PB8,LOW);
  }

  //PIN 7
  if(bitRead(BytIN,7)){
    digitalWrite(PB9,HIGH);
  }else {
    digitalWrite(PB9,LOW);
  }
}

//Please use this for actually Setting the pins
void setOnlyPin(uint8_t nmb){
  setDigit(1 << nmb);
}

//FOR TESTING ONLY
void DEBUG_TEST_PINCYCLE(){
	for(uint8_t i = 0; i < 8;i++){
		setOnlyPin(i);
		delay(1000);
	}
}


//Reads the key press of the asked pin
bool ReadKeypress(uint8_t pinMun){
	switch(pinMun){
		case 0:
			return digitalRead(PB1);
		break;
		case 1:
			return digitalRead(PB0);
		break;
		case 2:
	  	return digitalRead(PA7);
		break;
		case 3:
			return digitalRead(PA6);
		break;
		case 4:
			return digitalRead(PA5);
		break;
		case 5:
			return digitalRead(PA4);
		break;
		default:
			return -1;
		break;
	}
}

//Same as with the other just getting the key coords
void WriteKeyCoords(uint8_t x,uint8_t y){
	uint8_t i = 0;
	while(1){
		if(i > 2){//if the Array is full then set the GLOBAL detect
			bitSet(ModVAL,7);
			return;
		}
		
		// if the pin not used
		if(XKeycoords[i] == -1){
			XKeycoords[i] = x; // Writes the value to array
			Ykeycoords[i] = y;
			return;
		}
		i++; // the array loct is used then increment the number
	}
}

void ReadKeyboard(){
	Reset_XYcoordCount();// Resets the Array and ScanOver3
	
	//Read pin(0-5) = X function
	for(uint8_t ReadPIN = 0; ReadPIN < 6; ReadPIN++){
		
		// Scan Pin (0-7)= Y function
		for(uint8_t ScanPIN = 0; ScanPIN < 8; ScanPIN++){
			
			if(bitRead(ModVAL,7)){ // If the key there more than 3 key pressed, stop scanning for more
				return ;
			}
			
			setOnlyPin(ScanPIN); // set the scan line
			
			if(ReadKeypress(ReadPIN)){//If it HIGH then 
				WriteKeyCoords(ReadPIN,ScanPIN);//Puts the coordinates in the array
			}
			
			
		}
	}
}

//Read the Stick analog Value
void ReadMouse(){
	//0 = x value; 1 = Y value
	int16_t Actual[2];
	Actual[0] = analogRead(PA1); // reads the actual Value
	Actual[1] = analogRead(PA2); // Reads the actual Value
	Actual[0] = Actual[0] / 16; //Scale it down to 8 bits
	Actual[1] = Actual[1] / 16; //Scale it down to 8 bits
	PointerLoct[0] = Actual[0]; //Move the value to PointerLoct
	PointerLoct[1] = Actual[1]; 
}

//Calculate with averaging
int8_t MagnitueCal(double x){
	x -= 128;
	x /= 47;
	x = x * x * x;
	int8_t z = (int8_t) x;
	return z;
}

void MouseMagnitude(){
	PointerLoct[0] = MagnitueCal(PointerLoct[0]);
	PointerLoct[1] = ((MagnitueCal(PointerLoct[1])) * -1);
}


void DEBUG_SERIAL_PRINT(){
	Serial.println("VALUES :");
	for(uint8_t i = 0; i < 3; i++){
		Serial.print('(');
		Serial.print(XKeycoords[i]);
		Serial.print(',');
		Serial.print(Ykeycoords[i]);
		Serial.print(')');
		Serial.print('\t');
	}
	Serial.print(",,");
	Serial.print(PointerLoct[0]);
	Serial.print(',');
	Serial.print(PointerLoct[1]);
	Serial.print('\n');
	Serial.print('\n');
}


void MouseCall(){
	ReadMouse();
	MouseMagnitude();
	Mouse.move(PointerLoct[0],PointerLoct[1]);
}

void DEBUG_CALL(){
	ReadMouse();
	MouseMagnitude();
	ReadKeyboard();
	Get_ODD_CASES();
	DEBUG_SERIAL_PRINT();
	delay(100);
}

// The following keys functions call are needed to simplify the code hopefully...

//Gets the alternate Keys Functions
void GetAlternate(){
	//Caps Function...
	if(bitRead(ModVAL,2) || bitRead(ModVAL,1)){
		Keyboard.press(KEY_LEFT_SHIFT);
	}
	
	if(bitRead(ModVAL,3)){
		Keyboard.press(KEY_RIGHT_ALT);
	}
	
	if(bitRead(ModVAL,4)){
		Keyboard.press(KEY_LEFT_CTRL);
	}
}

void GetShiftToggle(){
	//if the counter reaches 1 and the shift key is active then 
	if(bitRead(ModVAL,5) && bitRead(ModVAL,1) ){
		bitClear(ModVAL,5);
		bitClear(ModVAL,1);
	}else if (bitRead(ModVAL,1) && bitRead(ModVAL,5) == 0){
		bitSet(ModVAL,5);
	}
}

// Keys are decided by they're location in X and Y


void Get_ButtonPress(uint8_t x,uint8_t y){
	GetAlternate();
	switch (x) {
		case 0:
			Get_X_0_Buttons(y);
		break;
		case 1:
			Get_X_1_Buttons(y);
		break;
		case 2:
			Get_X_2_Buttons(y);
		break;
		case 3:
			Get_X_3_Buttons(y);
		break;
		case 4:
			Get_X_4_Buttons(y);
		break;
		case 5:
			Get_X_5_Buttons(y);
		break;
		default:
			return ;
		break;
	}
	//keep it pressed for 30 mils
	delay(30);
	//Every Key Press uses the Keyboard press function 
	Keyboard.releaseAll();
	//Clears or add into counter shift function
	GetShiftToggle();
}

//The return the button press from the coordinates
void Get_X_0_Buttons(uint8_t y){
	switch (y) {
		//	0 , 0
		case 0:
			Keyboard.press('g');
		break;
		
		// 	0 , 1
		case 1:
			Keyboard.press('f');
		break;
		
		//	0 , 2
		case 2:
			Keyboard.press('y');
		break;
		
		//	0 , 3
		case 3:
			Keyboard.press('p');
		break;
		
		//	0 , 4
		case 4:
			Keyboard.releaseAll();
			Keyboard.press(KEY_RIGHT_ARROW);
		break;
		
		//	0 , 5
		case 5:
			Keyboard.releaseAll();
			Keyboard.press(KEY_LEFT_ARROW);
		break;
		
		//	0 , 6
		case 6:
			Keyboard.press('j');
		break;
		
		//	0 , 7
		case 7:
			//XKeycoords[0] = -1;
			//Ykeycoords[0] = -1;
			Keyboard.press('x');
		break;
		default:
			return ;
		break;
	}
}

void Get_X_1_Buttons(uint8_t y){
	switch (y) {
		//	1 , 0
		case 0:
			Keyboard.press('i');
		break;
		
		// 	1 , 1
		case 1:
			Keyboard.press('u');
		break;
		
		//	1 , 2
		case 2:
			Keyboard.press('e');
		break;
		
		//	1 , 3
		case 3:
			Keyboard.press('o');
		break;
		
		//	1 , 4
		case 4:
			Keyboard.press('a');
		break;
		
		//	1 , 5
		case 5:
			Keyboard.press('k');
		break;
		
		//	1 , 6
		case 6:
			Keyboard.releaseAll();
			Keyboard.press(KEY_TAB);
		break;
		
		//	1 , 7
		case 7:
			//Toggle Caps
			if(bitRead(ModVAL,1)){
				bitClear(ModVAL,2);
			}else{
				bitSet(ModVAL,1);
			}
		break;
		default:
			return ;
		break;
	}
}

void Get_X_2_Buttons(uint8_t y){
	switch (y) {
		//	2 , 0
		case 0:
			//	NC
		break;
		
		// 	2 , 1
		case 1:
			//ACTIVATE ALTERNATE 1
			if(bitRead(ModVAL,3) || bitRead(ModVAL,4)){
				//if any of alt function is on then clear both of them
				bitClear(ModVAL,3);
				bitClear(ModVAL,4);
			}else{
				//if not then toggle it on
				bitSet(ModVAL,3);
			}
		break;
		
		//	2 , 2
		case 2:
			Keyboard.press(',');
		break;
		
		//	2 , 3
		case 3:
			Keyboard.press('.');
		break;
		
		//	2 , 4
		case 4:
			Mouse.click(MOUSE_RIGHT);
		break;
		
		//	2 , 5
		case 5:
			Mouse.click(MOUSE_LEFT);
		break;
		
		//	2 , 6
		case 6:
			//	NC
		break;
		
		//	2 , 7
		case 7:
			Keyboard.press('q');
		break;
		default:
			return ;
		break;
	}
}

void Get_X_3_Buttons(uint8_t y){
	switch (y) {
		//	3 , 0
		case 0:
			Keyboard.press('b');
		break;
		
		// 	3 , 1
		case 1:
			Keyboard.press('c');
		break;
		
		//	3 , 2
		case 2:
			Keyboard.press('r');
		break;
		
		//	3 , 3
		case 3:
			Keyboard.press('l');//L
		break;
		
		//	3 , 4
		case 4:
			Keyboard.releaseAll();
			Keyboard.press(KEY_UP_ARROW);
		break;
		
		//	3 , 5
		case 5:
			Keyboard.releaseAll();
			Keyboard.press(KEY_DOWN_ARROW);
		break;
		
		//	3 , 6
		case 6:
			// NC
		break;
		
		//	3 , 7
		case 7:
			Keyboard.press('z');
		break;
		default:
			return ;
		break;
	}
}

void Get_X_4_Buttons(uint8_t y){
	switch (y) {
		//	4 , 0
		case 0:
			Keyboard.press('d');
		break;
		
		// 	4 , 1
		case 1:
			Keyboard.press('t');
		break;
		
		//	4 , 2
		case 2:
			Keyboard.press('h');
		break;
		
		//	4 , 3
		case 3:
			Keyboard.press('n');
		break;
		
		//	4 , 4
		case 4:
			Keyboard.press('s');
		break;
		
		//	4 , 5
		case 5:
			Keyboard.press('m');
		break;
		
		//	4 , 6
		case 6:
			Keyboard.press('w');
		break;
		
		//	4 , 7
		case 7:
			Keyboard.press('v');
		break;
		default:
			return ;
		break;
	}
}

void Get_X_5_Buttons(uint8_t y){
	switch (y) {
		//	5 , 0
		case 0:
			//	NC
		break;
		
		// 	5 , 1
		case 1:
			// ALTERNATE KEY TOGGLE 2
			if(bitRead(ModVAL,3) || bitRead(ModVAL,4)){
				//if any of alt function is on then clear both of them
				bitClear(ModVAL,3);
				bitClear(ModVAL,4);
			}else{
				//if not then toggle it on
				bitSet(ModVAL,4);
			}
		break;
		
		//	5 , 2
		case 2:
			Keyboard.press(0x20);	//SPACEBAR
		break;
		
		//	5 , 3
		case 3:
			//Windows key
      Keyboard.releaseAll();
      Keyboard.press(KEY_LEFT_GUI);
		break;
		
		//	5 , 4
		case 4:
			Keyboard.releaseAll();
			Keyboard.press(KEY_RETURN);
		break;
		
		//	5 , 5
		case 5:
			Keyboard.releaseAll();
			Keyboard.press(KEY_BACKSPACE);
		break;
		
		//	5 , 6
		case 6:
			//	NC
		break;
		
		//	5 , 7
		case 7:
			//	NC
		break;
		default:
			return ;
		break;
	}
}

void Get_ODD_CASES(){
	//The odd cases
	if( (XKeycoords[1] == 0) && (Ykeycoords[1] == 7) ){
		XKeycoords[0] = 0;
		Ykeycoords[0] = 7;
		XKeycoords[1] = -1;
		Ykeycoords[1] = -1;
	}
}

void KeyboardCall(){
	ReadKeyboard();
	Get_ODD_CASES();
	//for(uint8_t i = 0 ; i < 1; i++){
		// just 1 key rollover
		Get_ButtonPress(XKeycoords[0],Ykeycoords[0]);
	//}
}

void setup() {
  // put your setup code here, to run once:
  
  //Starts USB Keyboard and mouse for the pc
	HID.begin(HID_KEYBOARD_MOUSE);

  //Turn Off debug JTAG ports
  afio_cfg_debug_ports(AFIO_DEBUG_NONE);
  
  //Digital Output For Line Scan Pins
  pinMode(PB12, OUTPUT);	// line 0
  pinMode(PB13, OUTPUT); 	// Line 1
  pinMode(PB14, OUTPUT); 	// Line 2
  pinMode(PB5, OUTPUT); 	// Line 3
  pinMode(PB6, OUTPUT); 	// Line 4
  pinMode(PB7, OUTPUT); 	// Line 5
  pinMode(PB8, OUTPUT);  	// Line 6
  pinMode(PB9, OUTPUT);  	// Line 7

  //Digital Input For Scan Line Pins
  pinMode(PB1, INPUT_PULLDOWN);  //Input 0 Left key
  pinMode(PB0, INPUT_PULLDOWN);  //Input 1 Left key
  pinMode(PA7, INPUT_PULLDOWN);  //Input 2 Left key
  pinMode(PA6, INPUT_PULLDOWN);  //Input 3 Right key
  pinMode(PA5, INPUT_PULLDOWN);  //Input 4 Right key
  pinMode(PA4, INPUT_PULLDOWN);  //Input 5 Right key

  //Analog Input For Mouse
  pinMode(PA1, INPUT_ANALOG);  // X coordinates
  pinMode(PA2, INPUT_ANALOG);  // Y coordinates
  
  //Serial Initializations
 // Serial.begin(115200);
 // pinMode(PC13, OUTPUT); //LED PIN
 // digitalWrite(PC13, LOW); // SET it high so i know it using serial

	
	WorkingDelayTime[0] = millis();
	WorkingDelayTime[1] = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
  
	Mil = millis();
	
	//Mouse Delay
	if(Mil > WorkingDelayTime[1]){
		MouseCall();
		WorkingDelayTime[1] = millis() + DelayBetweenRuns[1];
	}
	
	
	if(Mil > WorkingDelayTime[0]){
		KeyboardCall();
		WorkingDelayTime[0] = millis() + DelayBetweenRuns[0];
	}
	
//		DEBUG_CALL();
}
