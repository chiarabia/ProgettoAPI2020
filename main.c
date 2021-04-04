#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define INPUT_COMMAND  20000
#define INPUT_LINE 1024

//the node of the LIST for database with the the strings
typedef struct node {
    char* string;
    struct node *next;
    struct node *prev;
}node_t;

//ACTION saves the data of requested actions
typedef struct{
    char command;
    int first_row;
    int last_row;
    int number;
    node_t* starting_ptr;
    node_t* ending_ptr;
    node_t* last_ptr;
    node_t* first_ptr;
} Action;

//the HISTORY for saving each actions modifications to the database
typedef struct {
    char command;
    int first_row;
    int last_row;
    node_t* first_node;
    node_t* last_node;
    node_t* new_lines_starting_pointer;
    node_t* new_lines_ending_pointer;
    node_t* old_lines_starting_pointer;
    node_t* old_lines_ending_pointer;
    int was_delete_not_necessary;
    int was_change_and_push;
    int number_of_push;
    int number_of_deletes;
}History;

typedef struct undo{
    History * action;
    struct undo * prev;
}undo_t;

typedef struct node line;
typedef struct undo list_undo;
typedef struct undo list_redo;

list_undo *tail_undo = NULL;
list_undo *new_action = NULL;
list_undo  *ptr_undo = NULL;

list_redo *tail_redo = NULL;
list_redo *ptr_redo = NULL;

//for managing the list
line *new = NULL, *ptr = NULL;
line * head = NULL, * tail = NULL;

#define ASSIGN_OLD_STARTING_PTR(ptr) (tail_undo->action->old_lines_starting_pointer = ptr)
#define ASSIGN_OLD_ENDING_PTR(ptr) (tail_undo->action->old_lines_ending_pointer = ptr)
#define ASSIGN_NEW_STARTING_PTR(ptr) (tail_undo->action->new_lines_starting_pointer = ptr)
#define ASSIGN_NEW_ENDING_PTR(ptr) (tail_undo->action->new_lines_ending_pointer = ptr)
#define ASSIGN_FIRST_NODE(ptr) (tail_undo->action->first_node = ptr)
#define ASSIGN_LAST_NODE(ptr) (tail_undo->action->last_node = ptr)
#define ASSIGN_WAS_DELETE(tmp) (tail_undo->action->was_delete_not_necessary = tmp)
#define ASSIGN_WAS_CHANGE_AND_PUSH(tmp) (tail_undo->action->was_change_and_push = tmp)
#define ASSIGN_NUMBER_OF_DELETE(tmp) (tail_undo->action->number_of_deletes = tmp)
#define ASSIGN_NUMBER_OF_PUSH(tmp) (tail_undo->action->number_of_push = tmp)

Action CURRENT_ACTION;
int IS_UNDO_WAITING = 0;
int IS_REDO_WAITING = 0;
int NUMBER_OF_UNDO = 0;
int NUMBER_OF_REDO = 0;
int TOTAL_NUMBER_OF_LINES = 0;

int debug_total_action = 0;

int NUMBER_OF_TOTAL_ACTIONS = 0;
int TMP_NUMB_OF_ACTIONS = 0;
int HISTORY_IS_BEING_EDITED = -1;

int ask_for_action(char *input);
void find_command(char *input);
void create_action(char command,char *input);
void handle_change();
void insert_new_line();
char* ask_for_line(char *line);
char *save_string(char *current_line);
void handle_delete();
void find_old_ptr();
void delete();
void undo();
void redo();
void delete_from_redu_list();
void handle_print();
void print();
void handle_undo();
void handle_redo();
void insert_for_undo();
void insert_for_redo();
void assign_all_history_pointers_to_null();
History* assign_to_undo_list(char command, int first_row,int last_row);
void assign_to_redo_list(list_undo* action);
void assign_to_undo_list_from_redu(list_redo* action);
void delete_redo_and_put_in_undu();
void delete_undo_and_put_in_redo();

int main() {
    //the command request
    char input [INPUT_COMMAND];
    //does the program needs to keep asking for input?
    int keep_asking = 1;
    while (keep_asking == 1){
        keep_asking = ask_for_action(input);
    }
    return 0;
}

/**
 * Reads the input from stdin and sends it to a parser
 * @param input the command input by the user
 * @return if the program needs to stop returns 0, otherwise 1
 */
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

/**
 * Asks to input a string when needed (change command)
 * @param line where to save the new string
 * @return the string now with the new data
 */
char* ask_for_line(char *line){
    //printf("i ll ask the line\n");
    fgets(line, INPUT_LINE, stdin);
    char *new_line = save_string(line);
    //printf(">%s\n",line);
    return new_line;
}

/**
 * Saves the string permanently
 * @param current_line the string that needs to be saved
 * @return where the string was saved
 */
char *save_string(char *current_line){
    long long size = strlen(current_line);
    char *new_string = malloc(size +1);
    strcpy(new_string,current_line);
    assert(new_string != NULL);
    return new_string;
}

/**
 * Creates a new node of the list with the string
 * @param new_line the string that needs to be saved in that node
 * @return the new node
 */
line* create_node(char *new_line){
    new = (line *)malloc(sizeof(line));
    assert(new != NULL) ;
    new -> string = new_line;
    new -> next = NULL;
    new -> prev = NULL;
    return new;
}

/**
 * Scans the input and populates the current_action object. Chooses which commands needs to be called
 * @param input the command input by the user
 */
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

/**
 * Updates the current_action with the command type, what lines need changes, and how many lines if undo/redo
 * @param command the command type of the action
 * @param input the command input of the user
 */
