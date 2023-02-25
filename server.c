/* QuizGame - server.c 
gcc -pthread server.c -o server -lsqlite3 -std=c99 && ./server
*/
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <stdint.h>
#include <sqlite3.h>
#define PORT 2908
#define TCP_QUICKACK  12
#define SERVER_BACKLOG 2 /* maximum value used by most systems*/
int NUMBER_OF_SECONDS=5;
pthread_mutex_t mutex= PTHREAD_MUTEX_INITIALIZER;

typedef struct thData{
	int idThread; /*id-ul thread-ului tinut in evidenta de acest program*/
	int cl; /*Descriptor returned by accept*/
}thData;

/* variadic function for error-checking*/
#define IF_EXIT(c, exit_code, ...)                  \
    {                                               \
        if ((c))                                    \
        {                                           \
            int errsv = errno;                      \
            printf("error at line %d\n", __LINE__); \
            printf(__VA_ARGS__);                    \
            if (errsv != 0)                         \
            {                                       \
                errno = errsv;                      \
                perror("Error");                    \
            }                                       \
            exit(exit_code);                        \
        }                                           \
    }


typedef struct node{  /*each node from the list is going to have info specific to a player*/
    char username[100];
    struct node* next; 
}node;

struct node* root=NULL;
struct player
{
  char* username;
  int score;
};

struct player *players;
int total_players;
int capacity= 100;
int complete;
int timeout;
int number;
void restart(){
    printf("Preparing a new game session...");
    fflush(stdout);
    for(int i=0;i<total_players;i++){
        players[i].score=0;
        free(players[i].username);
        //strcpy(players[i].username,"");
     }
    struct node *curr= (struct node*) malloc(sizeof(struct node));
    curr=root;
     while(curr!=NULL){
        delete_node(curr->username);
        curr=curr->next;
    }
    //free(root);
    //free(curr);
    timeout=0; /*because all the users got the winner, I can start a new session again*/
    complete=0;
    total_players=0;
    capacity=100;
    return(0);
}
void delete_node(char *username2){
    
    struct node *temp=(struct node*) malloc(sizeof(struct node));
    temp=root;
    struct node *prev= (struct node*) malloc(sizeof(struct node));
    prev=NULL;
    if(strcmp(root->username, username2)==0){
        root=temp->next;
        free(temp);
        return;
    }
    else{
        
        while(temp!=NULL && strcmp(temp->username,username2)!=0  ){
            
        
                prev=temp;
                temp=temp->next;
            
         }
         if(temp==NULL)
            return;
        prev->next=temp->next;
        free(temp);
        //free(prev);
    
    }
}

void logout(char* username2){
    //  int pos_delete;        
    //  for(int j=0;j<total_players;j++){
    //      if(strcmp(username2, players[j].username)==0){
    //          pos_delete=j;
    //      }
    //  }
    //  for(int j=pos_delete;j<total_players-1;j++){
    //      strcpy(players[j].username, players[j+1].username);
    //          players[j].score=players[j+1].score;
    //  }
    for(int i=0;i<total_players;i++){
        if(strcmp(username2,players[i].username)==0){
            free(players[i].username);
            players[i].score=0;
        }
    } 
    total_players--;  
    /*delete from the linked list*/  
    delete_node(username2);
}

void insert_end(char* username2){
    struct node* new_node = (struct node*)malloc(sizeof(struct node));
    if(new_node==NULL){
        printf("Out of memory space");
        exit(2);
    }
    new_node->next=NULL;
    strcpy(new_node->username, username2);
    if(root==NULL){
        root=new_node;
    }
    else{
        struct node* curr= root;
        while(curr->next!=NULL){
            curr=curr->next;
        }  
        curr->next=new_node;
    } 
}

int check(char* username2){
    struct node *curr= (struct node*) malloc(sizeof(struct node));
    curr=root;
    
    while(curr!=NULL && strcmp(curr->username, username2)!=0){
        curr=curr->next;
    }
    int success= curr!=NULL;
    if(success)
         return 1;
    else
         return 0;
}
char* get_correct_answer(int i, char* correct_answer){ /*check for every question*/
    sqlite3_stmt *res2;
    sqlite3 *db2;
    char *err_msg = 0;
    int rc2 = sqlite3_open("questions2.db", &db2);
     if (rc2 != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db2));
        sqlite3_close(db2);
    }
    char *sql2 = "SELECT q_id,actual_question,first,second,third,fourth, raspuns_corect  from questions2 WHERE q_id= ?;";
    rc2 = sqlite3_prepare_v2(db2, sql2, -1, &res2, 0); /*compile the sql query*/
    if (rc2 != SQLITE_OK) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db2));
    }
    else{
           sqlite3_bind_int(res2, 1, i); /*choose the i-th row. counting starts from 0 in SQLite dbs*/
    }
    int step2 = sqlite3_step(res2);
    
    if (step2 == SQLITE_ROW) { 
         strcpy(correct_answer, sqlite3_column_text(res2, 6)); /*choose the column that has the correct answer*/
    }
    sqlite3_finalize(res2);
    sqlite3_close(db2);
    return correct_answer;
}

