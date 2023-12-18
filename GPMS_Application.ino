/*

 Designer Systems DS-GPM Shield Arduino Demonstrator
 
 Requires Arduino Duemilanove or MEGA or NANO boards
 
 DS-GPM.S on I2C interface (ANALOG IN 4 [SDA] & 5 [SCL]) outputs data into Arduino
 serial terminal window at 9600 baud
 
 GPMS_Application.pde	Date: 23/5/10	Version: 1.01
 
 Version 1.01 - (13/5/11) Updated for compatibility with Arduino 1.0 environment
 
 */

#include <Wire.h>

#define GPM 0x68  	                              // GPM I2C Address

byte Address;                                         // Global variable
byte Data;                                            // Global variable

void setup()
{

  Wire.begin();                                       // Start I2C comms
  Serial.begin(9600);                                 // Start PC serial port @ 9600 baud
  delay(100);
  Serial.println("Application started...awaiting GPS data..");

  delay(3000);                                        // Wait 3 seconds for DS-GPM.S to initialise

}

void loop(){

for(int x=0; x <=2; x++){

  Serial.print("Time: ");
  Address = 0;                                        // Point to Hours register
  Data = GetDouble();                                 // Read registers from GPM
  Serial.print(Data, DEC);                            // and print to PC
  Serial.print(":");
  Address = 2;                                        // Point to Minutes register
  Data = GetDouble();                                 // Read registers from GPM
  Serial.print(Data, DEC);                            // and print to PC
  Serial.print(":");
  Address = 4;                                        // Point to Seconds register
  Data = GetDouble();                                 // Read registers from GPM
  Serial.println(Data, DEC);                          // and print to PC

  Serial.print("Date: ");
  Address = 6;                                        // Point to Day register
  Data = GetDouble();                                 // Read registers from GPM
  Serial.print(Data, DEC);                            // and print to PC
  Serial.print("/");
  Address = 8;                                        // Point to Month register
  Data = GetDouble();                                 // Read registers from GPM
  Serial.print(Data, DEC);                            // and print to PC
  Serial.print("/");
  Address = 10;                                       // Point to Year 1 register
  Data = GetDouble();                                 // Read registers from GPM
  Serial.print(Data, DEC);                            // and print to PC
  Address = 12;                                       // Point to Year 2 register
  Data = GetDouble();                                 // Read registers from GPM
  Serial.println(Data, DEC);                          // and print to PC
  delay(1000);                                        // Wait a second
  }

  Serial.print("Heading: ");
  Address = 44;                                       // Point to Heading 1 register
  Data = GetDouble();                                 // Read registers from GPM
  Serial.print(Data, DEC);                            // and print to PC
  Address = 46;                                       // Point to Heading 2 register
  Data = GetSingle();                                 // Read registers from GPM
  Serial.print(Data, DEC);                            // and print to PC
  Serial.print(".");
  Address = 47;                                       // Point to Heading 3 register
  Data = GetSingle();                                 // Read registers from GPM
  Serial.println(Data, DEC);                          // and print to PC

  Serial.print("Speed: ");
  Address = 52;                                       // Point to Speed 1 register
  Data = GetDouble();                                 // Read registers from GPM
  Serial.print(Data, DEC);                            // and print to PC
  Address = 54;                                       // Point to Speed 2 register
  Data = GetSingle();                                 // Read registers from GPM
  Serial.print(Data, DEC);                            // and print to PC
  Serial.print(".");
  Address = 55;                                       // Point to Speed 3 register
  Data = GetSingle();                                 // Read registers from GPM
  Serial.println(Data, DEC);                          // and print to PC

  delay(3000);                                        // Wait 3 seconds

  Serial.print("Latitide: ");
  Address = 14;                                       // Point to Latitude 1 register
  Data = GetSingle();                                 // Read registers from GPM
  Serial.print(Data, DEC);                            // and print to PC
  Address = 15;                                       // Point to Latitude 2 register
  Data = GetSingle();                                 // Read registers from GPM
  Serial.print(Data, DEC);                            // and print to PC
  Serial.print(" ");
  Address = 16;                                       // Point to Latitude 3 register
  Data = GetSingle();                                 // Read registers from GPM
  Serial.print(Data, DEC);                            // and print to PC
  Address = 17;                                       // Point to Latitude 4 register
  Data = GetSingle();                                 // Read registers from GPM
  Serial.print(Data, DEC);                            // and print to PC
  Serial.print(".");
  Address = 18;                                       // Point to Latitude 5 register
  Data = GetSingle();                                 // Read registers from GPM
  Serial.print(Data, DEC);                            // and print to PC
  Address = 19;                                       // Point to Latitude 6 register
  Data = GetSingle();                                 // Read registers from GPM
  Serial.print(Data, DEC);                            // and print to PC
  Address = 20;                                       // Point to Latitude 7 register
  Data = GetSingle();                                 // Read registers from GPM
  Serial.print(Data, DEC);                            // and print to PC
  Address = 21;                                       // Point to Latitude 8 register
  Data = GetSingle();                                 // Read registers from GPM
  Serial.print(Data, DEC);                            // and print to PC
  Address = 22;                                       // Point to Latitude character register
  Data = GetSingle();                                 // Read registers from GPM
  Serial.println(Data);                               // and print to PC

  Serial.print("Longitude: ");
  Address = 23;                                       // Point to Longitude 1 register
  Data = GetSingle();                                 // Read registers from GPM
  Serial.print(Data, DEC);                            // and print to PC
  Address = 24;                                       // Point to Longitude 2 register
  Data = GetSingle();                                 // Read registers from GPM
  Serial.print(Data, DEC);                            // and print to PC
  Address = 25;                                       // Point to Longitude 3 register
  Data = GetSingle();                                 // Read registers from GPM
  Serial.print(Data, DEC);                            // and print to PC
  Serial.print(" ");
  Address = 26;                                       // Point to Longitude 4 register
  Data = GetSingle();                                 // Read registers from GPM
  Serial.print(Data, DEC);                            // and print to PC
  Address = 27;                                       // Point to Longitude 5 register
  Data = GetSingle();                                 // Read registers from GPM
  Serial.print(Data, DEC);                            // and print to PC
  Serial.print(".");
  Address = 28;                                       // Point to Longitude 6 register
  Data = GetSingle();                                 // Read registers from GPM
  Serial.print(Data, DEC);                            // and print to PC
  Address = 29;                                       // Point to Longitude 7 register
  Data = GetSingle();                                 // Read registers from GPM
  Serial.print(Data, DEC);                            // and print to PC
  Address = 30;                                       // Point to Longitude 8 register
  Data = GetSingle();                                 // Read registers from GPM
  Serial.print(Data, DEC);                            // and print to PC
  Address = 31;                                       // Point to Longitude 9 register
  Data = GetSingle();                                 // Read registers from GPM
  Serial.print(Data, DEC);                            // and print to PC
  Address = 32;                                       // Point to Longitude character register
  Data = GetSingle();                                 // Read registers from GPM
  Serial.println(Data);                               // and print to PC

  delay(3000);                                        // Wait 3 seconds

}

int GetDouble(){				      // Get double register value from GPM

  int Value = 0; 
  byte H_Byte = 0;
  byte L_Byte = 0;

  Wire.beginTransmission(GPM);
  Wire.write(Address);				      // Send register start address
  Wire.endTransmission();

  Wire.requestFrom(GPM, 2);			      // Request double register
  while(Wire.available() < 2);			      // Wait for double byte
  H_Byte = Wire.read();                               // Store one
  L_Byte = Wire.read();                               // Store two

  Value = (H_Byte * 10) + L_Byte;                     // Adjust for one byte

  return(Value);	                          
}


int GetSingle(){				      // Get single register value from GPM

  int Value = 0; 

  Wire.beginTransmission(GPM);
  Wire.write(Address);				      // Send register start address
  Wire.endTransmission();

  Wire.requestFrom(GPM, 1);			      // Request register
  while(Wire.available() < 1);			      // Wait for single byte
  Value = Wire.read();                                // and store

  return(Value);	                          
}

