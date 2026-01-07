# Name: Multi information clock main
# Author: marksard
# Version: 2.0
# Python 3.6 or later (maybe)
# Require Device: Raspberry PI 1 Model B+ or later.
#                 Temperature and humidiry sensor BME280
#                   datasheet (http://akizukidenshi.com/download/ds/bosch/BST-BME280_DS001-10.pdf)
#                 WinSTAR OLED Display 20x4 WEH002004A (using controller WS0010)
#                   datasheet (http://blog.digit-parts.com/pdf/ws0010.pdf)

# ***************************
from queue import Queue
import time
import requests
import random

# import bme280
import weh002004a
import bigdigit

# ***************************

disp = weh002004a.WEH002004A(26, 19, 13, 6, 5, 11)

def main() ->  None:
    while True:
        cpu, gpu = get_cpu_gpu_stats_with_retry()
        if cpu is not None and gpu is not None:
            disp.write_line("cpu\xDF""c  slugon  gpu\xDF""c", 1)
            bigdigit.write_temps(disp, cpu, gpu)
            time.sleep(1)
        else:
            i = 0
            while i < 60:
                screensaver()
                time.sleep(1)
                i += 1

# ***************************
# Getting various information

def get_random_char():
    i = random.randint(8, 15)
    i = random.randint(0, 15)
    hex_code = f"{i:01x}{j:01x}"
    byte_value = int(hex_code, 16)
    return chr(byte_value)

def screensaver():
    disp.clear_display()
    idx = []
    for i in range(7):
        idx.append([random.randint(0, 3), random.randint(4, 20), random.randint(4, 9)])
    lines = [""] * 4
    for i in idx:
        lines[i[0]] += " " * (20 - i[1]) + get_random_char() * i[2]
    for index, value in enumerate(lines):
        disp.write_line(value[:20], index)

def get_cpu_gpu_stats_with_retry():
    try:
        response = requests.get(
            'http://192.168.0.60:5221/api/temps', 
            timeout=(5, 15)  # 5 seconds connect, 15 seconds read
        )
        response.raise_for_status()
        
        data = response.json()
        cpu_value = data.get('cpu')
        gpu_value = data.get('gpu')
        
        return cpu_value, gpu_value
        
    except requests.exceptions.Timeout:
        return None, None
    except requests.exceptions.ConnectionError as e:
        return None, None
    except Exception as e:
        return None, None

# ***************************
# Run Program

if __name__ == '__main__':
    try:
        disp.initialize()
        main()
    except KeyboardInterrupt:
        pass
    finally:
        disp.dispose()
