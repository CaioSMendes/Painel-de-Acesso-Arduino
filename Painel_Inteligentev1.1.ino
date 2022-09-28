//Bibliotecas Atualizado em 24-09-22
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <Key.h>
#include <Keypad.h>
#include <Keypad_I2C.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>




#define SS_PIN A1       //pino SDA do leitor
#define RST_PIN A3      //pino RST do leitor
#define I2CADDR 0x20    // Endereço IC2 PCF8574 Keypad
#define endereco  0x27  // Endereços IC2 LCD
#define colunas   16    // Colunas LCD
#define linhas    2     // Linhas LCD
#define SERVO A2        // Servo Motor
#define SIM800L_SOFTWARESERIAL_PIN_TX  10
#define SIM800L_SOFTWARESERIAL_PIN_RX 3

const byte ROWS = 4; // Set the number of Rows
const byte COLS = 4; // Set the number of Columns
const int PinoServo = 2; 
String IDtag = ""; //Variável que armazenará o ID da Tag
bool Permitido = false; //Variável que verifica a permissão 
String TagsCadastradas[] = {"b77a1a4f", "ID_2", "ID_3"};//Vetor responsável por armazenar os ID's das Tag's cadastradas

Servo ServoMotorSG90;                // Cria objeto Servo
int posicao = 0;                  // Variável posição Servo
char teclaRecebida;

// Set the Key at Use (4x4)
char keys [ROWS] [COLS] = {
  {'1', '4', '7', '*'},
  {'2', '5', '8', '0'},
  {'3', '6', '9', '#'},
  {'A', 'B', 'C', 'D'}
};

//Senha keypad
String senha="9657";
String digitada;
int estado=0;

// define active Pin (4x4)
byte rowPins [ROWS] = {0, 1, 2, 3}; // Connect to Keyboard Row Pin
byte colPins [COLS] = {4, 5, 6, 7}; // Connect to Pin column of keypad.

////////////////////////////////////////
char keypressed;                 //Where the keys are stored it changes very often
char code[]= {'9','6','5','7'};  //The default code, you can change it or make it a 'n' digits one

char senhaSalva[sizeof(code)];
char code_buff1[sizeof(code)];  //Where the new key is stored
char code_buff2[sizeof(code)];  //Where the new key is stored again so it's compared to the previous one

short a=0,i=0,s=0,j=0;          //Variables used later
////////////////////////////////////////
Servo sg90;
SoftwareSerial nodemcu(7, 6); //Initialise Arduino to NodeMCU (7=Rx & 6=Tx)
Keypad_I2C keypad (makeKeymap (keys), rowPins, colPins, ROWS, COLS, I2CADDR, PCF8574);
LiquidCrystal_I2C lcd(endereco, colunas, linhas); // INSTANCIANDO OBJETOS                            
MFRC522 LeitorRFID(SS_PIN, RST_PIN);    // Cria uma nova instância para o leitor e passa os pinos como parâmetro

void rfid_func();

void setup () {
  Wire .begin (); // Call the connection Wire
  keypad.begin (makeKeymap (keys)); // Call the connection
  SPI.begin();                               //inicializa bus SPI
  LeitorRFID.PCD_Init();                        //inicializa leitor RFID
  Serial.begin (9600);
  nodemcu.begin(9600);
  //espSerial.begin(115200); // ARRUMAR AQUI
  lcd.init();         // Inicia a Comunicação no Display
  lcd.backlight();    // Liga a Iluminação no Display
  lcd.print("Brasilia RFID");  
  sg90.attach(13);
  sg90.write(0);
  //Imprime mensagem inicial
  Serial.println("Trava Eletronica RFID WR Kits");
  Serial.println("v. 1.0.0           jan   2019");
  Serial.println("Aproxime as tags do leitor...");
  Serial.println();
}

void loop () {
   Teclado();
   //rfid_func();
}

