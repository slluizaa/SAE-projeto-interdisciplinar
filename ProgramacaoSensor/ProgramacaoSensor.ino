#include <dummy.h>
#include "Ultrasonic.h" //INCLUSÃO DA BIBLIOTECA NECESSÁRIA PARA FUNCIONAMENTO DO CÓDIGO
#include <ESP8266WiFi.h> // Importa a Biblioteca ESP8266WiFi
#include <PubSubClient.h> // Importa a Biblioteca PubSubClient
 
//////////////// VARIAVEIS A SER EDITADAS ////////////////

// Informações necessárias para o dispositivo
float nivelMax = 30; // NIVEL MAXIMO DO RIO/CORREGO/LOCAL
int idSensor = 1; // ID DO DISPOSITIVO QUE SERA REGISTRADO NO BANCO DE DADOS

// WIFI
const char* SSID = "Lucas"; // SSID / nome da rede WI-FI que deseja se conectar
const char* PASSWORD = "matheus66"; // Senha da rede WI-FI que deseja se conectar
  
// MQTT
const char* BROKER_MQTT = "54.174.238.244"; //URL do broker MQTT que se deseja utilizar
int BROKER_PORT = 1883; // Porta do Broker MQTT

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////// VARIAVEIS DOS SENSORES ////////////////

const int pinoSensor = D2; //PINO DIGITAL UTILIZADO PELO SENSOR CHUVA
const int echoPin = D6; //PINO DIGITAL UTILIZADO PELO HC-SR04 ECHO(RECEBE)
const int trigPin = D5; //PINO DIGITAL UTILIZADO PELO HC-SR04 TRIG(ENVIA)

Ultrasonic ultrasonic(trigPin,echoPin); //INICIALIZANDO OS PINOS DO ARDUINO
 
float distancia; // DISTANCIA CAPTADA DO SENSOR
float nivelMedido; // NIVEL MEDIDO DO RIO/CORREGO/LOCAL
String resultEcho; // STRING COM A DISTANCIA CAPTADA DO SENSOR
boolean chuva; //RESULTADO DO SENSOR DA CHUVA
String resultSensor; //RESULTADO EM STRING DO SENSOR DA CHUVA
String resultStatus; //STATUS ATUAL DO NIVEL DE PERICULOSIDADE (VERMELHO, AMARELO,VERDE)


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////// VARIAVEIS E DEFINIÇÕES PARA UTILIZAR O PROTOCOLO MQTT ////////////////
//Autor: Fábio Henrique Cabrini


//defines:
//defines de id mqtt e tópicos para publicação e subscribe
#define TOPICO_SUBSCRIBE "/iot/sensor001/cmd"     //tópico MQTT de escuta
#define TOPICO_PUBLISH   "/iot/sensor001/attrs/c"    //tópico MQTT de envio de informações para Broker - RESULTADO DO SENSOR DE CHUVA
#define TOPICO_PUBLISH_2   "/iot/sensor001/attrs/d"    //tópico MQTT de envio de informações para Broker - RESULTADOS AGREGADOS DO SENSOR
#define TOPICO_PUBLISH_3   "/iot/sensor001/attrs/s"    //tópico MQTT de envio de informações para Broker - RESULTADO DO STATUS DE PERICULOSIDADE
#define TOPICO_PUBLISH_4   "/iot/sensor001/attrs/i"    //tópico MQTT de envio de informações para Broker - ID DO SENSOR

                                                   
                                                   //IMPORTANTE: recomendamos fortemente alterar os nomes
                                                   //            desses tópicos. Caso contrário, há grandes
                                                   //            chances de você controlar e monitorar o NodeMCU
                                                   //            de outra pessoa.
#define ID_MQTT  "helix"     //id mqtt (para identificação de sessão)
                               //IMPORTANTE: este deve ser único no broker (ou seja, 
                               //            se um client MQTT tentar entrar com o mesmo 
                               //            id de outro já conectado ao broker, o broker 
                               //            irá fechar a conexão de um deles).
                                
 
//defines - mapeamento de pinos do NodeMCU
#define D0    16
#define D1    5
#define D2    4
#define D3    0
#define D4    2
#define D5    14
#define D6    12
#define D7    13
#define D8    15
#define D9    3
#define D10   1

 
//Variáveis e objetos globais
WiFiClient espClient; // Cria o objeto espClient
PubSubClient MQTT(espClient); // Instancia o Cliente MQTT passando o objeto espClient

  
//Prototypes
void initSerial();
void initWiFi();
void initMQTT();
void reconectWiFi(); 
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void VerificaConexoesWiFIEMQTT(void);
void InitOutput(void);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////// FUNÇÕES DO MQTT ////////////////
  
