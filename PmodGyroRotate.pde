/**
 * Read Serial port data, convert raw gyroscope data into angular displacement, and rotate an object along its
 * X, Y and Z axes accordingly
 * 
 */

import processing.serial.*;

// The serial port:
Serial myPort;

PImage img;
PShape globe;

int lf = 10;      // ASCII linefeed

String strx1;
String stry1;
String strz1;
String matchStringX = "X = ";
String matchStringY = "Y = ";
String matchStringZ = "Z = ";
String[] mx;
String[] my;
String[] mz;

// Variables for calibrating zero-rate level for X, Y and Z axes
int countX = 0;
int countY = 0;
int countZ = 0;
float sumX = 0.0;
float sumY = 0.0;
float sumZ = 0.0;

int rawX = 0;
int rawY = 0;
int rawZ = 0;

// Zero-rate level determined after collecting around 100 samples after power-up
float driftX = 37.9844;  
float driftY = -55.9115;
float driftZ = -22.7802;

float fx = 0.0;
float fy = 0.0;
float fz = 0.0;

float angDisplaceX = 0.0;
float angDisplaceY = 0.0;
float angDisplaceZ = 0.0;

float h = 0.0025;  // Sampling period for 400 Hz ODR

PShape rocket;

public void setup() {
  // Open the port you are using at the rate you want:
  myPort = new Serial(this, Serial.list()[1], 115200);
  size(1280, 1000, P3D);
  background(0);
  img = loadImage("earth.jpg");
  globe = createShape(SPHERE, 400);
  globe.setTexture(img);
  globe.setStroke(false);
}

public void draw() {
  
  background(0);
  lights();

  translate(width/2, height/2, 0);
  rotateZ(radians(angDisplaceZ));
  rotateY(radians(angDisplaceX));
  rotateX(radians(-angDisplaceY));
  //shape(rocket);
  shape(globe);
  
  if (myPort.available() > 0) 
  {  // if data is available
    serialEvent(myPort);
  }
}

void serialEvent(Serial myPort) 
{
    String inBuffer = myPort.readStringUntil(lf);  // Input string from serial port
    if (inBuffer != null) 
    {
      mx = match(inBuffer, matchStringX);
      my = match(inBuffer, matchStringY);
      mz = match(inBuffer, matchStringZ);
      
      // X axis
      if (mx != null) 
      {
        if (inBuffer.length() == 14) 
        {
          strx1 = inBuffer.substring(5, 13);
          rawX = unhex(strx1);
          if (rawX > 32768) 
          {
            rawX = - (65536 - rawX);
          }
          
          // Angular displacement calculation
          fx = (rawX - driftX)* 0.00875;
          angDisplaceX = angDisplaceX + h*fx;
          
          /*
          // Calibration
          countX = countX + 1;
          sumX = sumX + rawX;
          driftX = sumX / countX;
          println("Drift X = " + driftX);
          */
        }
      }
      
      // Y axis
      if (my != null) 
      {
        if (inBuffer.length() == 14) 
        {
          stry1 = inBuffer.substring(5, 13);
          rawY = unhex(stry1);
          if (rawY > 32768) 
          {
            rawY = - (65536 - rawY);
          }
          
          // Angular displacement calculation
          fy = (rawY - driftY)* 0.00875;
          angDisplaceY = angDisplaceY + h*fy;
          
          /*
          // Calibration
          countY = countY + 1;
          sumY = sumY + rawY;
          driftY = sumY / countY;
          println("Drift Y = " + driftY);
          */
        }
      }

      // Z axis
      if (mz != null) 
      {
        if (inBuffer.length() == 14) 
        {
          strz1 = inBuffer.substring(5, 13);
          rawZ = unhex(strz1);
          if (rawZ > 32768) 
          {
            rawZ = - (65536 - rawZ);
          }
          // Angular displacement calculation
          fz = (rawZ - driftZ)* 0.00875;
          angDisplaceZ = angDisplaceZ + h*fz;
          
          /*
          // Calibration
          countZ = countZ + 1;
          sumZ = sumZ + rawZ;
          driftZ = sumZ / countZ;
          println("Drift Z = " + driftZ);
          */
        }
      }

    }
  
}
