#include <SoftwareSerial.h>

const int lampada = 2;
const int rele = 3;
const int alarme = 4;
const int sirene = 5;

boolean alarmeAtivo= false;
boolean sireneAtiva = false;
 
void setup() 
{  
  Serial.begin(9600);  

  pinMode(lampada,OUTPUT);
  pinMode(rele,OUTPUT);
  pinMode(alarme,OUTPUT);
  pinMode(sirene,OUTPUT);
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
       if (sireneAtiva){
          disparaSirene();
       }
       digitoSenha = Serial.read();          
      if (digitoSenha == '8' || digitoSenha == '5' || digitoSenha == '#'){
        //  Serial.println(digitoSenha);
          senha[count] = digitoSenha;
          count ++;      
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
       if (sireneAtiva){
          disparaSirene();
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
          delay(1000);   
          countTime++;
          if (countTime > 60){
             senhaValida = false;
          }
       break;   
      }
 }
}
