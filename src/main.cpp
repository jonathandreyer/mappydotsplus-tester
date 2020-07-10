#include <Arduino.h>
#include <TFmini.h>
#include <Wire.h>
#include <SeeedOLED.h>

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

unsigned int distance;
unsigned int strength;
unsigned int integration_time;

TFmini tfmini;

/* Prototype */
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

    // Init. TFmini
    Serial3.begin(115200);
    tfmini.attach(Serial3);

    Serial.println("Init. OK");
}

void loop() {
    // Get actual time
    unsigned long time = millis();

    // Read distance
    if (tfmini.available())
    {
        distance = tfmini.getDistance();
        strength = tfmini.getStrength();
        integration_time = tfmini.getIntegrationTime();
        if (distance > 10000)
        {
            distance = 10000;
        }
        process_new_value(distance);
    }

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

    float average = 0;

    //Store value in array
    avg[idx] = v;

    //Update initial counter
    qty++;
    if (qty>=AVG_SIZE) {
      qty = AVG_SIZE;
    }

    //Compute average
    for (unsigned int i = 0; i < qty; i++) {
      average += float(avg[i]);
    }
    average = average / (qty*1.0);

    //Update index of array
    idx++;
    if(idx > (AVG_SIZE-1)) {
      idx = 0;
    }

    return (unsigned int)average;
}

void serial_print() {
    /*Serial.print("distance : ");
    Serial.print(distance);
    Serial.print(", strength : ");
    Serial.print(strength);
    Serial.print(", integration : ");
    Serial.print(integration_time);
    Serial.print(", ");*/

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
    sprintf(cstr, "%6u", actual_value);
    SeeedOled.putString(cstr);
    SeeedOled.putString("cm");

    SeeedOled.setTextXY(2,0);
    SeeedOled.putString("Min.:  ");
    sprintf(cstr, "%6u", min_value);
    SeeedOled.putString(cstr);
    SeeedOled.putString("cm");

    SeeedOled.setTextXY(4,0);
    SeeedOled.putString("Max.:  ");
    sprintf(cstr, "%6u", max_value);
    SeeedOled.putString(cstr);
    SeeedOled.putString("cm");

    SeeedOled.setTextXY(6,0);
    SeeedOled.putString("Avg.:  ");
    sprintf(cstr, "%6u", average_value);
    SeeedOled.putString(cstr);
    SeeedOled.putString("cm");
}
