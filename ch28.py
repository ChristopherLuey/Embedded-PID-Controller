import serial
from plotting import *
from genref import genRef

# Open ports
ser = serial.Serial('/dev/tty.SLAB_USBtoUART',230400)
print('Opening port: ')
print(ser.name)
print('PIC32 MOTOR DRIVER INTERFACE')

# Main loop
while True:
    selection = input('\nENTER COMMAND: ')
    selection_endline = selection+'\n'
    ser.write(selection_endline.encode()); # .encode() turns the string into a char array
    
    # Current read
    if (selection == 'b'):
        out = float(ser.read_until(b'\n'))
        print("Current: {} mA".format(out))
    
    # Angle read
    elif (selection == 'c'):
        out = float(ser.read_until(b'\n'))
        print("Angle (counts): {}".format(out))
    
    # Angle read
    elif (selection == 'd'):
        out = float(ser.read_until(b'\n'))
        print("Angle (deg): {}".format(out))
    
    # Reset
    elif (selection == 'e'):
        print("")
    
    # PWM activate
    elif (selection == 'f'):
        _in = int(input("Enter PWM (-100 to 100): "))
        ser.write((str(_in)+'\n').encode()); # send the number
        out = ser.read_until(b'\n')
        out = int(ser.read_until(b'\n'))
        print("PWM set to {}".format(out))
        print(out)
    
    # Set current gains
    elif (selection == 'g'):
        _in = input("Set current gains (Jp, Ji): ")
        ser.write((_in+'\n').encode())
        out = ser.read_until(b'\n')
        out = ser.read_until(b'\n')
        out = ser.read_until(b'\n').decode('utf-8').strip().split(" ")
        print("Jp: {}\nJi: {}".format(out[0], out[1]))

    # Get current gains
    elif (selection == 'h'):
        out = ser.read_until(b'\n').decode('utf-8').strip().split(" ")
        print("Jp: {}\nJi: {}".format(out[0], out[1]))
    
    # Set position gains
    elif (selection == 'i'):
        _in = input("Set position gains (Kp, Ki, Kd): ")
        ser.write((_in+'\n').encode())
        out = ser.read_until(b'\n')
        out = ser.read_until(b'\n')
        out = ser.read_until(b'\n').decode('utf-8').strip().split(" ")
        print("Kp: {}\nKi: {}\nKd: {}".format(out[0], out[1], out[2]))
    
    # Get position gains
    elif (selection == 'j'):
        out = ser.read_until(b'\n').decode('utf-8').strip().split(" ")
        print("Kp: {}\nKi: {}\n Kd: {}".format(out[0], out[1], out[2]))

    # Run current gain test
    elif (selection == 'k'):
        read_plot_matrix(ser)
    
    # Run angle control test
    elif (selection == 'l'):
        _in = input("Set angle (degrees): ")
        ser.write((_in+'\n').encode())

    # Generate step trajectory
    elif (selection == 'm'):
        ref = genRef('step')
        t = range(len(ref))
        plt.plot(t,ref,'r*-')
        plt.ylabel('angle in degrees')
        plt.xlabel('index')
        plt.show()
        ser.write((str(len(ref))+'\n').encode())
        for i in ref:
            ser.write((str(i)+'\n').encode())

    # Generate cubic trajectory
    elif (selection == 'n'):
        ref = genRef('cubic')
        t = range(len(ref))
        plt.plot(t,ref,'r*-')
        plt.ylabel('angle in degrees')
        plt.xlabel('index')
        plt.show()
        ser.write((str(len(ref))+'\n').encode())
        for i in ref:
            ser.write((str(i)+'\n').encode())
    
    # Read plot
    elif (selection == 'o'):
        read_plot_matrix_pos(ser)

    # Unpower the motor
    elif (selection == 'p'):
        out = ser.read_until(b'\n').decode('utf-8').strip()
        print(out)

    elif (selection == 'q'):
        print('Exiting client')
        ser.close()
        break

    elif (selection == 'r'):
        expected_lines = 10
        for _ in range(expected_lines):
            line = ser.readline().decode('utf-8')
            print(line, end='')
        print()

    else:
        print('Invalid Selection ' + selection_endline)
