#include <TimerOne.h>
#include <Arduino_FreeRTOS.h>
#include <SoftwareSerial.h>
#include <semphr.h>

byte Chave = 52;
byte Flag = 0;
int x[150]; // Vetor para as leituras da porta analógica A0 (DTMF)

// Especifica o tipo
SemaphoreHandle_t mtxSerial;

// define duas tarefas
void TaskLeChave(void *pvParameters);
void TaskGoertzel(void *pvParameters);
void TaskAnalogRead(void *pvParameters);


const int lampada = 2;
const int rele = 3;
const int alarme = 4;
const int sirene = 5;
const int sensorTemperatura = 6;

boolean alarmeAtivo= false;
boolean sireneAtiva = false;
int estadoAlarme = 0;
boolean sinalTeclado = false;
 
void setup() 
{  
  Serial.begin(9600);  
  pinMode(lampada,OUTPUT);
  pinMode(rele,OUTPUT);
  pinMode(alarme,INPUT);
  pinMode(sensorTemperatura,INPUT);
  pinMode(sirene,OUTPUT);

   Serial.begin(9600);
  pinMode(Chave,INPUT_PULLUP);
  pinMode(LED_BUILTIN,OUTPUT);

  byte i;
  int Fator = 64; // Fator de multiplicação para minimizar os erros de arredondamento
  int Limite = 4096; // Limite de cada ponteiro da tabela = N*fator, com N = 64 (qtde de pontos da tabela
  // f1 e f2 são as frequências que irão compor o sinal DTMF
  int F1 = 394; // Deslocamento na tabela para f1 = 770 Hz -> F1 = round(N*770*Fator/8000), com N = 64 (qtde de pontos da tabela
  int F2 = 619; // Deslocamento na tabela para f2 = 1209 Hz -> F2 = round(N*1209*Fator/8000), com N = 64 (qtde de pontos da tabela
  int idx1 = 0; // Índice da tabela para F1
  int idx2 = 0; // Índice da tabela para F2
  // Tabela do seno: y=round(63*sin(x)) com step de pi/32 
  int y[64]; 
  

  // Gera vetor de entrada (sinal DTMF)
  for (i = 0; i < 150; i++) {
    x[i] = y[idx1/Fator] + y[idx2/Fator] + 128;
    idx1 = idx1 + F1;
    if (idx1 >= Limite) idx1 = idx1 - Limite;
    idx2 = idx2 + F2;
    if (idx2 >= Limite) idx2 = idx2 - Limite;
  }
 
  if (mtxSerial == NULL) {
    mtxSerial = xSemaphoreCreateMutex(); // Cria o semáforo
    if (mtxSerial != NULL) {
      xSemaphoreGive(mtxSerial); // Semáforo livre
    }
  }

  xTaskCreate(TaskLeChave,(const portCHAR *) "",128,NULL,2,NULL);
  xTaskCreate(TaskGoertzel,(const portCHAR *) "",256,NULL,1,NULL);
  xTaskCreate(TaskAnalogRead,(const portCHAR *) "",128,NULL,2,NULL);
}

void TaskLeChave(void *pvParameters) {
   (void) pvParameters;
  
   for (;;)
   {
       digitalWrite(LED_BUILTIN, LOW);
       if (digitalRead(Chave) == 0) {
          Flag = 1; 
          digitalWrite(LED_BUILTIN, HIGH);
       }
       vTaskDelay(200/portTICK_PERIOD_MS);
   }
}

