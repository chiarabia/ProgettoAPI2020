#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define INPUT_COMMAND  20
#define INPUT_LINE 1023

typedef struct {
    char command;
    int first_row;
    int last_row;
    int number;
} Action;

typedef struct node {
    char* string;
    //int line_number;
    struct node *next;
    struct node *prev;
}node_t;

typedef struct node line;

int ask_for_action(char *input);
void find_command(char *input);
//int convert_to_char(char to_convert);
void create_action(char command,char *input);
char* ask_for_line(char *line);
void push();
line* create_node(char *new_line);
void add_line();
void delete_node(int pos_line);
void search_and_delete(int number_of_line);
void search_and_print(int number_of_line);
char *save_string(char *current_line);
void search_and_update(int number_of_line);


Action current_action;
int first_time = 1;
int total_number_of_lines = 0;

line *new, *ptr;
line * head = NULL, * tail = NULL;
int number = 0;
/*
 * indi1 indica la prima riga
 * (ind1,ind2)c -> cambia le righe
 * (ind1,ind2)d -> cancella le righe
 * (ind1,ind2)p -> printa le righe
 * (numero)u -> annullamento di un numero di comandi
 * (numero)r -> annulla l'effetto di undo per un certo numero
 * q -> chiude il programma //done
 *
 * una riga può contenere al massimo 1024 caratteri
 */

void handle_change(){
    int number_of_lines = current_action.last_row - current_action.first_row + 1;
    int current_line_number = current_action.first_row;
    int how_many_lines_left = 0;

    // if ( current_action.first row > tail->line_number) then push
    //else insert
    if (first_time == 1){
        add_line(current_line_number);
        //printf("%s\n","im changing first");
        current_line_number++;
        total_number_of_lines ++;
        how_many_lines_left ++;
    }
    first_time = 0;
    if (first_time ==0){
        while( how_many_lines_left< number_of_lines){
            if(current_line_number > total_number_of_lines ){
                push(current_line_number);
                total_number_of_lines ++;
            }
            else{
                search_and_update(current_line_number);
            }
            current_line_number ++;
            how_many_lines_left ++;
        }
    }
    char dot[1] = "" ;
    scanf("%c",dot);

}

void handle_delete(){
    if(current_action.first_row == 0 && current_action.last_row == 0)
        return;
    int current_line_number= current_action.first_row;
    int number_of_lines = current_action.last_row - current_action.first_row + 1;
    for (int i = 0; i < number_of_lines; i ++){
        search_and_delete(current_line_number);
        current_line_number ++;
        total_number_of_lines --;
    }
}

void handle_print(){
    if(current_action.first_row == 0 && current_action.last_row == 0)
        printf("%c\n",'.' );
    else{
        //printf("%s\n","im printing");
        int current_line_number= current_action.first_row;
        int number_of_lines = current_action.last_row - current_action.first_row + 1;
        for (int i = 0; i < number_of_lines; i ++){
            search_and_print(current_line_number);
            current_line_number ++;
        }
    }
}

void handle_undo(){

}

void handle_redo(){

}

//scans the input and populates the current_action object. Chooses which commands needs to be called
void find_command(char *input){

    if(strchr(input, 'c') != NULL){
        create_action('c',input);
        handle_change();
    }
    else if(strchr(input, 'd') != NULL ){
        create_action('d',input);
        handle_delete();
    }

    else if(strchr(input, 'p') != NULL ){
        create_action('p',input);
        handle_print();
    }

    else if(strchr(input, 'u') != NULL ){
        create_action('u',input);
        handle_undo();
    }

    else if(strchr(input,'r') != NULL){
        create_action('r',input);
        handle_redo();
    }

}

//reads the input from stdin and sends it to a parser
int ask_for_action(char *input) {
    fgets(input, INPUT_COMMAND, stdin);
    if(input[0] == 'q') {
        return 0;
    }
    else{
        find_command(input);
        return 1;
    }
}

int main(int argc, char*argv[]) {
    char input [INPUT_COMMAND];
    int keep_asking = 1;
    while (keep_asking == 1){
        keep_asking = ask_for_action(input);
    }
    return 0;
}