//Função: inicializa comunicação serial com baudrate 115200 (para fins de monitorar no terminal serial 
//        o que está acontecendo.
//Parâmetros: nenhum
//Retorno: nenhum
void initSerial() 
{
    Serial.begin(115200);
}
 
//Função: inicializa e conecta-se na rede WI-FI desejada
//Parâmetros: nenhum
//Retorno: nenhum
void initWiFi() 
{
    delay(10);
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");
     
    reconectWiFi();
}
  
//Função: inicializa parâmetros de conexão MQTT(endereço do 
//        broker, porta e seta função de callback)
//Parâmetros: nenhum
//Retorno: nenhum
void initMQTT() 
{
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);   //informa qual broker e porta deve ser conectado
    MQTT.setCallback(mqtt_callback);            //atribui função de callback (função chamada quando qualquer informação de um dos tópicos subescritos chega)
}
  
//Função: função de callback 
//        esta função é chamada toda vez que uma informação de 
//        um dos tópicos subescritos chega)
//Parâmetros: nenhum
//Retorno: nenhum
void mqtt_callback(char* topic, byte* payload, unsigned int length) 
{
    String msg;
 
    //obtem a string do payload recebido
    for(int i = 0; i < length; i++) 
    {
       char c = (char)payload[i];
       msg += c;
    }
   
    //toma ação dependendo da string recebida:
    if (msg.equals("sensor001@VERDE|"))
    {
        digitalWrite(LED_BUILTIN, LOW);
    }

    if (msg.equals("sensor001@AMARELO|"))
    {
        digitalWrite(LED_BUILTIN, LOW);
    }
 
    //verifica se deve colocar nivel alto de tensão na saída D0:
    if (msg.equals("sensor001@VERMELHO|"))
    {
        digitalWrite(LED_BUILTIN, HIGH);
    }
}
  
//Função: reconecta-se ao broker MQTT (caso ainda não esteja conectado ou em caso de a conexão cair)
//        em caso de sucesso na conexão ou reconexão, o subscribe dos tópicos é refeito.
//Parâmetros: nenhum
//Retorno: nenhum
void reconnectMQTT() 
{
    while (!MQTT.connected()) 
    {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) 
        {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPICO_SUBSCRIBE); 
        } 
        else
        {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Havera nova tentatica de conexao em 2s");
            delay(2000);
        }
    }
}
  
//Função: reconecta-se ao WiFi
//Parâmetros: nenhum
//Retorno: nenhum
void reconectWiFi() 
{
    //se já está conectado a rede WI-FI, nada é feito. 
    //Caso contrário, são efetuadas tentativas de conexão
    if (WiFi.status() == WL_CONNECTED)
        return;
         
    WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI
     
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(100);
        Serial.print(".");
    }
   
    Serial.println();
    Serial.print("Conectado com sucesso na rede ");
    Serial.print(SSID);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());
}
 
//Função: verifica o estado das conexões WiFI e ao broker MQTT. 
//        Em caso de desconexão (qualquer uma das duas), a conexão
//        é refeita.
//Parâmetros: nenhum
//Retorno: nenhum
void VerificaConexoesWiFIEMQTT(void)
{
    if (!MQTT.connected()) 
        reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita
     
     reconectWiFi(); //se não há conexão com o WiFI, a conexão é refeita
}


//////////////// FUNÇÕES DOS SENSORES ////////////////

//Função: Verifica se está chovendo ou não. 
//        Preenche as variaveis 'chuva' e 'resultSensor' além de
//        publicar no tópico específico do MQTT o status.
//Parâmetros: nenhum
//Retorno: nenhum
void verificaChuva(){
  if(digitalRead(pinoSensor) == LOW)
  { 
    chuva = true;
    resultSensor="TRUE";
  }
  else
  { 
    chuva = false;
    resultSensor="FALSE";
  } 
}

//Função: Utiliza o hcsr04 para verificar o nivel de profundidade. 
//        Preenche as variaveis 'distancia' e 'nivelMedio'  e 'resultEcho' além de
//        publicar no tópico específico do MQTT o status.
//Parâmetros: nenhum
//Retorno: nenhum
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
    if(nivelMax>distancia)
    {
      nivelMedido = (nivelMax-distancia);
    }
    else
    {
      nivelMedido = distancia;
    }
    resultEcho = String(distancia); //VARIÁVEL GLOBAL DO TIPO STRING RECEBE A DISTÂNCIA(CONVERTIDO DE INTEIRO PARA STRING)

 }


