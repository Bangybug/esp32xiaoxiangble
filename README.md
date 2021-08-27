# esp32xiaoxiangble
Connecting esp32 to xiaoxiang bms Bluetooth mobile app

My goal is to connect to the Xiaoxiang BMS using ESP32 as bluetooth BLE module. 

The xiaoxiang mobile app sees my ESP32, but the data interchange is not established, I've seen with my scope something is going on both rx and tx lines, but the app could not talk to the BMS. 

Warning: the BMS has Vdd line, don't use it. It's level is 12v.
