#include <Wire.h>

// APDS9960 (Gesture sensor)
#include <SparkFun_APDS9960.h>
#define APDS9960_INT 2

//DHT22
#include "DHT.h"
#define DHTTYPE DHT22 
#define DHT22_PIN 3
DHT dht = DHT(DHT22_PIN, DHTTYPE);;

// LED PIN
#define LEDPIN 13

SparkFun_APDS9960 apds = SparkFun_APDS9960();
int isr_flag = 0;

//I2C Display
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 20, 4);

// Countdown
bool countdown_status = false;
int countdown;

// Swipes
int swipes = 0;

// Direction of swipes 
int swipesLeft = 0;
int swipesRight = 0;
int swipesUp = 0;
int swipesDown = 0;

// LED
bool led = false;

// Just reset flag
bool justReset = false;

void setup() {

    // Set interrupt pin as input
    pinMode(APDS9960_INT, INPUT);
    
    pinMode(13, OUTPUT);

    //Setup DHT22
    dht.begin();

    // Initialize the LCD Display
    lcd.init();
    // or lcd.begin(); depends on the library you're using

    lcd.backlight();
    lcd.clear();
    
    lcd.setCursor(4, 1);
    lcd.print("Initializing");

    // Initialize interrupt service routine
    attachInterrupt(0, interruptRoutine, FALLING);

    delay(400);

    // Initialize APDS-9960
    if (apds.init() && apds.enableGestureSensor(true)) {
        lcd.clear();
        lcd.setCursor(7, 1);
        lcd.print("Ready!");
    }

    // APDS-9960 failed to initialize (Restart the board)
    else {
        lcd.setCursor(3, 1);
        lcd.print("Gesture sensor");
        lcd.setCursor(7, 2);
        lcd.print("error!");
        while(1);
    }

}


void loop() {

    // Handle and reset the sensor flag
    if(isr_flag == 1) {
        detachInterrupt(0);
        handleGesture();
        isr_flag = 0;
        attachInterrupt(0, interruptRoutine, FALLING);
    }

    // If there's a active countdown and not finished yet
    if(countdown != 0 && countdown_status) {

        lcd.setCursor(18, 0);
        lcd.print("   ");

        lcd.setCursor(18, 0);
        lcd.print(countdown);

        countdown--;

        delay(1);
        lcd.setCursor(18, 0);
        lcd.print(countdown);

        lcd.setCursor(18, 0);
        lcd.print("   ");

    }

    // If the countdown finishes
    else if (countdown_status) {

        lcd.clear();

        /*
           Modify events here
          
           NOTE: Put new conditions between
          
           if(......) {
            
           }
          
            <----- here <<< else if(......) {
          
                            }

           else {                   
          
           }

        */

        // Swipe left 1 time
        if(swipes == 1 && swipesLeft == 1) {
            // Random 1-6
            int randomInt = randomNumber(1, 6);

            // Foreach case of randomized number
            switch(randomInt) {
                case 1:
                    displayMessage("Pizza");
                    break;
                case 2:
                    displayMessage("Lay");
                    break;
                case 3:
                    displayMessage("Pasta");
                    break;
                case 4:
                    displayMessage("Bread");
                    break;
                case 5:
                    displayMessage("Sushi");
                    break;
                case 6:
                    displayMessage("Candy");
                    break;
            }
        }

        // Swipe right 1 time
        else if (swipes == 1 && swipesRight == 1) {
            int randomInt = randomNumber(1, 6);
            lcd.setCursor(1, 1);
            lcd.print(randomInt);
        }

        // Swipe right 2 time
        else if (swipes == 2 && swipesRight == 2) {
            
            if(!led) {
                displayMessage("On");
                digitalWrite(LEDPIN, HIGH);
                led = true;
            }
            else {
                displayMessage("Off");
                digitalWrite(LEDPIN, LOW);
                led = false;
            }
            
        }

        // Swipe up 1 time
        else if (swipes == 1 && swipesUp == 1) {
            float humidity = dht.readHumidity();
            float temperature = dht.readTemperature();
            float f_temperature = dht.readTemperature(true);
            float heatIndex = dht.computeHeatIndex(temperature, humidity, false);

            if(isnan(humidity) || isnan(temperature) || isnan(f_temperature) || isnan(heatIndex)) {
                displayMessage("DHT Sensor error");
            }

            else {
                displayRoomStatus(temperature, f_temperature, humidity, heatIndex);
            }

        }

        // Swipe down 1 time
        else if (swipes == 1 && swipesRight == 1) {
            displayMessage("Swipe down!");
        }

        // Something else
        else {
            lcd.setCursor(1, 1);
            lcd.print("Detected | ");
            lcd.setCursor(12, 1);
            lcd.print(swipes);
        }

        //
        // --------------------------------------------------------------
        //


        // Delay of displaying message (in ms 1sec = 1000ms)
        delay(3000);

        // Reset all swipes counts
        swipes = 0;
        swipesLeft = 0;
        swipesRight = 0;
        swipesUp = 0;
        swipesDown = 0;
        
        // Reset countdowns and flags
        countdown = 0;
        countdown_status = false;

        // Mark as just reset
        justReset = true;

        // Reset to ready display again
        lcd.clear();
        lcd.setCursor(7, 1);
        lcd.print("Ready!");
        
    }
}

