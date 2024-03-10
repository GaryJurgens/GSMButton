/*
 Name:		GSMButton.ino
 Created:	2/17/2024 8:40:45 PM
 Author:	garyj
*/
#define UART_BAUD   115200
#define MODEM_DTR     25
#define MODEM_TX      27
#define MODEM_RX      26
#define MODEM_PWRKEY  4
#define MODEM_DTR     32
#define MODEM_RI      33
#define MODEM_FLIGHT  25
#define MODEM_STATUS  34
#define TINY_GSM_MODEM_SIM7600
#define TINY_GSM_RX_BUFFER 1024 // Set RX buffer to 1Kb
#define SerialAT Serial1
#include <TinyGsmClient.h>


int standbyLightPin = 19;
int dialLightPin = 23;
int buttonPin = 14;
int resetButtonPin = 13;


// Initialize TinyGSM
TinyGsm modem(SerialAT);
void modemPowerOn()
{
    pinMode(MODEM_PWRKEY, OUTPUT);
    digitalWrite(MODEM_PWRKEY, LOW);
    delay(1000);    //Datasheet Ton mintues = 1S
    digitalWrite(MODEM_PWRKEY, HIGH);
}

void modemPowerOff()
{
    pinMode(MODEM_PWRKEY, OUTPUT);
    digitalWrite(MODEM_PWRKEY, LOW);
    delay(1500);    //Datasheet Ton mintues = 1.2S
    digitalWrite(MODEM_PWRKEY, HIGH);
}

void setup() {

   
    pinMode(standbyLightPin, OUTPUT);
    pinMode(dialLightPin,OUTPUT);
    pinMode(buttonPin, INPUT_PULLUP);
    pinMode(resetButtonPin, INPUT_PULLUP);
    digitalWrite(buttonPin, HIGH);
    digitalWrite(resetButtonPin, HIGH);
    


    // Begin serial communication with a baud rate of 9600
    Serial.begin(115200);
    delay(10);
    Serial.println("Starting up...");
    modemPowerOff();
    delay(5000);
    modemPowerOn();
    Serial.println("Modem powered on");
    delay(2000);
    pinMode(MODEM_FLIGHT, OUTPUT);
    digitalWrite(MODEM_FLIGHT, HIGH);
    delay(1000);
    // Start communication with the GSM module
    SerialAT.begin(UART_BAUD, SERIAL_8N1, MODEM_RX, MODEM_TX);
    delay(5000);

    Serial.println("Initializing modem...");
    
    modem.setNetworkMode(2);

    int16_t sq = modem.getSignalQuality();

   
    bool regstatus = false;
       while (!regstatus)
       {
           RegStatus status = modem.getRegistrationStatus();

           if (status == REG_OK_HOME) {
               Serial.println("Registered to the network");
               regstatus = true;
           }
           else {
               Serial.println("Failed to register to the network");
           }
           delay(1000);
       }
    

    // Wait for network registration (may take some time)
    Serial.println("Waiting for network...");
    bool connected = false;
while(!connected)
{
    if (modem.isNetworkConnected()) {
        connected = true;
        modem.gprsConnect("internet", "", "");
        Serial.println("Connected to the network");
        break;
    }
    else

    {
        Serial.println("Wating to connect to network...");
    }
		delay(2000);
	}
    

    
    
}
bool ButtonPressed = false;
bool resetButtonPressed = false;

unsigned long lastButtonPress = 0;
unsigned long TimeBetweenButtonPress = 300000; // 5 minutes in milliseconds

unsigned long lastDialLightToggle = 0;
unsigned long dialLightFlashInterval = 500; // Flash interval for dial light
bool firstCallMade = false; // Flag to indicate if the first call has been made
unsigned long lastDebounceTime = 0;  // the last time the input pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

void loop() {
    // Do nothing
    Serial.println("in main loop wating for button press");
    
    if (digitalRead(buttonPin) == LOW && !ButtonPressed && (millis() - lastButtonPress) > debounceDelay) {
        ButtonPressed = true;
        lastButtonPress = millis(); // Record time of this press
        firstCallMade = false; // Reset first call flag for a new button press cycle
    }

    // Handle making phone call immediately after button press, then every 5 minutes
    if (ButtonPressed) {
        if (!firstCallMade || (millis() - lastButtonPress) >= TimeBetweenButtonPress) {
            Serial.println("Making phone call...");
            MakePhoneCall(); // Placeholder for actual phone call function
            delay(10000);
            MakePhoneCall1();
            lastButtonPress = millis(); // Reset timer for next call
            firstCallMade = true; // Mark that the first call has been made
        }
    }

    // Flash dial light while button press action is active
    if (ButtonPressed && (millis() - lastDialLightToggle) >= dialLightFlashInterval) {
        digitalWrite(dialLightPin, !digitalRead(dialLightPin)); // Toggle light state
        lastDialLightToggle = millis(); // Record time of this toggle
    }

    // Check for reset button press and handle debounce
    if (digitalRead(resetButtonPin) == LOW && !resetButtonPressed && (millis() - lastButtonPress) > debounceDelay) {
        Serial.println("Resetting...");
        resetButtonPressed = true;
        ButtonPressed = false; // Cancel phone call action
        digitalWrite(dialLightPin, LOW); // Ensure dial light is off
        // Reset any necessary variables or states here
    }

    // Optional: Reset resetButtonPressed state after handling reset
    if (resetButtonPressed && (millis() - lastButtonPress) > debounceDelay) {
        resetButtonPressed = false; // Ready for next reset press
    }
    
    
    bool connected = false;
    while (!connected)
    {
        digitalWrite(standbyLightPin, HIGH);

        if (modem.isNetworkConnected()) {
            connected = true;
            //modem.gprsConnect("internet", "", "");
            Serial.println("Connected to the network");
            break;
        }
        else

        {
            Serial.println("Wating to connect to network...");
        }
        delay(500);
        digitalWrite(standbyLightPin, LOW);
        delay(500);
    }


}
void MakePhoneCall()
{
    digitalWrite(dialLightPin, HIGH);
    Serial.println("Sending an SMS...");
    modem.sendSMS("+27xxxxxxx", " Emergency Button Pressed"); // Replace <PHONE_NUMBER> with the number you want to send the SMS to)
    delay(1000);
    modem.sendSMS("+27xxxxxxx", " Emergency Button Pressed");
    Serial.println("SMS sent");
    modem.callNumber("+27xxxxx");
    delay(10000);// Replace <PHONE_NUMBER> with the number you want to call)
    modem.callNumber("+27xxxxxxxxx");

    // Making a voice call using direct AT commands
}
void MakePhoneCall1()
{
    digitalWrite(dialLightPin, HIGH);
    Serial.println("Sending an SMS...");
   
    Serial.println("Phonecall 2");
    
    modem.callNumber("+27xxxxxxx");

    // Making a voice call using direct AT commands
}
