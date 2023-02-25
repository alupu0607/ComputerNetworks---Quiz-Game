#include <sqlite3.h>
#include <stdio.h>
/*gcc QuestionsDB.c -o QuestionsDB -lsqlite3 -std=c99 && ./QuestionsDB*/
int main(void) {
    
    sqlite3 *db;
    char *err_msg = 0;
    
    int rc = sqlite3_open("questions2.db", &db);
    
    if (rc != SQLITE_OK) {
        
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        
        return 1;
    }
    
    char *sql ="DROP TABLE IF EXISTS questions2;"
               "CREATE TABLE questions2(q_id INT, actual_question TEXT, first TEXT, second Text, third TEXT, fourth TEXT, raspuns_corect TEXT);"
               "INSERT INTO questions2 VALUES(1,'What is 1+1?', 'a. 2','b. 3', 'c. 4', 'd. 5', 'a');"
               "INSERT INTO questions2 VALUES(2,'Which is the capital of Romania?', 'a. Bucharest','b. Tecuci', 'c. Paris', 'd. New York', 'a');"
               "INSERT INTO questions2 VALUES(3,'What is red+white?', 'a. purple','b. grey', 'c. pink', 'd. green', 'c');"
               "INSERT INTO questions2 VALUES(4,'When did WW1 start in Romania?', 'a. 2002','b. 1978', 'c. 1916', 'd. 1914', 'c');"
               "INSERT INTO questions2 VALUES(5,'Where is Moscow?', 'a. Russia','b. America', 'c. India', 'd. Singapore', 'a');"
               "INSERT INTO questions2 VALUES(6,'What is 4/2+7 ?', 'a. 6','b. 9', 'c. 7', 'd. 8', 'b');"
               "INSERT INTO questions2 VALUES(7,'Which one is spicy?', 'a. potato','b. hot pepper', 'c. apple', 'd. camel', 'b');"
               "INSERT INTO questions2 VALUES(8,'Which one is a plant?', 'a. tulip','b. dog', 'c. apple', 'd. camel', 'a');"
               "INSERT INTO questions2 VALUES(9,'Which one is a dog breed?', 'a. Blue Whale','b. Pomeranian', 'c. Pony', 'd. Black Mamba', 'b');";
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    
    if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "SQL error: %s\n", err_msg);
        
        sqlite3_free(err_msg);        
        sqlite3_close(db);
        
        return 1;
    } 
    
    sqlite3_close(db);
    
    return 0;
}


