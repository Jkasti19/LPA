//processing_code.pde       run the arduino code first.
//This Code is Developed by Sdev
//Follow Us Here : https://youtube.com/sdevelectronics

//on 4-1-23 Processing takes over the serial port and I can no longer view it or connect on the arduino end of things...
import controlP5.*;                           //import ControlP5 library
import processing.serial.*;
Serial port;
ControlP5 cp5;                                   //create ControlP5 object

PFont font;
String date= "Date: "+month()+"-"+day()+"-"+year();
String time= "Time: "+hour()+":"+minute();
String timeui="Enter # of hours (1-9) to output your wavelength";

void setup() {

  printArray (Serial.list());  
  size(1000, 420);                                //window size, (width, height) og 480, 360
  port = new Serial(this, "/dev/cu.usbmodem141401", 9600);         // /dev/tty.usbmodem143301Change Your COM Port Here  perhaps /dev/cu.usbmodem 143401     [3] "/dev/cu.usbmodem143401" With the cu (calling unit) in this it actually hogs the port
  cp5 = new ControlP5(this);
  font = createFont("calibri light bold", 20);   //Custom Font
  
  cp5.addButton("green_525nm")                      //Name of the Button
    .setPosition(40, 150)                        //(x,y) top left Corner
    .setSize(200, 100)                            //(width, height)
    .setFont(font)
    .setColorBackground(color(0, 255, 0))
    ;
  cp5.addButton("blue_469nm")                       //Name of the Button
    .setPosition(280, 290)                        //(x,y) top left Corner
    .setSize(200, 100)                            //(width, height)
    .setFont(font)
    .setColorBackground(color(0, 0, 255))
    ;
  cp5.addButton("red_627nm")                        //Name of the Button
    .setPosition(280, 150)                         //(x,y) top left Corner
    .setSize(200, 100)                             //(width, height)
    .setFont(font)
    .setColorBackground(color(255, 0, 0))
    ;   
  cp5.addButton("UV_398nm")                     //Name of the Button
    .setPosition(520, 150)                       //(x,y) top left Corner
    .setSize(200, 100)                            //(width, height)
    .setFont(font)
    .setColorBackground(color(150,0, 200))
    ;
  cp5.addButton("Off")                      //Name of the Button
    .setPosition(40, 290)                       //(x,y) top left Corner
    .setSize(200, 100)                            //(width, height)
    .setFont(font)
    .setColorBackground(color(1, 1, 1))
    ;
  cp5.addButton("Test_Pattern")                        //Name of the Button
    .setPosition(760, 150)                         //(x,y) top left Corner
    .setSize(200, 100)                             //(width, height)
    .setFont(font)
    .setColorBackground(color(255, 0, 255))
    ;   
  cp5.addButton("Press_Me")                        //Name of the Button
    .setPosition(760, 290)                         //(x,y) top left Corner
    .setSize(200, 100)                             //(width, height)
    .setFont(font)
    .setColorBackground(color(0,200,100 ))
    ; 
  cp5.addButton("Smile")                        //Name of the Button
    .setPosition(520, 290)                         //(x,y) top left Corner
    .setSize(200, 100)                            //(width, height)
    .setFont(font)
    .setColorBackground(color(255,200,0 ))
    ; 
}

void draw() {

  background(50, 50, 50);                        //background color of window (r, g, b)
  //Title
  fill(255, 255, 255);                           //text color (r, g, b)
  textFont(font);
  text("Light Plate Apparatus", 400, 100);   //("text", x, y)
  text("Rylan Hanson, Joseph Kasti, & Kai Nellermoe", 300, 125);
  text(time, 40, 725);
  text(date, 40, 700);
  textSize(50);
  text("Welcome to the Design Expo 2023!!!",55,60);
  //textSize(30);
  //text(timeui,40,500);
  //text("Enter desired wavelength x (x00 nm)",40,550);

}
void keyPressed(){
//  if(key=='9'){
    timeui="You entered " +key+ " hours!";
//  }
}
void red_627nm() {
  port.write('R'); //sending a character over the serial port to initiate a change in the 
}

void green_525nm() {
  port.write('G');
}

void blue_469nm() {
  port.write('B');
}

void UV() {
  port.write('U');
}

void Test_Pattern(){
  port.write('t');
}

void Press_Me() {
  port.write('P'); 
}
void Smile() {
  port.write('S'); 
}
void Off() {
  port.write('o');
}


//This Code is Developed by Sdev
//Follow Us Here : https://youtube.com/sdevelectronics

//Corresponding ARDUINO code:
//void loop() {
//  if (Serial.available()) {
//    char val = Serial.read();
//    if (val == 'R') {
//      digitalWrite(led1, HIGH);   // we would probs write a  tlc.set_pixel_all_16bit_value(value_low, value_low, value_low);      tlc.show();
//    } etc.... 
