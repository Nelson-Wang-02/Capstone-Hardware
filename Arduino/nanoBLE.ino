#include <ArduinoBLE.h>
#include <Arduino_LSM9DS1.h>


//----------------------------------------------------------------------------------------------------------------------
// BLE UUIDs
//----------------------------------------------------------------------------------------------------------------------

#define BLE_UUID_SENSOR_DATA_SERVICE              "181A"
#define BLE_UUID_GY_DATA                "2B44"
#define BLE_UUID_ACC_DATA                "2B9B"
#define BLE_UUID_MAG_DATA                "2A2C"
#define BLE_DEVICE_NAME                           "Arduino Nano 33 BLE"
#define BLE_LOCAL_NAME                            "Arduino Nano 33 BLE"

#define NUMBER_OF_SENSORS 3

const int ledPin = LED_BUILTIN; // set ledPin to on-board LED

union 
{
  float a[3];
  unsigned char bytes[12];
} gyData;

union 
{
  float a[3];
  unsigned char bytes[12];
} accData;

union 
{
  float a[3];
  unsigned char bytes[12];
} magData;


#define BLE_LED_PIN                               LED_BUILTIN
#define SENSOR_UPDATE_INTERVAL                    (250)

//Service for publish Accelerometer value...
BLEService sensorDataService( BLE_UUID_SENSOR_DATA_SERVICE );
BLECharacteristic gyDataCharacteristic( BLE_UUID_GY_DATA, BLERead | BLENotify, 12 );
BLECharacteristic accDataCharacteristic( BLE_UUID_ACC_DATA, BLERead | BLENotify, 12 );
BLECharacteristic magDataCharacteristic( BLE_UUID_MAG_DATA, BLERead | BLENotify, 12 );

float gy_x, gy_y, gy_z, acc_x, acc_y, acc_z, mag_x, mag_y, mag_z;
bool sensorDataupdated;

void setup()
{
  Serial.begin( 9600 );
  while ( !Serial );

  pinMode( BLE_LED_PIN, OUTPUT );
  digitalWrite( BLE_LED_PIN, LOW );

  // Without Serial when using USB power bank HTS sensor seems to needs some time for setup
  delay( 10 );

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  if ( !setupBleMode() )
  {
    Serial.println( "Failed to initialize BLE!" );
    while ( 1 );
  }
  else
  {
    Serial.println( "BLE initialized. Waiting for clients to connect." );
  }
}


void loop()
{
  bleTask();
  if ( sensorTask() )
  {
    //printTask();
  }
}


bool sensorTask()
{
  static long previousMillis = 0;

  unsigned long currentMillis = millis();
  if ( currentMillis - previousMillis < SENSOR_UPDATE_INTERVAL )
  {
    return false;
  }
  previousMillis = currentMillis;

  if (IMU.gyroscopeAvailable()) {
		IMU.readGyroscope(gy_x, gy_y, gy_z);
  }
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(acc_x, acc_y, acc_z);
  }
  if (IMU.magneticFieldAvailable()) {
    IMU.readMagneticField(mag_x, mag_y, mag_z);
  }
  
  gyData.a[0] = gy_x;
	gyData.a[1] = gy_y;
  gyData.a[2] = gy_z;
  
  accData.a[0] = acc_x;
  accData.a[1] = acc_y;
  accData.a[2] = acc_z;

  magData.a[0] = mag_x;
  magData.a[1] = mag_y;
  magData.a[2] = mag_z;
  
  sensorDataupdated = true;

  return sensorDataupdated;
}


void printTask()
{
  
  Serial.print( "X = " );
  Serial.print( gy_x );
  Serial.print( " Y = " );
  Serial.print( gy_y );
  Serial.print( " Z = " );
  Serial.println( gy_y );

}


bool setupBleMode()
{
  const unsigned char initial[12] = { 0,0,0,0,0,0,0,0,0,0,0,0 };
  
  if ( !BLE.begin() )
  {
    return false;
  }

  // set advertised local name and service UUID
  BLE.setDeviceName(BLE_DEVICE_NAME);
  BLE.setLocalName(BLE_LOCAL_NAME);
  BLE.setAdvertisedService(sensorDataService);

  // BLE add characteristics
  sensorDataService.addCharacteristic( gyDataCharacteristic );
  sensorDataService.addCharacteristic( accDataCharacteristic );
  sensorDataService.addCharacteristic( magDataCharacteristic );
  // add service
  BLE.addService( sensorDataService );

  // set the initial value for the characeristic
  gyDataCharacteristic.writeValue( initial, 12 );
  accDataCharacteristic.writeValue( initial, 12 );
  magDataCharacteristic.writeValue( initial, 12 );

  // set BLE event handlers
  BLE.setEventHandler( BLEConnected, blePeripheralConnectHandler );
  BLE.setEventHandler( BLEDisconnected, blePeripheralDisconnectHandler );

  // start advertising
  BLE.advertise();

  return true;
}


void bleTask()
{
  const uint32_t BLE_UPDATE_INTERVAL = 10;
  static uint32_t previousMillis = 0;

  uint32_t currentMillis = millis();
  if ( currentMillis - previousMillis >= BLE_UPDATE_INTERVAL )
  {
    previousMillis = currentMillis;
    BLE.poll();
  }

  if ( sensorDataupdated )
  {
    unsigned char *gyd = (unsigned char *)&gyData;
    gyDataCharacteristic.setValue( gyd, 12 );
    unsigned char *accd = (unsigned char *)&accData;
    accDataCharacteristic.setValue( accd, 12 );
    unsigned char *magd = (unsigned char *)&magData;
    magDataCharacteristic.setValue( magd, 12 );

    sensorDataupdated = false;
  }
}


void blePeripheralConnectHandler( BLEDevice central )
{
  digitalWrite( BLE_LED_PIN, HIGH );
  Serial.print( F ( "Connected to central: " ) );
  Serial.println( central.address() );
}


void blePeripheralDisconnectHandler( BLEDevice central )
{
  digitalWrite( BLE_LED_PIN, LOW );
  Serial.print( F( "Disconnected from central: " ) );
  Serial.println( central.address() );
}