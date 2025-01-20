#include <Servo.h>

#include <ArduinoJson.h>
Servo A;
Servo B;
Servo C;
int P2 = 7;
int P3 = 2;
int P4 = 3;
int P5 = 4;
float M1;
float M2;
float M3;
float M4;

String receivedData = "";
#include <SoftwareSerial.h>
bool endOfMessage = false;
SoftwareSerial bt(12,13);
void setup() {
  // put your setup code here, to run once:
  delay(1000);
  Serial.begin(38400);
  pinMode(P2,OUTPUT);
  pinMode(P3,OUTPUT);
  pinMode(P4,OUTPUT);
  pinMode(P5,OUTPUT);
  A.attach(5);
  B.attach(6);
  C.attach(9);

  delay(1000);

  bt.begin(9600);
  delay(1000);


}
void right(int p1,int p2, int p3, int p4){
  digitalWrite(p1,0);
  digitalWrite(p2,1);
  digitalWrite(p3,0);
  digitalWrite(p4,1);
  Serial.println("forward");
  delay(100);
  rest(P2,P3,P4,P5);
  return;
}
void back(int p1,int p2, int p3, int p4){
  digitalWrite(p1,1);
  digitalWrite(p2,1);
  digitalWrite(p3,0);
  digitalWrite(p4,0);
  Serial.println("right");

  delay(100);
  rest(P2,P3,P4,P5);
  return;
}
void forward(int p1,int p2, int p3, int p4){
  digitalWrite(p1,0);
  digitalWrite(p2,0);
  digitalWrite(p3,1);
  digitalWrite(p4,1);
  Serial.println("left");
  delay(100);
  rest(P2,P3,P4,P5);
  return;
}
void left(int p1,int p2, int p3, int p4){
  digitalWrite(p1,1);
  digitalWrite(p2,0);
  digitalWrite(p3,1);
  digitalWrite(p4,0);
  Serial.println("back");
  
  delay(100);
  rest(P2,P3,P4,P5);
  return;
}
void rest(int p1,int p2, int p3, int p4){
  digitalWrite(p1,0);
  digitalWrite(p2,0);
  digitalWrite(p3,0);
  digitalWrite(p4,0);

  return;

}
void Parser(String receivedData){
  String jsonString = receivedData;

  // Allocate the JSON document
  // Use https://arduinojson.org/v6/assistant/ to compute the capacity
  StaticJsonDocument<200> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, jsonString);

  // Test if parsing succeeds
  if (error) {
    Serial.println(F("deserializeJson() failed: "));
    
    
    return;
  }



  // Fetch values
  if (doc.containsKey("m")){
    const char* m = doc["m"];
    
    char mChar = m[0];
    Serial.println(mChar);
    switch(mChar){
      case 'f':
        forward(P2,P3,P4,P5);
        break;
      case 'a':
        right(P2,P3,P4,P5);
        break;
      case 'd':
        left(P2,P3,P4,P5);
        break;
      case 's':
        back(P2,P3,P4,P5);
        break;
      default:
        rest(P2,P3,P4,P5);
        
    }
  }
  if (doc.containsKey("M1")){
    int nM1 = doc["M1"];
    if (M1 != nM1){
      M1 = nM1;
      A.write(M1);
      delay(100);
      Serial.println(M1);
    }
  

  }
  if (doc.containsKey("M2")){
    int nM2 = doc["M2"];
    if (M2 != nM2){
      M2 = nM2;
      B.write(M2);
      Serial.println(M2) ;
    }
    
  }
  if (doc.containsKey("M3")){
    int nM3 = doc["M3"];
    if (M3 != nM3){
      M3 = nM3;
      C.write(M3);
      Serial.println(M3);
    }
  }
  if (doc.containsKey("M4")){
    int nM4 = doc["M4"];
    if (M4 != nM4){
      M4 = nM4;
      Serial.println(M1);
    }
    
  } 
  


}


void loop() {
  // put your main code here, to run repeatedly:
  while(bt.available()>0){
    
    char inChar = (char)bt.read();
   
    
    // Just for demonstration, print the received data
    switch (inChar) {
      case  '\n':
        endOfMessage = true;
        break;
      default:
      
        receivedData += inChar;
        break;
    }

   
    
  }
  if (endOfMessage) {
    Parser(receivedData);
   


    
    // Reset for next message
    receivedData = "";
    endOfMessage = false;
  }

  

  
} 
