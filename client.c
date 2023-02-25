/* QuizGame - client.c
gcc -pthread client.c -o client -lsqlite3 -std=c99 && ./client 
*/

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
#include <sqlite3.h>
#include <assert.h>
#include <sys/time.h>

/*port used to connect to the server*/
#define PORT 2908

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



int main(int argc, char *argv[])
{   
    /*The clien't won't be able to kill the process via signals, he can do that only if he writes quit*/
    struct sigaction act;
    act.sa_handler = SIG_IGN;
    for(int i = 1 ; i < 65 ; i++) {
        // 9 and 19 cannot be caught or ignored                                                                                                       
        // 32 and 33 do not exist                                                                                                                     
        if((i != SIGKILL) && (i != SIGSTOP) && (i != 32) && (i != 33)) {
            assert(sigaction(i, &act, NULL) == 0);
        }
    }
    
    int sd;			
    struct sockaddr_in server;
 
    IF_EXIT((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1, 1, "Can't create socket in client.");

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); /*IP address of server */
    server.sin_port = htons(PORT);
    
    IF_EXIT (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1, 1, "Can't connect at server");
    char username[100]="", control_answer[100]="";
    
    /*username check*/
    username[0]='\0';
    control_answer[0]='\0';
    int timeout;/*determines if the session is still on or not*/
    if(read(sd, &timeout,sizeof(timeout))==-1){
        perror("Can't read timeout in client");
        fflush(stdout);
    }
    if(timeout==0){
        printf("The session is still on for me!\n");
        printf("If I want to quit, I will have to write quit in the terminal\n");
        fflush(stdout);
    }
    else{
        printf("I can't play this game right now because someone finished answering. Hopefully I will come back later!\n");
        return(0);
    }
    int number_of_seconds; /*The player is going to know how many seconds he has allocated for answering*/
    if(read(sd, &number_of_seconds,sizeof(number_of_seconds))==-1){
        perror("Can't read control_answer in client");
        fflush(stdout);
    }
    printf("I have to answer the questions in %d seconds/question!, but first I must login (one word only)\n", number_of_seconds);
    sleep(1);
    
    scanf("%s",username);
    if(write(sd, username,sizeof(username))==-1){
        perror("Can't write username in client");
    }
    if(strcmp("quit", username)==0){
        return 0;
    }

    if(read(sd, control_answer,sizeof(control_answer))==-1){
          perror("Can't read control_answer in client");
    }
    
    if(strcmp(control_answer,"Username already taken, please reintroduce")!=0){
        
        printf("Username ok...\n");
        fflush(stdout);
    }
    else{
        while(strcmp(control_answer,"Username already taken, please reintroduce")==0){
        printf("%s \n", control_answer);
        fflush(stdout);
        username[0]='\0';
        scanf("%s", username);
        if(write(sd, username,sizeof(username))==-1){
            perror("Can't write username in client");
        }
        if(strcmp("quit", username)==0){
            return 0;
        }
        control_answer[0]="\0";
        if(read(sd, control_answer,sizeof(control_answer))==-1){
          perror("Can't read control_answer in client");
        }
        }  
    }
    sleep(1);

    /*play_game or quit anytime*/
    printf("%s","The game is about to start. Correct answer format: 1 lowercase letter \n");
    fflush(stdout); 
    for(int i=1;i<=9;i++){
        char question[400]="";
        question[0]='\0';
        if(read(sd, question,sizeof(question))==-1){
            perror("Can't read question in client");
        }
        printf("%s \n", question);
        fflush(stdout);
        
        /*Let the user give input only for number_of_seconds seconds*/
        char answer[400]="";
        answer[0]='\0';
        struct timeval tmo;
        fd_set readfds;

        FD_ZERO(&readfds);
        FD_SET(0, &readfds);
        tmo.tv_sec = number_of_seconds;
        tmo.tv_usec = 0;

         int retval;
        retval = select(1, &readfds, NULL, NULL, &tmo);
        if (retval == -1)
            perror("select()");
        else if (retval){
            scanf("%s", answer);
            if (strcmp(answer,"")!=0){
                if(write(sd, answer,sizeof(answer))==-1){
                    perror("Can't write answer in client");
                }
                if(strcmp("quit", answer)==0){
                    return 0;
                }
            }
        }
        else
        {
            printf("No answer within %d seconds.\n", number_of_seconds);
            fflush(stdout);
            strcpy(answer,"No answer");
            if(write(sd, answer,sizeof(answer))==-1){
                    perror("Can't write answer in client");
            }
        }
            
    }     
    int score=0;
    /*Reading my own score*/
    if(read(sd, &score,sizeof(score))==-1){
            perror("Can't read answer in client");
    }
    printf("My own score is %d \n", score);
    fflush(stdout);

    char final_results[400]="";
    final_results[0]='\0';
    
    printf("Now I am waiting for everybody to finish so I can get the final results... \n");
    if(read(sd, final_results,sizeof(final_results))==-1){
        perror("Can't read answer in client");
    }
    printf("%s\n", final_results);

    // printf("Congrats! Please press any key if you want to quit \n");
    // char final_quit[400]="";
    // final_quit[0]='\0';
    // scanf("%s", final_quit);
    // if(write(sd, final_quit,sizeof(final_quit))==-1){
    //     perror("Can't read answer in client");
    // }
    close (sd);
    return 0;
}