// Flag when interrupt
void interruptRoutine() {
    isr_flag = 1;
}

void handleGesture() {
    
    if(apds.isGestureAvailable() && !justReset) {

        if(!countdown_status) {
            countdown_status = true;
        }

        countdown = 99;
        swipes++;

        switch (apds.readGesture())
        {
             case DIR_UP:
                swipesUp++;
                displaySwipe("Up");
                break;
            case DIR_DOWN:
                swipesDown++;
                displaySwipe("Down");
                break;
            case DIR_LEFT:
                swipesLeft++;
                displaySwipe("Left");
                break;
            case DIR_RIGHT:
                swipesRight++;
                displaySwipe("Right");
                break;
            //Near and far seem not to be that reliable, also there're no real usage for it 
            case DIR_NEAR:
                displaySwipe("Near");
                break;
            case DIR_FAR:
                displaySwipe("Far");
                break;
            default:
                displaySwipe("N/A");
                break;
        }
    }

    // If it was just reset, discard all inputs while it was being reset
    else if(justReset) {
        if(apds.isGestureAvailable()) {
            apds.readGesture();
        }
        justReset = false;
    }

}

// Display swipes detected while cooldown is active
void displaySwipe(String text) {
    lcd.clear();

    lcd.setCursor(1, 1);
    lcd.print("Detected | ");
    lcd.setCursor(12, 1);
    lcd.print(swipes);

    lcd.setCursor(1, 2);
    lcd.print("Direction | ");
    lcd.setCursor(13, 2);
    lcd.print(text);
}

// Display message
void displayMessage(String text) {
    lcd.clear();

    lcd.setCursor(1, 1);
    lcd.print(text);
}

// Number randomizer
int randomNumber(int min, int max) {
    return random(min, max + 1);
}

// Show room temperature and humidity
void displayRoomStatus(float temperature, float f_temperature, float humidity, float heatIndex) {
    lcd.clear();

    lcd.setCursor(1, 0);
    lcd.print("Temperature | ");
    lcd.setCursor(15, 0);
    lcd.print(temperature);

    lcd.setCursor(1, 1);
    lcd.print("Temperature | ");
    lcd.setCursor(15, 1);
    lcd.print(f_temperature);

    lcd.setCursor(1, 2);
    lcd.print("Humidity    | ");
    lcd.setCursor(15, 2);
    lcd.print(humidity);

    lcd.setCursor(1, 3);
    lcd.print("Heat Index  | ");
    lcd.setCursor(15, 3);
    lcd.print(heatIndex);
}
