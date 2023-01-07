
// Server side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
	
#define PORT	 8080
#define MAXLINE 1494

//padding n_seg 000000
char * padding(int p) {
    char *str = malloc(1500);

    sprintf(str, "%06d", p);
         
    return str;
}






//bytes
char *concat(char const*str1, char const*str2, int count) {
        size_t const l1 = 6 ;
        size_t const l2 = count ;

        char *result = malloc(l1 + l2 + 1);
        if(!result) return result;
        memcpy(result, str1, l1) ;
        memcpy(result + l1, str2, l2 + 1);
        return result;
    }

int readFile(char* nameFile){
	char buffer[1494]; // Buffer to store data
  	FILE * stream;
  	stream = fopen("random.txt", "r");
	//il faut connaire la taiille total en octet pour gerer le dernier ?? 
	do{
		int count = fread(&buffer, 1494, 1, stream);
		//printf("Data read from file: %s \n", buffer);
		//printf(" data %d",count);
	}while (!(feof(stream)));
	fclose(stream);
	
  	return 0;

}

int lostCheck(int fd, int us, int s){
		struct timeval timeout;
		fd_set set;
		timeout.tv_sec = s;
  		timeout.tv_usec =us;
    	FD_ZERO(&set);
    	FD_SET(fd, &set);
    	int retour =select(fd+1, &set, NULL, NULL, &timeout);
		return retour;
	}
	


