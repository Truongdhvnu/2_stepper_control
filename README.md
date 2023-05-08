# 2_stepper_control
## Project's name and motivation
- This project is to familiarize myself with microcontrollers so i used arduino flatform
- This project uses two stepers to control exactly the locate of a pen so we can paint any thing that we had programed for uC
- Just feel interested in this product so i choose it to get acquainted with microcontrollers
- After this project, change this arduino code to an orther uC that based on ARM architecture
## Hardware
- Arduino Uno R3
- 2 stepper driver A4988
- An adapter 12V-5A to run the two motors
- HC- 05 to comunicate with Paint app (a desktop app that i am programming. This app allows to draw on a workspace and send this drawing via HC-05 to arduino)
- L7805 to perform as a voltage regulater. This module is used to supply 5V DC for arduino, A4988s and HC-05
## Libraries and Sofwares
- Using accelStepper Library to control the two steppers
- Using KiCad 6.0 for skematic and PCB
## Main feature
- using fuction pain_func() to paint any function (add function into source code before upload the code to arduino), this funtion will calculate points of the function
and control a pen according to the points.
-Demo: (using test2() funtion)
- using Paint app to create pictuce and send data through HC-05 to paint it! 
debugging :(
![demo](https://user-images.githubusercontent.com/122275694/236265364-1ec3fe52-bba9-41c8-bd6c-d334d6a53197.jpg)
## Skematic
![skematic](https://user-images.githubusercontent.com/122275694/236263339-7b05cdb7-8c22-4552-9d30-42c8c14bec10.png)
##PCB and 3D-view
PCB
![PCB](https://user-images.githubusercontent.com/122275694/236263855-1c826858-c4c8-4b8c-8ba0-c79a43d198d7.png)
3D-view
![3D](https://user-images.githubusercontent.com/122275694/236263976-9a4d33b3-9ee4-4d81-8ee5-199ba8eb78da.png)