/*int convert_to_char(char to_convert){
    char temp[2];
    temp[0] = to_convert;
    temp[1] = '\0';
    int converted = (int) strtol(temp,NULL,10);
    return converted;
}*/

void create_action(char command,char *input){
    current_action.command = command;
    if(command == 'u' || command == 'r'){
        current_action.number = atoi(strtok(&input,&command));
        return;
    }
    char delim[] = ",";
    current_action.first_row =  atoi(strtok(input,delim));
    current_action.last_row = atoi(strtok(NULL,&command));
}

char* ask_for_line(char *line){
    //printf("i ll ask the line\n");
    fgets(line, INPUT_LINE, stdin);
    line[strlen(line) -1 ] = '\0';
    char *new_line = save_string(line);
    //printf(">%s\n",line);
    return new_line;
}

void push(){
    char current_line [INPUT_LINE];
    char* new_line = ask_for_line(current_line);
    new = create_node(new_line);
    if (head == tail && head == NULL){
        head = tail = new;
        head -> next = tail -> next = NULL;
        head -> prev = tail -> prev = NULL;
    }
    else {
        tail -> next = new;
        new -> prev = tail;
        tail = new;
        head -> prev = tail;
        tail -> next = head;
        //printf("%s\n","i pushed a line");
    }
}

line* create_node(char *new_line){
    number ++;
    new = (line *)malloc(sizeof(line));
    assert(new != NULL) ;
    new -> string = new_line;
    new -> next = NULL;
    new -> prev = NULL;
    return new;
}

void add_line(){
    //printf("i'll save the line\n");
    char current_line [INPUT_LINE];
    char* new_line = ask_for_line(current_line);
    new = create_node(new_line);
    if (head == tail && head == NULL){
        head = tail = new;
        head -> next = tail -> next = NULL;
        head -> prev = tail -> prev = NULL;
    }
    else{
        tail ->next = new;
        new -> prev = tail;
        tail = new;
        tail -> next = head;
        head -> prev = tail;
        //printf("i saved the line\n");
    }
}

void delete_node(int pos_line){
    int count = 0, i;
    line * temp, *prevnode;

    if(head == tail && head == NULL){ printf("ciao");}

    if (number < pos_line ){
        for (ptr = head, i = 1; i <= number; i++){
            prevnode = ptr;
            ptr = ptr -> next;
            if (pos_line ==1){
                number --;
                tail -> next = prevnode -> next;
                ptr -> prev = prevnode -> prev;
                head = ptr;
                free(prevnode);
                break;
            }
            else if(i == pos_line - 1){
                number --;
                prevnode -> next = ptr ->next;
                ptr -> next -> prev = prevnode;
                free (ptr);
                break;
            }
        }
    }
}

void search_and_print(int number_of_line){
    int count_head = 1,i;
    int count_tail = total_number_of_lines;
    if(head == tail && head == NULL){ printf("ciao");}
    if(number_of_line <= (total_number_of_lines/2)){
        for (ptr = head, i = 0; i < number; i ++, ptr = ptr -> next){
            if(count_head == number_of_line){
                if (ptr->string != NULL){
                    printf("%s\n",ptr->string);
                    return;
                }
                else{
                    printf("%c\n",'.' );
                    return;
                }
            }
            count_head++;
        }
    }
    else if(number_of_line > (total_number_of_lines/2)){
        for (ptr = tail, i = 0; i < number; i ++, ptr = ptr -> prev){
            if(count_tail == number_of_line){
                if (ptr->string != NULL){
                    printf("%s\n",ptr->string);
                    return;
                }
                else{
                    printf("%c\n",'.' );
                    return;
                }
            }
            else if(count_tail < number_of_line){
                printf("%c\n",'.' );
                return;
            }
            count_tail--;
        }
    }

}