void create_action(char command,char *input){
    CURRENT_ACTION.command = command;
    if(command == 'u'){
        CURRENT_ACTION.number = atoi(strtok(input,"u"));
        return;
    }
    else if(command == 'r'){
        CURRENT_ACTION.number = atoi(strtok(input,"r"));
        return;
    }
    char delim[] = ",";
    CURRENT_ACTION.first_row =  atoi(strtok(input,delim));
    if(command == 'c')
        CURRENT_ACTION.last_row = atoi(strtok(NULL,"c"));
    else if(command == 'd')
        CURRENT_ACTION.last_row = atoi(strtok(NULL,"d"));
    else if(command == 'p')
        CURRENT_ACTION.last_row = atoi(strtok(NULL,"p"));
}

/**
 * Handles the change command.
 * If first time == 1 we need to create the first node of the list.
 * Otherwise every c command is either an update (changes an existent line) or a push (creates a new node at the end of the list.
 * If is_undo_waiting == 1 then the undo command needs to be processed before the change
 */
void handle_change(){
    int i;
    debug_total_action ++;
    if(IS_UNDO_WAITING == 1){
        undo();
        NUMBER_OF_UNDO = 0;
        IS_UNDO_WAITING = 0;
        if(TMP_NUMB_OF_ACTIONS != NUMBER_OF_TOTAL_ACTIONS){
            int number_of_frees = NUMBER_OF_TOTAL_ACTIONS - TMP_NUMB_OF_ACTIONS;
            for (i = 0; i< number_of_frees; i++){
                delete_from_redu_list();
            }
        }
        NUMBER_OF_TOTAL_ACTIONS = TMP_NUMB_OF_ACTIONS;
        HISTORY_IS_BEING_EDITED = -1;

    }
    if(IS_REDO_WAITING == 1){
        redo();
        NUMBER_OF_REDO = 0;
        IS_REDO_WAITING = 0;
        if(TMP_NUMB_OF_ACTIONS != NUMBER_OF_TOTAL_ACTIONS){
            int number_of_frees = NUMBER_OF_TOTAL_ACTIONS - TMP_NUMB_OF_ACTIONS;
            for (i = 0; i< number_of_frees; i++){
                delete_from_redu_list();
            }
        }
        NUMBER_OF_TOTAL_ACTIONS = TMP_NUMB_OF_ACTIONS;
        HISTORY_IS_BEING_EDITED = -1;

    }

    if(HISTORY_IS_BEING_EDITED != -1){
        HISTORY_IS_BEING_EDITED = -1;
        if(TMP_NUMB_OF_ACTIONS != NUMBER_OF_TOTAL_ACTIONS){
            int number_of_frees = NUMBER_OF_TOTAL_ACTIONS - TMP_NUMB_OF_ACTIONS;
            for (i = 0; i< number_of_frees; i++){
                delete_from_redu_list();
            }
        }
        NUMBER_OF_TOTAL_ACTIONS = TMP_NUMB_OF_ACTIONS;
    }

    NUMBER_OF_TOTAL_ACTIONS++;
    TMP_NUMB_OF_ACTIONS ++;
    History* current_change = assign_to_undo_list(CURRENT_ACTION.command,CURRENT_ACTION.first_row,CURRENT_ACTION.last_row);
    ASSIGN_NUMBER_OF_DELETE(0);
    ASSIGN_NUMBER_OF_PUSH(-1);
    ASSIGN_WAS_DELETE(-1);
    ASSIGN_NUMBER_OF_PUSH(-1);

    insert_new_line();
    if(current_change->number_of_push == 0)
        ASSIGN_NUMBER_OF_PUSH(-1);

    //the last input will always be a dot
    char dot[1] = "" ;
    scanf("%c",dot);

    /*printf("Printa new_start: %s",current_change->new_lines_starting_pointer->string);
    printf("Printa new_end: %s",current_change->new_lines_ending_pointer->string);
    if(current_change->old_lines_starting_pointer != NULL)
        printf("Printa old_start: %s",current_change->old_lines_starting_pointer->string);
    if(current_change->old_lines_ending_pointer != NULL)
        printf("Printa old_end: %s",current_change->old_lines_ending_pointer->string);
    if(current_change->first_node != NULL)
        printf("Printa nodo antecedente: %s",current_change->first_node->string);
    if(current_change->last_node != NULL)
        printf("Printa nodo dopo: %s",current_change->last_node->string);
    printf("Printa 1 se ci sono state sia change che push: %d\n", current_change -> was_change_and_push);
    printf("Printa il numero di push: %d\n", current_change->number_of_push);*/
}

