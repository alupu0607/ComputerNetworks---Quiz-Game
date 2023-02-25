# ComputerNetworks---Quiz-Game

## ABOUT
Multithreaded program in C, based on TCP protocol. It also uses a SQLite3 database to store the questions.\
The clients have to register with different names and answer every question in 5 seconds. After every client finishes the game, the winner(s) is/are announced. The client may leave the game at any given moment only by typing "quit" in the terminal (CTRL+C signal is blocked) without interrupting the game session.

More about the project [ROMANIAN]:\
[Tema2_Retele_Lupu_Andreea_Daniela_2A6-4.pdf](https://github.com/alupu0607/ComputerNetworks---Quiz-Game/files/10830903/Tema2_Retele_Lupu_Andreea_Daniela_2A6-4.pdf)

## How to start the game
**1.** Open a terminal and copy/paste the following: \
gcc QuestionsDB.c -o QuestionsDB -lsqlite3 -std=c99 && ./QuestionsDB \
gcc -pthread server.c -o server -lsqlite3 -std=c99 && ./server \
**2.** Open as many terminals you want and copy/paste the following: \
gcc -pthread client.c -o client -lsqlite3 -std=c99 && ./client 

### Future improvements/ vulnerabilities/ bugs
The timer for the client's input should have been written in server.c




