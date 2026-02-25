# ESP32_LiLygo_GSM_ThinkSpeak
ESP32 LiLygo wifi and GSM , data capture fo air quality CO2, Temp and Humidity and data send to ThinkSpeak

# ESG Sensor Data Capture & Cloud Integration
**Developed by:** Hidayat Harmoni
**Lead Coder:** 01 (Override)
**Hardware Platform:** LilyGo T-Display (ESP32)

---

## 📌 Project Overview
This repository contains the firmware for an **Environmental, Social, and Governance (ESG)** data logger. The system is designed to monitor critical indoor and outdoor air quality parameters, providing real-time telemetry to the **ThingSpeak** IoT platform.

The system is versatile, supporting **WiFi** for office/factory environments and **GSM (GPRS)** for remote or mobile monitoring where local networks are unavailable.



---

## 🛠 Hardware Configuration

### Pin Mapping (ESP32)
The LilyGo T-Display interacts with the following sensors:

| Component | Pin | Protocol | Description |
| :--- | :--- | :--- | :--- |
| **DHT22** | GPIO 4 | One-Wire | Ambient Temp & Humidity |
| **MQ135** | GPIO 34 | Analog | VOCs / Air Quality Index |
| **SCD30 (SDA)** | GPIO 21 | I2C | $CO_2$ Data Line |
| **SCD30 (SCL)** | GPIO 22 | I2C | $CO_2$ Clock Line |
| **SIM800 TX** | GPIO 16 | UART | GSM Transmit |
| **SIM800 RX** | GPIO 17 | UART | GSM Receive |

---

## 💻 Software Stack

### Dependencies
The following libraries must be installed via the Arduino Library Manager:
* **WiFiManager**: For captive portal WiFi configuration.
* **TinyGSM**: For SIM800L GPRS data handling.
* **Sensirion I2C SCD30**: For high-precision NDIR $CO_2$ sensing.
* **DHT Sensor Library**: By Adafruit.

### Connectivity Logic
The code uses a boolean toggle system to define the primary communication path:
* `useWiFi = true`: Enables `WiFiManager`. If no saved SSID is found, the device creates an AP named **"AutoConnectAP"**.
* `useGSM = true`: Initializes the SIM800 modem using the defined `apn`.

---

## 📊 Data Mapping (ThingSpeak Fields)
Data is pushed to the cloud every 60 seconds ($1 \text{ minute}$ interval) to comply with ThingSpeak's rate limits and provide granular reporting.

| ThingSpeak Field | Parameter | Unit | Source |
| :--- | :--- | :--- | :--- |
| **Field 1** | Temperature | °C | DHT22 |
| **Field 2** | Humidity | % | DHT22 |
| **Field 3** | Air Quality | % (Scaled) | MQ135 |
| **Field 4** | $CO_2$ | ppm | SCD30 |
| **Field 5** | Temperature | °C | SCD30 |
| **Field 6** | Humidity | % | SCD30 |

---

## ⚙️ Setup Instructions
1. **API Key**: Replace `api\_key` in the source code with your ThingSpeak Write API Key.
2. **GSM APN**: If using GSM, ensure the `apn` matches your service provider (e.g., "internet", "diginet", "celcom").
3. **Baud Rate**: Ensure your Serial Monitor is set to `115200`.
4. **Power**: If using GSM, ensure the power source can provide at least $2\text{A}$ peak current during transmission.

---

## 📝 Change Log
* **v1.0**: Initial Release. Support for dual-mode transmission and multi-sensor fusion (DHT22 + SCD30).

**IT Team KISMEC** *Empowering Industrial Digitalization*
