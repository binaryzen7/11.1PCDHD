import smbus
import serial
import time
import csv
from datetime import datetime

# BH1750 I2C Setup
DEVICE = 0x23  # or 0x5c depending on your sensor
POWER_ON = 0x01
RESET = 0x07
CONTINUOUS_HIGH_RES_MODE = 0x10

bus = smbus.SMBus(1)

def read_light(addr=DEVICE):
    try:
        bus.write_byte(addr, POWER_ON)
        time.sleep(0.01)
        bus.write_byte(addr, RESET)
        time.sleep(0.01)
        bus.write_byte(addr, CONTINUOUS_HIGH_RES_MODE)
        time.sleep(0.2)
        data = bus.read_i2c_block_data(addr, CONTINUOUS_HIGH_RES_MODE, 2)
        result = ((data[0] << 8) + data[1]) / 1.2
        return round(result, 2)
    except Exception as e:
        print("BH1750 Error:", e)
        return None

try:
    ser = serial.Serial('/dev/ttyACM0', 9600, timeout=2)
    time.sleep(2)  # Allow Arduino to reset

    # Open the CSV file in append mode
    with open('sensor_data.csv', mode='a', newline='') as file:
        writer = csv.writer(file)
        
        # Optional: Write header if file is empty
        if file.tell() == 0:
            writer.writerow(["Timestamp", "Temperature (C)", "Humidity (%)", "Soil Moisture", "Light Intensity (lux)"])

        print("Listening to Arduino and BH1750...")

        while True:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            if line:
                print("Received:", line)
                if line.startswith("TEMP"):
                    parts = line.split(',')
                    try:
                        temp = float(parts[0].split(':')[1])
                        hum = float(parts[1].split(':')[1])
                        soil = int(parts[2].split(':')[1])

                        # Read BH1750
                        lux = read_light()

                        print(f"  Temperature: {temp} C")
                        print(f"  Humidity: {hum} %")
                        print(f"  Soil Moisture: {soil}")
                        print(f"  Light Intensity: {lux} lux")

                        # Write all sensor data to CSV
                        timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
                        writer.writerow([timestamp, temp, hum, soil, lux])
                        file.flush()

                    except Exception as e:
                        print("Parsing error:", e)

except serial.SerialException as e:
    print(f"Failed to open serial port: {e}")
finally:
    if 'ser' in locals() and ser.is_open:
        ser.close()
        print("Serial connection closed.")
