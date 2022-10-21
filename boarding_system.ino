//for the LCD
#include <LiquidCrystal.h>
//LCD pin to Arduino
const int pin_RS = 8; 
const int pin_EN = 9; 
const int pin_d4 = 4; 
const int pin_d5 = 5; 
const int pin_d6 = 6; 
const int pin_d7 = 7; 
const int pin_BL = 10; 
LiquidCrystal lcd( pin_RS,  pin_EN,  pin_d4,  pin_d5,  pin_d6,  pin_d7);


//for the keypad
#include <Keypad.h>

const int ROW_NUM = 4; //four rows
const int COLUMN_NUM = 4; //three columns

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3', 'A'},
  {'4','5','6', 'B'},
  {'7','8','9', 'C'},
  {'*','0','#', 'D'}
};

byte pin_rows[ROW_NUM] = {25,24,23,22}; //connect to the row pinouts of the keypad
byte pin_column[COLUMN_NUM] = {29,28,27,26}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

const String password = "281"; // change your password here
String input_password;

//this is for the red led that flags high temperature
int red_led = 50; // define LED pin
int temp_input = A7; // define temperature signal pin
int val; //define a numeric variable

//for the motor (controlling the gate)
const int motorPin1  = 30;  // Pin 14 de L293
const int motorPin2  = 31;  // Pin 10 de L293
void gateOpen();
void gateClose();
int check_completion(int flight_n);
void write_database(int num);

//for the optocoupler input
int optocouplerpin = 52; // define temperature signal pin
int green_led = 53; //define led for the optocoupler

//for timing
unsigned long start_time;
unsigned long current_time;
const unsigned long period =5*60000;

//defining the database as arrays
bool flight1[70] = {0};
bool flight2[84] = {0};
bool flight3[66] = {0};
bool flight4[89] = {0};
bool flight5[72] = {0};
bool flight6[92] = {0};
bool flight7[98] = {0};


void setup() {

   //this is the setup needed for timing the flight 
   start_time = millis();
   Serial.begin(9600);
   
   
   //just for the password
   input_password.reserve(3); // maximum input characters is 4

   //this is for the LCD
   lcd.begin(16, 2);
   lcd.setCursor(0,0);

    //this is for the red led that flags high temperature
    pinMode(red_led, OUTPUT); // red LED pin as output
    pinMode(temp_input, INPUT); //photo interrupter pin as input

    //this code is for the motor (gate control)
    pinMode(motorPin1, OUTPUT);
    pinMode(motorPin2, OUTPUT);
    
    //for the optocoupler input
    pinMode(optocouplerpin, INPUT); //optocouplerpin pin as input
    pinMode(green_led, OUTPUT); // green LED pin as output

}

void loop() {

 //this part measures the time of the flight and exits if time is exceeded

  current_time = millis();
  int x = current_time - start_time;
  if(x>=period){
    gateClose();
    delay(2000);
    lcd.clear();
    lcd.print("flight time");
    lcd.setCursor(0,1);
    lcd.print("is over");
    Serial.print("\n");
    Serial.print("flight time is over");

    for(int i =1;i<=7;i++){
        if(check_completion(i)){
          lcd.print("flight ");
          lcd.print(i);
          lcd.print("\nboarding");
        }else{
          lcd.print("flight ");
          lcd.print(i);
          lcd.print("\nnot boarding");  
        }
        exit(1);
      }
  }

  
    //part I: letting passengers in
    //1. call the function passenger_passing 
    //2. if it returns 0 don't do anything 
    //3. if it return 1 we will do some checks
    digitalWrite(green_led,LOW);
    if(passenger_passing()==1){        
          //4. the checks will be calling the funciton temp_flag to know if the temperature is safe 
          if(temp_flag()==1){
              //5. if temp is safe we will call the funcion that opens the gate 
              gateOpen();
              
              //6. if it is not safe, we will not, and the red light will be on from that function (hajar made it )
              //7. while openning the gate, keep checking optocoupler using the function, and only when it returns 0
              while(optocoupler_flag()==1){
                  digitalWrite(green_led,LOW);
                  Serial.print("\n");
                  Serial.print(digitalRead(optocouplerpin));
                  delay(500);  
              }
                  digitalWrite(green_led,HIGH);
                  Serial.print("\n");
                  Serial.print(digitalRead(optocouplerpin));
              
              //8. close the gate using the function close_gate()
              gateClose();
              //9. flag the the passenger (or staff) as cheked in
              //this step is included already in passenger passing function in case it was a correct cod
          }
          }
    
    
    //part II: letting an airplane 
    //9. check if the pilot and copilot are both on the plane
    //10. check how many passengers are on board, if all are, mark completed
    //if (check_completion()==1){
        //11. print flight # is completed on LCD
      //  lcd.clear();
       // lcd.print("flight complete");
    //}

}

//these funcitons are to open and close the gate
void gateOpen(){
    digitalWrite(motorPin1, HIGH); // Al activar gira el motor A en sentido de las agujas del reloj. 
    digitalWrite(motorPin2, LOW); // Al activar gira el motor A en sentido contra las agujas del reloj. 
    delay(3000);
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW);
}

