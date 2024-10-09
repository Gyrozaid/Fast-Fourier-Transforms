import serial
import pandas as pd

port = 'COM8'
baud_rate = 115200  
ser = serial.Serial(port, baud_rate)
df_list = []

try:
    print("Listening for data from ESP32...")
    
    while True:
        if ser.in_waiting > 0:  
            line = ser.readline().decode('utf-8').strip() 
            data_str = line.split(':')[-1]
            data_list = []
            for x in data_str.split(","):
                try:
                    data_list.append(float(x))
                except ValueError:
                    pass
                
            if len(data_list) > 0:
                df_list.append(data_list)
                print(data_list) 

except KeyboardInterrupt:
    df = pd.DataFrame(df_list, columns=['Sample Num', 'acc_x', 'acc_y', 'acc_z', 'gyro_x', 'gyro_y', 'gyro_z', 'temp'])
    df.to_csv("C:\\Users\\ryanz\\Documents\\CS528\\gyroscope\\IMU\\sensor_data_down.csv")
    
    print("Stopping data capture, saving to csv...")

finally:
    ser.close() 
    print("Serial connection closed.")
