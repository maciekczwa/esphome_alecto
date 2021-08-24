# esphome_alecto
EspHome Alecto wheater station component
The component supports LIDL wheather station H13726 which communicates using alecto protocol
The protocol is described here: https://wiki.pilight.org/auriol_protocol_v20.pdf

# Hardware

To use this component you will need esp32 or esp8266 and generic 433 MHz receiver.

# Setup

remote receiver component works good using following settings:

```yaml
remote_receiver:
  id: receiver
  pin: 
    number: GPIO27
  idle: 6ms
  filter: 200us
  tolerance: 35%
```

You will need to set id's of  wind/temperature and rain  sensor here (0x6c - wind and 0x0b - rain in example):

```yaml
sensor:
- platform: custom
  lambda: |-
    auto alecto_receiver = new AlectoComponent(id(receiver),0x6c,0x0b);
    App.register_component(alecto_receiver);
    return {&alecto_receiver->tempSensor, &alecto_receiver->humiditySensor, &alecto_receiver->rainSensor,  &alecto_receiver->windAvgSensor,  &alecto_receiver->windGustSensor,  &alecto_receiver->windDirSensor,  &alecto_receiver->tempBatterySensor,  &alecto_receiver->rainBatterySensor};
  sensors:
  - name: "Temperature meteo"
    unit_of_measurement: °C
    accuracy_decimals: 1
  - name: "Humidity meteo"
    unit_of_measurement: "%"
    accuracy_decimals: 0
  - name: "Rain meteo"
    unit_of_measurement: "mm"
    accuracy_decimals: "1"
  - name: "Wind avg meteo"
    unit_of_measurement: "m/s"
    accuracy_decimals: "1"
  - name: "Wind gust meteo"
    unit_of_measurement: "m/s"
    accuracy_decimals: "1"
  - name: "Wind dir meteo"
    unit_of_measurement: "°"
    accuracy_decimals: "1"
  - name: "Battery temp meteo"
    unit_of_measurement: "%"
    accuracy_decimals: "0"
  - name: "Battery rain meteo"
    unit_of_measurement: "%"
    accuracy_decimals: "0"
```

The id's of sensors are shown in esphome log sou you can figure it out there and set it to yours. The ids will be reset after battery change.
The rain and wind/temp sensors transmits data every few minutes.

