#include <SPI.h>
#include "printf.h"
#include "RF24.h"

#define CE_PIN 7
#define CSN_PIN 8

#define MSG 0
#define ACK 1
#define RTS 2
#define CTS 3

#define TEMPO_LIMITE 500 
#define LED_PIN 13

unsigned long tempo_espera = 0;
unsigned long tempo_recebe = 0;

RF24 radio(CE_PIN,CSN_PIN);

uint64_t address = 0x3030303030LL;

uint8_t id = 0;         //mudar para o id das plaquinhas
uint8_t id_gateway = 34;

byte payload[8];
byte payloadrx[8];

//------------------------------------------------------------------------------------------------

void setup(){
  Serial.begin(115200);
  while (!Serial) {}

  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1) {}
  }

  radio.setPALevel(RF24_PA_MAX);  
  radio.setChannel(101);
  radio.setPayloadSize(sizeof(payload)); 
  radio.setAutoAck(false);
  radio.setCRCLength(RF24_CRC_DISABLED);
  radio.setDataRate(RF24_2MBPS);

  radio.openWritingPipe(address);
  radio.openReadingPipe(1, address); 
  
  radio.startListening();

  printf_begin();             
  radio.printPrettyDetails(); 

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
}

//------------------------------------------------------------------------------------------------

bool mandaPayload(uint8_t destinatario, uint8_t tipo){
  radio.stopListening();

  payload[0] = id;
  payload[1] = destinatario;
  payload[2] = tipo;
  payload[7] = 0;
  
  radio.write(&payload, 8); 
  radio.startListening();
  return true;
}

//------------------------------------------------------------------------------------------------

bool recebePayload(uint8_t remetente, uint8_t tipo){
  radio.startListening();
  radio.flush_rx();

  tempo_recebe = millis();

  while(millis() - tempo_recebe < TEMPO_LIMITE){ 
    if(radio.available()){
      radio.read(&payloadrx, 8); // Lê para o Inbox
      
      if((payloadrx[0] == remetente && payloadrx[1] == id && payloadrx[2] == tipo)){
        return true;
      }
    }
  }
  return false; // Retorna falso se o tempo esgotar
}

//------------------------------------------------------------------------------------------------

bool identificaPayload(){
  
  if(payloadrx[1] == id){
    if(payloadrx[2] == RTS){
      trataRTS(payloadrx[0]);
    }
  } else if(payloadrx[2] == CTS){  
    Serial.println("Meio ocupado! Outro node esta transmitindo.");
    tempo_espera = millis() + 200; 
    return false;
  }
  return true;
}

void trataRTS(uint32_t remetente){
  Serial.println("RTS recebido");
  delay(10);
  if(mandaPayload(remetente, CTS)){
    Serial.println("aaa");
   // if(recebePayload(remetente, MSG)){

      Serial.println("bbbbb");
      mandaPayload(remetente, ACK);
      digitalWrite(LED_PIN, HIGH);
      delay(2000);
      digitalWrite(LED_PIN, LOW);
   // }
  }
  radio.startListening();
}
//---------------------------------------------
void loop(){
  if(radio.available()){
    radio.read(&payloadrx, 8);
    identificaPayload();
  }
}

