#include <Wire.h>
#include <SeeedOLED.h>


/* Basics */
#define SET_CONTINUOUS_RANGING_MODE (0x63)
#define RANGING_MEASUREMENT_MODE    (0x6d)

/* Ranging Modes */
#define SHORT_RANGE                 (0x73)
#define MED_RANGE                   (0x6d)
#define LONG_RANGE                  (0x6c)

/* Others */
#define AVG_SIZE                    25
#define UPDATE_TIME_DISPLAY         100
#define UPDATE_TIME_SERIAL          500

/* Variables */
int address = 0x08;
unsigned long time_last_display = 0;
unsigned long time_last_serial = 0;
unsigned long qty_value = 0;

unsigned int actual_value;
unsigned int min_value = 50000;
unsigned int max_value = 0;
unsigned int average_value;

/* Prototype */
unsigned int read_distance();
void process_new_value(unsigned int);
unsigned int process_avg(unsigned int);
void serial_print();
void update_display();

void setup() {
    Serial.begin(9600);

    // Init. I2C
    Wire.begin();

    // Init. OLED
    SeeedOled.init();  //initialze SEEED OLED display

    SeeedOled.clearDisplay();           //clear the screen and set start position to top left corner
    SeeedOled.setNormalDisplay();       //Set display to Normal mode
    //SeeedOled.setPageMode();            //Set addressing mode to Page Mode
    SeeedOled.setTextXY(0,0);           //Set the cursor to 0th Page, 0th Column
    SeeedOled.putString("Value: ");//Print the String

    SeeedOled.setTextXY(2,0);           //Set the cursor to 0th Page, 0th Column
    SeeedOled.putString("Min.:  ");//Print the String

    SeeedOled.setTextXY(4,0);           //Set the cursor to 0th Page, 0th Column
    SeeedOled.putString("Max.:  ");//Print the String
    //SeeedOled.putString("4800mm");//Print the String

    SeeedOled.setTextXY(6,0);           //Set the cursor to 0th Page, 0th Column
    SeeedOled.putString("Avg.:  ");//Print the String

    // Init. MappyDots
    Wire.beginTransmission(address);
    Wire.write(SET_CONTINUOUS_RANGING_MODE);
    Wire.endTransmission();

    Wire.beginTransmission(address);
    Wire.write(RANGING_MEASUREMENT_MODE);
    Wire.write(LONG_RANGE);
    Wire.endTransmission();

    Serial.println("Init. OK");
}

void loop() {
    // Get actual time
    unsigned long time = millis();

    // Read distance
    unsigned int distance = read_distance();
    process_new_value(distance);

    //Update display
    if (time > (time_last_display+UPDATE_TIME_DISPLAY)) {
        update_display();

        time_last_display = time;
    }

    if (time > (time_last_serial+UPDATE_TIME_SERIAL)) {
        serial_print();

        time_last_serial = time;
    }

    qty_value++;
}

/* Functions */
unsigned int read_distance() {
    unsigned int distance;
    Wire.requestFrom(address, 2);
    distance = Wire.read() << 8;
    distance |= Wire.read();
    return distance;
}

void process_new_value(unsigned int v) {
    actual_value = v;

    if(v < min_value) {
        min_value = v;
    }

    if(v > max_value) {
        max_value = v;
    }

    average_value = process_avg(v);
}

unsigned int process_avg(unsigned int v) {
    static unsigned int idx = 0;
    static unsigned int qty = 0;
    static unsigned int avg[AVG_SIZE] = {0};

    unsigned long average;

    //Store value in array
    avg[idx] = v;

    //Update initial counter
    qty++;
    if (qty>=AVG_SIZE) {
      qty = AVG_SIZE;
    }

    //Compute average
    for (unsigned int i = 0; i < qty; i++) {
      average += avg[i];
    }
    average = average / qty;

    //Update index of array
    idx++;
    if(idx > (AVG_SIZE-1)) {
      idx = 0;
    }

    return average;
}

void serial_print() {
    Serial.print("actual : ");
    Serial.print(actual_value);
    Serial.print(", min : ");
    Serial.print(min_value);
    Serial.print(", max : ");
    Serial.print(max_value);
    Serial.print(", avg : ");
    Serial.print(average_value);
    Serial.print("mm, qty_value=");
    Serial.println(qty_value);
}

void update_display() {
    char cstr[5];

    SeeedOled.setTextXY(0,0);
    SeeedOled.putString("Value: ");
    sprintf(cstr, "%4d", actual_value);
    SeeedOled.putString(cstr);
    SeeedOled.putString("mm");

    SeeedOled.setTextXY(2,0);
    SeeedOled.putString("Min.:  ");
    sprintf(cstr, "%4d", min_value);
    SeeedOled.putString(cstr);
    SeeedOled.putString("mm");

    SeeedOled.setTextXY(4,0);
    SeeedOled.putString("Max.:  ");
    sprintf(cstr, "%4d", max_value);
    SeeedOled.putString(cstr);
    SeeedOled.putString("mm");

    SeeedOled.setTextXY(6,0);
    SeeedOled.putString("Avg.:  ");
    sprintf(cstr, "%4d", average_value);
    SeeedOled.putString(cstr);
    SeeedOled.putString("mm");
}
