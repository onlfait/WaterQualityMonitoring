/*
  Underwater stations to monitor the quality of water in rivers and lakes
  OpenScience Hub project: https://oshub.network/local_OSHub_CH.html
  Balise de mesure de Qualite de l'Eau
  Developped at fablab On l'fait 
  by p@ddY and students from Centre de Formation Professionnelle et Technique (CFPT electronique)
  and Centre de formation pré-professionnelle (CFPP) 
  
  Microcontroller:
  Adafruit Feather M0 Adalogger https://adafru.it/2796 
  Sensors included:
  Temperature DS18b20 https://www.velleman.eu/products/view/?id=439184
  Module TDS Gravity SEN0244 pour la qualité de l'eau
  https://www.gotronic.fr/art-module-tds-gravity-sen0244-28277.htm
  To be added:
  pH conductivity ASP200 (Phidgets)
  https://www.phidgets.com/?tier=3&catid=11&pcid=9&prodid=412
  Capteur de turbidité SEN0189 
  https://www.gotronic.fr/art-capteur-de-turbidite-sen0189-27864.htm
  
  Thanks to: 
  Tom Igoe (SD card library & exemple)
  Jason <jason.ling@dfrobot.com@dfrobot.com> (TDS SEN0244 library & exemple)
  SD card datalogger

  Ajout du module RTC avec RTClib.h
*/

#include <SPI.h>
#include <SD.h>                 // Library for SD card
#include <OneWire.h>            // Library for I2C
#include <DallasTemperature.h>  // Library for temperature
#include <DS1302.h>             // Library for RTC
#include <TimeLib.h>            // library for Time

// temperature data wire is plugged into port 5 on the Arduino 
#define ONE_WIRE_BUS 5
#define TEMPERATURE_PRECISION 9 // Lower resolution
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

//define the input for battery voltage reading
#define VBATPIN A7
float VoltageBatterie=0.0;

// DS1302:  CE pin (RST)  -> Adalogger Digital 10
//          I/O pin (DAT) -> Adalogger Digital 11
//          SCLK pin      -> Adalogger Digital 12
// Init the DS1302 (RTC module)
DS1302 rtc(10, 11, 12);

// Init a Time-data structure
Time t;

int Attente=10000; // Duree d'attente (en ms) entre 2 series de mesure

float Celcius=0;
float Fahrenheit=0;

#define TdsSensorPin A1 // A1 pin for TDS sensor
#define VREF 3.3        // analog reference voltage(Volt) of the ADC
#define SCOUNT  30           // sum of sample point
int analogBuffer[SCOUNT];    // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0,copyIndex = 0;
float averageVoltage = 0,tdsValue = 0,temperature = 25; //temperature à modifier avec valeur capteur

const int chipSelect = 4;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Set the clock to run-mode, and disable the write protection
  rtc.halt(false);
  rtc.writeProtect(false);
  
  // The following lines can be commented out to use the values already stored in the DS1302
  /* 
   rtc.setDOW(THURSDAY);        // Set Day-of-Week to FRIDAY
   rtc.setTime(14, 21, 0);     // Set the time to 12:37:00 (24hr format)
   rtc.setDate(27, 5, 2021);   // Set the date to 27 Mai 2021
   Serial.println("Time initialized.");  
  */  
  // Start up the library Dallas Temp IC control
  sensors.begin();

  pinMode(TdsSensorPin,INPUT);

  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");
}

int getMedianNum(int bArray[], int iFilterLen) 
{
      int bTab[iFilterLen];
      for (byte i = 0; i<iFilterLen; i++)
      bTab[i] = bArray[i];
      int i, j, bTemp;
      for (j = 0; j < iFilterLen - 1; j++) 
      {
      for (i = 0; i < iFilterLen - j - 1; i++) 
          {
        if (bTab[i] > bTab[i + 1]) 
            {
        bTemp = bTab[i];
            bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
         }
      }
      }
      if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
      else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
      return bTemp;
}

