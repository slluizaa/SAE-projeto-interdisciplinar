#include <dummy.h>

#include "Ultrasonic.h" //INCLUSÃO DA BIBLIOTECA NECESSÁRIA PARA FUNCIONAMENTO DO CÓDIGO

const int pinoSensor = D2; //PINO DIGITAL UTILIZADO PELO SENSOR CHUVA
const int echoPin = D6; //PINO DIGITAL UTILIZADO PELO HC-SR04 ECHO(RECEBE)
const int trigPin = D5; //PINO DIGITAL UTILIZADO PELO HC-SR04 TRIG(ENVIA)
 
Ultrasonic ultrasonic(trigPin,echoPin); //INICIALIZANDO OS PINOS DO ARDUINO
 
float distancia; //VARIÁVEL DO TIPO INTEIRO
float distanciaMax = 10; 
String resultEcho; //VARIÁVEL DO TIPO STRING
boolean chuva;
String resultSensor; //VARIÁVEL DO TIPO STRING
String resultStatus;

void setup(){
  pinMode(pinoSensor, INPUT); //DEFINE O PINO COMO ENTRADA
  pinMode(LED_BUILTIN, OUTPUT); //LED DO NODEMCU
  pinMode(echoPin, INPUT); //DEFINE O PINO COMO ENTRADA (RECEBE)
  pinMode(trigPin, OUTPUT); //DEFINE O PINO COMO SAIDA (ENVIA)
  Serial.begin(115200); //INICIALIZA A PORTA SERIAL
  }
void loop(){
  
  hcsr04(); // FAZ A CHAMADA DO MÉTODO "hcsr04()"
  verificaChuva(); //FAZ A CHAMADA DO MÉTODO DE CHUBA
  verificaStatus();
  Serial.print("Distancia "); //IMPRIME O TEXTO NO MONITOR SERIAL
  Serial.print(resultEcho); ////IMPRIME NO MONITOR SERIAL A DISTÂNCIA MEDIDA
  Serial.println("cm"); //IMPRIME O TEXTO NO MONITOR SERIAL
  Serial.print("Status chuva: ");
  Serial.println(resultSensor);
  Serial.print("Status: ");
  Serial.println(resultStatus);
  Serial.println("");
  delay(1000); //INTERVALO DE 500 MILISSEGUNDOS
}

void verificaChuva(){
  if(digitalRead(pinoSensor) == LOW)
  { 
    chuva = true;
    resultSensor="TRUE";
    digitalWrite(LED_BUILTIN, HIGH); 
  }
  else
  { 
    chuva = false;
    resultSensor="FALSE";
    digitalWrite(LED_BUILTIN, LOW); 
  } 
}

//MÉTODO RESPONSÁVEL POR CALCULAR A DISTÂNCIA
void hcsr04(){
    digitalWrite(trigPin, LOW); //SETA O PINO 6 COM UM PULSO BAIXO "LOW"
    delayMicroseconds(2); //INTERVALO DE 2 MICROSSEGUNDOS
    digitalWrite(trigPin, HIGH); //SETA O PINO 6 COM PULSO ALTO "HIGH"
    delayMicroseconds(10); //INTERVALO DE 10 MICROSSEGUNDOS
    digitalWrite(trigPin, LOW); //SETA O PINO 6 COM PULSO BAIXO "LOW" NOVAMENTE
    //FUNÇÃO RANGING, FAZ A CONVERSÃO DO TEMPO DE
    //RESPOSTA DO ECHO EM CENTIMETROS, E ARMAZENA
    //NA VARIAVEL "distancia"
    distancia = (ultrasonic.Ranging(CM)); //VARIÁVEL GLOBAL RECEBE O VALOR DA DISTÂNCIA MEDIDA
    resultEcho = String(distancia); //VARIÁVEL GLOBAL DO TIPO STRING RECEBE A DISTÂNCIA(CONVERTIDO DE INTEIRO PARA STRING)
 }

 void verificaStatus()
 {
    if(distancia>=(distanciaMax))
    {
      resultStatus = "VERMELHO";
    }
    else
    {
      if(distancia>(0.6*distanciaMax))
      {
        if(distancia>(0.8*distanciaMax))
        {
          if(chuva == false)
           {
             resultStatus = "AMARELO";
           }
          else
          {
            resultStatus = "VERMELHO";
          }
        }
        else
        {
          if(chuva == false)
           {
             resultStatus = "VERDE";
           }
           else
           {
             resultStatus = "AMARELO";
           }
        }
      }
      else
      {
        resultStatus = "VERDE";
      }
    }
 }
