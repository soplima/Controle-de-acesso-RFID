#include <SPI.h>
#include <rdm6300.h>
#include "printf.h"
#include "RF24.h"

#define CE_PIN 7
#define CSN_PIN 8

#define RDM6300_RX_PIN 2
#define READ_LED_PIN 4

#define MSG 0
#define ACK 1
#define RTS 2
#define CTS 3

#define TEMPO_LIMITE 2000 

unsigned long tempo_espera = 0;
unsigned long tempo_recebe = 0;

RF24 radio(CE_PIN,CSN_PIN);
Rdm6300 leitor;

uint64_t address = 0x3030303030LL;

uint8_t id = 50;         //mudar para o id das plaquinhas
uint8_t id_gateway = 34;

byte payload[8];
byte payloadrx[8];
//por que tem dois paylaods? simplesmente, o payload normal é o nosso payload para mandar, e o payloadrx é meio que o buffer para receber as coisas

//------------------------------------------------------------------------------------------------

void setup(){
  Serial.begin(115200);
  while (!Serial) {}

  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1) {}
  }
  leitor.begin(RDM6300_RX_PIN);

  radio.setPALevel(RF24_PA_MAX);  
  radio.setChannel(37);
  radio.setPayloadSize(sizeof(payload)); 
  radio.setAutoAck(false);
  radio.setCRCLength(RF24_CRC_DISABLED); 
  radio.setDataRate(RF24_250KBPS);

  radio.openWritingPipe(address);
  radio.openReadingPipe(1, address); 
  
  radio.startListening();

  printf_begin();             
  radio.printPrettyDetails(); 
}

//------------------------------------------------------------------------------------------------

void mandaPayload(uint8_t destinatario, uint8_t tipo){
  radio.stopListening();

  payload[0] = id;
  payload[1] = destinatario;
  payload[2] = tipo;
  payload[7] = 0;
  
  radio.write(&payload, 8); 
  radio.startListening();
}

//------------------------------------------------------------------------------------------------

bool recebePayload(uint8_t remetente, uint8_t tipo){
  radio.startListening();
  radio.flush_rx();

  tempo_recebe = millis();

  while(millis() - tempo_recebe < TEMPO_LIMITE){ 
    if(radio.available()){
      radio.read(&payloadrx, 8); // Lê para o Inbox
      Serial.print("Recebi: ");
      Serial.print(payloadrx[0]);
      Serial.print(" ");
      Serial.print(payloadrx[1]);
      Serial.print(" ");
      Serial.println(payloadrx[2]);
      
      if((payloadrx[0] == remetente && payloadrx[1] == id && payloadrx[2] == tipo)){
        return true;
      }
    }
  }
  return false; // Retorna falso se o tempo esgotar
}

// bool recebePayload(uint8_t remetente, uint8_t tipo){

//   radio.startListening();

//   unsigned long inicio = millis();

//   while(millis() - inicio < 1000){

//     if(radio.available()){

//       radio.read(&payloadrx,8);

//       Serial.print("PACOTE RECEBIDO -> ");
//       Serial.print(payloadrx[0]);
//       Serial.print(" ");
//       Serial.print(payloadrx[1]);
//       Serial.print(" ");
//       Serial.println(payloadrx[2]);

//       return true;
//     }
//   }

//   Serial.println("Nenhum pacote chegou.");

//   return false;
// }

//------------------------------------------------------------------------------------------------

bool identificaPayload(){
  radio.read(&payloadrx, 8); // Lê para o Inbox
  
  if(payloadrx[1] != id && payloadrx[2] == CTS){
    Serial.println("Meio ocupado! Outro node esta transmitindo.");
    tempo_espera = millis() + 200; 
    return false;
  }
  return true;
}

//------------------------------------------------------------------------------------------------

void handshake(){
  Serial.println("Mandando RTS");
  mandaPayload(id_gateway, RTS);

  if(recebePayload(id_gateway, CTS)){
    Serial.println("CTS recebido");
    Serial.println("Enviando MSG");
    delay(10);
    mandaPayload(id_gateway, MSG);

    if(recebePayload(id_gateway, ACK)){
      Serial.println("ACK recebido. Sucesso!");
	    //payload[3]=0;
	    //payload[4]=0;
	    //payload[5]=0;
	    //payload[6]=0;
    }
    else{
      Serial.println("Erro: Tempo de resposta excedido, sem ACK");
    }
  }
  else{
    Serial.println("Erro: CTS não concedido (timeout ou colisao)");
  }
  
  radio.startListening();
}
//---------------------------------------------------------------------------------------------------

bool lerRFID(){
  if(leitor.get_new_tag_id()){
    Serial.println(leitor.get_tag_id(), HEX);
    int32_t tag_id = leitor.get_tag_id();

    payload[3] = (tag_id >>24) & 0XFF;
    payload[4] = (tag_id >>16) & 0XFF;
    payload[5] = (tag_id >>8) & 0XFF;
    payload[6] =  tag_id & 0XFF;

    return true;
  }
  return false;
}

//------------------------------------------------------------------------------------------------

void loop(){
  if(radio.available()){
    identificaPayload();
  }
  
  if(lerRFID()){
    if(millis() < tempo_espera){
        Serial.println("Aguarde: Durante tempo de espera (NAV ativo)");
      }
      else{
        handshake();
      }
  }
  /*if(Serial.available()){
    char c = Serial.read();
    if(toupper(c) == 'T'){
      if(millis() < tempo_espera){
        Serial.println("Aguarde: Durante tempo de espera (NAV ativo)");
      }
      else{
        handshake();
      }
    }
  }*/
}