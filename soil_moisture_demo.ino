#define MOISTURE_SENSOR_POWER D6
#define MOISTURE_SENSOR A7
#define PUBLISH_TTL 60
#define PUBLISH_HOUR 5
#define PUBLISH_MINUTE 30
#define SOIL_MOISTURE_THRESHOLD_VALUE 2300

// whether or not an event has been fired today (allowed once per day now)
bool publishedToday = false;
// day of month during last loop
int previousDay = -1;
// analog value for soil moisture
int soilMoistureSensorValue = 0;

void setup() {
  // set up moisture sensor pin
  pinMode(MOISTURE_SENSOR_POWER, OUTPUT);
  pinMode(MOISTURE_SENSOR, INPUT);

  // set time zone to Central Standard Time
  Time.zone(-5);
  previousDay = Time.day();

  // turn off status LED
  RGB.control(true);
  RGB.brightness(0);

  // expose soil moisture as a variable
  Spark.variable("moisture", &soilMoistureSensorValue, INT);
}

void loop() {
  // get current day of the month, hour, and minute of current loop
  int currentDay = Time.day();
  int currentHour = Time.hour();
  int currentMinute = Time.minute();

  // activate and read the moisture sensor
  soilMoistureSensorValue = getSoilMoistureReading();

  // if a new day has begun, allow a sensor event to be published
  if (previousDay != currentDay) {
    publishedToday = false;
    // sync time with Particle Cloud to prevent clock drift
    Spark.syncTime();
  }

  // if a sensor event can be published
  if (!publishedToday && currentHour >= PUBLISH_HOUR && currentMinute >= PUBLISH_MINUTE) {
    publishedToday = true;

    // if the soil is too dry, publish an event
    if (soilMoistureSensorValue <= SOIL_MOISTURE_THRESHOLD_VALUE) {
      Spark.publish("soil-moisture", "need-water", PUBLISH_TTL, PRIVATE);
    }

    // publish the soil moisture analog value
    Spark.publish("soil-moisture-value", String(soilMoistureSensorValue), PUBLISH_TTL);
  }

  // set the current day to the previous so we can check an see if the current day has changed next loop
  previousDay = currentDay;

  // pause for 10 seconds between loops
  delay(10000);
}

// activate sensor, get sensor reading, and deactivate sensor
int getSoilMoistureReading() {
  int sensorReading = 0;

  // power up the sensor
  digitalWrite(MOISTURE_SENSOR_POWER, HIGH);
  // allow 5 seconds to stabilize
  delay(5000);

  sensorReading = analogRead(MOISTURE_SENSOR);

  // power down the sensor
  digitalWrite(MOISTURE_SENSOR_POWER, LOW);

  return sensorReading;
}
