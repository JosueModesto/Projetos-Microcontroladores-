#define set_bit(y,bit)  (y|=(1<<bit)) //coloca em 1 o bit x da variável Y
#define clr_bit(y,bit)  (y&=~(1<<bit))  //coloca em 0 o bit x da variável Y
#define cpl_bit(y,bit)  (y^=(1<<bit)) //troca o estado lógico do bit x da variável Y
#define tst_bit(y,bit)  (y&(1<<bit))  //retorna 0 ou 1 conforme leitura do bit


#define DADOS_LCD      PORTD   //4 bits de dados do LCD no PORTD 
#define nibble_dados  1   //0 para via de dados do LCD nos 4 LSBs do PORT empregado (Px0-D4, Px1-D5, Px2-D6, Px3-D7) 
                //1 para via de dados do LCD nos 4 MSBs do PORT empregado (Px4-D4, Px5-D5, Px6-D6, Px7-D7) 
#define CONTR_LCD     PORTB   //PORT com os pinos de controle do LCD (pino R/W em 0).
#define E         PB1    //pino de habilitação do LCD (enable)
#define RS        PB0     //pino para informar se o dado é uma instrução ou caractere

// sinal de habilitação para o LCD
#define pulso_enable()  _delay_us(1); set_bit(CONTR_LCD,E); _delay_us(1); clr_bit(CONTR_LCD,E); _delay_us(45)

// protótipo das funções
void cmd_LCD(unsigned char c, char cd);
void inic_LCD_4bits();    

//---------------------------------------------------------------------------------------------
// Sub-rotina para enviar caracteres e comandos ao LCD com via de dados de 4 bits
//---------------------------------------------------------------------------------------------
void cmd_LCD(unsigned char c, char cd)        //c é o dado  e cd indica se é instrução ou caractere
{
  if(cd==0)
    clr_bit(CONTR_LCD,RS);
  else
    set_bit(CONTR_LCD,RS);

  //primeiro nibble de dados - 4 MSB
  #if (nibble_dados)                //compila código para os pinos de dados do LCD nos 4 MSB do PORT
    DADOS_LCD = (DADOS_LCD & 0b00001111) | (0b11110000 & c);
  #else                     //compila código para os pinos de dados do LCD nos 4 LSB do PORT
    DADOS_LCD = (DADOS_LCD & 0xF0)|(c>>4);  
  #endif
  
  pulso_enable();

  //segundo nibble de dados - 4 LSB
  #if (nibble_dados)                //compila código para os pinos de dados do LCD nos 4 MSB do PORT 
    DADOS_LCD = (DADOS_LCD & 0b00001111) | (0b11110000 & (c<<4));
  #else                     //compila código para os pinos de dados do LCD nos 4 LSB do PORT
    DADOS_LCD = (DADOS_LCD & 0xF0) | (0x0F & c);
  #endif
  
  pulso_enable();
  
  if((cd==0) && (c<4))        //se for instrução de retorno ou limpeza espera LCD estar pronto
    _delay_ms(2);
}
//---------------------------------------------------------------------------------------------
//Sub-rotina para inicialização do LCD com via de dados de 4 bits
//---------------------------------------------------------------------------------------------

void inic_LCD_4bits()   //sequência ditada pelo fabricando do circuito integrado HD44780
{             //o LCD será só escrito. Então, R/W é sempre zero.

  clr_bit(CONTR_LCD,RS);  //RS em zero indicando que o dado para o LCD será uma instrução 
  clr_bit(CONTR_LCD,E); //pino de habilitação em zero
  
  _delay_ms(20);      //tempo para estabilizar a tensão do LCD, após VCC ultrapassar 4.5 V (na prática pode
              //ser maior). 
  
  cmd_LCD(0x30,0);
              
  pulso_enable();     //habilitação respeitando os tempos de resposta do LCD
  _delay_ms(5);   
  pulso_enable();
  _delay_us(200);
  pulso_enable(); /*até aqui ainda é uma interface de 8 bits.
          Muitos programadores desprezam os comandos acima, respeitando apenas o tempo de
          estabilização da tensão (geralmente funciona). Se o LCD não for inicializado primeiro no 
          modo de 8 bits, haverá problemas se o microcontrolador for inicializado e o display já o tiver sido.*/
  
  //interface de 4 bits, deve ser enviado duas vezes (a outra está abaixo)
  cmd_LCD(0x20,0);
  
  pulso_enable();   
    cmd_LCD(0x28,0);    //interface de 4 bits 2 linhas (aqui se habilita as 2 linhas) 
              //são enviados os 2 nibbles (0x2 e 0x8)
    cmd_LCD(0x08,0);    //desliga o display
    cmd_LCD(0x01,0);    //limpa todo o display
    cmd_LCD(0x0C,0);    //mensagem aparente cursor inativo não piscando   
    cmd_LCD(0x80,0);    //inicializa cursor na primeira posição a esquerda - 1a linha
	
}

