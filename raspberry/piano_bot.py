import serial
from binascii import hexlify
from time import sleep

# arrays to be transmitted
NOTES = ["D2#", "B1", "D2#", "D2#", "A1#", "F2#", "E2", "D2#", 
        "D2#", "B1", "D2#", "D2#", "A1#", "F2#", "E2", "D2#",
        "D2#", "B1", "G2#", "E2", "A1#", "F2#", "E2", "C2#"]
DURATIONS = [2, 2, 2, 2, 2, 2, 1, 1, 
            2, 2, 2, 2, 2, 2, 1, 1, 
            2, 2, 2, 2, 2, 2, 1, 1]

ser =  serial.Serial("/dev/ttyAMA0")

def parse_notes(notes): # function for translating notes into array of order numbers of keys

    key_matching = {"C1":1, "C1#":2, "D1":3, "D1#":4, "E1":5, "F1":6, "F1#":7, "G1":8, "G1#":9, "A1":10, "A1#":11, "B1":12,
                "C2":13, "C2#":14, "D2":15, "D2#":16, "E2":17, "F2":18, "F2#":19, "G2":20, "G2#":21, "A2":22, "A2#":23, "B2":24,
                "C2":25}
    keys = []
    for note in notes:
        keys.append(key_matching[note])
    return keys

def send_data(data): #funciton for sending one byte of data with error check
    read = hexlify(ser.read())
    print("Received: ", read)

    # if READY_FOR_RECEIVING byte received, start the transmition
    # otherwise break the connection
    if read == hexlify(chr(0xAA)):       
        ser.write(chr(data))                  
        print("Sent: ", data)
        read = hexlify(ser.read())
        print("Received: ", read)
        if read != hexlify(chr(0xAB)):
            print("Error code has been received. Retrying...")
            ser.write(chr(data))
            print("Sent: ", data)
            read = hexlify(ser.read())
            print("Received: ", read)
            if read != hexlify(chr(0xAB)):
                print("The retry failed. Exiting.")
                exit()
    else:
        print("Wrong ready code. Exiting.")
        ser.close()
        exit()
    

if __name__ == '__main__':

    keys = parse_notes(NOTES)
    print(keys)
    length = len(keys)          
    send_data(length)           
    for key in keys:
        send_data(key)
    send_data(30)               # sending END_OF_KEYS_ARRAY byte
    for duration in DURATIONS:
        send_data(duration)
            
    ser.close()