int nouvelleCO(int port){
	fd_set read_fds;
	FD_ZERO(&read_fds);
	int it=0;
	//char buffer[MAXLINE];
	char fileName[MAXLINE];
	int comp;
	int n;
	socklen_t len= sizeof(struct sockaddr);
	int newsockfd;
	struct sockaddr_in servaddr, cliaddr;
	int index =0;
	int count =0;
		
	// Creating socket file descriptor
	if ( (newsockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
		
	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));
		
	// Filling server information
	servaddr.sin_family = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(port);
		
	// Bind the socket with the server address
	 bind(newsockfd, (const struct sockaddr *)&servaddr,sizeof(servaddr));

		n = recvfrom(newsockfd, (char *)fileName, MAXLINE,
				MSG_WAITALL, ( struct sockaddr *) &cliaddr,
				&len);
		//senfFile
		fileName[n] = '\0';
		//printf("sending file %s\n",fileName);
		 // Buffer to store data
  		FILE * stream;
  		stream = fopen(fileName, "r");
		//il faut connaire la taiille total en octet pour gerer le dernier ?? 
		int n_seg =1;
		int tailleF = 4; 
		int notAcked =1;
		int windowSize=10;
		int lastAck=0;
		int ACkdup=0;
		int datasize=200;
		char *Data[datasize][1500];//buffer contenant les messages envoyé
		int lastseg;
		int it2=0;
		int last =0;
		int dejadup=-1;
		int dupmax=2;
		int lastresent=-1;
		//printf("Start TIMER ______\n");
		struct timeval start, end;
		//gettimeofday(&start, NULL);
		while ((notAcked>0)||(!(feof(stream)))){ //LIRE ENVOYER ATTENDRE ACK



			int lost;
			char *seg;
			char *toSend;
			FD_SET(newsockfd,&read_fds);
			
			if (windowSize>0){
				if (it2==0){
					notAcked=0;
					it2=1;
				}
				//lire du fichier dans Buffer
				index = (n_seg-1)%(datasize);

				// on rentre quand on a quelque chose à ecrire quand on a pas de rentrasmissionn et quand la retranmission arrive à index
				////printf("WINDOW SIZE ------------------------ %d notack %d ,  lastack : %d , dejadup : %d   \n",windowSize,notAcked, lastAck,dejadup);
				if ((!(feof(stream)))){
					char buffer[MAXLINE];
					memset(buffer,0,MAXLINE);
					memset(&Data[index][0],0,1500);

					
					count = fread(buffer, 1, 1494, stream);
					//Garder le segment: pour le segment x est placé en x-1 car le tableau commence à 0
					//

					////printf("j'ai lu : %s \n",buffer);

					// padding et ajout au segment 
					seg=padding(n_seg);
					
					//memset(toSend,0,1500);

					toSend=concat(seg,buffer,count);
					
					// garder tosend dans les msg ENV
					memcpy( &Data[index][0], toSend,count+6);

					//printf("j'ai lu : %d pour le seg %d  indexé %d :\n",count,n_seg,index);
					
					
					lastseg=n_seg;
					n_seg=n_seg+1;
					//usleep(500); // taille de la fenetre  et le delai du deuxieme select // ackdupmax 

					// envoyer segment et n_ seg:
					int sent =sendto(newsockfd, &Data[index][0], count+6,
						0, (struct sockaddr *) &cliaddr,
						len);
					
					windowSize= windowSize-1;
					notAcked=notAcked+1;
					
					//printf("sent %d \n",sent);
					//printf("WINDOW SIZE ------------------------ %d notack %d \n",windowSize,notAcked);
				} else{
					//to get more acks : 
					//windowSize=0;
					last=1;
					dupmax=1;

				}

				

				//window -1
				//usleep(10000);
				
				//SI notACK >0!!

				// Recevoir ack :
				//lost = lostCheck(newsockfd,0,0);
				if (notAcked>0){

					struct timeval timeout_zero;
                	timeout_zero.tv_sec = 0;
                	timeout_zero.tv_usec = 100;
				
                	int res =select(newsockfd + 1 , &read_fds , NULL , NULL , &timeout_zero);
					//printf(" res %d\n", res);
                	if(FD_ISSET(newsockfd,&read_fds)){
            
						char buffer[MAXLINE];
						// recevoir ACK :
						//printf("Attend ACK \n");
						n = recvfrom(newsockfd, (char *)buffer, MAXLINE,
							0, ( struct sockaddr *) &cliaddr,
							&len);
						buffer[n] = '\0';
						//printf("Client : %s\n", buffer);


						// Extract n_seg 
						char Nseg[7];
    					memcpy( Nseg, &buffer[4], 6 );
    					Nseg[6] = '\0';

						if (it==0){
							//printf("c'est le premier ack \n");
							lastAck=atoi(Nseg);
							notAcked=notAcked-lastAck;
							windowSize=windowSize+lastAck;
							it=it+1;
						}else{
							if((atoi(Nseg)==lastAck)&&(dejadup!=lastAck)){
								ACkdup = ACkdup+1;
								
						 		//printf("ACK dupliqué %d fois : %d \n", atoi(Nseg), ACkdup);
								if (ACkdup ==dupmax){ // recevoir plusieurs ACKdup
									
									ACkdup=0;
									//printf("Retransmettre  index dddd %d\n", lastAck+1);
									//break;
									//usleep(100);
									int i ;
									for(i=lastAck;i<lastseg;i++){
										int sent =sendto(newsockfd, &Data[(i)%(datasize)][0], 1500,
										0, (struct sockaddr *) &cliaddr,
										len);

									}
									//printf("retransmi: %s \n",&Data[(lastAck)%(datasize)][0]);;

									// on garde en memoire le dernier dup et envoyé : 
									dejadup=lastAck;
								
								}	
								//highest

							}else if(lastAck<(atoi(Nseg))){
								notAcked=notAcked-(atoi(Nseg)-lastAck);
								windowSize= windowSize+(atoi(Nseg)-lastAck);
								//printf("***Window augmentée de ***  %d\n",(atoi(Nseg)-lastAck));
								lastAck=atoi(Nseg);
								//printf("bien recu jusqu'à %d\n",lastAck);
								//remettre la window size et not acked 

							

							
								// augmenter changer seg
							}
						}

					} 

				}
			}	
			if ((windowSize==0)||(last==1)){
					//!!!!! ajouter une fonction check finish

					//window zero attends ACK 
					//while (1){

					//printf("window size EMPTYYYYYYYY\n");
					struct timeval timeout;
                	timeout.tv_sec = 0;
                	timeout.tv_usec = 1600;//1500 c'est mieux
					//usleep(100);
					
                	int res =select(newsockfd + 1 , &read_fds , NULL , NULL , &timeout);
					//printf(" res %d \n",res);

					if(FD_ISSET(newsockfd,&read_fds)){
						char buffer[MAXLINE];
						//printf("Attend ACK \n");
						n = recvfrom(newsockfd, (char *)buffer, MAXLINE,
							MSG_WAITALL, ( struct sockaddr *) &cliaddr,
							&len);
						buffer[n] = '\0';
						//printf("Client : %s\n", buffer);

						char Nseg[7];
    					memcpy( Nseg, &buffer[4], 6 );
    					Nseg[6] = '\0';
						//AJOUTER CONDITION !!!!! vers la fin o,n parle polus de dup 
						if (it==0){
							//printf("c'est le premier ack \n");
							lastAck=atoi(Nseg);
							notAcked=notAcked-lastAck;
							windowSize= windowSize+lastAck;
							it=it+1;
						}else{
							if((atoi(Nseg)==lastAck)&&(dejadup!=lastAck)){
								ACkdup = ACkdup+1;

						 		//printf("ACK dupliqué2  %d  fous : %d \n", atoi(Nseg),ACkdup);
								if (ACkdup ==dupmax){ // recevoir plusieurs ACKdup

									ACkdup=0;
									//printf("Retransmettre  index dddd %d\n", lastAck+1);
									//usleep(100);
									int i ;
									for(i=lastAck;i<lastseg;i++){
										int sent =sendto(newsockfd, &Data[(i)%(datasize)][0], 1500,
										0, (struct sockaddr *) &cliaddr,
										len);

									}
									//printf("retransmi: %s \n",&Data[(lastAck)%(datasize)][0]);

									// on garde en memoire le dernier dup et envoyé : 
									dejadup=lastAck;
								}

							}else if(lastAck<(atoi(Nseg))){
								notAcked=notAcked-(atoi(Nseg)-lastAck);
								
								windowSize= windowSize+(atoi(Nseg)-lastAck);
								//printf("***Window augmentée de ***  %d\n",(atoi(Nseg)-lastAck));
								lastAck=atoi(Nseg);
								//printf("bien recu jusqu'à %d\n",lastAck);
								//remettre la window size et not acked 
								
								
								// augmenter changer seg
							}	
						}


					}else {
						
							//prmintf("Retransmettre Trop attendu index send !! %d\n", lastAck+1);
							//if (lastresent != dejadup){ //IMPORTANT il faut gerer meme paquet supprimé 
								int sent =sendto(newsockfd, &Data[(lastAck)%(datasize)][0], 1500,
									0, (struct sockaddr *) &cliaddr,
									len);
								//printf("retransmi: %s \n",&Data[(lastAck)%(datasize)][0]);
								lastresent = lastAck;
							//}
					}

						
						


				}
				//}

			


			
		}

		// fin du fichier envoyer fin 

		//gettimeofday(&end, NULL);
		//long seconds = (end.tv_sec - start.tv_sec);
		//long micros = ((seconds * 1000000) + end.tv_usec) - (start.tv_usec);


		//printf("time s is : %d -----\n", seconds);

		//printf("time us is : %d ------  \n", micros);

		sendto(newsockfd, (const char *)"FIN", strlen("FIN"),
			MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
			len);
		//printf("sent fin \n");

		//printf("Client FIN");


		fclose(stream);

  	return 0;

	

}

