# generate a step or cubic response trajectory based on a list of via points

def genRef(method):
    if method == 'step':
        print('Step!')
    elif method == 'cubic':
        print('Cubic!')
    else:
        print('Not a valid method')
        return [-1]

    reflist = [0,0,1,90,2,45,3,45] # defualt refs
    
    refs_str = input('Enter times and angles, starting at t=0 (ex: 0 0 1 90 2 90 3 0): ')
    reflist = list(map(float,refs_str.split()))

    # check to see if the reflist is even and the odd values (times) are increasing
    if (len(reflist)%2!=0 or len(reflist) < 2 or reflist[0]!=0):
        print('Not a valid input: odd number of inputs or too short')
        return [-1]
    dataok = 1
    for i in range(2, len(reflist), 2):
        if reflist[i] <= reflist[i-2]:
            dataok = 0
    if dataok == 0:
        print('Not a valid input: time must increase')
        return [-1]

    MOTOR_SERVO_RATE = 200 # the position control ISR is 200Hz
    dt = 1/MOTOR_SERVO_RATE # time per control cycle

    numpos = int(len(reflist)/2)

    if method == 'step':
        sample_list = [] # time
        angle_list = [] # position
        for i in range(0, len(reflist), 2):
            sample_list.append(reflist[i]*MOTOR_SERVO_RATE)
        for i in range(1, len(reflist), 2):
            angle_list.append(reflist[i])
        ref = [0]*int(sample_list[-1])
        last_sample = 0
        for i in range(1, numpos):
            for samp in range(int(last_sample), int(sample_list[i])):
                ref[samp] = angle_list[i-1]
            last_sample = sample_list[i]
        ref[samp] = angle_list[-1]
    
    if method == 'cubic':
        ref = [] # store the output trajectory
        time_list = [] # time
        pos_list = [] # position
        for i in range(0, len(reflist), 2):
            time_list.append(reflist[i])
        for i in range(1, len(reflist), 2):
            pos_list.append(reflist[i])
        vel_list=[0]*numpos
        if numpos >= 3:
            for i in range(1, numpos-1):
                vel_list[i] = (pos_list[i+1]-pos_list[i-1])/(time_list[i+1]-time_list[i-1])
        #print(vel_list)
        refCtr = 0
        for i in range(0,numpos-1):
            timestart = time_list[i]
            timeend = time_list[i+1]
            deltaT = timeend-timestart
            posstart = pos_list[i]
            posend = pos_list[i+1]
            velstart = vel_list[i]
            velend = vel_list[i+1]
            a0 = posstart # calculate coeffs of traj pos = a0+a1*t+a2*t^2+a3*t^3
            a1 = velstart
            a2 = (3*posend - 3*posstart - 2*velstart*deltaT - velend*deltaT)/(deltaT*deltaT)
            a3 = (2*posstart + (velstart+velend)*deltaT - 2*posend)/(deltaT*deltaT*deltaT)
            while (refCtr)*dt < time_list[i+1]:
                tseg = (refCtr)*dt -time_list[i]
                ref.append(a0 + a1*tseg + a2*tseg*tseg + a3*tseg*tseg*tseg)
                refCtr = refCtr + 1

    return ref