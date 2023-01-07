import atexit
import string
import sysv_ipc
import time
import multiprocessing as mp
import numpy as np
import os
from shared_memory_dict import SharedMemoryDict
from multiprocessing import Semaphore, shared_memory
import time
import signal
import sys

print("######################## Welcome to Combiecolo ########################")
print("--------> RULES :")
print("_ The goal is presenting a hand of 5 cards of the same transport means \n_The player who succeeds is awarded the points of the transport they put together \n_You have to exchange with other players 1 to 3 IDENTICAL cards \n_ If you find that you have a hand of 5 identical  cards PRESS 1")





def yes_or_no():
    try:
        txt=input("yes/no ------>")
        if ((txt == "yes") or (txt=="no")):
            return txt
        print("!----- Wrong input ----!")
        return yes_or_no()
    except ValueError:
        return yes_or_no()

def from_1to3():
    try:
        txt=int(input("(1,2 or 3)------>"))
        if((txt>=1) and (txt<=3)):
            return txt
        print("!----- Wrong input ----!")
        return from_1to3()
    except ValueError:
        return from_1to3()


try:
    sem = sysv_ipc.Semaphore(31, sysv_ipc.IPC_CREX)
except sysv_ipc.ExistentialError:
    # One of my peers created the semaphore already
    sem = sysv_ipc.Semaphore(31)
    sem.release()
    # Waiting for that peer to do the first acquire or release
    while not sem.o_time:
        time.sleep(.1)
else:
    # Initializing sem.o_time to nonzero value
    sem.release()
# now the semaphore is safe to use.

def carte_valide(n,carte,liste):
    if ((carte in liste) and (n <= Cards.count(carte))):
        return n*[carte]

    return carte_valide(n,input("Choose a type that you have atleast "+ str(n) + " card(s) of it  ----->"),liste)

def points(carte):
    if carte == "airplane":
        return 5*25
    elif carte == "train":
        return 5*20
    elif carte == "car":
        return 5*15
    elif carte == "bike":
        return 5*10
    return 5*5




def Check_win(liste):

    i=0
    while i<len(liste)-1:
        if liste[i]!=liste[i+1] :
            sem.release()
            return False
        
        i=i+1
    return True

def handler(sig, frame):
    if sig == signal.SIGBUS:
        print("")
        print("################################################################### ")
        print("################## WE HAVE A WINNER! END OF THE GAME ############## ")
        print("################################################################### ")
        mq2.remove()

        try:
            sem.remove()
        except:
            pass
        
        smd.shm.close()

        sys.exit(0)
        
signal.signal(signal.SIGBUS, handler)

######### Initialisation #########
sys.tracebacklimit = 0
mq=sysv_ipc.MessageQueue(1)
pid=os.getpid()
key=pid
mq.send(str(pid),type=1)
print("_are you the last player ? : ")
last=yes_or_no()
mq.send(last,type=5)
if last=="no":
    print('Please wait for the other players ...') 
Pid_G,t=mq.receive(type=6)
mq2=sysv_ipc.MessageQueue(key,sysv_ipc.IPC_CREX)
Cards=[]
smd = SharedMemoryDict('Offers',size=1024)


