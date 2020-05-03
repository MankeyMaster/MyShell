//Zachary Hale 02/24/2020 CS240 ass2 Shell
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <termios.h>
//How many rows in an array

struct alias{

char* alias;
char* cmd;
struct alias* next;

};

int rmNode(char** term, struct alias* head);
char* search(char** term, struct alias* head);
void insert(char** aliKey, struct alias* head);
void setArray(char*** arr);
//Memory requilishment
void freeArray(char*** arr);
//Parse command with return of double char pointer
int parseCmd(char***, char history[100][25], int i);
void parseCmdList(char*** cmds, char* list);
//Main entrypoint
int main(){

//The pid of the child and parent process
pid_t cmdPid = -2, wpid = -2;
//Allocating temporary memory for ops.
char **ops = (char**)malloc(10 * sizeof(char*));
int x = 0;
for(x = 0; x < 10; x++){
ops[x] = (char*)malloc( 25 * sizeof(char));
}
char temp[25];
struct alias* head = (struct alias*)malloc(sizeof(struct alias));
head->next = NULL;
head->alias = NULL;
head->cmd = NULL;
//location of the command
char loc[25] = "/bin/";
//current wait status
int status = 0, l = 0;
//command list
char **cmdList = (char**)malloc( 10 * sizeof(char*));
char *path = getenv("PATH");
for(x = 0; x < 10; x++){

cmdList[x] = (char*)malloc(25 * sizeof(char));
cmdList[x] = "\0";

}

char history[100][25];
int flagg = 0;
for(x = 0; x < 100; x++){

history[x][0] = '\0';

}

FILE *ls_proc, *grep_proc, *MsRc;
//Read rc file
char rcFile[25][25];
MsRc = fopen("mshrc", "r");
char line[25] = "\0";
int i = 0;
x = 0;
while(fgets(line, sizeof(line), MsRc)){

        while(line[x] != '\n'){
        x++;
        }
        strcpy(rcFile[i], line);
        cmdList[i] = rcFile[i];
        cmdList[i][x] = '\0';
        x = 0;
        i++;

}

fclose(MsRc);

int it = 0, v = 0, flag = 1;
i = 0;
//while the command does not equal exit
while(strcmp(ops[0], "exit") != 0){
//If the child process returns -1 then it is failed
if(cmdPid == -1){
fprintf(stderr, "Fork failed, we'll get 'em next time");
} else if(cmdPid == 0){
//If the execvp returns less than 0 then the exec failed
if(execvp(loc, ops) < 0){
printf("execution error\n");
exit(1);
}
exit(0);
}
//parse commands into an array
if(cmdList[i] == "\0"){
setArray(&ops);
l = parseCmd(&cmdList, history, it);
i = 0;
//Log History
strcpy(history[it], cmdList[0]);
it++;
//Temp variable for strtok
strcpy(temp, cmdList[0]);
//Check if it is export or new Path
if(strcmp(strtok(temp, " "),"New") == 0 || strcmp(strtok(temp," "),"export") == 0){
x = 0;
while(cmdList[0][x] != '\0'){
if(flagg == 1)strncat(path, &cmdList[0][x], 1);
if(cmdList[0][x] == '+' || cmdList[0][x] == ':'){
flagg = 1;
}
x++;
}
printf("%s\n",path);
ops[0] = "echo";
flagg = 0;
//Check if it is a pipe command
}else if(l == 1){
strcpy(temp, strtok(cmdList[0], "|"));
ls_proc = popen(temp, "r");
strcpy(temp, strtok(NULL, "|"));
grep_proc = popen(temp, "w");

char buffer[128];

while (fgets(buffer, 128, ls_proc) != NULL)
    fprintf(grep_proc, "%s", buffer);
pclose(ls_proc);
pclose(grep_proc);
ops[0] = "echo";
l = 0;
//Check if it is a history call
}else if(strcmp(cmdList[0], "history") == 0){
if(it >= 20) v = it-20;
while(history[v][0] != '\0'){
printf("%d: %s\n", v, history[v]);
v++;
}
v = 0;
ops[0] = "echo";

//Check if it is a command to use the previous command
}else if(strcmp(cmdList[0], "!!") == 0){

cmdList[0] = history[it-2];
strcpy(history[it-1], history[it-2]);
if(strcmp(cmdList[0], "history") == 0){
if(it >= 20) v = it-20;
while(history[v][0] != '\0'){
printf("%d: %s\n", v, history[v]);
v++;
}
v = 0;
ops[0] = "echo";
} else {

parseCmdList(&ops, cmdList[0]);

}

//Check if it is a specific previous command
}else if(cmdList[0][0] == '!'){
i = 1;
v = 0;
while(cmdList[0][i] != '\0'){
temp[v] = cmdList[0][i];
v++;
i++;
}
temp[v] = '\0';
i = atoi(temp);
if(history[i][0] != '\0'){
strcpy(history[it-1], history[i]);
cmdList[0] = history[i];
if(strcmp(cmdList[0], "history") == 0){
if(it >= 20) v = it-20;
while(history[v][0] != '\0'){
printf("%d: %s\n", v, history[v]);
v++;
}
v = 0;
ops[0] = "echo";
} else {

parseCmdList(&ops, cmdList[0]);

}
} else{
printf("Outside History\n");
ops[0] = "echo";
}
//Check if it is an alias command
}else if(strcmp(strtok(temp, " "), "alias") == 0){
insert(&(cmdList[0]), head);
ops[0] = "echo";
//Check if it is an unalias command
}else if(strcmp(strtok(temp, " "), "unalias") == 0){
rmNode(&(cmdList[0]), head);
ops[0] = "echo";
//Just a normal command
}else{
cmdList[0] = search(&(cmdList[0]), head);
parseCmdList(&ops, cmdList[i]);
}
i++;
} else {
strcpy(history[it], cmdList[i]);
it++;
if(flag == 0) cmdList[i] =  search(&(cmdList[i]), head);
parseCmdList(&ops, cmdList[i]);
flag = 0;
i++;
}

loc[5] = '\0';
loc[6] = '\0';
loc[7] = '\0';
loc[8] = '\0';
loc[9] = '\0';
//put in the location of the operators
strcat(loc, ops[0]);
//Fork command

cmdPid = fork();

while((wpid = wait(&status)) > 0);

}
return 1;
}

