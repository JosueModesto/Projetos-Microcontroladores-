#include "HX711.h"
#include <avr/io.h>
#include <util/delay.h> 
#include <avr/pgmspace.h>

//Definições de macros
#define set_bit(adress,bit) (adress|=(1<<bit))  
#define clr_bit(adress,bit) (adress&=~(1<<bit)) 
#define TOP 39999         //valor para a máxima contagem
#define BAUD   9600    //taxa de 9600 bps
#define MYUBRR  F_CPU/16/BAUD-1

#define BUFFSIZE 10
char buff[BUFFSIZE];
char cnt=0, last=0;

float peso = 100;
unsigned long clique = 0;

char last_state = (1<<PD4);
char last_state2 = (1<<PD5);
char last_state3 = (1<<PD6);
char last_state4 = (1<<PD7);

const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN = 3;

float qtd_dispenser = 0;

HX711 scale;

void setup(){
  
  Serial.begin(9600);
   scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
   
  DDRD &= ~(0b11110100); // PD6 entrada
  DDRB |= (1<<PB1); // PB1 

  PORTB |= (1<<PB1);

  PORTD |= (1<<PD4);
  PORTD |= (1<<PD5);
  PORTD |= (1<<PD6);
  PORTD |= (1<<PD7);

        //TOP = (F_CPU/(N*F_PWM))-1, com N = 8 e F_PWM = 50 Hz
      ICR1 = TOP;           //configura o período do PWM (20 ms)

      // Configura o TC1 para o modo PWM rápido via ICR1, prescaler = 8
      TCCR1A = (1 << WGM11);
      TCCR1B = (1 << WGM13) | (1<<WGM12) | (1 << CS11);

      set_bit(TCCR1A,COM1A1);     //ativa o PWM no OC1A, modo de comparação não-invertido
                    //para desabilitar empregar clr_bit(TCCR1A, COM1A1)
      //Pulso de 2 ms em OC1A
      //OCR1A = 4000;   //regra de três para determinar este valor: ICR1(TOP) = 20 ms, OCR1A (4000) = 2 ms)
        OCR1A = 1096;   //valor ajustado, OCR1A (1096) = 0,548 ms - valor mínimo  0 grau para servo motor Tower Pro não vibrar
        _delay_ms(1000);  

}

int valor_pedido;
long lido;

 void loop(){
  char leitura = PIND & (1<<PD4);
  char leitura2 = PIND & (1<<PD5);
  char leitura3 = PIND & (1<<PD6);
  char leitura4 = PIND & (1<<PD7);
  
  if (leitura!=last_state&& (millis()-clique)>1) {
    clique = millis();
    if(leitura == 0){
    Serial.println("botao a");
    lido = scale.read();
    Serial.println(lido);
    lido = scale.read_average(20);
    Serial.println("Tens 3 segundos para colocar o controle");
    delay(3000);
    long lido2 = scale.read_average(20);
    Serial.println(lido2);
    scale.set_scale(((lido2-lido)/peso));
    Serial.println("Valor da Calibração");
    Serial.println((lido2-lido)/peso);
    }
  }

  if (leitura2!=last_state2 && (millis()-clique)>1) {
    clique = millis();
    
    if(leitura2 == 0){
    Serial.println("botao b");
      //scale.tare();    
    }
  }

  if (leitura3!=last_state3 && (millis()-clique)>1) {
    clique = millis();
    if(leitura3 == 0){
      qtd_dispenser = 0;
      for(int i = 0; i < 3; i++ ){ 
      float peso_inicial = scale.get_units();
          OCR1A = 3700;   //valor ajustado, OCR1A (3700) = 1,850 ms - valor mínimo  90 graus para servo motor Tower Pro/Simulado
          _delay_ms(3000);  
          OCR1A = 1096;   //valor ajustado, OCR1A (1096) = 0,548 ms - valor mínimo  0 grau para servo motor Tower Pro não vibrar
          _delay_ms(1000); 
      float peso_final = scale.get_units();  
      qtd_dispenser = (peso_inicial - peso_final) + qtd_dispenser;
      }
    qtd_dispenser = qtd_dispenser/3;
    Serial.println(qtd_dispenser);
    }
  }

    if (leitura4!=last_state4 && (millis()-clique)>1) {
    clique = millis();
      if(leitura4 == 0){
        Serial.println("Digite o valor desejado em gramas!");
        while(Serial.available() == 0){
          valor_pedido = Serial.parseInt();
        }      
        Serial.println(valor_pedido);    
        for(int j = 0; j < valor_pedido; j+= qtd_dispenser){
          OCR1A = 3700;   //valor ajustado, OCR1A (3700) = 1,850 ms - valor mínimo  90 graus para servo motor Tower Pro/Simulado
            _delay_ms(3000);  
          OCR1A = 1096;   //valor ajustado, OCR1A (1096) = 0,548 ms - valor mínimo  0 grau para servo motor Tower Pro não vibrar
            _delay_ms(1000);
        }   
      }
    }
  last_state = leitura;
  last_state2 = leitura2;
  last_state3 = leitura3;
  last_state4 = leitura4;
}

