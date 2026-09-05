# 🔌 ESP32 Hardware Pin Connections

## I2C LCD

```text
LCD VCC  → ESP32 5V
LCD GND  → ESP32 GND
LCD SDA  → GPIO21
LCD SCL  → GPIO22
```

## NEO-6M GPS

```text
GPS VCC  → Appropriate supply
GPS GND  → ESP32 GND
GPS TX   → GPIO16
GPS RX   → GPIO17
```

## HW-748 GSM

```text
GSM TX   → GPIO26
GSM RX   → GPIO27
GSM GND  → ESP32 GND
GSM VCC  → External suitable supply
```

## HW201 IR Sensor

```text
IR VCC   → Appropriate supply
IR GND   → ESP32 GND
IR OUT   → GPIO34
```

## HC-SR04

```text
HC-SR04 VCC   → 5V
HC-SR04 GND   → GND
HC-SR04 TRIG  → GPIO5
HC-SR04 ECHO  → Voltage Divider → GPIO18
```

## Motor Driver

```text
IN1 → GPIO25
IN2 → GPIO33
IN3 → GPIO32
IN4 → GPIO23

GND → ESP32 GND
```

## Important Power Rule

All modules must share a common ground:

```text
ESP32 GND
     │
     ├── GPS GND
     ├── GSM GND
     ├── LCD GND
     ├── IR GND
     ├── HC-SR04 GND
     └── Motor Driver GND
```

The GSM and motors should have appropriate external power supplies.

Do not attempt to power motors or a GSM transmitter directly from an ESP32 GPIO.
