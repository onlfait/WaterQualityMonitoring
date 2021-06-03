const int PHprobe = 1;


void setup() {
  Serial.begin(9600);
}

void loop() {
int WaterTemp = 18;
float pH1, pH2, pH3;
int i;

for(i = 0;i <= 7;i++) {
pH2 += 0.0178 * analogRead(PHprobe) - 1.889;
  pH3 += 7 - (2.5 - analogRead(PHprobe)/1024) / (0.257179 + 0.000941468 * WaterTemp);
            delay(1000);
      } 

pH1 = analogRead(PHprobe);
       Serial.print("Mesure du pH");

       Serial.print(pH1,  DEC);
       Serial.print("\n");
       pH2 = pH2 / 8;
       Serial.print(pH2,  DEC);
       Serial.print("\n");
       pH3 = pH3 / 8;
       Serial.print(pH3,  DEC);
       Serial.print("\n");
       Serial.print("\n");
       delay(10000);



       Serial.print("Mesure de la Turbidite");

  int sensorValue = analogRead(A0);// read the input on analog pin 0:
  Serial.print(sensorValue); // print out the value you read:
  Serial.print(" / ");
  float voltage = sensorValue * (5.0 / 1024.0); // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  Serial.println(voltage); // print out the value you read:
       Serial.print("====================================");

  delay(500);
}
