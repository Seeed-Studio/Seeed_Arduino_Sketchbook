# PC Stats Display on Wio Terminal

## Introduction

This demo can be used to display various different statistics of your computer such as:
- CPU Utilization 
- CPU Speed
- GPU Utilization 
- GPU Used Memory 
- Used Memory 
- Free Memory 
- Used SSD/ HDD
- Free SSD/ HDD

It simply grabs all the information from the computer using a Python script and sends that information to the Wio Terminal via serial interface and displays on the Wio Terminal LCD.
Here [pyserial](https://pypi.org/project/pyserial/), [psutil](https://pypi.org/project/psutil/) and [gpuinfo](https://pypi.org/project/gpuinfo/) libraries are used for the Python code. 

<p style="text-align:center;"><img src="https://files.seeedstudio.com/wiki/Wio%20Terminal%20PC%20Stats/WeChat%20Image_20201102093059.jpg" alt="pir" width="850" height="auto"></p>

## Prerequisites
- [Wio Terminal](https://www.seeedstudio.com/Wio-Terminal-p-4509.html) 
- [Arduino](https://www.arduino.cc/en/software) installed on PC
- [Python](https://www.python.org/downloads/) installed on PC 



## How to Use?
- Upload `PC_stats_display.ino` to Wio Terminal 
- Navigate to the location of the folder containing the .py file on command prompt
```sh
Example:
C:\Users\user\Desktop>Wio_Terminal_PC_Stats
```
- Run the .py using the below command 
```python
python PC_stats_send.py
```

<p style="text-align:center;"><img src="https://files.seeedstudio.com/wiki/Wio%20Terminal%20PC%20Stats/5f813bb2d2d6b50349f796ff672f806.jpg" alt="pir" width="850" height="auto"></p>