float getVoltage()
{
  // Mesure du niveau de la batterie sur VBATPIN 
  // renvoi la valeur
  float measuredvbat = analogRead(VBATPIN);
  measuredvbat *= 2; //multiplies reading back to get voltage
  measuredvbat *= 3.3; // multiply by 3.3V, reference voltage
  measuredvbat /= 1024; // convert to voltage
  //logfile.print("VBat = "); logfile.println(measuredvbat);
  //logfile.flush();  //hopefully this fixes data not being written to file
  Serial.print("Voltage of Battery: "); 
  Serial.println(measuredvbat);
  return measuredvbat;
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void digitalClockDisplay(){
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.println(year());  
}

void loop() {
  // make a string for assembling the data to log:
  String dataString = "";
  // read three sensors and append to the string:
  /*for (int analogPin = 0; analogPin < 3; analogPin++) {
    int sensor = analogRead(analogPin);
    dataString += String(sensor);
    if (analogPin < 2) {
      dataString += ",";
    }
  }
  */
  // Send a divider for readability
  Serial.println("  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -");
 
  // Get data from the DS1302
  t = rtc.getTime();
  //Serial.println(String(t.date) , String (t.time));
  // Send date over serial connection
  Serial.print("Today is the ");
  Serial.print(t.date, DEC);
  Serial.print(".");
  Serial.print(rtc.getMonthStr());
  Serial.print(".");
  Serial.print(t.year, DEC);
  Serial.print(",");
  
  
  // Send Day-of-Week and time
  Serial.print(" the time is ");
  Serial.print(t.hour, DEC);
  Serial.print(":");
  Serial.print(t.min, DEC);
  Serial.print(":");
  Serial.print(t.sec, DEC);
  Serial.print(".");
  dataString = String(t.date) + "." + String(rtc.getMonthStr()) + "." + String(t.year) + "," + String(t.hour)+ ":" + String(t.min) + ":" + String(t.sec) + ",";

  // call sensors.requestTemperatures() to issue a global temperature 
  //Serial.println("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Celcius=sensors.getTempCByIndex(0);
  Fahrenheit=sensors.toFahrenheit(Celcius);
  Serial.print(" Temperature C :");
  Serial.print(Celcius);
  Serial.print("° / F  :");
  Serial.println(Fahrenheit);
  dataString += String(Celcius) + "," + String(Fahrenheit);
  temperature = Celcius;

  static unsigned long analogSampleTimepoint = millis();
  if(millis()-analogSampleTimepoint > 40U)     //every 40 milliseconds,read the analog value from the ADC
  {
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
    analogBufferIndex++;
    if(analogBufferIndex == SCOUNT) 
       analogBufferIndex = 0;
  }   
  static unsigned long printTimepoint = millis();
  if(millis()-printTimepoint > 800U)
  {
     printTimepoint = millis();
     for(copyIndex=0;copyIndex<SCOUNT;copyIndex++)
       analogBufferTemp[copyIndex]= analogBuffer[copyIndex];
     averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
     float compensationCoefficient=1.0+0.02*(temperature-25.0);    //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
     float compensationVoltage=averageVoltage/compensationCoefficient;  //temperature compensation
     tdsValue=(133.42*compensationVoltage*compensationVoltage*compensationVoltage - 255.86*compensationVoltage*compensationVoltage + 857.39*compensationVoltage)*0.5; //convert voltage value to tds value
     //Serial.print("voltage:");
     //Serial.print(averageVoltage,2);
     //Serial.print("V   ");
     Serial.print("TDS Value:");
     Serial.print(tdsValue,0);
     Serial.println(" ppm");
  }
  dataString += "," + String(tdsValue);

  // Mesure du voltage de la batterie
  VoltageBatterie = getVoltage();
  dataString += "," + String(VoltageBatterie);
  
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.print("DataLog: ");
    Serial.println(dataString);
    }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
  delay(Attente);  
}