char* get_question(int i, char* question){
   sqlite3_stmt *res2;
    sqlite3 *db2;
    char *err_msg = 0;
    int rc2 = sqlite3_open("questions2.db", &db2);
     if (rc2 != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db2));
        sqlite3_close(db2);
    }
    char *sql2 = "SELECT q_id,actual_question,first,second,third,fourth from questions2 WHERE q_id= ?;";
    rc2 = sqlite3_prepare_v2(db2, sql2, -1, &res2, 0); /*compile the sql query*/
    if (rc2 != SQLITE_OK) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db2));
    }
    else{
           sqlite3_bind_int(res2, 1, i); /*choose the i-th row*/
    }
    int step2 = sqlite3_step(res2);
    
    if (step2 == SQLITE_ROW) { 
         strcpy(question, sqlite3_column_text(res2, 0)); /*choose the column that has the correct answer*/
         strcat(question, " ");
         strcat(question, sqlite3_column_text(res2, 1)); /*choose the column that has the correct answer*/
         strcat(question, " ");
         strcat(question, sqlite3_column_text(res2, 2)); /*choose the column that has the correct answer*/
         strcat(question, " ");
         strcat(question, sqlite3_column_text(res2, 3)); /*choose the column that has the correct answer*/
         strcat(question, " ");
         strcat(question, sqlite3_column_text(res2, 4)); /*choose the column that has the correct answer*/
         strcat(question, " ");
         strcat(question, sqlite3_column_text(res2, 5)); /*choose the column that has the correct answer*/
    }
    
    sqlite3_finalize(res2);
    sqlite3_close(db2);
    return question;
}
void *answer(void* arg ){
    char username[100]="";
    char buf[100]="";
    username[0]='\0';
    buf[0]='\0';
	struct thData tdL; 
	tdL= *((struct thData*)arg);

    

    if(read(tdL.cl, username,sizeof(username))==-1){
         perror("Can't read in server");
    }
    if(strcmp(username, "quit")==0){
        return -1;
    }
 
       
    if(check(username)==0){

        if(write(tdL.cl, "ok",sizeof("ok"))==-1){
         perror("Can't write in server");
        }  
    }
    else{
        strcpy(buf,"Username already taken, please reintroduce");
        if(write(tdL.cl, buf,sizeof(buf))==-1){
         perror("Can't write in server");
        }

        while(check(username)==1){
            username[0]="\0";
            if(read(tdL.cl, username,sizeof(username))==-1){
                perror("Can't read username in server from client");
            }
            if(strcmp("quit", username)==0){
                return -1;
            }
            if(check(username)==0)
            {
                if(write(tdL.cl, "ok",sizeof("ok"))==-1){
                    perror("Can't write username in server from client");
                }
                break;
            }
            else
            {
                if(write(tdL.cl, buf,sizeof(buf))==-1){
                    perror("Can't write username in server from client");
                }
            }
        }
    }
   
    if(total_players+ 1 > capacity)
    {
      capacity = capacity*2;
      struct player * tmp = realloc(players,capacity);
      if (tmp == NULL) 
      {
          return 0;
      }
      players = tmp;
    }
    players[total_players].username=malloc(100);
    pthread_mutex_lock(&mutex);
    strcpy(players[total_players].username,username);
    players[total_players].score=0;
    total_players++;
    insert_end(username); 
    pthread_mutex_unlock(&mutex);
    /*the duration of the lobby 15 seconds*/
    game((struct thData*)arg, username);
   
}
void announce_winner(void *arg){
    struct thData tdL; 
    tdL= *((struct thData*)arg);
    char winner[400]="";
    winner[0]='\0';
    char *maxstring;
    maxstring[0]='\0';
    int max=-2;
    //printf("I arrived here!!!!!!!!!!\n");
    //fflush(stdout);
    /*the last player finished his set of questions*/
        
        for(int i=0;i<total_players;i++){
            if(max<=players[i].score){
                max=players[i].score;
            }
        }
        strcpy(winner,"The biggest obtained score (out of 9) is: ");
        sprintf(maxstring, "%d", max);
        strcat(winner,maxstring);
        strcat(winner, " and the winner(s) is/are: ");
      
        for(int i=0;i<total_players;i++){
            if(max==players[i].score){
                strcat(winner,players[i].username);
                strcat(winner, " ");
            }
        }
        // printf("%s\n", winner);
        // fflush(stdout);
        //pthread_mutex_lock(&mutex);
        if(write(tdL.cl,winner,sizeof(winner))==-1)
        {
            printf("[Thread %d] ",tdL.idThread);
            perror ("[Thread]Eroare la write() catre client.\n");
        } 
        pthread_mutex_lock(&mutex);
        complete++;
        pthread_mutex_unlock(&mutex);
        printf("%s\n","We are getting closer and closer....");
        fflush(stdout);
        //pthread_mutex_unlock(&mutex);

        if(complete<2*total_players){
            
        }
        else{
            printf("%d %d",total_players, complete);
            fflush(stdout);
            restart();
           
        }
        
}