void insert_new_line() {
    int i;
    line *ask_line = NULL;
    line *tmp = NULL;
    int number_of_push = 0;
    int number_of_lines = CURRENT_ACTION.last_row - CURRENT_ACTION.first_row + 1;

    if (CURRENT_ACTION.first_row <= TOTAL_NUMBER_OF_LINES && CURRENT_ACTION.last_row > TOTAL_NUMBER_OF_LINES)
        ASSIGN_WAS_CHANGE_AND_PUSH(1);
    //trovo i vecchi puntatori
    find_old_ptr();
    //chiedo le nuove linee e li segno come nuovi puntatori
    for (i = 0; i < number_of_lines; i++) {
        char current_line[INPUT_LINE];
        char *new_line = ask_for_line(current_line);
        if (ask_line != NULL) {
            tmp = ask_line;
        }
        ask_line = create_node(new_line);
        if (i == 0) {
            tmp = ask_line;
        } else if (tmp != NULL && i != 0) {
            ask_line->prev = tmp;
            tmp->next = ask_line;
        }
        if (i == 0) {
            ASSIGN_NEW_STARTING_PTR(ask_line);
            CURRENT_ACTION.starting_ptr = ask_line;
        }
        if (i == number_of_lines - 1) {
            ASSIGN_NEW_ENDING_PTR(ask_line);
            CURRENT_ACTION.ending_ptr = ask_line;
        }
    }
    //se ancora non c'è la testa
    if (head == NULL) {
        ASSIGN_FIRST_NODE(NULL);
        ASSIGN_LAST_NODE(NULL);
        ASSIGN_OLD_ENDING_PTR(NULL);
        ASSIGN_OLD_STARTING_PTR(NULL);
        head = CURRENT_ACTION.starting_ptr;
        tail = CURRENT_ACTION.ending_ptr;
        tail -> next = NULL;
        head->prev = NULL;
        TOTAL_NUMBER_OF_LINES = number_of_lines;
        ASSIGN_NUMBER_OF_PUSH(number_of_lines);
    }
        //se già esiste la lista
    else {
        //se sto modificando la testa
        if (CURRENT_ACTION.first_row == 1) {
            head = CURRENT_ACTION.starting_ptr;
            head->prev = NULL;
            assert(tail_undo->action->first_node == NULL);
            //se ho anche righe dopo la coda
            if (CURRENT_ACTION.last_row >= TOTAL_NUMBER_OF_LINES) {
                tail = CURRENT_ACTION.ending_ptr;
                tail -> next = NULL;
                assert(tail_undo->action->last_node == NULL);
                number_of_push = CURRENT_ACTION.last_row - TOTAL_NUMBER_OF_LINES;
                TOTAL_NUMBER_OF_LINES = TOTAL_NUMBER_OF_LINES + number_of_push;
                ASSIGN_NUMBER_OF_PUSH(number_of_push);
            }
                //se la modifica si ferma prima della coda
            else if (CURRENT_ACTION.last_row < TOTAL_NUMBER_OF_LINES) {
                CURRENT_ACTION.last_ptr->prev = CURRENT_ACTION.ending_ptr;
                CURRENT_ACTION.ending_ptr->next = CURRENT_ACTION.last_ptr;
                assert(tail_undo->action->last_node != NULL);
            }
            if (tail_undo->action->old_lines_starting_pointer != NULL)
                tail_undo->action->old_lines_starting_pointer->prev = NULL;
            if (tail_undo->action->old_lines_ending_pointer != NULL)
                tail_undo->action->old_lines_ending_pointer->next = NULL;
        }
            //se NON sto modificando la testa ma sto modificando la coda o dopo la coda
        else if (CURRENT_ACTION.last_row >= TOTAL_NUMBER_OF_LINES && CURRENT_ACTION.first_row != 1) {
            if (CURRENT_ACTION.first_row <= TOTAL_NUMBER_OF_LINES)
                tail = CURRENT_ACTION.first_ptr;
            tail->next = CURRENT_ACTION.starting_ptr;
            CURRENT_ACTION.starting_ptr->prev = tail;
            tail = CURRENT_ACTION.ending_ptr;
            tail->next = NULL;
            if (CURRENT_ACTION.first_row > TOTAL_NUMBER_OF_LINES) {
                ASSIGN_OLD_STARTING_PTR(NULL);
                ASSIGN_OLD_ENDING_PTR(NULL);
                ASSIGN_LAST_NODE(NULL);
                ASSIGN_FIRST_NODE(tail_undo->action->new_lines_starting_pointer->prev);
                TOTAL_NUMBER_OF_LINES = TOTAL_NUMBER_OF_LINES + number_of_lines;
                ASSIGN_NUMBER_OF_PUSH(number_of_lines);
            } else if (CURRENT_ACTION.first_row <= TOTAL_NUMBER_OF_LINES &&
                       CURRENT_ACTION.last_row > TOTAL_NUMBER_OF_LINES) {
                number_of_push = CURRENT_ACTION.last_row - TOTAL_NUMBER_OF_LINES;
                TOTAL_NUMBER_OF_LINES = TOTAL_NUMBER_OF_LINES + number_of_push;
                ASSIGN_NUMBER_OF_PUSH(number_of_push);
            }
            if (tail_undo->action->old_lines_starting_pointer != NULL)
                tail_undo->action->old_lines_starting_pointer->prev = NULL;
            if (tail_undo->action->old_lines_ending_pointer != NULL)
                tail_undo->action->old_lines_ending_pointer->next = NULL;
        }
            //se la modifica è interna
        else if (CURRENT_ACTION.first_row != 1 && CURRENT_ACTION.first_row < TOTAL_NUMBER_OF_LINES &&
                 CURRENT_ACTION.last_row < TOTAL_NUMBER_OF_LINES) {
            CURRENT_ACTION.first_ptr->next = CURRENT_ACTION.starting_ptr;
            CURRENT_ACTION.starting_ptr->prev = CURRENT_ACTION.first_ptr;
            CURRENT_ACTION.last_ptr->prev = CURRENT_ACTION.ending_ptr;
            CURRENT_ACTION.ending_ptr->next = CURRENT_ACTION.last_ptr;
            tail_undo->action->old_lines_starting_pointer->prev = NULL;
            tail_undo->action->old_lines_ending_pointer->next = NULL;
        }

    }
}

/**
 * Handles the delete command.
 * If we have a 0,0d command we can resolve it immediately. Otherwise searches for the row that needs
 * to be deleted and deletes it if it exists.
 * If is_undo_waiting == 1 then the undo command needs to be processed before the delete
 */
