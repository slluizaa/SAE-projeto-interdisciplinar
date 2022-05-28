int red = D5;
int green = D6;
int som = 4;

int distancia = 70;
String chuva = "TRUE";


void setup() {
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(som, OUTPUT);
}

void loop() {
  if((distancia >= 50) && (chuva = "TRUE")){
    digitalWrite(green, LOW);
    digitalWrite(red, HIGH);
    tone(som, 330); //MI
    delay(1000);
  }
  else if((distancia < 50) || (chuva = "FALSE")){
    digitalWrite(green, HIGH);
    delay(1000);
  }
  digitalWrite(red, LOW);
  noTone(som);
  delay(1000);

}