void game(void *arg, char* username2){
    struct thData tdL; 
    tdL= *((struct thData*)arg);
    /*The server is going to send and read answers from the clients*/
    for(int i=1;i<=9;i++){
        /*send question answer from server*/
        char output[400]="", question[400]="";
        question[0]='\0';
        output[0]='\0';
        strcpy(output,get_question(i, question)); /*get the i-th question*/
        
        if(write(tdL.cl, output,sizeof(output))==-1){ /*send the question to the client*/
            perror("Can't write question in server");
        }
        /*get client's response*/
        char answer[400]="";
        answer[0]='\0';
        if(read(tdL.cl, answer,sizeof(answer))==-1){
            perror("Can't read answer in server");
        }
        if(strcmp("quit", answer)==0){
            logout(username2);
            return -1;
        }
        /*get correct answer from the DB*/
        char output2[400]="", correct_answer[400]="";
        correct_answer[0]='\0';
        output2[0]='\0';
        strcpy(output2,get_correct_answer(i, correct_answer));

        /*if the player responded correctly, we add points*/
        if(strcmp(answer,output2)==0){
            /*I have to check to which player I am going to add the points*/
            for(int j=0;j<total_players;j++){
                if(strcmp(username2, players[j].username)==0){
                    pthread_mutex_lock(&mutex);
                    players[j].score++;
                    pthread_mutex_unlock(&mutex);
                }
            }
        }
    }
    pthread_mutex_lock(&mutex);
    complete++;
    timeout=1;
    for(int j=0;j<total_players;j++){
        if(strcmp(username2,players[j].username)==0){
            if(write(tdL.cl, &players[j].score,sizeof(players[j].score))==-1){
                perror("Can't write score in server");
            }
        }
    }
     pthread_mutex_unlock(&mutex); 
    
    while(1){ /*the server is stuck here until all the clients finish the questions*/
        if(complete==total_players){
            printf("%s\n", "Someone is going to get the results very soon...!");
            fflush(stdout); /*now it works correctly*/
            break;
        }
            
    }
    
    
    announce_winner((struct thData*)arg);
   
}


static void* treat(void* arg)
{
    struct thData tdL; 
	tdL= *((struct thData*)arg);	
	printf ("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
	fflush (stdout);		 
	pthread_detach(pthread_self());
    /*timeout determines if the session is still on or not. A session times out if someone responds to all the questions*/
   
    if(timeout==0){
        /*session is on*/
         if(write(tdL.cl, &timeout,sizeof(timeout))==-1){
              perror("Can't write in server");
         }
         if(write(tdL.cl, &NUMBER_OF_SECONDS,sizeof(NUMBER_OF_SECONDS))==-1){
              perror("Can't write in server");
         }
        answer((struct thData*)arg);
    }
    else
    {
         /*session is not on*/
        if(write(tdL.cl, &timeout,sizeof(timeout))==-1){
              perror("Can't write in server");
         }
       
    }
	close ((intptr_t)arg);
	return(NULL); 
}


int main()
{
    players= (struct player*)malloc(capacity);
    struct sockaddr_in server;
    struct sockaddr_in from;
    int sd;	/*socket descriptor*/
    pthread_t th[100];//Thread Id's
    int i=0;
     pthread_mutex_init(&mutex, NULL);
    /* TCP socket */
    IF_EXIT((sd = socket(AF_INET, SOCK_STREAM, 0))==-1,1,"Socket error");

    /*SO_REUSEADDR allows your server to bind to an address which is in a TIME_WAIT state.*/
    int option = 1;
    IF_EXIT(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option))==-1,1,"SO_REUSEADDR failed");
    int one = 1;
    IF_EXIT(setsockopt(sd, IPPROTO_TCP, TCP_QUICKACK, &one, sizeof(one))==-1,1,"TCP QUICKACK failed");
    
     
    /* (re)initializing data strcutures */
    bzero (&server, sizeof (server));
    bzero (&from, sizeof (from));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl (INADDR_ANY);
    server.sin_port = htons (PORT);

   IF_EXIT(bind(sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1, 1, "Can't bind in server.");
    
   IF_EXIT (listen(sd, SERVER_BACKLOG) == -1, 1,"Can't listen in server.");

    

    /* server treats clients concurrently*/
    while(1){


        int client;/*client descriptor*/
        thData * td;   
        int length = sizeof (from);

        printf ("[server]Asteptam la portul %d...\n",PORT);
        fflush (stdout);

        if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0)
	    {
	        perror ("[server]Eroare la accept().\n");
	        continue;
	    }
        printf("Connected!\n");

        td=(struct thData*)malloc(sizeof(struct thData));	
        td->idThread=i++;
        td->cl=client; /*descriptorul clientului*/

        pthread_create(&th[i], NULL, &treat, td); 
    }
        return 0;
    }