void handle_delete(){
    debug_total_action ++;
    int i;
    if(IS_UNDO_WAITING == 1){
        undo();
        NUMBER_OF_UNDO = 0;
        IS_UNDO_WAITING = 0;
        if(TMP_NUMB_OF_ACTIONS != NUMBER_OF_TOTAL_ACTIONS){
            int number_of_frees = NUMBER_OF_TOTAL_ACTIONS - TMP_NUMB_OF_ACTIONS;
            for (i = 0; i< number_of_frees; i++){
                delete_from_redu_list();
            }
        }
        NUMBER_OF_TOTAL_ACTIONS = TMP_NUMB_OF_ACTIONS;
        HISTORY_IS_BEING_EDITED = -1;
    }
    if(IS_REDO_WAITING == 1){
        redo();
        NUMBER_OF_REDO = 0;
        IS_REDO_WAITING = 0;
        if(TMP_NUMB_OF_ACTIONS != NUMBER_OF_TOTAL_ACTIONS){
            int number_of_frees = NUMBER_OF_TOTAL_ACTIONS - TMP_NUMB_OF_ACTIONS;
            for (i = 0; i< number_of_frees; i++){
                delete_from_redu_list();
            }
        }
        NUMBER_OF_TOTAL_ACTIONS = TMP_NUMB_OF_ACTIONS;
        HISTORY_IS_BEING_EDITED = -1;
    }

    NUMBER_OF_TOTAL_ACTIONS++;
    TMP_NUMB_OF_ACTIONS ++;
    History* current_delete = assign_to_undo_list(CURRENT_ACTION.command,CURRENT_ACTION.first_row,CURRENT_ACTION.last_row);
    ASSIGN_NUMBER_OF_PUSH(-1);
    ASSIGN_WAS_CHANGE_AND_PUSH(-1);
    ASSIGN_WAS_DELETE(-1);
    ASSIGN_NUMBER_OF_DELETE(-1);

    if((CURRENT_ACTION.first_row == 0 && CURRENT_ACTION.last_row == 0) || CURRENT_ACTION.first_row > TOTAL_NUMBER_OF_LINES) {
        assign_all_history_pointers_to_null();
        ASSIGN_WAS_DELETE(1);
        /*if(current_delete->old_lines_starting_pointer != NULL)
            printf("Printa il primo nodo: %s",current_delete->old_lines_starting_pointer->string);
        if(current_delete->old_lines_ending_pointer != NULL)
            printf("Printa l'ultimo nodo: %s",current_delete->old_lines_ending_pointer->string);
        if(current_delete->first_node!=NULL)
            printf("Printa il nodo antecedente: %s",current_delete->first_node->string);
        if(current_delete->last_node != NULL)
            printf("Printa il nodo dopo: %s",current_delete->last_node->string);
        printf("Printa 1 se la current delete non e necessaria: %d\n",current_delete->was_delete_not_necessary);
        printf("Printa il numero delle delete: %d",current_delete->number_of_deletes);*/
        return;
    }

    delete();
    /*if(current_delete->old_lines_starting_pointer != NULL)
        printf("Printa il primo nodo: %s",current_delete->old_lines_starting_pointer->string);
    if(current_delete->old_lines_ending_pointer != NULL)
        printf("Printa l'ultimo nodo: %s",current_delete->old_lines_ending_pointer->string);
    if(current_delete->first_node!=NULL)
        printf("Printa il nodo antecedente: %s",current_delete->first_node->string);
    if(current_delete->last_node != NULL)
        printf("Printa il nodo dopo: %s",current_delete->last_node->string);
    printf("Printa 1 se la current delete non e necessaria: %d\n",current_delete->was_delete_not_necessary);
    printf("Printa il numero delle delete: %d",current_delete->number_of_deletes);*/
}

void delete(){
    int number_of_lines = CURRENT_ACTION.last_row - CURRENT_ACTION.first_row + 1;
    int number_of_deletes;

    find_old_ptr();
    ASSIGN_NEW_ENDING_PTR(NULL);
    ASSIGN_NEW_STARTING_PTR(NULL);
    tail_undo->action->old_lines_starting_pointer->prev = NULL;
    tail_undo->action->old_lines_ending_pointer->next = NULL;

    if(CURRENT_ACTION.first_row == 1){
        if(CURRENT_ACTION.last_row >= TOTAL_NUMBER_OF_LINES){
            head = NULL;
            tail = NULL;
            ASSIGN_NUMBER_OF_DELETE(TOTAL_NUMBER_OF_LINES);
            TOTAL_NUMBER_OF_LINES = 0;
            ASSIGN_FIRST_NODE(NULL);
            ASSIGN_LAST_NODE(NULL);
            return;
        }
        head = CURRENT_ACTION.last_ptr;
        head->prev = NULL;
        TOTAL_NUMBER_OF_LINES = TOTAL_NUMBER_OF_LINES - CURRENT_ACTION.last_row;
        ASSIGN_NUMBER_OF_DELETE(CURRENT_ACTION.last_row);
        ASSIGN_FIRST_NODE(NULL);
        return;
    }
    if(CURRENT_ACTION.last_row >= TOTAL_NUMBER_OF_LINES){
        tail = CURRENT_ACTION.first_ptr;
        tail -> next = NULL;
        number_of_deletes = TOTAL_NUMBER_OF_LINES - CURRENT_ACTION.first_row + 1;
        ASSIGN_NUMBER_OF_DELETE(number_of_deletes);
        TOTAL_NUMBER_OF_LINES = TOTAL_NUMBER_OF_LINES - number_of_deletes;
        ASSIGN_LAST_NODE(NULL);
        return;
    }
    else if(CURRENT_ACTION.last_row < TOTAL_NUMBER_OF_LINES && CURRENT_ACTION.first_row > 1){
        CURRENT_ACTION.first_ptr -> next = CURRENT_ACTION.last_ptr;
        CURRENT_ACTION.last_ptr -> prev = CURRENT_ACTION.first_ptr;
        ASSIGN_NUMBER_OF_DELETE(number_of_lines);
        TOTAL_NUMBER_OF_LINES = TOTAL_NUMBER_OF_LINES - number_of_lines;
        return;
    }
}

void find_old_ptr(){
    int i;
    if (head != NULL && CURRENT_ACTION.first_row <= TOTAL_NUMBER_OF_LINES) {
        for (i = 1, ptr = head; i <= TOTAL_NUMBER_OF_LINES; i++, ptr = ptr->next) {
            if (i == CURRENT_ACTION.first_row) {
                ASSIGN_OLD_STARTING_PTR(ptr);
                //se non sto modificando anche la testa
                if (ptr->prev != NULL) {
                    ASSIGN_FIRST_NODE(ptr->prev);
                    CURRENT_ACTION.first_ptr = ptr->prev;
                } else {
                    //se sto modificando la testa
                    ASSIGN_FIRST_NODE(NULL);
                    CURRENT_ACTION.first_ptr = NULL;
                }
            }
            if (i == CURRENT_ACTION.last_row || i == TOTAL_NUMBER_OF_LINES) {
                ASSIGN_OLD_ENDING_PTR(ptr);
                //se non sto modificando anche la coda
                if (ptr->next != NULL) {
                    if (i == CURRENT_ACTION.last_row) {
                        ASSIGN_LAST_NODE(ptr->next);
                        CURRENT_ACTION.last_ptr = ptr->next;
                        break;
                    }
                } else {
                    ASSIGN_LAST_NODE(NULL);
                    CURRENT_ACTION.last_ptr = NULL;
                    break;
                }
            }
        }
    }
}