void parseCmdList(char*** cmd ,char *List){
int b = 0;
//strtok and interate through the double array
(*cmd)[b] = strtok(List, " ");
b++;

while((*cmd)[b] != NULL)
{
(*cmd)[b] = strtok(NULL, " ");
b++;
}

}

//Function definition returns whether or not it is a pipe command
int parseCmd(char*** str, char history[100][25], int i){
//char pointer and allocating memory
char* stri = (char*)malloc(20*sizeof(char));

int b = 0, x = i, flag = 0, ret = 0;
//enable raw input
system("/bin/stty raw");
//disable auto echo of getchar
struct termios t;
tcgetattr(STDIN_FILENO, &t);
t.c_lflag &= ~ECHO;
tcsetattr(STDIN_FILENO, TCSANOW, &t);

printf("?:");
//Get char loop
while((stri[b] = getchar()) != 13){
//Backspace
if(stri[b] == 127 && b != 0){

printf("\b");
printf(" ");
printf("\b");
b--;
//Escape sequence to check for arrow keys
}else if(stri[b] == 27){
//Throw away one entered key
getchar();
//Switch for what arrow key is pressed.
switch(getchar()){
case 'A':
if(x != 0){
x--;
printf("\r?:%s                                ",history[x]);
} else{
printf("\r?:%s                                ",history[x]);
}
flag = 1;
break;
case 'B':
if(x < i){
x++;
printf("\r?:%s            ",history[x]);
}else{
printf("\r?:%s            ",history[x]);
}
flag = 1;
break;

default:
break;
}

}else if(stri[b] != 127 && stri[b] != 3){
//If flag is true then arrow key is not pressed and command is being inputted normally
if(flag == 1){
printf("\r?:    \b\b\b\b");
flag = 0;
}
//If it is a pipe command ret = 1;
if(stri[b] == '|')ret=1;
printf("%c", stri[b]);
b++;

}
}
//adds a null terminating if the flag is unset, flag is set when it is an arrowkey, otherwise set it to the history
if(flag == 0){
stri[b] = '\0';
}else {
stri = history[x];
}
//Flush standard out
fflush(stdout);
//clear and return
printf("\r\n");
//Reset echo on stdin
t.c_lflag |= ECHO;
tcsetattr(STDIN_FILENO, TCSANOW, &t);
//set stdin to unraw.
system ("/bin/stty cooked");

b = 0;
//strtok and interate through the double array
(*str)[b] = strtok(stri, ";");
b++;
while((*str)[b] != NULL){
(*str)[b] = strtok(NULL, ";");
if((*str)[b] == NULL){
(*str)[b] = "\0";
}
b++;
}

return ret;
}


//Reset all characters to null
void setArray(char *** arr){

int i = 0, x = 0;
for(i = 0; i < 10; i++){
(*arr)[i] = "\0";
}

}

//Free array memory
void freeArray(char *** arr){
int i = 0;
for(i=0; i < 10; ++i){
free(arr[i]);
}
free(arr);
}


//Remove a node from the alias linked list
int rmNode(char** term, struct alias* head){

struct alias* it = head;
struct alias* tmp = NULL;
strtok(*term, " ");
*term = strtok(NULL, "\0");
while(it->next != NULL){

if(strcmp(it->next->alias, *term) == 0){

tmp = it->next;
it->next = it->next->next;
free(tmp);
return 1;
}
it = it->next;
}

return 0;
}

//Search through nodes to see if there is an alias matching the inputted command
char* search(char** term, struct alias* head){

struct alias* it = head->next;
while(it != NULL){

if(strcmp(it->alias, *term) == 0){

return it->cmd;

}

it = it->next;

}
return *term;

}

//Insert a new alias at the end of the linked list
void insert(char** aliKey, struct alias* head){
char alias[25] = "\0", cmd[25] = "\0";
struct alias* it = head;
int i = 0, x = 0, b = 0;
while(aliKey[0][i] != '\0'){
if(aliKey[0][i] == ' '){
b = i;
b++;
while(aliKey[0][b] != '='){
alias[x] = aliKey[0][b];
b++;
x++;
}
alias[x] = '\0';
x = 0;
b += 2;
while(aliKey[0][b] != '\''){
cmd[x] = aliKey[0][b];
x++;
b++;
}
cmd[x] = '\0';
break;
}

i++;

}

struct alias* newAlias = (struct alias*)malloc(sizeof(struct alias));
newAlias->next = NULL;
newAlias->cmd = (char *)malloc( 25 * sizeof(char));
newAlias->alias = (char *)malloc( 25 * sizeof(char));
strcpy(newAlias->cmd, cmd);
strcpy(newAlias->alias, alias);
while(it->next != NULL){

it = it->next;

}
it->next = newAlias;












