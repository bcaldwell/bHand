void setup() {
  // put your setup code here, to run once:
  Serial.begin (115200);
  Serial1.begin(57600);
}

void loop() {
  // put your main code here, to run repeatedly:
     int incomingByte;
     Serial1.println ("Dote");
      
  /*if (Serial1.available() > 0) {
    incomingByte = Serial1.read();
    Serial1.print("UART received:");
    Serial1.println(incomingByte, DEC);
  }*/

}