int handleCO(int sockfd, struct sockaddr_in srvaddr){ 
	struct sockaddr_in cliaddr;
	char buffer[MAXLINE];
	int comp;
	int n;
	socklen_t len= sizeof(struct sockaddr);
	int port = 1495;
	char *SYNACK = "SYN-ACK1495";
	len = sizeof(cliaddr); //len is value/result
	
	n = recvfrom(sockfd, (char *)buffer, MAXLINE,
				MSG_WAITALL, ( struct sockaddr *) &cliaddr,
				&len);
	buffer[n] = '\0';
	printf("Client : %s\n", buffer);
	comp=strcmp(buffer,"SYN"); 
	if (comp==0){
		sendto(sockfd, (const char *)SYNACK, strlen(SYNACK),
		MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
			len);
		printf("SYN-ACK message sent.\n");
		// recevoir ACK :
		n = recvfrom(sockfd, (char *)buffer, MAXLINE,
				MSG_WAITALL, ( struct sockaddr *) &cliaddr,
				&len);
		buffer[n] = '\0';
		printf("Client : %s\n", buffer);
		int comp=strcmp(buffer,"ACK"); 
		fork();
		if (comp ==0){
			nouvelleCO(port);
		}

	}
	close(sockfd);
	return 1;

}
	
// Driver code
int main(int argc, char **argv) {
	int sockfd;
	struct sockaddr_in servaddr, cliaddr;
		
	// Creating socket file descriptor
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
		
	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));
		
	// Filling server information
	servaddr.sin_family = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = INADDR_ANY; 
	servaddr.sin_port = htons(atoi(argv[1]));

	int reuse= 1 ;
	setsockopt(sockfd, SOL_SOCKET,SO_REUSEADDR,&reuse, sizeof(reuse)) ;
		
	// Bind the socket with the server address
	if ( bind(sockfd, (const struct sockaddr *)&servaddr,
			sizeof(servaddr)) < 0 )
	{
		perror("bind failed A"); 
		exit(EXIT_FAILURE);
	}
	handleCO(sockfd,servaddr);
	return 0;
}