//Função: A partir de uma cadeia de if, realiza um fluxo de decisão 
//        para determinar o status de periculosidade do momento.
//        Utiliza as variaveis 'nivelMedido', 'nivelMax' e 'chuva'.
//        Ele publica o status no tópico específico do MQTT.
//Parâmetros: nenhum
//Retorno: nenhum
 void verificaStatus()
 {
    if(nivelMedido>=(nivelMax))
    {
      resultStatus = "VERMELHO";
      MQTT.publish(TOPICO_PUBLISH_3, "VERMELHO"); //PUBLICA NO MQTT O STATUS
    }
    else
    {
      if(nivelMedido>=(0.6*nivelMax))
      {
        if(nivelMedido>=(0.8*nivelMax))
        {
          if(chuva == false)
           {
             resultStatus = "AMARELO";
             MQTT.publish(TOPICO_PUBLISH_3, "AMARELO"); //PUBLICA NO MQTT O STATUS
           }
          else
          {
            resultStatus = "VERMELHO";
            MQTT.publish(TOPICO_PUBLISH_3, "VERMELHO"); //PUBLICA NO MQTT O STATUS
          }
        }
        else
        {
          if(chuva == false)
           {
             resultStatus = "VERDE";
             MQTT.publish(TOPICO_PUBLISH_3, "VERDE"); //PUBLICA NO MQTT O STATUS
           }
           else
           {
             resultStatus = "AMARELO";
             MQTT.publish(TOPICO_PUBLISH_3, "AMARELO"); //PUBLICA NO MQTT O STATUS
           }
        }
      }
      else
      {
        resultStatus = "VERDE";
        MQTT.publish(TOPICO_PUBLISH_3, "VERDE"); //PUBLICA NO MQTT O STATUS
      }
    }
 }


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////// SETUP E VOID EM LOOP ////////////////

void setup() 
{
  //VOID SETUP LUCAS
  pinMode(pinoSensor, INPUT); //DEFINE O PINO COMO ENTRADA
  pinMode(LED_BUILTIN, OUTPUT); //LED DO NODEMCU
  pinMode(echoPin, INPUT); //DEFINE O PINO COMO ENTRADA (RECEBE)
  pinMode(trigPin, OUTPUT); //DEFINE O PINO COMO SAIDA (ENVIA)
  
    //inicializações:
    initSerial();
    initWiFi();
    initMQTT();
    delay(5000);
}

void loop() 
{   
    char msgBuffer[1];
    //garante funcionamento das conexões WiFi e ao broker MQTT
    VerificaConexoesWiFIEMQTT();
    

    hcsr04(); // FAZ A CHAMADA DO MÉTODO "hcsr04()"
    verificaChuva(); //FAZ A CHAMADA DO MÉTODO DE CHUVA
    verificaStatus(); //FAZ A CHAMADA DO METODO DE STATUS
    
    Serial.print("Distancia "); //IMPRIME O TEXTO NO MONITOR SERIAL
    Serial.print(resultEcho); ////IMPRIME NO MONITOR SERIAL A DISTÂNCIA MEDIDA
    Serial.println("cm"); //IMPRIME O TEXTO NO MONITOR SERIAL
    
    Serial.print("Status chuva: ");    
    Serial.println(resultSensor);

    Serial.print("Status: ");
    Serial.println(resultStatus);

    Serial.print("ID Sensor: ");
    Serial.print(String(idSensor));
    char msg_id[20];
    sprintf(msg_id, "%d",idSensor);

    Serial.println("");

    String DADOS = (String(idSensor) + "|" + resultSensor + "|" + String(nivelMedido) + "|" + resultStatus );

    // Length (with one extra character for the null terminator)
    int str_len = DADOS.length() + 1; 

    // Prepare the character array (the buffer) 
    char char_array[str_len];

    // Copy it over 
    DADOS.toCharArray(char_array, str_len);
    
    MQTT.publish(TOPICO_PUBLISH_2, char_array ); //PUBLICA NO MQTT O STATUS
    Serial.println("");
    delay(180000); //INTERVALO DE 180000 MILISSEGUNDOS
    
    //keep-alive da comunicação com broker MQTT
    MQTT.loop();
}
