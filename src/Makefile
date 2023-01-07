all:  serveur1-Lonely serveur2-Lonely serveur3-Lonely

serveur1-Lonely: serveur1-Lonely.o
		gcc serveur1-Lonely.o -o serveur1-Lonely

serveur1-Lonely.o: serveur1-Lonely.c 
		gcc -c serveur1-Lonely.c -o serveur1-Lonely.o

serveur2-Lonely: serveur2-Lonely.o
		gcc serveur2-Lonely.o -o serveur2-Lonely

serveur2-Lonely.o: serveur2-Lonely.c 
		gcc -c serveur2-Lonely.c -o serveur2-Lonely.o

serveur3-Lonely: serveur3-Lonely.o
		gcc serveur3-Lonely.o -o serveur3-Lonely

serveur3-Lonely.o: serveur3-Lonely.c 
		gcc -c serveur3-Lonely.c -o serveur3-Lonely.o


clean: 
		rm -f serveur1-Lonely *.oS
		rm -f serveur2-Lonely *.oS
		rm -f serveur3-Lonely *.oS