void Teclado(){
  keypressed = keypad.getKey();               //Constantly waiting for a key to be pressed
    if(keypressed){
     //Serial.print("\nTecla: ");
     //Serial.print(keypressed);
      if(keypressed == '*'){                      // * to open the lock
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("Digite a senha");            //Message to show
              GetCode();                          //Getting code function
              if(a==sizeof(code)){           //The GetCode function assign a value to a (it's correct when it has the size of the code array) 
                 lcd.print("Acesso Autorizado");          //Message to print when the code is wrong
                 //////////////////// - Manda para Node MCU
                 nodemcu.println("Enviado para o Node MCU:\n");
                  for (int i = 0; i <= 4; i++) {
                    Serial.print("\nEnviando para o Node MCU: ");
                    Serial.print(senhaSalva[i]);
                    nodemcu.println(senhaSalva[i]); // TIRAR DPS
                    delay(500);
                  }
                  nodemcu.println("Vetor Enviado:");
                  nodemcu.println(senhaSalva); // TIRAR DPS
                  ////////////////////  
                 sg90.write(90); // Door Opened
                 delay(3000);
                 sg90.write(0);
              }else{
                    lcd.clear();               
                    lcd.print("Acesso Negado");          //Message to print when the code is wrong
                    }
              delay(2000);
              lcd.clear();
              lcd.print("Digite a senha");
              lcd.print("Novamente"); 
        //Return to standby mode it's the message do display when waiting
          }

       if(keypressed == '#'){                  //To change the code it calls the changecode function
        ChangeCode();
        lcd.clear();
        lcd.print("Standby3");                 //When done it returns to standby mode
       }

       //if(digitalRead(O_Button)==HIGH){      //Opening by the push button
       //digitalWrite(Solenoid,HIGH);
       //delay(3000);                        //Opens for 3s you can change
       //digitalWrite(Solenoid,LOW);
      }
}

void GetCode(){                  //Getting code sequence
       i=0;                      //All variables set to 0
       a=0;
       j=0; 
                                     
     while(keypressed != 'A'){                                     //The user press A to confirm the code otherwise he can keep typing
           keypressed = keypad.getKey();                         
             if(keypressed != NO_KEY && keypressed != 'A' ){       //If the char typed isn't A and neither "nothing"
              lcd.setCursor(j,1);                                  //This to write "*" on the LCD whenever a key is pressed it's position is controlled by j
              //Serial.print("Tecla Pressionada:\n");
              Serial.print(keypressed);
              lcd.print("*");
              j++;
            if(keypressed == code[i]&& i<sizeof(code)){            //if the char typed is correct a and i increments to verify the next caracter
              Serial.print("\nSalva senha no Vetor");
              senhaSalva[i] = code[i];
              a++;                                              //Now I think maybe I should have use only a or i ... too lazy to test it -_-'
              i++;
            }
            else
            a--;                                               //if the character typed is wrong a decrements and cannot equal the size of code []
            }
      }
    keypressed = NO_KEY;
}

void ChangeCode(){                      //Change code sequence
      lcd.clear();
      lcd.print("Changing code");
      delay(1000);
      lcd.clear();
      lcd.print("Enter old code");
      GetCode();                      //verify the old code first so you can change it
      
            if(a==sizeof(code)){      //again verifying the a value
            lcd.clear();
            lcd.print("Changing code");
            GetNewCode1();            //Get the new code
            GetNewCode2();            //Get the new code again to confirm it
            s=0;
              for(i=0 ; i<sizeof(code) ; i++){     //Compare codes in array 1 and array 2 from two previous functions
              if(code_buff1[i]==code_buff2[i])
              s++;                                //again this how we verifiy, increment s whenever codes are matching
              }
                  if(s==sizeof(code)){            //Correct is always the size of the array
                  
                   for(i=0 ; i<sizeof(code) ; i++){
                  code[i]=code_buff2[i];         //the code array now receives the new code
                  EEPROM.put(i, code[i]);        //And stores it in the EEPROM
                  
                  }
                  lcd.clear();
                  lcd.print("Code Changed");
                  delay(2000);
                  }
                  else{                         //In case the new codes aren't matching
                  lcd.clear();
                  lcd.print("Codes are not");
                  lcd.setCursor(0,1);
                  lcd.print("matching !!");
                  delay(2000);
                  }
            
          }
          
          else{                     //In case the old code is wrong you can't change it
          lcd.clear();
          lcd.print("Wrong");
          delay(2000);
          }
}

