# Fire Suppression OTV

## Overview
This repository contains the code and design files for an autonomous Over Terrain Vehicle (OTV) built for the University of Maryland's Balloon Payload Program. The vehicle was designed to navigate to a fire site, identify and extinguish active flames while leaving the middle flame lit, and transmit mission data back to a command center, all without any human presence in the hazardous area.

## The Problem
A BPP payload experienced an emergency landing near a remote lake. An electrical component failure started a fire at the landing site, and a hazardous material payload stopped transmitting on impact, making the area unsafe for humans. An autonomous vehicle was needed to handle the situation in place of a human team.

## Our Solution
We designed and built a fully autonomous ground vehicle capable of:
- Navigating to within 150mm of the fire site using an overhead vision system
- Detecting active flames using IR sensors
- Extinguishing all flames except the middle flame using onboard fans
- Transmitting flame count and topography orientation data via WiFi
- Navigating past obstacles and into a destination zone

## Hardware
- Arduino based microcontroller
- ESP8266 WiFi module for data transmission
- IR sensors for flame detection
- Ultrasonic sensors for distance measurement
- Buck converter for voltage regulation
- 3D printed chassis and front arm assembly
- Dual fans for flame extinguishing

## Software
We used PlatformIO on VSCode to implement all of the software logic for the OTV navigating to the mission site, completing its mission objectives, transmitting data to the handler, and autonomously navigating through the obstacles toward the end-zone.

## Designs & Media
<img width="1279" height="816" alt="image" src="https://github.com/user-attachments/assets/5642429b-5e1f-4328-aef8-d928d31d665a" />

[![Watch the video of our OTV in action here!]({https://github.com/user-attachments/assets/bd428044-291a-48c9-bb53-e285a748d05f})]({https://drive.google.com/file/d/1sTOnqRoYdJUCLPXdOUjCKNKuKrmgNfFM/view?usp=sharing} "OTV - Fire Suppression")

## Results
We ended up completing all of our mission objectives but we believe because of some electronics issues (our arduino ultrasonic sensors getting fried, our OTV was unable to traverse to the end of the arena.

## Team
| Name | Role |
|------|------|
| Syed Nazeer Ahmed | Software Lead |
| Ananya Vivek | Software Engineer |
| Anna Bannerman-Zukerberg | Software Engineer |
| Jamil Sheikh | CAD Designer |
| Mohamed Abdalla | CAD Designer |
| Eric Jiang | Mechanical Assembly Lead |
| Jacky Zeng | Electrical & Circuit Engineer |
| Jacob Myers | Electrical & Circuit Engineer |
