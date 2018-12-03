#include <SoftwareSerial.h>


boolean senhaAcesso = false;
boolean alarmeLigado = false;
int countSenha = 0;
boolean errouSenha = false;
boolean senhaCorreta = false;
char digitoSenha;
 
void setup() 
{  
  Serial.begin(9600);  
}

boolean verificarSenhaCorreta (char digitoSenha){
   Serial.println("entrou!");
  switch (countSenha) {
  case 0:
     if (digitoSenha != '8'){
         errouSenha = true;
     }     
    break;
  case 1:
    if (digitoSenha != '5'){
         errouSenha = true;
     }  
    break;
  case 2:
    if (digitoSenha != '#'){
         errouSenha = true;
     }else{
         senhaCorreta = true; 
     }   
    break;
 default:         
    break;   
}
countSenha ++;
  
}
 
void loop()
{    

if (Serial.available() > 0) {
    char digitoSenha = Serial.read();    
    Serial.println(digitoSenha);
    if (digitoSenha > 0){
       if (!errouSenha){
           verificarSenhaCorreta(digitoSenha);       
       }
       digitoSenha = 0;  
    }
}    
  
  



if (senhaCorreta){
    Serial.println("senha correta!");
}


}





            