int estado = 0;
unsigned long clique = 0;
unsigned long clique2 = 0;
unsigned long clique3 = 0;

unsigned long lastupdate = 0;
unsigned long lastupdate2 = 0;
unsigned long lastupdate3 = 0;

unsigned long lastLCDRefresh = 0; 

char last_state = (1<<PB2);
char last_state2 = (1<<PB3);
char last_state3 = (1<<PB4);

int d0 = 0;
int d1 = 0;
int d2 = 0;
int d3 = 0;
int d4 = 0;
int d5 = 0;

char saida;

int tempo_sova = 5;
int tempo_crescimento = 5;
int tempo_assadura = 5;
int temperatura = 60;

char buffer[16];

int cont = 0;

unsigned char valor = 0;	//declara variável local

void setup()
{
  //TODO: o que fazer para inicializar corretamento o LCD?
  
  // lcd output
  DDRB |= 0b00000011;
  DDRD |= 0b11111100;
  DDRC |= 0b00111000;

  
  
  PORTC |= (1<<PC3);
  PORTC |= (1<<PC4); 
  PORTC |= (1<<PC5); 
   
  inic_LCD_4bits();
  
  // TODO: enable external interrups
  UCSR0B = 0x00;
}

void loop(){
  char leitura = PINC & (1<<PC3);
  char leitura2 = PINC & (1<<PC4);
  char leitura3 = PINC & (1<<PC5);


  if (leitura!=last_state && (millis()-clique)>1) {
    clique = millis();

    if (leitura == 0) {
      estado == 8?estado = 0:estado++;
    }
  }
  
    
  if (leitura2!=last_state2 && (millis()-clique2)>1) {
    clique2 = millis();

    if (leitura2 == 0) {
     if(estado == 1){
        if(tempo_sova > 0){
        	tempo_sova--;	
        }
      }
      else if(estado == 2){
      	if(tempo_crescimento > 0){
        	tempo_crescimento--;	
        }
      }
      else if(estado == 3){
      	if(tempo_assadura > 0){
        	tempo_assadura--;	
        }
      }
      else if(estado == 4){
      	if(temperatura > 60){
        	temperatura--;	
        }
      }
    }
  }

  
  if (leitura3!=last_state3 && (millis()-clique3)>1) {
    clique3 = millis();

    if (leitura3 == 0) {
      if(estado == 1){
        if(tempo_sova < 60){
        	tempo_sova++;	
        }
      }
      else if(estado == 2){
      	if(tempo_crescimento < 100){
        	tempo_crescimento++;	
        }
      }
      else if(estado == 3){
      	if(tempo_assadura < 120){
        	tempo_assadura++;	
        }
      }
      else if(estado == 4){
      	if(temperatura < 150){
        	temperatura++;	
        }
      }
    }
  }
  
   if(estado == 0){//tela inicial
       if (millis()>(lastLCDRefresh+100)) {
          lastLCDRefresh = millis();
          cmd_LCD(0x80, 0);

          sprintf(buffer, "   Bem Vindo");
          for(int i = 0; i < 12; i++){
          cmd_LCD(buffer[i],1); 
          }
          cmd_LCD(0xc0, 0);
          sprintf(buffer, "a Panificadora");
          for(int i = 0; i < 14; i++){
          cmd_LCD(buffer[i],1); 
          }
      }
  	}
      
    else if(estado == 1){//tela de definição do tempo de sova
          if (millis()>(lastLCDRefresh+100)) {
            lastLCDRefresh = millis();
            cmd_LCD(0x80, 0);

            sprintf(buffer, "      Sova   ");
            for(int i = 0; i < 12; i++){
            cmd_LCD(buffer[i],1); 
            }
            d0 = tempo_sova% 60;
            d1 = tempo_sova/60;
            cmd_LCD(0xc0, 0);
            sprintf(buffer, "  Tempo: %02d:%02d", d1,d0);
            for(int i = 0; i < 14; i++){
            cmd_LCD(buffer[i],1); 
            }
      	  }
      }
    else if(estado == 2){//tela de definição do tempo de crescimento
          if (millis()>(lastLCDRefresh+100)) {
            lastLCDRefresh = millis();
            cmd_LCD(0x80, 0);

            sprintf(buffer, "  Crescimento");
            for(int i = 0; i < 13; i++){
            cmd_LCD(buffer[i],1); 
            }
            d2 = tempo_crescimento% 60;
            d3 = tempo_crescimento/60;
            cmd_LCD(0xc0, 0);
            sprintf(buffer, "  Tempo: %02d:%02d", d3,d2);
            for(int i = 0; i < 14; i++){
            cmd_LCD(buffer[i],1); 
            }
      	  }		
      }
  	else if(estado == 3){//tela de definição do tempo de assadura
          if (millis()>(lastLCDRefresh+100)) {
            lastLCDRefresh = millis();
            cmd_LCD(0x80, 0);

            sprintf(buffer, "    Assadura  ");
            for(int i = 0; i < 13; i++){
            cmd_LCD(buffer[i],1); 
            }
            d4 = tempo_assadura% 60;
            d5 = tempo_assadura/60;
            cmd_LCD(0xc0, 0);
            sprintf(buffer, "  Tempo: %02d:%02d", d5,d4);
            for(int i = 0; i < 14; i++){
            cmd_LCD(buffer[i],1); 
            }
      	  }
      }	
  	else if(estado == 4){//tela de definição do temperatura da assadura
          if (millis()>(lastLCDRefresh+100)) {
            lastLCDRefresh = millis();
            cmd_LCD(0x80, 0);

            sprintf(buffer, "    Assadura  ");
            for(int i = 0; i < 13; i++){
            cmd_LCD(buffer[i],1); 
            }
 
            cmd_LCD(0xc0, 0);
            sprintf(buffer, " Temp: %02d graus",temperatura);
            for(int i = 0; i < 15; i++){
            cmd_LCD(buffer[i],1); 
            }
      	  }
      }
     else if(estado == 5){//tela sovando 
          if(tempo_sova == 0)
            	estado++;
       // else if (millis()>=lastLCDRefresh+(1000*60)) {      
          else if (millis()>(lastLCDRefresh+1000)) {
            PORTD |= (1<<PD3);//liga o motor
            lastLCDRefresh = millis();
            
            cmd_LCD(0x80, 0);

            sprintf(buffer, "     Sovando  ");
            for(int i = 0; i < 13; i++){
             cmd_LCD(buffer[i],1); 
            }
            d0 = tempo_sova% 60;
            d1 = tempo_sova/60;
            cmd_LCD(0xc0, 0);
            sprintf(buffer, "      %02d:%02d    ",d1,d0);
            for(int i = 0; i < 15; i++){
             cmd_LCD(buffer[i],1); 
            }
            tempo_sova--;
      	  }
      }
  
      else if(estado == 6){//tela crescendo 
        if(tempo_crescimento == 0){
           estado = 7;
        }
      //else if (millis()>=lastLCDRefresh+(1000*60)) {
        else if (millis()>(lastLCDRefresh+1000)) {
          PORTD &= ~(1<<PD3);//desliga o motor
            lastLCDRefresh = millis();
            
            cmd_LCD(0x80, 0);

            sprintf(buffer, "    Crescendo  ");
            for(int i = 0; i < 13; i++){
            cmd_LCD(buffer[i],1); 
            }
            d0 = tempo_crescimento% 60;
            d1 = tempo_crescimento/60; 
            cmd_LCD(0xc0, 0);
            sprintf(buffer, "      %02d:%02d    ",d1,d0);
            for(int i = 0; i < 15; i++){
            cmd_LCD(buffer[i],1); 
      	  	}
            tempo_crescimento--;
        }
      }
  
      else if(estado == 7){//tela assando 
        if(tempo_assadura == 0){
          estado++;
        }
       // else if (millis()>=lastLCDRefresh+(1000*60)) { descrementa o tempo 1 minuto
        else if (millis()>=lastLCDRefresh+1000) {//decrementa o tempo 1 segundo
       		lastLCDRefresh = millis();
            PORTD |= (1<<PD2);//liga a resistência
            cmd_LCD(0x80, 0);

            sprintf(buffer, "     Assando  ");
            for(int i = 0; i < 13; i++){
            	cmd_LCD(buffer[i],1); 
            }
 			
            d0 = tempo_assadura% 60;
            d1 = tempo_assadura/60;
            cmd_LCD(0xc0, 0);
            sprintf(buffer, "      %02d:%02d    ",d1,d0);
            for(int i = 0; i < 15;  i++){
            	cmd_LCD(buffer[i],1); 
            }
            tempo_assadura--;
        }
       	 	
      }
      else if(estado == 8){//tela final
       	if (millis()>(lastLCDRefresh+100)) {
          PORTD &= ~(1<<PD2);//desliga o resistencia
          lastLCDRefresh = millis();
          cmd_LCD(0x80, 0);

          sprintf(buffer, "   Terminou!");
          for(int i = 0; i < 12; i++){
            cmd_LCD(buffer[i],1); 
          }
          cmd_LCD(0xc0, 0);
          sprintf(buffer, "  Retire o Pao ");
          for(int i = 0; i < 14; i++){
          	cmd_LCD(buffer[i],1); 
          }
      	}
      	//reseta as variáveis para o valor padrão
      	tempo_sova = 25;
      	tempo_crescimento = 90;
      	tempo_assadura = 40;
      	temperatura = 60;
  	 }
  last_state = leitura;
  last_state2 = leitura2;
  last_state3 = leitura3;

 

  _delay_ms(1);
}