void gateClose(){
    digitalWrite(motorPin2, HIGH); // Al activar gira el motor B en sentido de las agujas del reloj. 
    digitalWrite(motorPin1, LOW); // Al activar gira el motor B en sentido contra las agujas del reloj. 
    delay(3000);
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW);
}

//funciton that makes the LCD ask the person for an input, take it, and decide if it's correct
int passenger_passing(){
  //it returns 0 when password is wrong and 1 if it's correct
  //it also gives 0 if the passenger checked in before
   lcd.print("Enter your code");
   delay(1000);
   lcd.clear(); 

  while(1){ 
      char key = keypad.getKey();
      if (key){
        Serial.println(key);
        if (key != '#' && key != 'D' ){
          lcd.write(key);
        }
        if(key == '#') {
          input_password = ""; // clear input password
    
          Serial.println("input is cleared");
          lcd.clear();
          lcd.print("input is cleared");
          delay(100); 
          lcd.clear();
        } 
        
        if(key == 'D') {
          //const char *c = input_password.c_str();
          if(!read_database(input_password.toInt())) {
            
            lcd.clear();
            Serial.println("allowed");
            lcd.setCursor(0,0);
            lcd.print("allowed");
            write_database(input_password.toInt());
            input_password = "";
            
            return 1;
            
          } else {
            lcd.clear();
            Serial.println("not allowed");
            delay(500);
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("not");
            lcd.setCursor(0,1);
            lcd.print("allowed");
            delay(500);
            lcd.clear();
            input_password = ""; // clear input password
            return 0;
          }
        } 
          
          if (key != '#' && key != 'D' ){
          input_password += key; // append new character to input password string
        }
      }
}
}
int temp_flag(){
  //this funciton returns 0 as long as the temperature is safe and 1 when it is not
  double celsius = 0;
  int fahrenheit = 0;
  celsius = map(((analogRead(temp_input))), 0, 1023, 20, 70);
    //fahrenheit = ((celsius * 9) / 5 + 32);
    Serial.print("\n temp : ");
    Serial.print(celsius);

  if (celsius > 30){
    digitalWrite(red_led,HIGH);
    lcd.clear();
    lcd.print("DANGER!");
    return 0;
  }else{ 
    digitalWrite(red_led,LOW);
    return 1;
    
  }
}

int optocoupler_flag(){
 //this funciton returns 0 if there was no one at the gate and 1 if there is someone passing 
 
  return digitalRead(optocouplerpin);
 }


int read_database(int num){
  //this function returns 1 when the person is on board or seat not available
  //this function returns 0 when seat is there and can board 


  int passenger_n = (num %100);
  int flight_n = (num - passenger_n)/100;
  
  switch(flight_n){
    case 1:
      if (sizeof(flight1)-1< passenger_n){
                return 1;
          }else{
              int state = flight1[passenger_n];
                return state;
          }
      break;
    case 2:
      if (sizeof(flight2)-1< passenger_n){
                return 1;
          }else{
              int state = flight2[passenger_n];
                return state;
          }
      break;
    case 3:
      if (sizeof(flight3)-1< passenger_n){
                return 1;
          }else{
              int state = flight3[passenger_n];
                return state;
          }
      break;
    case 4:
      if (sizeof(flight4)-1< passenger_n){
                return 1;
          }else{
              int state = flight4[passenger_n];
                return state;
          }
      break;
    case 5:
      if (sizeof(flight5)-1< passenger_n){
                return 1;
          }else{
              int state = flight5[passenger_n];
                return state;
          }
      break;
    case 6:
      if (sizeof(flight6)-1< passenger_n){
                return 1;
          }else{
              int state = flight6[passenger_n];
                return state;
          }
      break;
    case 7:
      if (sizeof(flight7)-1< passenger_n){
                return 1;
          }else{
              int state = flight7[passenger_n];
                return state;
          }
      break;
    default:
      return 1; 
  }
}


//this is the function that will access the database and flag the passenger who entered 
void write_database(int num){
  int passenger_n = (num %100);
  int flight_n = (num - passenger_n)/100;

    Serial.print("\nI am in write data base\n");
    Serial.print("\n num : ");
    Serial.print(num);
    Serial.print("\n passenger : ");
    Serial.print(passenger_n);
      if (flight_n == 1){
          flight1[passenger_n]= 1;
      }else if(flight_n == 2){
          flight2[passenger_n]= 1;
      }else if(flight_n == 3){
          flight3[passenger_n]= 1;
      }else if(flight_n == 4){
          flight4[passenger_n]= 1;
      }else if(flight_n == 5){
          flight5[passenger_n]= 1;
      }else if(flight_n == 6){
          flight6[passenger_n]= 1;
      }else if(flight_n == 7){
          flight7[passenger_n]= 1;
      }
}

//9. check if the pilot and copilot are both on the plane
//10. check how many passengers are on board, if all are, mark complete
//11. print flight # is completed


//this is a funciton that checks if the flight is complete and ready to take off
int check_completion(int flight_n){
  //check if the pilot and copilot is there
  // if they are there return 1 and if not return 0
  
  int pilot = 100*flight_n;
  int copilot = pilot+1;
  int x = read_database(pilot);
  int y = read_database(copilot);
  if (x&&y){
      return 1;
  }else{
      return 0;
  }  
}