void GetNewCode1(){                      
  i=0;
  j=0;
  lcd.clear();
  lcd.print("Enter new code");   //tell the user to enter the new code and press A
  lcd.setCursor(0,1);
  lcd.print("and press A");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("and press A");     //Press A keep showing while the top row print ***
             
         while(keypressed != 'A'){            //A to confirm and quits the loop
             keypressed = keypad.getKey();
               if(keypressed != NO_KEY && keypressed != 'A' ){
                lcd.setCursor(j,0);
                lcd.print("*");               //On the new code you can show * as I did or change it to keypressed to show the keys
                code_buff1[i]=keypressed;     //Store caracters in the array
                i++;
                j++;                    
                }
                }
keypressed = NO_KEY;
}

void GetNewCode2(){                         //This is exactly like the GetNewCode1 function but this time the code is stored in another array
  i=0;
  j=0;
  
  lcd.clear();
  lcd.print("Confirm code");
  lcd.setCursor(0,1);
  lcd.print("and press A");
  delay(3000);
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("and press A");

         while(keypressed != 'A'){
             keypressed = keypad.getKey();
               if(keypressed != NO_KEY && keypressed != 'A' ){
                lcd.setCursor(j,0);
                lcd.print("*");
                code_buff2[i]=keypressed;
                i++;
                j++;                    
                }
                }
keypressed = NO_KEY;
}

void S90g(){
  for(posicao = 0; posicao < 90; posicao++){
    ServoMotorSG90.write(posicao);
    delay(15);
  }
  delay(1000);
  for(posicao = 90; posicao >= 0; posicao--){
    ServoMotorSG90.write(posicao);
    delay(15);
  }
}

void rfid_func(){                         //Função para identificação das tags RFID
      // -- Verifica novas tags --
      if(!LeitorRFID.PICC_IsNewCardPresent()) return;
      // Seleciona uma das tags
      if(!LeitorRFID.PICC_ReadCardSerial())   return;
      // Mostra UID na serial
      Serial.println("Aproxime as tags do leitor...");
      Serial.print("UID da tag :");
      String conteudo= "";
      byte letra;
      for(byte i = 0; i < LeitorRFID.uid.size; i++) {
         Serial.print(LeitorRFID.uid.uidByte[i] < 0x10 ? " 0" : " ");
         Serial.print(LeitorRFID.uid.uidByte[i], HEX);
         conteudo.concat(String(LeitorRFID.uid.uidByte[i] < 0x10 ? " 0" : " "));
         conteudo.concat(String(LeitorRFID.uid.uidByte[i], HEX));
      }
      Serial.println();
      Serial.print("Mensagem : ");
      conteudo.toUpperCase();
      //tags para liberar acesso
      if (conteudo.substring(1) == "B7 7A 1A 4F" |
          conteudo.substring(1) == "F9 4C DF 39" |
          conteudo.substring(1) == "26 3E B3 2D"){ //b7 7a 1a 4f ou B7 7A 1A 4F
        Serial.println("Acesso liberado!");
        Serial.println();  
        delay(5000);
        //Libera trava
        //digitalWrite(lock,   HIGH);
        //delay(2555);
       // digitalWrite(lock,    LOW);  
      }else{
        Serial.println("Acesso Negado");
        Serial.println(); 
      }
}