void TaskGoertzel(void *pvParameters) {
   (void) pvParameters;
  
   // Coeficientes do Algoritmo de Goertzel (fa = 8000 Hz)
   byte c1 = 219; // 2*cos(2*pi*697/fa)*128
   byte c2 = 211; // 2*cos(2*pi*770/fa)*128
   byte c3 = 201; // 2*cos(2*pi*852/fa)*128
   byte c4 = 189; // 2*cos(2*pi*941/fa)*128
   byte c5 = 149; // 2*cos(2*pi*1209/fa)*128
   byte c6 = 128; // 2*cos(2*pi*1336/fa)*128
   byte c7 = 102; // 2*cos(2*pi*1477/fa)*128
   // Variáveis das iterações
   int s11; //v(i) p/ f1 = 697Hz
   int s12; //v(i-1) p/ f1 = 697Hz
   int s13; //v(i-2) p/ f1 = 697Hz
   int s21; //v(i) p/ f2 = 770Hz
   int s22; //v(i-1) p/ f2 = 770Hz
   int s23; //v(i-2) p/ f2 = 770Hz
   int s31; //v(i) p/ f3 = 852Hz
   int s32; //v(i-1) p/ f3 = 852Hz
   int s33; //v(i-2) p/ f3 = 852Hz
   int s41; //v(i) p/ f4 = 941Hz
   int s42; //v(i-1) p/ f4 = 941Hz
   int s43; //v(i-2) p/ f4 = 941Hz
   int s51; //v(i) p/ f5 = 1209Hz
   int s52; //v(i-1) p/ f5 = 1209Hz
   int s53; //v(i-2) p/ f5 = 1209Hz
   int s61; //v(i) p/ f6 = 1336Hz
   int s62; //v(i-1) p/ f6 = 1336Hz
   int s63; //v(i-2) p/ f6 = 1336Hz
   int s71; //v(i) p/ f7 = 1477Hz
   int s72; //v(i-1) p/ f7 = 1477Hz
   int s73; //v(i-2) p/ f7 = 1477Hz
  
   byte i;
   int valor;
   long k;
   long l;
   long m;
   for (;;)
   {
      // Aguarda chave ser acionada (sinal ser colocado em nível baixo)
      if (Flag == 1) {
        s12 = 0;
        s13 = 0;
        s22 = 0;
        s23 = 0;
        s32 = 0;
        s33 = 0;
        s42 = 0;
        s43 = 0;
        s52 = 0;
        s53 = 0;
        s62 = 0;
        s63 = 0;
        s72 = 0;
        s73 = 0;

        long tempo = micros();
        for (i = 0; i < 150; i = i + 1) {
          valor = x[i] - 128; // Leitura do seno e retirada da componente DC

          k = long(c1)*long(s12)/128;
          s11 = valor + int(k) - s13;
          s13 = s12;
          s12 = s11;
  
          k = long(c2)*long(s22)/128;
          s21 = valor + int(k) - s23;
          s23 = s22;
          s22 = s21;
 
          k = long(c3)*long(s32)/128;
          s31 = valor + int(k) - s33;
          s33 = s32;
          s32 = s31;

          k = long(c4)*long(s42)/128;
          s41 = valor + int(k) - s43;
          s43 = s42;
          s42 = s41;

          k = long(c5)*long(s52)/128;
          s51 = valor + int(k) - s53;
          s53 = s52;
          s52 = s51;

          k = long(c6)*long(s62)/128;
          s61 = valor + int(k) - s63;
          s63 = s62;
          s62 = s61;

          k = long(c7)*long(s72)/128;
          s71 = valor + int(k) - s73;
          s73 = s72;
          s72 = s71;
        }
        tempo = micros() - tempo;
    
        k = long(s12);
        l = long(s13);
        m = long(c1)*k/128;
        long p1 = k*k + l*l - m*l;
      
        k = long(s22);
        l = long(s23);
        m = long(c2)*k/128;
        long p2 = k*k + l*l - m*l;
    
        k = long(s32);
        l = long(s33);
        m = long(c3)*k/128;
        long p3 = k*k + l*l - m*l;
    
        k = long(s42);
        l = long(s43);
        m = long(c4)*k/128;
        long p4 = k*k + l*l - m*l;

        k = long(s52);
        l = long(s53);
        m = long(c5)*k/128;
        long p5 = k*k + l*l - m*l;

        k = long(s62);
        l = long(s63);
        m = long(c6)*k/128;
        long p6 = k*k + l*l - m*l;

        k = long(s72);
        l = long(s73);
        m = long(c7)*k/128;
        long p7 = k*k + l*l - m*l;

        if (xSemaphoreTake(mtxSerial,(TickType_t)5) == pdTRUE) {
            Serial.print("Tempo decorrido para as 150 iterações: ");
            Serial.println(tempo);
            Serial.println("Valores finais:");
            Serial.println(p1);
            Serial.println(p2);
            Serial.println(p3);
            Serial.println(p4);
            Serial.println(p5);
            Serial.println(p6);
            Serial.println(p7);
            xSemaphoreGive(mtxSerial); // Após utilizar, libera semáforo
        }

        Flag = 0;
      }
      vTaskDelay(200/portTICK_PERIOD_MS);
   }
}

