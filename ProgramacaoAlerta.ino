//////////////// VARIAVEIS A SER EDITADAS ////////////////

// WIFI
const char* SSID = "Lucas"; // SSID / nome da rede WI-FI que deseja se conectar
const char* PASSWORD = "matheus66"; // Senha da rede WI-FI que deseja se conectar
  
// MQTT
const char* BROKER_MQTT = "34.199.31.113"; //URL do broker MQTT que se deseja utilizar
int BROKER_PORT = 1883; // Porta do Broker MQTT

//PARTE LUCAS - DISPOSITIVO DE SENSORES
#include "Ultrasonic.h" //INCLUSÃO DA BIBLIOTECA NECESSÁRIA PARA FUNCIONAMENTO DO CÓDIGO

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////// VARIAVEIS A SER EDITADAS ////////////////

int red = D5; //PINO UTILIZADO PARA O LED VERMELHO
int green = D6; //PINO UTILIZADO PARA O LED VERDE
int som = 4; //PINO UTILIZADO PARA O BUZZER
String statusInscrito; //VARIAVEL QUE ARMAZENA O STATUS RECEBIDO NO TOPICO INSCRITO

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////// VARIAVEIS E DEFINIÇÕES PARA UTILIZAR O PROTOCOLO MQTT ////////////////
//Autor: Fábio Henrique Cabrini
 
#include <ESP8266WiFi.h> // Importa a Biblioteca ESP8266WiFi
#include <PubSubClient.h> // Importa a Biblioteca PubSubClient
 
//defines:
//defines de id mqtt e tópicos para publicação e subscribe
#define TOPICO_SUBSCRIBE "/iot/sensor001/attrs/s"     //tópico MQTT de escuta - RESULTADO DO STATUS DE PERICULOSIDADE

                                                   
                                                   //IMPORTANTE: recomendamos fortemente alterar os nomes
                                                   //            desses tópicos. Caso contrário, há grandes
                                                   //            chances de você controlar e monitorar o NodeMCU
                                                   //            de outra pessoa.
#define ID_MQTT  "helixAlerta"     //id mqtt (para identificação de sessão)
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
    if (msg.equals("VERMELHO"))
    {
            statusInscrito = "VERMELHO";
    }

    if (msg.equals("AMARELO"))
    {
        statusInscrito = "AMARELO";

    }
 
    //verifica se deve colocar nivel alto de tensão na saída D0:
    if (msg.equals("VERDE"))
    {
        statusInscrito = "VERDE";
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


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////// SETUP E VOID EM LOOP ////////////////

void setup() 
{
  //VOID SETUP LUCAS
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(som, OUTPUT);
  
    //inicializações:
    initSerial();
    initWiFi();
    initMQTT();
    delay(5000);
}

//programa principal
void loop() 
{   
    //keep-alive da comunicação com broker MQTT
    MQTT.loop();

    VerificaConexoesWiFIEMQTT();
    
    //toma ação dependendo da string recebida:
    if (statusInscrito.equals("VERMELHO"))
    {
            digitalWrite(green, LOW);
            digitalWrite(red, HIGH);
            tone(som, 330); //MI
            delay(1000);
            digitalWrite(red, LOW);
            noTone(som);
            delay(1000);

    }

    

    if (statusInscrito.equals("AMARELO"))
    {
        digitalWrite(green, HIGH);
        digitalWrite(red, HIGH);
    }
 
    //verifica se deve colocar nivel alto de tensão na saída D0:
    if (statusInscrito.equals("VERDE"))
    {
        digitalWrite(green, HIGH);
        digitalWrite(red, LOW);
    }
}
