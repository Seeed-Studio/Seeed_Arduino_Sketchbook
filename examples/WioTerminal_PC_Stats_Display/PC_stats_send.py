# include libraries 
import psutil # monitor CPU, RAM, Disks
import serial # send data through serial interface
from gpuinfo import GPUInfo # monitor GPU

#pySerial settings
ser = serial.Serial() # make instance of Serial
ser.baudrate = 9600 # set serial baud rate 
ser.port = "COM4"  # set serial port, replace with the port your Wio Terminal is connceted to the PC 
ser.open() # open the serial port

while True: # while loop

    # CPU Utilization
    cpu = psutil.cpu_percent(interval=1.2) # get usage of CPU in percentage with interval of 1.2s 

    if cpu < 10:
        cpuStr = "  " + str(cpu) # if CPU usage is under 10%, put 2 artificial characters (spaces) before the value (to make 5 spaces in total)
    elif cpu < 100:
        cpuStr = " " + str(cpu) # here the same, but only 1 space. because 85.6 have only 4 characters 
    else:
        cpuStr = str(cpu) #100.0 is 5 characters so there is no need to put spaces in before

    # CPU Speed
    cpufreq = round(psutil.cpu_freq().current/1000, 1) # get cpu frequency, divide by 1000 because 1GHz = 1000MHz, round to one decimal place
    cpufreqStr = "  " + str(cpufreq) # convert to string and make 5 spaces in total

    # GPU Info 
    percent,memory = GPUInfo.gpu_usage() # get GPU utilization and used memory info

    # GPU Utilization
    percentNum = percent[0] # obtain number from the percent list 

    if percentNum < 10:      # same as for CPU (make 5 spaces)
        percentStr = "    " + str(percentNum)                                
    elif percentNum < 100:
        percentStr = "   " + str(percentNum)
    else:
        percentStr = "  " + str(percentNum)

    # GPU Used Memory
    memoryNum = round(memory[0] / 1024, 1) # obtain number from the memory list and round to one decimal place
    memoryStr = "  " + str(memoryNum) # convert to string and make 5 spaces in total

    # RAM Used
    ramUsed = round(psutil.virtual_memory().used / 1073741824,1) # 1GB = 1073741824bytes

    if ramUsed < 10: # make 5 spaces in total
        ramUsedStr = "  " + str(ramUsed)
    else:
        ramUsedStr = " " + str(ramUsed)

    # RAM Free
    ramFree = (round(psutil.virtual_memory().free / 1073741824,1))

    if ramFree < 10: # make 5 spaces in total
        ramFreeStr = "  " + str(ramFree)
    else:
        ramFreeStr = " " + str(ramFree)

    # SSD and HDD Usage
    sddused = round(psutil.disk_usage("C:").used / 1073741824) # get used space of C: disk (SSD)
    sddfree = round(psutil.disk_usage("C:").free / 1073741824) # get free space of C: disk (SSD)
    hddused = round(psutil.disk_usage("D:").used / 1073741824) # get free space of D: disk (HDD)                 
    hddfree = round(psutil.disk_usage("D:").free / 1073741824) # get free space of D: disk (HDD) 
    
    if sddused < 100:  # make 5 spaces in total
        sddUsedStr = "   " + str(sddused)
    else:
        sddUsedStr = "  " + str(sddused)

    if sddfree < 100:
        sddFreeStr = "   " + str(sddfree)
    else:
        sddFreeStr = "  " + str(sddfree)

    if hddused < 100:                                         
        hddUsedStr = "   " + str(hddused)                           
    else:                                         
        hddUsedStr = "  " + str(hddused)                             

    if hddfree < 100:                                         
        hddFreeStr = "   " + str(hddfree)                           
    else:                                           
        hddFreeStr = "  " + str(hddfree)                            

    serialDataStr = cpuStr + cpufreqStr + ramUsedStr + ramFreeStr + sddUsedStr + sddFreeStr + hddUsedStr + hddFreeStr + percentStr + memoryStr # concatenate all strings together by using "+" operand to form one long string of data
    serialDataBytes = serialDataStr.encode("UTF-8") # encode it to UTF-8 standard, since the strings should be a series of BYTES             

    print(serialDataBytes) # print serial string, used for debugging
    ser.write(serialDataBytes) # send the long encoded string throught serial interface

ser.close()                                                     
