##09/06/2022
##End of year project
##Client code
##Cameron Wade

##Code to connect to local client
import socket
import time
import pygame ##for controller connection


##h1 = http.client.HTTPConnection('192.168.137.197')

class Controllers:       ##creates class for joysticks
    def __init__(self, iD, nAxis):
        self.iD = iD
        self.nAxis = nAxis
    def axis(axisNum):
        print("axisNum" + axisNum)



pygame.init()

    ##following is an array of all joysticks in the system
joysticks = [pygame.joystick.Joystick(x) for x in range(pygame.joystick.get_count())]
print(joysticks)

LMotor = 1
RMotor = 3
BHalt = 0

LeftStick = Controllers("LMotor", 1)
RightStick = Controllers("RMotor",3)
Halt = Controllers ("Halt", 0)

controller = joysticks[0]
print("number of axis:\t", controller.get_numaxes())
print("\naxis position:\t" + str(controller.get_axis(0)))
print(  "\nnumber of buttons:\t" + str(controller.get_numbuttons() )     )


##UDP setup
##serverPort   = ("192.168.137.96", 2390)
##serverPort   = ("127.0.0.1", 20001)
##bufferSize          = 1024
##UDPClientSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)


##s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
##s.connect((HOST, PORT))



def control():
    HaltVal = controller.get_button(BHalt)
    LVal = controller.get_axis(LMotor)
    RVal = controller.get_axis(RMotor)
    pygame.event.pump()

##        print("Begin")        ##Debugging
##        print( "LVal:\t" + str(LVal)   )
##        print( "RVal:\t"+str(RVal)   )
##        print("END")
##        return "halt"

    
## H high, L low, + = +ve, - = -ve
##fastForwards      L: H+ R: H+
##slowForwards      L: L+ R: L+
##fastRight         L: H+ R: L+
##fastLeft          L: L+ R: H+
##fastReverse       L: H- R: H-
##slowReverse       L: L- R: L-
##acTurn            L: H- R: H+
##cTurn
##halt              L: 0  R: 0
##sense             L: X  R: X
        ##joysticks[0].get_button(0) true if button pressed
    H = 0.6
    L = 0.1
    if (controller.get_button(0)):
        return "sense"
    elif(controller.get_button(1)): ##B button
        return "off"
    elif(controller.get_button(3)): ##Y button
        return "on"
    elif((LVal > +H)and(RVal < -H)): ##acTurn
        
##      print("acTurn")
        return "AT"
    elif((LVal < -H)and(RVal > +H)): ##cTurn
##      print("cTurn")
        return "CT"
    elif((LVal < -H)and(RVal < -H)): ##fF
##      print("Fast Forwards")
        return "FF"
    elif((LVal < -L)and(LVal > -H)and(RVal < -H)): ##fL
##      print("fastLeft")
        return "FL"
    elif((LVal < -H)and(RVal < -L)and(RVal > -H)): ##fRight
##      print("fastRight")
        return "FR"    
    elif((LVal > +H)and(RVal > +H)): ##fReverse
##      print("fastReverse")
        return "FRe"
    elif((LVal < -L)and(RVal < -L)): ##sF
##      print("slow Forwards")
        return "SF"
    elif((LVal > +L)and(RVal > +L)): ##sReverse
##      print("slowReverse")
        return "SRe"
    elif((LVal>-L)and(LVal<L)and(RVal>-L)and(RVal<L)): ##not required, just else halt
##      print("halt")
        ##controller.stop_rumble()
        return "H"
    else:
        return          
             

    ##to send data to server
def upload(command):
    print("uploading start")
    print(command)
    header = command
    receive = requests.post('http://192.168.137.96/' + header, data={'key':""})
    print("receive", receive.text)
    print("uploading END")
    

    
def loop(previous):
    pygame.event.pump()
        ##tmp used as tmp can be none
    tmp = control()
    if (not(tmp is None)):
            ##if tmp is none, command stays as previous 
            ##command is data to send to arduino if not the same as previous command
        command = tmp
    else:
        command = previous
    if((command!= previous)and not(tmp is None)):
        upload(command)
        
    return command

    ##control()
    ##time.sleep(1) ##in seconds
    



control()
previous = "halt"
while (True):
    previous = loop(previous)
    received = sock.recvfrom(bufferSize)
    if (not(received is None)):
            ##if tmp is none, command stays as previous 
            ##command is data to send to arduino if not the same as previous command
        print(received)



pygame.joystick.quit()
