#define ledPin 4
void setup(){
  pinMode(ledPin,OUTPUT); }
void loop(){
  digitalWrite(ledPin,HIGH); 
  delay(3000); 
  digitalWrite(ledPin,LOW); 
  delay(3000);
}