def check_solicitation():
    try:
            m,t=mq2.receive(block=False,)
            if t==1 :
                print("Someone is interested in your offer of  "+m.decode()+" card(s). If it's still available, Do you want to accepte it? ---->yes/no ") ### our noP si noP c'est que elle est déja prise par quelqu'un d'autre et si no c'est juste qu'il ne veut pas donc la remettre dans la shm
                dec=yes_or_no()
                if dec =="yes":
                    mq2.send("yes",type=5)
                    typeofcard=str(input('Which type of cards you want to offer ----->'))## verif carte
                    offer=carte_valide(int(m.decode()),typeofcard,Cards)
                    print(".... Exchanging Cards ....")
                    for i in range(int(m.decode())):
                        mq2.send(offer[i],type=2)
                        Cards.remove(offer[i])
                    for i in range (int(m.decode())):
                        m,t=mq2.receive(type=3)
                        Cards.append(m.decode())
                    print("...... Exchanged ......")
                    print("________________Here are your cards: ",Cards)
                    try:
                        win=int(input("Check your cards before moving forward if you have 5 similar cards -----> Press 1 if it's the case ;)  "))
                        if win ==1:
                            sem.acquire()
                            if Check_win(Cards):
                                print("##############################################################################################")
                                print("################ YOU WON ! You got "+str(points(Cards[0]))+" Points !!! ####################")
                                print("##############################################################################################")
                                os.kill(int(Pid_G),signal.SIGBUS)
                                time.sleep(5)
                                sem.release()
                            else:
                                print("________STOP Lying, you are not a winner yet !!!!________ ")
                                sem.release()
                    except:
                        pass

                    

                else:
                    mq2.send("no",type=5)
                    print("Offer rejected")
                    Attente= input("/!/Important/!/ : The offer of  " + m.decode() + " card(s) that you rejected is no longer in the current offers, if needed you can make it again. ")
                    print("________________Here are your cards: ",Cards)
    except sysv_ipc.BusyError:
        print(" No request for you for now :(")


while True:
    if len(Cards)<5:
        m1,t=mq2.receive(type=6)
        m1=m1.decode()
        Cards.append(m1)
    else:
        print("############################### STARTING GAME ##################################")
        print("________________Here are your cards: ",Cards)
        break




while True:


        print("_______________Here are the offers",smd.keys())     
        print('Please make an offer :')
        Make= from_1to3()
        if (Make in smd.keys()): 
            if smd[Make]!= pid:
                print("There 's already an offer of:", Make, "card(s)")
                print("Want to take it ? You have to be quick ;)")
                Decision=yes_or_no()
                if Decision=="yes":
                    if (Make in smd.keys()):
                        sem.acquire()
                        mq=sysv_ipc.MessageQueue(smd[Make])
                        del smd[Make] 
                        print("####### Offer Secured #####")
                        sem.release()
                        Attente=input("_______________Press enter to check if someone is requesting you ______________")
                        check_solicitation()
                        mq.send(str(Make),type=1)
                        print("####### The Request of "+str(Make)+" cards(s) you Secured before is sent now #######")
                        m,t=mq.receive(type=5)
                        if m.decode()=="yes":
                            print("######## Request accepted #########")
                            print("....... Exchanging cards ....... ")
                            typeofcard=str(input("which type of card do you want to exchange in return -------->")) #####VERIF DU noMBRE
                            offer=carte_valide(Make,typeofcard,Cards)
                            for i in range (Make):
                                print(".", end = '')
                                m,t=mq.receive(type=2)
                                Cards.append(m.decode())
                            for i in range (Make):
                                print(".", end = '')
                                mq.send(offer[i],type=3)
                                Cards.remove(offer[i])
                            print("..... Exchanged ....")
                            print("___________________here are your cards: ",Cards)
                            try:
                                win=int(input("Check your cards before moving forward if you have 5 similar cards -----> Press 1 if it's the case ;)  "))
                                if win ==1:
                                    sem.acquire()
                                    if Check_win(Cards):
                                        print("##############################################################################################")
                                        print("################### YOU WON ! You got "+str(points(Cards[0]))+" Points !!! ###################")
                                        print("##############################################################################################")
                                        os.kill(int(Pid_G),signal.SIGBUS)
                                        time.sleep(5)
                                        sem.release()
                                    else:
                                        sem.release()
                            except:
                                pass
                        
                        elif m.decode()== "no":
                            print("Demande refusée")
                            print("________________here are your cards: ",Cards)
   
                    else:
                        print("pas assez rapide pour la saisir :'(")   
                        print("Here are your cards: ",Cards)  
            else: 
                Attente =input("This offer is yours : wait for someone to take it :  _______ Press Enter to continue ____  ")       
        else:
            sem.acquire()
            if Make in smd.keys():
                print("offre deja proposé par un autre joueur.")
                sem.release()
            else:
                smd[Make]=key
                sem.release()
                print("######## Offer Made ##########")
                print("________________Here are the offers",smd.keys())
                Attente=input("Look around you if there's someone interested: ----- Press enter to continue -----")

        
        check_solicitation()

        
            


  


            

        