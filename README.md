# esp32-mqtt-water-level-alarm

![Alt text](preview.png?raw=true "Device in actin with a 3D printed enclosure")

I have a water tank on my balcony collecting condensation water from the AC.
To prevent potential water overflow, I made a little system that shuts down the AC if the tank gets too full.
The system consists of
1) an esp32 board with a liquid level sensor powered by a LiPo battery, sending MQTT messages to a
2) <a href="https://www.home-assistant.io/">HomeAssistant OS</a> running on a <a href="https://www.raspberrypi.org/">Raspberry PI</a>, which in turn shuts down the AC upon critical water level

## Hardware

### ESP32 board
Adafruit ESP32-S3 Feather (https://learn.adafruit.com/adafruit-esp32-s3-feather/overview)

### Liquid level sensor
Crowtail- Non-contact liquid level sensor 2.0 (https://www.elecrow.com/crowtail-noncontact-liquid-level-sensor-p-1656.html)

### Battery
A Li-Po battery 3,7V 800mAh 

## C++ notes
To compile, add and edit the "src/secrets.h" file with your wifi/mqtt secrets. 
There's a "secrets.h.example" file with required definitions.

## Home assistant integration
This is how I've configured HomeAssistant

### configuration.yaml (adds two mqtt sensors)

~~~code
...
mqtt:
  sensor:
    - name: "Water Tank Sensor Battery"
      unique_id: "water-tank-sensor-battery"
      state_topic: "/home/balcony/watertank/sensor"
      json_attributes_topic: "/home/balcony/watertank/sensorr"
      value_template: "{{ value_json.battery }}"
      json_attributes_template: "{{ value_json.battery | tojson }}"
      unit_of_measurement: "%"
      device_class: "battery"
  binary_sensor:
    - name: "Water Tank Sensor Alarm"
      unique_id: "water-tank-sensor-alarm"
      state_topic: "/home/balcony/watertank/sensor"
      json_attributes_topic: "/home/balcony/watertank/sensor"
      value_template: "{{ value_json.full }}"
      json_attributes_template: "{{ value_json.full | tojson }}"
      payload_on: true
      payload_off: false
      device_class: "problem"
...
~~~

### Automation configuration example

~~~code
alias: Turn Off AC when watertank is full
description: ""
trigger:
  - platform: state
    entity_id:
      - binary_sensor.water_tank_sensor_alarm
    to: "on"
condition: []
action:
  - repeat:
      while:
        - condition: state
          entity_id: binary_sensor.water_tank_sensor_alarm
          state: "on"
      sequence:
        - device_id: 0b4465ab94e18bb160c19ce772ebd205
          domain: climate
          entity_id: climate.ac
          type: set_hvac_mode
          hvac_mode: "off"
        - delay:
            minutes: 1
mode: single
~~~

## 3D enclosure
The 3D enclosure STL and OpenSCAD files can be found in the "3d-enclosure" folder. The enclosure is a modified version of the <a href="https://www.thingiverse.com/thing:2411898">box with snap on cover - parametrised OpenSCAD</a>.