/**
 * Handles the print command.
 * If we have 0,0p we can print a dot immediately.
 * Otherwise it prints the rows requested
 * If is_undo_waiting == 1 then the undo command needs to be processed before the print
 */
void handle_print(){
    int i;
    int number_of_lines = CURRENT_ACTION.last_row - CURRENT_ACTION.first_row + 1;
    if(IS_UNDO_WAITING == 1){
        undo();
        IS_UNDO_WAITING = 0;
        NUMBER_OF_UNDO = 0;
        HISTORY_IS_BEING_EDITED = 1;
    }
    if(IS_REDO_WAITING == 1){
        redo();
        IS_REDO_WAITING = 0;
        NUMBER_OF_REDO = 0;
        HISTORY_IS_BEING_EDITED = 1;
    }
    if(CURRENT_ACTION.first_row == 0 && CURRENT_ACTION.last_row == 0){
        printf("%c\n",'.' );
        return;
    }

    if(CURRENT_ACTION.first_row > TOTAL_NUMBER_OF_LINES){
        for(i = 0; i < number_of_lines; i++){
            printf("%c\n",'.' );
        }
    }
    else{
       print();
    }
}

void print(){
    int number_of_lines = CURRENT_ACTION.last_row - CURRENT_ACTION.first_row + 1;
    int i,j, number_of_dots;
    int starting_point = CURRENT_ACTION.first_row -1;
    int temp_number_of_lines = TOTAL_NUMBER_OF_LINES;

    for(j=1, ptr = head; j<= starting_point; j++,ptr= ptr -> next);

   /* if (CURRENT_ACTION.last_row > TOTAL_NUMBER_OF_LINES){
        number_of_dots = CURRENT_ACTION.last_row - TOTAL_NUMBER_OF_LINES;
        temp_number_of_lines = temp_number_of_lines + number_of_dots;
    }*/

    for(i = 1; i <= number_of_lines; i++){
            if (ptr != NULL){
                printf("%s",ptr->string);
                ptr = ptr->next;
            }
            else
                printf("%c\n",'.' );
    }

    /*starting_point = starting_point - CURRENT_ACTION.first_row;
    if(starting_point <= number_of_lines){
        int remaining_lines = number_of_lines - (starting_point-1);
        for(i = 0; i < remaining_lines; i++){
            printf("%c\n",'.' );
        }
    }*/
}


void handle_undo(){
    int tmp_undo = CURRENT_ACTION.number;
    //se la cronologia è intatta  ed ho un undu
    if(HISTORY_IS_BEING_EDITED == -1 && IS_UNDO_WAITING == 0){
        if(NUMBER_OF_TOTAL_ACTIONS - tmp_undo <= 0){
            tmp_undo = NUMBER_OF_TOTAL_ACTIONS;
        }
        IS_UNDO_WAITING = 1;
        NUMBER_OF_UNDO = tmp_undo;
        return;
    }
    //se la cronologia è intatta è c'è un undo che aspetta
    if(HISTORY_IS_BEING_EDITED == -1 && IS_UNDO_WAITING == 1){
        tmp_undo = NUMBER_OF_UNDO + CURRENT_ACTION.number;
        if(NUMBER_OF_TOTAL_ACTIONS - tmp_undo <= 0){
            tmp_undo = NUMBER_OF_TOTAL_ACTIONS;
        }
        IS_UNDO_WAITING = 1;
        NUMBER_OF_UNDO = tmp_undo;
        return;
    }
    //se la cronologia è stata modificata e nessuno aspetta (quindi c'è stato un print)
    if(HISTORY_IS_BEING_EDITED != -1 && IS_UNDO_WAITING == 0 && IS_REDO_WAITING == 0){
        tmp_undo = CURRENT_ACTION.number;
        if(TMP_NUMB_OF_ACTIONS - tmp_undo <= 0){
            tmp_undo = TMP_NUMB_OF_ACTIONS;
        }
        IS_UNDO_WAITING = 1;
        NUMBER_OF_UNDO = tmp_undo;
        return;
    }
        //se la cronologia è stata modificata e un redu sta aspettando
    else if(HISTORY_IS_BEING_EDITED != -1 && IS_UNDO_WAITING == 0 && IS_REDO_WAITING == 1){
        //se il numero degli undu è uguale a quello dei redu
        if(NUMBER_OF_REDO == CURRENT_ACTION.number){
            IS_REDO_WAITING = 0;
            NUMBER_OF_REDO = 0;
            return;
        }
        //se il numero degli undu è minore a quello dei redu
        if(NUMBER_OF_REDO > CURRENT_ACTION.number){
            NUMBER_OF_REDO = NUMBER_OF_REDO - CURRENT_ACTION.number;
            IS_REDO_WAITING = 1;
            return;
        }
        //se il numero degli undu è maggiore a quello dei redu
        if(NUMBER_OF_REDO < CURRENT_ACTION.number){
            tmp_undo = CURRENT_ACTION.number - NUMBER_OF_REDO;
            if(TMP_NUMB_OF_ACTIONS - tmp_undo <= 0){
                tmp_undo = TMP_NUMB_OF_ACTIONS;
            }
            NUMBER_OF_UNDO = tmp_undo;
            IS_REDO_WAITING = 0;
            NUMBER_OF_REDO = 0;
            IS_UNDO_WAITING = 1;
            return;
        }
    }
        //se la cronologia è stata modificata e un undu sta aspettando
    else if(HISTORY_IS_BEING_EDITED != -1 && IS_UNDO_WAITING == 1 && IS_REDO_WAITING == 0){
        tmp_undo = NUMBER_OF_UNDO + CURRENT_ACTION.number;
        if(TMP_NUMB_OF_ACTIONS - tmp_undo <= 0){
            tmp_undo = TMP_NUMB_OF_ACTIONS;
        }
        IS_UNDO_WAITING = 1;
        NUMBER_OF_UNDO = tmp_undo;
        return;
    }
}

