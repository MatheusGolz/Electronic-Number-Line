#include <Wire.h> // Necessary for I2C
#include <LiquidCrystal_I2C.h> // Commands for I2C LCD display
#include <Adafruit_NeoPixel.h> // NeoPixel library for LED strip control

// LCD configuration
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pin definitions
#define LED_PIN 9            // NeoPixel LED strip pin
#define LED_COUNT 101      // NeoPixel LED strip length
#define MOMENTARY_SWITCH_PIN 10 // Momentary switch pin

// NeoPixel setup
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Flags and variables
volatile bool kiBlinking = false;
int randomNumber = 0;         // Variable to store random number for Part 1 of Ki mode
int randomNumber2 = 0;        // Variable to store second random number for Part 2
bool randomGenerated = false; // Flag to ensure random number is generated only once
int ledIndex = 0;             // Variable to store mapped LED index from analog input
int indexesSaved = 0;         // Variable to count how many indexes have been saved
int currentPart = 1;          // Variable to track which part (1 or 2) of Ki mode we're in
int ledIndex1 = 0;            // First LED index for Part 2
int ledIndex2 = 0;            // Second LED index for Part 2

void setup() {
  // LCD setup
  lcd.init();
  lcd.backlight();
  
  pinMode(MOMENTARY_SWITCH_PIN, INPUT_PULLUP);

  // Serial communication setup
  Serial.begin(9600);

  // NeoPixel setup
  strip.begin();
  strip.show();
  strip.setBrightness(50);

  // Random number seed setup for Ki mode
  randomSeed(analogRead(0));

  // Initialize Ki mode
  kiBlinking = true;
}

void loop() {
  if (kiBlinking) {
    if (!randomGenerated) {
      generateAndDisplayRandomNumber(); // Display random number once
      randomGenerated = true;           // Prevent further generation
    }
    mapAnalogToLED();                   // Adjust LED based on analog input

    // Check if momentary switch is pressed to send data over Serial
    if (digitalRead(MOMENTARY_SWITCH_PIN) == LOW) {
      sendRandomAndLEDIndex();
      delay(500); // Debounce delay to prevent repeated triggers
    }
  }
}

// Ki mode: Generate a random number once and display it on the LCD
void generateAndDisplayRandomNumber() {
  if (currentPart == 1) {
    randomNumber = random(1, 101);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Random Number:");
    lcd.setCursor(0, 1);
    lcd.print(randomNumber);
  }
}

// Ki mode: Generate and display two random numbers for Part 2
void generateAndDisplayTwoRandomNumbers() {
  randomNumber = random(1, 101);
  randomNumber2 = random(1, 101);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Random Numbers:");
  lcd.setCursor(0, 1);
  lcd.print(randomNumber);
  lcd.print(" ");
  lcd.print(randomNumber2);
}

// Ki mode: Map analog input to a single LED on NeoPixel strip
void mapAnalogToLED() {
  int analogValue = analogRead(1);
  ledIndex = map(analogValue, 0, 1023, LED_COUNT - 1,0);

  // Clear all LEDs, then light up only the mapped LED
  strip.clear();
  strip.setPixelColor(ledIndex, strip.Color(255, 0, 0)); // Red color
  strip.show();
}

// Function to handle the sending of data over Serial
void sendRandomAndLEDIndex() {
  if (currentPart == 1) {  // Part 1 logic
    // Send the first random number and LED index to serial
    Serial.print("Generated: ");
    Serial.print(randomNumber);
    Serial.print("    Response: ");
    Serial.println(ledIndex);

    indexesSaved++;  // Increment index counter for Part 1

    if (indexesSaved >= 1) {
      currentPart = 2;          // Transition to Part 2
      indexesSaved = 0;         // Reset index counter for Part 2
      generateAndDisplayTwoRandomNumbers(); // Display two random numbers for Part 2
    }
  } 
  else if (currentPart == 2) {  // Part 2 logic
    if (indexesSaved == 0) {
      // First click in Part 2: Save the first LED index
      ledIndex1 = map(analogRead(1), 0, 1023, LED_COUNT - 1,0);
      indexesSaved++;  // Increment to signal first index saved
    }
    else if (indexesSaved == 1) {
      // Second click in Part 2: Save the second LED index
      ledIndex2 = map(analogRead(1), 0, 1023, LED_COUNT - 1,0);

      // Now, output both random numbers and both LED indices
      Serial.print("Random Numbers: ");
      Serial.print(randomNumber);  // First random number
      Serial.print(" ");
      Serial.print(randomNumber2); // Second random number
      Serial.print("    Response saved: ");
      Serial.print(ledIndex1);     // First LED index
      Serial.print(" ");
      Serial.println(ledIndex2);   // Second LED index

      // Now that both selections are done, reset the Ki mode
      currentPart = 1;             // Reset to Part 1
      indexesSaved = 0;            // Reset counter
      randomGenerated = false;     // Allow new random generation for Part 1
      generateAndDisplayRandomNumber(); // Start with new random for Part 1
    }
  }
}