void TaskAnalogRead(void *pvParameters) {
   (void) pvParameters;
  
   for (;;)
   {
       int sensorValue = analogRead(A1);
       if (xSemaphoreTake(mtxSerial,(TickType_t)5) == pdTRUE) {
          Serial.println(sensorValue);
          xSemaphoreGive(mtxSerial); // Após utilizar, libera semáforo
       }
       vTaskDelay(1000/portTICK_PERIOD_MS);
   }
}

boolean verificarSenhaCorreta(char senhaDigitada[2]){
 //  Serial.println(senhaDigitada);
   for(int count = 0; count <= 2; ++count){
     switch (count) {
       case 0:
         if (senhaDigitada[count] != '8'){         
            return false;
         }     
        break;
      case 1:
        if (senhaDigitada[count] != '5'){         
           return false;
        }  
        break;
      case 2:
        if (senhaDigitada[count] != '#'){
           return false;
        }else{
           return true;
        }   
        break;
      default:    
          return false;     
       break;   
      }  
  }
} 

void disparaSirene(){
    tone(sirene, 1100);   //Define a frequência em 1100Hz 
    delay(100);         
    tone(sirene, 1110);   //Define a frequência em 1100Hz 
    delay(100);
}
 
void loop()
{    
char senha [2] = "";
char digitoSenha = "";
int count = 0;
int countTime = 0;
boolean senhaValida = false;
char opcao = "";

Serial.println("Sistema iniciado/reiniciado por favor informe a senha!");

 while (!senhaValida){
       if (alarmeAtivo){
          estadoAlarme = digitalRead(alarme);
          if (estadoAlarme == HIGH && sireneAtiva){
          disparaSirene();
          }
       }
       digitoSenha = Serial.read();          
      if (digitoSenha == '8' || digitoSenha == '5' || digitoSenha == '#'){
          Serial.println(digitoSenha);
          senha[count] = digitoSenha;
          count ++;      
      }
      if (digitoSenha == '0' || digitoSenha == '1' || digitoSenha == '2' || digitoSenha == '3' || 
          digitoSenha == '4' || digitoSenha == '6' || digitoSenha == '7'|| digitoSenha == '9' || 
          digitoSenha == '*'){
         Serial.println(digitoSenha);
         count = 0;
      }
   if (count > 2){
       senhaValida = verificarSenhaCorreta(senha);
       if (senhaValida){
          Serial.println("Senha Correta! Informe Opção Desejada:");
        } 
       count = 0;
   } 
}

 while (senhaValida){
       if (alarmeAtivo){
          estadoAlarme = digitalRead(alarme);
          if (estadoAlarme == HIGH && sireneAtiva){
          disparaSirene();
          }
       }
  
     opcao = Serial.read();

     switch (opcao) {
       case '8':
         Serial.println("Ligar Lampada");
         digitalWrite(lampada,HIGH);
         countTime = 0;
        break;
      case '9':
        Serial.println("Desligar Lampada");
        digitalWrite(lampada,LOW);
        countTime = 0;
        break;
      case '0':
        Serial.println("Ligar Rele");
        digitalWrite(rele,HIGH);
        countTime = 0;
        break;
      case '1':
        Serial.println("Desligar Rele");
        digitalWrite(rele,LOW);
        countTime = 0;
        break;  
      case '2':
        Serial.println("Armar Alarme");
        digitalWrite(alarme,HIGH);
        alarmeAtivo = true;
        countTime = 0;
        break;  
      case '3':
        Serial.println("Desarmar Alarme");
        digitalWrite(alarme,LOW);
        alarmeAtivo = false;
        countTime = 0;
        break;      
      case '4':
        Serial.println("Ligar Sirene");
        sireneAtiva = true;
        countTime = 0;
        break;        
      case '5':
        Serial.println("Desligar Sirene");
        sireneAtiva = false;
        countTime = 0;
        break;          
      default:    
       // calcula tempo sem acao de 1 minuto para sair das opcoes           
          countTime++;
          if (countTime > 50){
             senhaValida = false;
          }
       break;   
      }
 }
}