void search_and_delete(int number_of_line){
    line * temp, *prevnode,*nextnode;
    int count_head = 1,i;
    int count_tail = total_number_of_lines;
    if(head == tail && head == NULL){ printf("ciao");}
    if (number_of_line == 1){
        prevnode = ptr;
        ptr = ptr -> next;
        tail -> next = prevnode ->next;
        ptr -> prev = prevnode -> prev;
        head = ptr;
        free(prevnode);
        return;
    }
    else if(number_of_line <= (total_number_of_lines/2)){
        for (ptr = head, i = 0; i < number; i ++, ptr = ptr -> next){
            if(count_head == number_of_line){
                prevnode = ptr -> prev;
                nextnode = ptr-> next;
                prevnode -> next = nextnode;
                nextnode -> prev = prevnode;
                free(ptr);
                return;
            }
            count_head ++;
        }
    }
    else if(number_of_line > (total_number_of_lines/2)){
        for (ptr = tail, i = 0; i < number; i ++, ptr = ptr -> prev){
            if(count_tail == number_of_line){
                nextnode = ptr -> next;
                prevnode = ptr -> prev;
                prevnode -> next = nextnode;
                nextnode -> prev = prevnode;
                free(ptr);
                return;
            }
            count_tail--;
        }
    }
}

char *save_string(char *current_line){
    int size = strlen(current_line);
    char *new_string = malloc(size +1);
    strcpy(new_string,current_line);
    assert(new_string != NULL);
    return new_string;
}

void search_and_update(int number_of_line) {
    int i, count_head = 1;
    int count_tail = total_number_of_lines;
    char current_line[INPUT_LINE];
    char *new_line = ask_for_line(current_line);

    if (head == tail && head == NULL) {
        if (number_of_line == 1) {
            new = create_node(new_line);
            head = tail = new;
            head->next = tail->next = NULL;
            head->prev = tail->prev = NULL;
        }

    }
    else {
        if (number_of_line < (total_number_of_lines / 2)) {
            for (ptr = head, i = 1; i <= number; i++) {
                if (count_head == number_of_line) {
                    ptr->string = new_line;
                    return;
                }
                ptr = ptr->next;
                count_head++;
            }
        }
        else if (number_of_line >= (total_number_of_lines / 2)) {
            for (ptr = tail, i = 1; i <= number; i++) {
                if (count_tail == number_of_line) {
                        ptr->string = new_line;
                        return;
                }
                count_tail--;
                ptr = ptr->prev;

            }
        }
    }
}

    void search_and_insert(int number_of_line) {
        int i, count_head = 1;
        int count_tail = total_number_of_lines;
        char current_line[INPUT_LINE];
        char *new_line = ask_for_line(current_line);

        if (head == tail && head == NULL) {
            if (number_of_line == 1) {
                new = create_node(new_line);
                head = tail = new;
                head->next = tail->next = NULL;
                head->prev = tail->prev = NULL;
            }
        }
        else {
            if (number_of_line <= (total_number_of_lines / 2)) {
                for (ptr = head, i = 1; i <= number; i++) {
                    if (count_head < number_of_line) {
                        count_head++;
                        if (count_head > number_of_line) {
                            new = create_node(new_line);
                            new->next = ptr->next;
                            new->prev = ptr;
                            (new->next)->prev = new;
                            ptr->next = new;
                            return;
                        }
                        else if (count_head == number_of_line) {
                            ptr->string = new_line;
                            return;
                        }
                        count_head--;
                    }
                    else if (count_head == number_of_line) {
                        ptr->string = new_line;
                        return;
                    }
                    ptr = ptr->next;
                    count_head++;
                }
            }
            else if (number_of_line >= (total_number_of_lines / 2)) {
                for (ptr = tail, i = 1; i <= number; i++) {
                    if (count_tail > number_of_line) {
                        count_tail--;
                        if (count_tail < number_of_line) {
                            new = create_node(new_line);
                            new->prev = ptr->prev;
                            new->next = ptr;
                            (new->prev)->next = new;
                            ptr->prev = new;
                            return;
                        }
                        else if (count_tail == number_of_line) {
                            ptr->string = new_line;
                            return;
                        }
                        count_head++;
                    }
                    else if (count_tail == number_of_line) {
                        ptr->string = new_line;
                        return;
                    }
                    count_tail--;
                    ptr = ptr->prev;
                }
            }

        }
    }

