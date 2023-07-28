#ifndef _SENSOR_H
#define _SENSOR_H
#ifndef ARDUINO
#elif ARDUINO >= 100
#include "Arduino.h"
#include "Print.h"
#endif

typedef enum
{
  SENSOR_TYPE_RELATIVE_HUMIDITY = (12),
  SENSOR_TYPE_AMBIENT_TEMPERATURE = (13),
  SENSOR_TYPE_OBJECT_TEMPERATURE = (14),
} sensors_type_t;
typedef struct
{
  int8_t status;       //< Status byte
  uint8_t reserved[3]; //< Reserved
} sensors_vec_t;
typedef struct
{
  int32_t version;   /**< must be sizeof(struct sensors_event_t) */
  int32_t sensor_id; /**< unique sensor identifier */
  int32_t type;      /**< sensor type */
  int32_t reserved0; /**< reserved */
  int32_t timestamp; /**< time is in milliseconds */
  union
  {
    float data[4];           ///< Raw data
    float temperature;       /**< temperature is in degrees centigrade (Celsius) */
    float relative_humidity; /**< relative humidity in percent */
  };                         ///< Union for the wide ranges of data we can carry
} sensors_event_t;
typedef struct
{
  char name[12];     /**< sensor name */
  int32_t version;   /**< version of the hardware + driver */
  int32_t sensor_id; /**< unique sensor identifier */
  int32_t type;      /**< this sensor's type (ex. SENSOR_TYPE_LIGHT) */
  float max_value;   /**< maximum value of this sensor's value in SI units */
  float min_value;   /**< minimum value of this sensor's value in SI units */
  float resolution;  /**< smallest difference between two values reported by this sensor */
  int32_t min_delay;
} sensor_t;
class Adafruit_Sensor
{
public:
  virtual void enableAutoRange(bool enabled)
  {
    (void)enabled;
  };
  virtual bool getEvent(sensors_event_t *) = 0;
  virtual void getSensor(sensor_t *) = 0;
  void printSensorDetails(void);

private:
  bool _autoRange;
};

#endif