void undo(){
    int i;
    if(TMP_NUMB_OF_ACTIONS == 0) return;
    for(i = 0; i<NUMBER_OF_UNDO; i++){
        insert_for_undo();
        TMP_NUMB_OF_ACTIONS--;
        delete_undo_and_put_in_redo();
    }
}

void insert_for_undo(){
    char command = tail_undo->action->command;
    node_t* first_node = tail_undo->action->first_node;
    node_t* last_node = tail_undo->action->last_node;
    int first_row = tail_undo->action->first_row;
    node_t* new_lines_starting_pointer = tail_undo->action -> new_lines_starting_pointer;
    node_t* new_lines_ending_pointer = tail_undo->action -> new_lines_ending_pointer;
    node_t* old_lines_starting_pointer = tail_undo->action -> old_lines_starting_pointer;
    node_t* old_lines_ending_pointer = tail_undo->action -> old_lines_ending_pointer;
    int number_of_deletes = tail_undo->action ->number_of_deletes;
    int number_of_push = tail_undo->action -> number_of_push;

    if(command == 'd'){
        //se cancello niente
        if(tail_undo->action->was_delete_not_necessary == 1){
            return;
        }
        //vuol dire che ho cancellato tutta la lista
        if(first_node == NULL && last_node == NULL){
            head = old_lines_starting_pointer;
            assert(head->prev == NULL);
            tail = old_lines_ending_pointer;
            assert(tail->next == NULL);
            TOTAL_NUMBER_OF_LINES = number_of_deletes;
            return;
        }
        //se cancello dalla prima riga
        if(first_node == NULL){
            head = old_lines_starting_pointer;
            assert(head->prev == NULL);
            last_node -> prev = old_lines_ending_pointer;
            old_lines_ending_pointer -> next = last_node;
            TOTAL_NUMBER_OF_LINES = TOTAL_NUMBER_OF_LINES + number_of_deletes;
            return;
        }
        //se cancello l'ultima riga
        if(last_node == NULL){
            tail = old_lines_ending_pointer;
            assert(tail->next == NULL);
            first_node -> next = old_lines_starting_pointer;
            old_lines_starting_pointer -> prev = first_node;
            TOTAL_NUMBER_OF_LINES = TOTAL_NUMBER_OF_LINES + number_of_deletes;
            return;
        }
        //se cancello in mezzo
        if(first_node != NULL && last_node != NULL){
            first_node -> next = old_lines_starting_pointer;
            old_lines_starting_pointer -> prev = first_node;
            last_node -> prev = old_lines_ending_pointer;
            old_lines_ending_pointer -> next = last_node;
            TOTAL_NUMBER_OF_LINES = TOTAL_NUMBER_OF_LINES + number_of_deletes;
            return;
        }
    }
    else {
        if(first_node == NULL && last_node != NULL){
            if(number_of_push == -1){
                head = old_lines_starting_pointer;
                assert(head->prev == NULL);
                old_lines_ending_pointer -> next = last_node;
                last_node -> prev = old_lines_ending_pointer;
                new_lines_ending_pointer->next = NULL;
                new_lines_starting_pointer-> prev = NULL;
            }
            return;
        }
        //here
        if(first_node != NULL && last_node == NULL){
            if(old_lines_starting_pointer == NULL){
                tail = first_node;
                first_node -> next = NULL;
            }
            else{
                tail = old_lines_ending_pointer;
                first_node -> next = old_lines_starting_pointer;
                old_lines_starting_pointer -> prev = first_node;
            }
            new_lines_ending_pointer->next = NULL;
            new_lines_starting_pointer-> prev = NULL;
            assert(tail->next == NULL);
            if(number_of_push != -1){
                TOTAL_NUMBER_OF_LINES = TOTAL_NUMBER_OF_LINES - number_of_push;
            }
            return;
        }
        if(first_node == NULL && last_node == NULL){
            head = old_lines_starting_pointer;
            if(old_lines_starting_pointer != NULL)
                assert(head->prev == NULL);
            tail = old_lines_ending_pointer;
            if(old_lines_ending_pointer != NULL)
                assert(tail->next == NULL);
            if(number_of_push != -1){
                TOTAL_NUMBER_OF_LINES = TOTAL_NUMBER_OF_LINES - number_of_push;
            }
            new_lines_ending_pointer->next = NULL;
            new_lines_starting_pointer-> prev = NULL;
            return;
        }
        if(first_node != NULL && last_node != NULL){
            first_node -> next = old_lines_starting_pointer;
            old_lines_starting_pointer -> prev = first_node;
            last_node -> prev = old_lines_ending_pointer;
            old_lines_ending_pointer -> next = last_node;
            new_lines_ending_pointer->next = NULL;
            new_lines_starting_pointer-> prev = NULL;
            return;
        }
    }

}

/**
 * Handles the redo command
 */
