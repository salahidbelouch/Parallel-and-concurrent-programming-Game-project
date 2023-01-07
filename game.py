import sysv_ipc
import atexit
import random
import multiprocessing as mp
import numpy as np
from shared_memory_dict import SharedMemoryDict
import signal
import os
import sys

key=1



mq=sysv_ipc.MessageQueue(key,sysv_ipc.IPC_CREX)
print("Welcome to Cambiecolo!")

Types=["train","airplane","car","bike" ,"shoes"]
IDs=[]
### Création shared memory 
smd = SharedMemoryDict(name='Offers', size=1024)

             



def handler(sig, frame):
	if sig == signal.SIGBUS:
		print("################## WE HAVE A WINNER ! END OF THE GAME ############## ")
		send_signal(IDs)
		mq.remove()
		smd.shm.close()
		smd.shm.unlink()
		print("########### Libération de ressource #########")
		sys.exit(0)


def send_signal(liste):

	for i in range(len(liste)):
			os.kill(int(liste[i]),signal.SIGBUS)

signal.signal(signal.SIGBUS, handler)


while True:

	m,t=mq.receive(type=1)
	m=m.decode()
	IDs.append(m)
	last,t =mq.receive(type=5)
	mq.send(str(os.getpid()),type=6)


	if last.decode()=='yes':
		random.shuffle(Types)
		Cards=5*Types[:len(IDs)]
		for i in range(len(IDs)):
			mq1=sysv_ipc.MessageQueue(int(IDs[i]))
			Cards_player=Cards[:5]

			del Cards[:5]
			random.shuffle(Cards_player)
			for i in range(5):
				mq1.send(Cards_player[i],type=6)
		break

while True:
	pass ## game just running 
