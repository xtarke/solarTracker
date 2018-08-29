# -*- coding: utf-8 -*-
# *
# * camera.py
# *
# *  Created on: May 29, 2018
# *      Author: Renan Augusto Starke
# *********************************
#
# sudo modprobe bcm2835-v4l2
# pip3 install paho-mqtt
# pip3 install opencv-python
# apt install python-opencv
# apt install libcblas3
# apt install libatlas3-base
# libjasper1/stable
# libgstreamer1.0
# libqtgui4
# libqt4-test
# python3-numpy


import cv2
# import numpy as np
# import paho.mqtt.client as mqtt
import time

<<<<<<< HEAD
broker_address="localhost" 
client = mqtt.Client("P1") #create new instance
=======
# broker_address="localhost" 
# client = mqtt.Client("P1") #create new instance
>>>>>>> be0ea9d1ddd58cec4a96c18003de0bb4ef3113fb
# client.connect(broker_address) #connect to broker

# Create a VideoCapture object and read from input file
# If the input is the camera, pass 0 instead of the video file name
cap = cv2.VideoCapture(2)
# 
## Check if camera opened successfully
if (cap.isOpened()== False): 
    print("Error opening video stream or file")
# 
## Read until video is completed
while(cap.isOpened()):
  # Capture frame-by-frame
  ret, frame = cap.read()
  if ret == True:
 
    # Display the resulting frame
    cv2.imshow('Frame',frame)
    
    # Save frame
    cv2.imwrite("Image.jpg", frame)    
    imageFile = open("Image.jpg", "rb")

    try:
        data = imageFile.read()        
        # client.publish("image",data)   #publish       
        
    finally:
        imageFile.close()
     
    
    # Press Q on keyboard to  exit
    if cv2.waitKey(25) & 0xFF == ord('q'):
      break
  
  #  time.sleep(0.5)
 
  # Break the loop
  else: 
    break
 
# When everything done, release the video capture object
cap.release()
 
# Closes all the frames
cv2.destroyAllWindows()







#client.publish("house/main-light","OFF")#publish
