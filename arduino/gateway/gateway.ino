//Gateway
#include <SPI.h>
#include <printf.h>
#include <RF24.h>

#define CE_PIN 7
#define CNS_PIN 8

#define TEMPO_LIMITE 1000

#define MSG 0
#define ACK 1
#define RTS 2
#define CTS 3

RF24 radio(CE_PIN, CNS_PIN);

uint64_t address = 0x3030303030LL;

byte payload[8];
byte payloadrx[8];

uint8_t Id = 34; //mudar para o id da praquinha
uint8_t led_id = 9;

uint32_t numero_tag;

void setup(){
  Serial.begin(115200);
  Serial.println("Gateway iniciado");
  
  while(!Serial){};
  
  if (!radio.begin()){ 
      Serial.println("Radio nao funciona"); 
      while(1){}; 
  }
    
  radio.setPALevel(RF24_PA_MAX); 
  radio.setChannel(37);
  radio.setPayloadSize(sizeof(payloadrx));
  radio.setAutoAck(false);
  radio.setCRCLength(RF24_CRC_DISABLED); 
  radio.setDataRate(RF24_250KBPS);

  radio.openWritingPipe(address);
  radio.openReadingPipe(1, address);
  radio.startListening();
  
  printf_begin(); 
  radio.printPrettyDetails(); 
}

void printPayload(byte *dados, int tamanho){
  Serial.print(F("Rcvd "));
  Serial.print(tamanho); 
  Serial.print(F(" ID: "));
  Serial.print(dados[0]);  
  Serial.print(F(" Destinatario: "));
  Serial.print(dados[1]);  
  Serial.print(F(" Tipo: "));
  Serial.print(dados[2]);  
  Serial.print(F(" Tag: "));
  
  Serial.print("{\"node\": ");
  Serial.print(payloadrx[0]);
  Serial.print(", \"tag\": ");
  numero_tag =((uint32_t)payloadrx[3]<<24)|((uint32_t)payloadrx[4]<<16)|((uint32_t)payloadrx[5]<<8)|((uint32_t)payloadrx[6]);
  Serial.print(numero_tag, HEX);
  Serial.print("}");


  Serial.print(F(" : "));
  for(int i=7; i<tamanho; i++){
    Serial.print(dados[i]);
  }
  Serial.println();
}

bool mandaPayload(uint8_t destinatario, uint8_t tipo){
  radio.stopListening();

  payload[0] = Id;
  payload[1] = destinatario;
  payload[2] = tipo;
  
  Serial.print("Enviando: ");
  Serial.print(payload[0]);
  Serial.print(" ");
  Serial.print(payload[1]);
  Serial.print(" ");
  Serial.println(payload[2]);

  // radio.startListening();
  // delayMicroseconds(150);
  // if (radio.testCarrier()) {
  //   radio.stopListening();
  //   return false;
  // }
  // radio.stopListening();
  bool ok = radio.write(&payload, 8);
  radio.startListening();

  // return radio.write(&payload, 8);
  return  ok;
}

bool aguardaMSG(uint8_t remetente, uint8_t tipo) {
  radio.startListening();
  radio.flush_rx();
  
  unsigned long start = millis();
  
  while(millis() - start < TEMPO_LIMITE) { 
    if(radio.available()){
      radio.read(&payloadrx, 8);
      
      if(payloadrx[0] == remetente && payloadrx[1] == Id && payloadrx[2] == tipo) {
        if(tipo == MSG){printPayload(payloadrx, 8);}
        return true;
      }
    }
  }
  return false; 
}

void trataRTS(uint8_t remetente) {
  Serial.print("RTS de ");
  Serial.println(remetente);
  
  delay(50); 
  
  bool ctsEnviado = mandaPayload(remetente, CTS);
  
  if (ctsEnviado) {
    Serial.println("CTS enviado. Aguardando MSG...");
    
    if (aguardaMSG(remetente, MSG)) {
      Serial.println("MSG Recebida, mandando ACK");
      delay(50);
      mandaPayload(remetente, ACK);
      delay(1000);
      handshake_led(led_id);
    } else {
      Serial.println("Timeout: MSG nao chegou.");
    }
  } else {
    Serial.println("Falha ao enviar CTS: Meio ocupado.");
  }
  
  radio.startListening(); 
}

void handshake_led(uint8_t led_id){
  Serial.println("Mandando RTS");
  mandaPayload(led_id, RTS);

  if(aguardaMSG(led_id, CTS)){
    Serial.println("CTS recebido");
    Serial.println("Enviando MSG");
    
    mandaPayload(led_id, MSG);

    if(aguardaMSG(led_id, ACK)){
      Serial.println("ACK recebido. Sucesso!");
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


void processaPayload(byte *dados) {
  uint8_t remetente = dados[0];
  uint8_t destinatario = dados[1];
  uint8_t tipo = dados[2];

  if (destinatario == Id) { 
    switch(tipo) {
      case RTS:
        trataRTS(remetente);
        break;
    }
  }
}
void loop() {
  if (radio.available()) {
    radio.read(&payloadrx, 8); // Grava os dados da antena no Inbox (payloadrx)
    
    //printPayload(payloadrx, 8);
    processaPayload(payloadrx);
  }
}