void handle_redo(){
    int tmp_redo;
    //se la cronologia è intatta  ed ho un redo
    if(HISTORY_IS_BEING_EDITED == -1 && IS_UNDO_WAITING == 0) return;
    //se la cronologia è intatta è c'è un undo che aspetta
    if(HISTORY_IS_BEING_EDITED == -1 && IS_UNDO_WAITING == 1){
        //se voglio fare più redo che undo
        if(CURRENT_ACTION.number >= NUMBER_OF_UNDO){
            IS_UNDO_WAITING = 0;
            NUMBER_OF_UNDO = 0;
            return;
        }
            //se voglio fare più undu di redu
        else{
            NUMBER_OF_UNDO = NUMBER_OF_UNDO - CURRENT_ACTION.number;
            return;
        }
    }
    //se la cronologia è stata modificata e nessuno aspetta (quindi c'è stato un print)
    if(HISTORY_IS_BEING_EDITED != -1 && IS_UNDO_WAITING == 0 && IS_REDO_WAITING == 0){
        tmp_redo = CURRENT_ACTION.number;
        if(TMP_NUMB_OF_ACTIONS + tmp_redo > NUMBER_OF_TOTAL_ACTIONS){
            tmp_redo = NUMBER_OF_TOTAL_ACTIONS-TMP_NUMB_OF_ACTIONS;
        }
        NUMBER_OF_REDO = tmp_redo;
        IS_REDO_WAITING = 1;
        return;
    }
        //se la cronologia è stata modificata e un redu sta aspettando
    else if(HISTORY_IS_BEING_EDITED != -1 && IS_UNDO_WAITING == 0 && IS_REDO_WAITING == 1){
        tmp_redo = NUMBER_OF_REDO + CURRENT_ACTION.number;
        if(TMP_NUMB_OF_ACTIONS + tmp_redo > NUMBER_OF_TOTAL_ACTIONS){
            tmp_redo = NUMBER_OF_TOTAL_ACTIONS-TMP_NUMB_OF_ACTIONS;
        }
        NUMBER_OF_REDO = tmp_redo;
        IS_REDO_WAITING = 1;
        return;
    }
        //se la cronologia è stata modificata e un undu sta aspettando
    else if(HISTORY_IS_BEING_EDITED != -1 && IS_UNDO_WAITING == 1 && IS_REDO_WAITING == 0){
        //se il numero degli undu è uguale a quello dei redu
        if(NUMBER_OF_UNDO == CURRENT_ACTION.number){
            NUMBER_OF_UNDO = 0;
            IS_UNDO_WAITING = 0;
            return;
        }
            //se il numero degli undu è maggiore a quello dei redu
        else if(NUMBER_OF_UNDO > CURRENT_ACTION.number){
            NUMBER_OF_UNDO = NUMBER_OF_UNDO - CURRENT_ACTION.number;
            IS_UNDO_WAITING = 1;
            return;
        }
            //se il numero degli undu è minore a quello dei redu
        else if(NUMBER_OF_UNDO < CURRENT_ACTION.number){
            tmp_redo = CURRENT_ACTION.number - NUMBER_OF_UNDO;
            if(TMP_NUMB_OF_ACTIONS + tmp_redo > NUMBER_OF_TOTAL_ACTIONS){
                tmp_redo = NUMBER_OF_TOTAL_ACTIONS-TMP_NUMB_OF_ACTIONS;
            }
            NUMBER_OF_REDO = tmp_redo;
            NUMBER_OF_UNDO = 0;
            IS_UNDO_WAITING = 0;
            IS_REDO_WAITING = 1;
            return;
        }
    }
}

void redo(){
    int i;
    if(TMP_NUMB_OF_ACTIONS == NUMBER_OF_TOTAL_ACTIONS) return;
    for(i = 0; i<NUMBER_OF_REDO; i++){
        insert_for_redo();
        TMP_NUMB_OF_ACTIONS ++;
        delete_redo_and_put_in_undu();
    }
}

void insert_for_redo() {
    char command = tail_redo->action->command;
    node_t *first_node = tail_redo->action->first_node;
    node_t *last_node = tail_redo->action->last_node;
    node_t *new_lines_starting_pointer = tail_redo->action->new_lines_starting_pointer;
    node_t *new_lines_ending_pointer = tail_redo->action->new_lines_ending_pointer;
    node_t *old_lines_starting_pointer = tail_redo->action->old_lines_starting_pointer;
    node_t *old_lines_ending_pointer = tail_redo->action->old_lines_ending_pointer;
    int number_of_deletes = tail_redo->action->number_of_deletes;
    int number_of_push = tail_redo->action->number_of_push;

    if (command == 'd') {
        if (tail_redo->action->was_delete_not_necessary == 1) {
            return;
        }
        if (first_node == NULL && last_node == NULL) {
            head = tail = NULL;
            TOTAL_NUMBER_OF_LINES = 0;
            return;
        }
        if (first_node == NULL) {
            head = last_node;
            last_node->prev = NULL;
            assert(head->prev == NULL);
            old_lines_ending_pointer->next = NULL;
            TOTAL_NUMBER_OF_LINES = TOTAL_NUMBER_OF_LINES - number_of_deletes;
            return;
        }
        if (last_node == NULL) {
            tail = first_node;
            first_node->next = NULL;
            assert(tail->next == NULL);
            old_lines_starting_pointer->prev = NULL;
            TOTAL_NUMBER_OF_LINES = TOTAL_NUMBER_OF_LINES - number_of_deletes;
            return;
        }
        if (first_node != NULL && last_node != NULL) {
            first_node->next = last_node;
            last_node->prev = first_node;
            old_lines_starting_pointer->prev = NULL;
            old_lines_ending_pointer->next = NULL;
            TOTAL_NUMBER_OF_LINES = TOTAL_NUMBER_OF_LINES - number_of_deletes;
            return;
        }
    } else {
        if(first_node == NULL && last_node != NULL){
            if(number_of_push == -1){
                head = new_lines_starting_pointer;
                assert(head->prev == NULL);
                new_lines_ending_pointer -> next = last_node;
                last_node -> prev = new_lines_ending_pointer;
                old_lines_ending_pointer->next = NULL;
                old_lines_starting_pointer->prev = NULL;
            }
            return;
        }
        if(first_node != NULL && last_node == NULL){
            tail = new_lines_ending_pointer;
            assert(tail->next == NULL);
            first_node -> next = new_lines_starting_pointer;
            new_lines_starting_pointer -> prev = first_node;
            if(old_lines_starting_pointer != NULL)
                old_lines_starting_pointer-> prev = NULL;
            if(old_lines_ending_pointer != NULL)
                old_lines_ending_pointer->next = NULL;
            if(number_of_push != -1){
                TOTAL_NUMBER_OF_LINES = TOTAL_NUMBER_OF_LINES + number_of_push;
            }
            return;
        }
        if(first_node == NULL && last_node == NULL){
            head = new_lines_starting_pointer;
            assert(head->prev == NULL);
            tail = new_lines_ending_pointer;
            assert(tail->next == NULL);
            if(number_of_push != -1){
                TOTAL_NUMBER_OF_LINES = TOTAL_NUMBER_OF_LINES + number_of_push;
            }
            if(old_lines_starting_pointer!= NULL)
                old_lines_starting_pointer->prev = NULL;
            if(old_lines_ending_pointer!= NULL)
                old_lines_ending_pointer-> next = NULL;
            return;
        }
        if(first_node != NULL && last_node != NULL){
            first_node -> next = new_lines_starting_pointer;
            new_lines_starting_pointer -> prev = first_node;
            last_node -> prev = new_lines_ending_pointer;
            new_lines_ending_pointer -> next = last_node;
            old_lines_ending_pointer->next = NULL;
            old_lines_starting_pointer-> prev = NULL;
            return;
        }
    }
}

void assign_all_history_pointers_to_null(){
    ASSIGN_OLD_STARTING_PTR(NULL);
    ASSIGN_OLD_ENDING_PTR(NULL);
    ASSIGN_NEW_STARTING_PTR(NULL);
    ASSIGN_NEW_ENDING_PTR(NULL);
    ASSIGN_FIRST_NODE(NULL);
    ASSIGN_LAST_NODE(NULL);
}

void delete_from_redu_list(){
    int number_of_strings = 0;
    int i;
    if(tail_redo -> prev == NULL){
        if(tail_redo->action->command == 'c'){
            number_of_strings = tail_redo->action->last_row - tail_redo->action->first_row + 1;
        }
        else
            number_of_strings = tail_redo -> action -> number_of_deletes;
        /* for (i = 0; i < number_of_strings; i ++){
             if(tail_redo->action->old_lines_starting_pointer != NULL){
                 free(tail_redo->action->old_lines_starting_pointer->string);
                 if(tail_redo->action->old_lines_starting_pointer->next != NULL)
                     tail_redo->action->old_lines_starting_pointer = tail_redo->action->old_lines_starting_pointer->next;
             }
         }*/
        free(tail_redo->action);
        ptr_redo = tail_redo;
        free(ptr_redo);
        tail_redo = NULL;
    }
    else{
        if(tail_redo->action->command == 'c'){
            number_of_strings = tail_redo->action->last_row - tail_redo->action->first_row + 1;
        }
        else
            number_of_strings = tail_redo -> action -> number_of_deletes;
        /*for (i = 0; i < number_of_strings; i ++){
            if(tail_redo->action->old_lines_starting_pointer != NULL){
                free(tail_redo->action->old_lines_starting_pointer->string);
                if(tail_redo->action->old_lines_starting_pointer->next != NULL)
                    tail_redo->action->old_lines_starting_pointer = tail_redo->action->old_lines_starting_pointer->next;
            }
        }*/
        free(tail_redo->action);
        ptr_redo = tail_redo;
        ptr_redo -> prev = tail_redo->prev;
        tail_redo = ptr_redo -> prev;
        if((ptr_redo->prev)->prev != NULL)
            tail_redo -> prev = (ptr_redo -> prev)->prev;
        free(ptr_redo);
    }

}
History* assign_to_undo_list(char command, int first_row,int last_row){
    History* current_history = malloc(sizeof(History));
    assert(current_history!= NULL);
    current_history->old_lines_ending_pointer = NULL;
    current_history->old_lines_starting_pointer = NULL;
    current_history->new_lines_starting_pointer = NULL;
    current_history -> new_lines_ending_pointer = NULL;
    current_history -> first_node = NULL;
    current_history -> last_node = NULL;
    current_history->number_of_push = -1;
    current_history->number_of_deletes = -1;
    current_history->was_change_and_push = -1;
    current_history->was_delete_not_necessary = -1;
    new_action = malloc(sizeof(list_undo));
    assert(new_action != NULL);
    new_action->action = NULL;
    new_action->prev = NULL;
    new_action->action = current_history;
    new_action->action->command = command;
    new_action->action->first_row = first_row;
    new_action->action->last_row = last_row;
    if(tail_undo == NULL){
        tail_undo = new_action;
        assert(tail_undo->prev == NULL);
    }
    else{
        ptr_undo = tail_undo;
        ptr_undo -> prev = tail_undo -> prev;
        tail_undo = new_action;
        tail_undo -> prev = ptr_undo;
    }
    return current_history;
}

void assign_to_redo_list(list_undo* action){
    if(tail_redo == NULL){
        tail_redo = action;
        tail_redo -> prev = NULL;
    }
    else{
        ptr_redo = tail_redo;
        ptr_redo -> prev = tail_redo ->prev;
        tail_redo = action;
        tail_redo -> prev = ptr_redo;
    }
}

void assign_to_undo_list_from_redu(list_redo* action){
    if(tail_undo == NULL){
        tail_undo = action;
        tail_undo -> prev = NULL;
    }
    else{
        ptr_undo = tail_undo;
        ptr_undo -> prev = tail_undo -> prev;
        tail_undo = action;
        tail_undo -> prev = ptr_undo;
    }
}

void delete_redo_and_put_in_undu(){
    if(tail_redo == NULL) return;
    list_undo* oldTail = tail_redo;
    tail_redo = tail_redo->prev;
    assign_to_undo_list_from_redu(oldTail);
}

void delete_undo_and_put_in_redo(){
    if(tail_undo == NULL) return;
    list_undo* oldTail = tail_undo;
    tail_undo = tail_undo->prev;
    assign_to_redo_list(oldTail);
}