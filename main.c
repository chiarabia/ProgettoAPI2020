#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define INPUT_COMMAND  20
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
    node_t* end_change;
    int was_delete_not_necessary;
    int was_change_and_push;
    int number_of_push;
    int number_of_deletes;
}History;

typedef struct undo{
    History * action;
    struct undo * prev;
}undo_t;



//History** all_history = NULL;
int number_of_total_actions = 0;
int history_tmp = 0;
int edit_history = -1;

typedef struct node line;
typedef struct undo list_undo;
typedef struct undo list_redo;

list_undo *tail_undo = NULL;
list_undo *new_action;
list_undo  *ptr_undo;

list_redo *tail_redo = NULL;
list_redo *ptr_redo;

int ask_for_action(char *input);
void find_command(char *input);
void create_action(char command,char *input);
char* ask_for_line(char *line);
void push();
void insert_new_line();
line* create_node(char *new_line);
void add_line(int is_first_and_only);
void search_and_delete(int number_of_line, int starting_or_ending, int end_delete);
void search_and_print(int number_of_line);
char *save_string(char *current_line);
void search_and_update();
void undo();
void assign_starting_pointer(node_t* pointer, int new_or_old);
void assign_ending_pointer(node_t* pointer, int new_or_old);
void assign_border_pointers(node_t* pointer, int first_or_last);
void assign_all_history_pointers_to_null();
void redo();
void insert_for_undo();
void insert_for_redo();
void delete_from_redu_list();
void delete_undo_and_put_in_redo();
void delete_redo_and_put_in_undu();
void assign_to_undo_list_from_redu(list_redo* action);
void assign_to_redo_list(list_undo* action);
History* assign_to_undo_list(char command, int first_row,int last_row);

Action current_action;
//is it the first time we have to save a string? -> if we need to create the first node of the list
int first_time = 1;
//the total number of rows that can be found in the database. This number is updated as soon as a line is deleted
int total_number_of_lines = 0;
//if an undo is waiting to be processed
int is_undo_waiting = 0;
//how many actions we need to undo
int number_of_undo = 0;
int is_redo_waiting = 0;
int number_of_redo = 0;


//for managing the list
line *new, *ptr;
line * head = NULL, * tail = NULL;
int number = 0;

/**
 * Handles the change command.
 * If first time == 1 we need to create the first node of the list.
 * Otherwise every c command is either an update (changes an existent line) or a push (creates a new node at the end of the list.
 * If is_undo_waiting == 1 then the undo command needs to be processed before the change
 */
void handle_change(){
    int i;

    if(is_undo_waiting == 1){
        undo();
        number_of_undo = 0;
        is_undo_waiting = 0;
        if(history_tmp != number_of_total_actions){
            int number_of_frees = number_of_total_actions - history_tmp;
            for (i = 0; i< number_of_frees; i++){
                delete_from_redu_list();
            }
        }
        number_of_total_actions = history_tmp;
        edit_history = -1;

    }
    if(is_redo_waiting == 1){
        redo();
        number_of_redo = 0;
        is_redo_waiting = 0;
        if(history_tmp != number_of_total_actions){
            int number_of_frees = number_of_total_actions - history_tmp;
            for (i = 0; i< number_of_frees; i++){
                delete_from_redu_list();
            }
        }
        number_of_total_actions = history_tmp;
        edit_history = -1;

    }

    if(edit_history != -1){
        edit_history = -1;
        if(history_tmp != number_of_total_actions){
            int number_of_frees = number_of_total_actions - history_tmp;
            for (i = 0; i< number_of_frees; i++){
                delete_from_redu_list();
            }
        }
        number_of_total_actions = history_tmp;
    }

    number_of_total_actions++;
    history_tmp ++;
    History* current_change = assign_to_undo_list(current_action.command,current_action.first_row,current_action.last_row);
    current_change->number_of_deletes = 0;
    current_change->was_change_and_push = -1;
    current_change->was_delete_not_necessary = -1;
    current_change->number_of_push = -1;
    current_change->end_change = NULL;

    insert_new_line();
    if(current_change->number_of_push == 0)
        current_change->number_of_push = -1;

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

/**
 * Handles the delete command.
 * If we have a 0,0d command we can resolve it immediately. Otherwise searches for the row that needs
 * to be deleted and deletes it if it exists.
 * If is_undo_waiting == 1 then the undo command needs to be processed before the delete
 */
void handle_delete(){
    int i;
    if(is_undo_waiting == 1){
        undo();
        number_of_undo = 0;
        is_undo_waiting = 0;
        if(history_tmp != number_of_total_actions){
            int number_of_frees = number_of_total_actions - history_tmp;
            for (i = 0; i< number_of_frees; i++){
                delete_from_redu_list();
            }
        }
        number_of_total_actions = history_tmp;
        edit_history = -1;
    }
    if(is_redo_waiting == 1){
        redo();
        number_of_redo = 0;
        is_redo_waiting = 0;
        if(history_tmp != number_of_total_actions){
            int number_of_frees = number_of_total_actions - history_tmp;
            for (i = 0; i< number_of_frees; i++){
                delete_from_redu_list();
            }
        }
        number_of_total_actions = history_tmp;
        edit_history = -1;
    }

    int first_delete = 1;
    int end_delete = 0;
    int is_delete_at_the_tails = 3;
    int number_of_deletes = 0;
    number_of_total_actions++;
    history_tmp ++;
    History* current_delete = assign_to_undo_list(current_action.command,current_action.first_row,current_action.last_row);
    current_delete->number_of_deletes = -1;
    current_delete->was_change_and_push = -1;
    current_delete->was_delete_not_necessary = -1;
    current_delete->number_of_push = -1;
    current_delete->end_change = NULL;
    if((current_action.first_row == 0 && current_action.last_row == 0) || current_action.first_row > total_number_of_lines) {
        assign_all_history_pointers_to_null();
        current_delete->was_delete_not_necessary = 1;
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
    //which is the current line we are working with
    int current_line_number= current_action.first_row;
    //how many lines we need to work with
    int number_of_lines = current_action.last_row - current_action.first_row + 1;

    for (i = 0; i < number_of_lines; i ++){
        if(current_action.first_row == current_action.last_row) is_delete_at_the_tails = 4; //only one delete
        else if(first_delete == 1) is_delete_at_the_tails  = 0; //the first delete
        else if(number_of_lines == number_of_deletes + 1) is_delete_at_the_tails = 1; //the last delete
        else is_delete_at_the_tails = 3;
        //if the current row that needs to be deleted doesnt exist then no action can be taken
        if(current_line_number > total_number_of_lines) {
            if(current_action.first_row>total_number_of_lines){
                current_delete->number_of_deletes = number_of_deletes;
                assign_starting_pointer(NULL,1);
                assign_ending_pointer(NULL,1);
                assign_border_pointers(NULL,0);
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
            assign_border_pointers(NULL,0);
            assign_ending_pointer(NULL,1);
            current_delete->number_of_deletes = number_of_deletes;
            //assign_ending_pointer(NULL,0);
            return;
        }

        //otherwise deletes the row
        else{
            if(current_line_number == total_number_of_lines) end_delete = 1;
            search_and_delete(current_line_number,is_delete_at_the_tails,end_delete);
        }
        is_delete_at_the_tails = 3;
        number_of_deletes ++;
        first_delete = 0;
    }
    current_delete->number_of_deletes = number_of_deletes;
    assign_starting_pointer(NULL,1);
    assign_ending_pointer(NULL,1);
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

/**
 * Handles the print command.
 * If we have 0,0p we can print a dot immediately.
 * Otherwise it prints the rows requested
 * If is_undo_waiting == 1 then the undo command needs to be processed before the print
 */
void handle_print(){
    int i;
    int number_of_lines = current_action.last_row - current_action.first_row + 1;
    if(is_undo_waiting == 1){
        undo();
        is_undo_waiting = 0;
        number_of_undo = 0;
        edit_history = 1;
    }
    if(is_redo_waiting == 1){
        redo();
        is_redo_waiting = 0;
        number_of_redo = 0;
        edit_history = 1;
    }
    if(current_action.first_row == 0 && current_action.last_row == 0)
        printf("%c\n",'.' );
    if(current_action.first_row > total_number_of_lines){
        for(i = 0; i < number_of_lines; i++){
            printf("%c\n",'.' );
        }
    }
    else{
        int current_line_number= current_action.first_row;

        for ( i = 0; i < number_of_lines; i ++){
            search_and_print(current_line_number);
            current_line_number ++;
        }
    }
}

/**
 * Handles the undo command.
 * If an undo command presents itself then is_undo_waiting is set to true. The undo will take action only if necessary.
 * If an undo command presents itself after another undo command then the number of actions that will need to be undo will be updated
 * but the same waiting logic applies
 */
void handle_undo(){
    int tmp_undo = current_action.number;
    //se la cronologia è intatta  ed ho un undu
    if(edit_history == -1 && is_undo_waiting == 0){
        if(number_of_total_actions - tmp_undo <= 0){
            tmp_undo = number_of_total_actions;
        }
        is_undo_waiting = 1;
        number_of_undo = tmp_undo;
        return;
    }
    //se la cronologia è intatta è c'è un undo che aspetta
    if(edit_history == -1 && is_undo_waiting == 1){
        tmp_undo = number_of_undo + current_action.number;
        if(number_of_total_actions - tmp_undo <= 0){
            tmp_undo = number_of_total_actions;
        }
        is_undo_waiting = 1;
        number_of_undo = tmp_undo;
        return;
    }
    //se la cronologia è stata modificata e nessuno aspetta (quindi c'è stato un print)
    if(edit_history != -1 && is_undo_waiting == 0 && is_redo_waiting == 0){
        tmp_undo = current_action.number;
        if(history_tmp - tmp_undo <= 0){
            tmp_undo = history_tmp;
        }
        is_undo_waiting = 1;
        number_of_undo = tmp_undo;
        return;
    }
    //se la cronologia è stata modificata e un redu sta aspettando
    else if(edit_history != -1 && is_undo_waiting == 0 && is_redo_waiting == 1){
        //se il numero degli undu è uguale a quello dei redu
        if(number_of_redo == current_action.number){
            is_redo_waiting = 0;
            number_of_redo = 0;
            return;
        }
        //se il numero degli undu è minore a quello dei redu
        if(number_of_redo > current_action.number){
            number_of_redo = number_of_redo - current_action.number;
            is_redo_waiting = 1;
            return;
        }
        //se il numero degli undu è maggiore a quello dei redu
        if(number_of_redo < number_of_undo){
            tmp_undo = current_action.number - number_of_redo;
            if(history_tmp - tmp_undo <= 0){
                tmp_undo = history_tmp;
            }
            number_of_undo = tmp_undo;
            is_redo_waiting = 0;
            number_of_redo = 0;
            is_undo_waiting = 1;
            return;
        }
    }
    //se la cronologia è stata modificata e un undu sta aspettando
    else if(edit_history != -1 && is_undo_waiting == 1 && is_redo_waiting == 0){
        tmp_undo = number_of_undo + current_action.number;
        if(history_tmp - tmp_undo <= 0){
            tmp_undo = history_tmp;
        }
        is_undo_waiting = 1;
        number_of_undo = tmp_undo;
        return;
    }
}

/**
 * Handles the redo command
 */
void handle_redo(){
    int tmp_redo = current_action.number;
    //se la cronologia è intatta  ed ho un redo
    if(edit_history == -1 && is_undo_waiting == 0) return;
    //se la cronologia è intatta è c'è un undo che aspetta
    if(edit_history == -1 && is_undo_waiting == 1){
        //se voglio fare più redo che undo
        if(current_action.number >= number_of_undo){
            is_undo_waiting = 0;
            number_of_undo = 0;
            return;
        }
        //se voglio fare più undu di redu
        else{
            number_of_undo = number_of_undo - current_action.number;
            return;
        }
    }
    //se la cronologia è stata modificata e nessuno aspetta (quindi c'è stato un print)
    if(edit_history != -1 && is_undo_waiting == 0 && is_redo_waiting == 0){
        tmp_redo = current_action.number;
        if(history_tmp + tmp_redo > number_of_total_actions){
            tmp_redo = number_of_total_actions-history_tmp;
        }
        number_of_redo = tmp_redo;
        is_redo_waiting = 1;
        return;
    }
    //se la cronologia è stata modificata e un redu sta aspettando
    else if(edit_history != -1 && is_undo_waiting == 0 && is_redo_waiting == 1){
        tmp_redo = number_of_redo + current_action.number;
        if(history_tmp + tmp_redo > number_of_total_actions){
            tmp_redo = number_of_total_actions-history_tmp;
        }
        number_of_redo = tmp_redo;
        is_redo_waiting = 1;
        return;
    }
    //se la cronologia è stata modificata e un undu sta aspettando
    else if(edit_history != -1 && is_undo_waiting == 1 && is_redo_waiting == 0){
        //se il numero degli undu è uguale a quello dei redu
        if(number_of_undo == current_action.number){
            number_of_undo = 0;
            is_undo_waiting = 0;
            return;
        }
        //se il numero degli undu è maggiore a quello dei redu
        else if(number_of_undo > current_action.number){
            number_of_undo = number_of_undo - current_action.number;
            is_undo_waiting = 1;
            return;
        }
        //se il numero degli undu è minore a quello dei redu
        else if(number_of_undo < current_action.number){
            tmp_redo = current_action.number - number_of_undo;
            if(history_tmp + tmp_redo > number_of_total_actions){
                tmp_redo = number_of_total_actions-history_tmp;
            }
            number_of_redo = tmp_redo;
            number_of_undo = 0;
            is_undo_waiting = 0;
            is_redo_waiting = 1;
            return;
        }
    }
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

int main(int argc, char*argv[]) {
    //all_history = malloc(BUFFER_DIMENSION * sizeof(History*));
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
 * Updates the current_action with the command type, what lines need changes, and how many lines if undo/redo
 * @param command the command type of the action
 * @param input the command input of the user
 */
void create_action(char command,char *input){
    current_action.command = command;
    if(command == 'u'){
        current_action.number = atoi(strtok(input,"u"));
        return;
    }
    else if(command == 'r'){
        current_action.number = atoi(strtok(input,"r"));
        return;
    }
    char delim[] = ",";
    current_action.first_row =  atoi(strtok(input,delim));
    if(command == 'c')
        current_action.last_row = atoi(strtok(NULL,"c"));
    else if(command == 'd')
        current_action.last_row = atoi(strtok(NULL,"d"));
    else if(command == 'p')
        current_action.last_row = atoi(strtok(NULL,"p"));
}

/**
 * Asks to input a string when needed (change command)
 * @param line where to save the new string
 * @return the string now with the new data
 */
char* ask_for_line(char *line){
    //printf("i ll ask the line\n");
    fgets(line, INPUT_LINE, stdin);
    //removes the \n as the last char and updates it to \0
    //line[strlen(line) -1 ] = '\0';
    char *new_line = save_string(line);
    //printf(">%s\n",line);
    return new_line;
}


/**
 * Creates a new node of the list with the string
 * @param new_line the string that needs to be saved in that node
 * @return the new node
 */
line* create_node(char *new_line){
    number ++;
    new = (line *)malloc(sizeof(line));
    assert(new != NULL) ;
    new -> string = new_line;
    new -> next = NULL;
    new -> prev = NULL;
    return new;
}


/**
 * Searches for the needed row and prints it
 * @param number_of_line the row number we want to print
 */
void search_and_print(int number_of_line){
    int count_head = 1,i;
    //if no Lists exists just print a dot
    if( head == NULL){
        printf(". \n");
        return;
    }
    //if the number is lower than the total number of rows/2 the search can start at the head of the list
    else {
        for (ptr = head, i = 1; i <=total_number_of_lines; i ++, ptr = ptr -> next){
            if(count_head == number_of_line){
                //if the string exist it can be printed
                if (ptr != NULL){
                    printf("%s",ptr->string);
                    return;
                }
                //otherwise a dot
                else{
                    printf("%c\n",'.' );
                    return;
                }
            }
            count_head++;
        }
    }
    if(number_of_line > total_number_of_lines){
        printf("%c\n",'.' );
        return;
    }

}

/**
 * Searches and deletes the node (but not the string) of the row that needs to be deleted
 * @param number_of_line the row that needs to be deleted
 */
void search_and_delete(int number_of_line,int starting_or_ending, int end_delete){
    line *prevnode,*nextnode;
    int count_head = 1,i;
    //if the lists does not exists
    if(head == tail && head == NULL){
        //the list will need to be created at the next change
        first_time =1 ;
        return;}
    //if the row is the first one
    if (number_of_line == 1){
        ptr = head;
        //if the node is the only one
        if(head == tail){
            //resets the Lists
            head = tail = NULL;
            if(end_delete == 1) assign_ending_pointer(ptr, 0);
            if(starting_or_ending == 0){
                assign_starting_pointer(ptr,0);
                assign_border_pointers(NULL,1);
            }
            else if(starting_or_ending == 4){
                assign_starting_pointer(ptr,0);
                assign_ending_pointer(ptr,0);
                assign_border_pointers(NULL,1);
                assign_border_pointers(NULL,0);
            }
            else if(starting_or_ending == 1){
                assign_ending_pointer(ptr,0);
                assign_border_pointers(NULL,0);
            }
            first_time = 1;
            total_number_of_lines--;
            return;
        }

        //the position of the head is shifted
        head = ptr -> next;
        head -> prev = NULL;
        if(ptr->next != NULL)
            head -> next = (ptr -> next) -> next;
        /*if(ptr->prev != NULL)
            head -> prev = (ptr->prev) -> prev;*/
        if(end_delete == 1) assign_ending_pointer(ptr, 0);
        if(starting_or_ending == 0){
            assign_starting_pointer(ptr,0);
            assign_border_pointers(NULL,1);
            ptr -> prev = NULL;
        }
        else if(starting_or_ending == 1){
            assign_ending_pointer(ptr,0);
            assign_border_pointers(ptr->next,0);
        }
        else if(starting_or_ending == 4){
            assign_starting_pointer(ptr,0);
            assign_ending_pointer(ptr,0);
            assign_border_pointers(NULL,1);
            assign_border_pointers(ptr->next,0);
        }
        //ptr changes
        total_number_of_lines--;
        return;
    }
    //if the row is the last node
    else if(number_of_line > 1 && number_of_line == total_number_of_lines){
        //the tail is shifted
        ptr = tail;
        tail = ptr -> prev;
        tail -> next = head;
        if(end_delete == 1) assign_ending_pointer(ptr, 0);
        if(starting_or_ending == 0){
            assign_starting_pointer(ptr,0);
            assign_ending_pointer(ptr,0);
            assign_border_pointers(ptr->prev,1);
            assign_border_pointers(NULL,0);
            ptr->prev = NULL;
            ptr->next = NULL;
        }
        else if(starting_or_ending == 1){
            assign_ending_pointer(ptr,0);
            assign_border_pointers(NULL,0);
        }
        else if(starting_or_ending == 4){
            assign_ending_pointer(ptr,0);
            assign_starting_pointer(ptr,0);
            assign_border_pointers(NULL,0);
            assign_border_pointers(ptr->prev,1);
            ptr-> prev = NULL;
            ptr-> next = NULL;
        }
        total_number_of_lines--;
        return;
    }
    else {
        for (ptr = head, i = 1; i <= total_number_of_lines; i ++, ptr = ptr -> next){
            if(count_head == number_of_line){
                prevnode = ptr -> prev;
                nextnode = ptr-> next;
                prevnode -> next = nextnode;
                nextnode -> prev = prevnode;
                if(end_delete == 1) assign_ending_pointer(ptr, 0);
                if(starting_or_ending == 1){
                    assign_ending_pointer(ptr,0);
                    assign_border_pointers(ptr->next,0);
                    ptr -> next = NULL;
                }
                else if(starting_or_ending == 0){
                    assign_starting_pointer(ptr,0);
                    assign_border_pointers(ptr->prev,1);
                    ptr->prev = NULL;
                }
                else if(starting_or_ending == 4){
                    assign_starting_pointer(ptr,0);
                    assign_ending_pointer(ptr,0);
                    assign_border_pointers(ptr->prev,1);
                    assign_border_pointers(ptr->next,0);
                    ptr->next = NULL;
                    ptr->prev = NULL;
                }
                total_number_of_lines--;
                return;
            }
            count_head ++;
        }
    }

}

/**
 * Saves the string permanently
 * @param current_line the string that needs to be saved
 * @return where the string was saved
 */
char *save_string(char *current_line){
    int size = strlen(current_line);
    char *new_string = malloc(size +1);
    strcpy(new_string,current_line);
    assert(new_string != NULL);
    return new_string;
}

History* assign_to_undo_list(char command, int first_row,int last_row){
    History* current_history = malloc(sizeof(History));
    new_action = (list_undo *)malloc(sizeof(list_undo));
    new_action->action = current_history;
    new_action->action->command = command;
    new_action->action->first_row = first_row;
    new_action->action->last_row = last_row;
    if(tail_undo == NULL){
        tail_undo = new_action;
        tail_undo -> prev = NULL;
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
    if(tail_redo -> prev == NULL){
        assign_to_undo_list_from_redu(tail_redo);
        tail_redo = NULL;
    }
    else{
        ptr_redo = tail_redo -> prev;
        if((tail_redo->prev)->prev != NULL)
            ptr_redo ->prev = (tail_redo ->prev)->prev;
        else
            ptr_redo -> prev = NULL;
        assign_to_undo_list_from_redu(tail_redo);
        tail_redo = ptr_redo;
    }
}

void delete_undo_and_put_in_redo(){
    if(tail_undo == NULL) return;
    if(tail_undo -> prev == NULL){
        assign_to_redo_list(tail_undo);
        tail_undo = NULL;
    }
    else{
        ptr_undo = tail_undo -> prev;
        if((tail_undo->prev)->prev != NULL)
            ptr_undo -> prev = (tail_undo -> prev)->prev;
        else
            ptr_undo -> prev = NULL;
        assign_to_redo_list(tail_undo);
        tail_undo = ptr_undo;
    }
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

};

/**
 *
 * @param pointer
 * @param new_or_old 1 for new, 0 for old
 */
void assign_starting_pointer(node_t* pointer, int new_or_old){
    if(new_or_old == 1)
        tail_undo->action->new_lines_starting_pointer = pointer;
    else
        tail_undo->action->old_lines_starting_pointer = pointer;

}

/**
 *
 * @param pointer
 * @param new_or_old 1 for new, 0 for old
 */
void assign_ending_pointer(node_t* pointer, int new_or_old){
    if(new_or_old ==1)
        tail_undo->action->new_lines_ending_pointer = pointer;
    else
        tail_undo->action->old_lines_ending_pointer = pointer;
}

/**
 *
 * @param pointer
 * @param first_or_last 1 for first, 0 for last
 */
void assign_border_pointers(node_t* pointer, int first_or_last) {
    if (first_or_last == 1)
        tail_undo->action->first_node = pointer;
    else
        tail_undo->action->last_node = pointer;
}

void assign_all_history_pointers_to_null(){
    assign_starting_pointer(NULL,0);
    assign_ending_pointer(NULL,0);
    assign_starting_pointer(NULL,1);
    assign_ending_pointer(NULL,1);
    assign_border_pointers(NULL,1);
    assign_border_pointers(NULL,0);
}

void undo(){
    int i;
    if(history_tmp == 0) return;
    for(i = 0; i<number_of_undo; i++){
        insert_for_undo();
        history_tmp--;
        delete_undo_and_put_in_redo();
    }
}

void redo(){
    int i;
    if(history_tmp == number_of_total_actions) return;
    for(i = 0; i<number_of_redo; i++){
        insert_for_redo();
        history_tmp ++;
        delete_redo_and_put_in_undu();
    }
}

void insert_for_undo(){
    char command = tail_undo->action->command;
    int first_row = tail_undo->action->first_row;
    int last_row = tail_undo->action->last_row;
    node_t* first_node = tail_undo->action->first_node;
    node_t* last_node = tail_undo->action->last_node;
    node_t* new_lines_starting_pointer = tail_undo->action -> new_lines_starting_pointer;
    node_t* new_lines_ending_pointer = tail_undo->action -> new_lines_ending_pointer;
    node_t* old_lines_starting_pointer = tail_undo->action -> old_lines_starting_pointer;
    node_t* old_lines_ending_pointer = tail_undo->action -> old_lines_ending_pointer;
    int number_of_deletes = tail_undo->action ->number_of_deletes;
    int number_of_push = tail_undo->action -> number_of_push;
    int change_and_push = tail_undo->action -> was_change_and_push;

    if(command == 'd'){
        //se cancello niente
        if(tail_undo->action->was_delete_not_necessary == 1){
            return;
        }
        //vuol dire che ho cancellato tutta la lista
        if(first_node == NULL && last_node == NULL){
            head = old_lines_starting_pointer;
            head -> next = old_lines_starting_pointer -> next;
            if(old_lines_starting_pointer -> next != NULL)
                (old_lines_starting_pointer -> next) -> prev = head;
            tail = old_lines_ending_pointer;
            tail -> prev = old_lines_ending_pointer -> prev;
            if(old_lines_ending_pointer ->prev != NULL)
                (old_lines_ending_pointer->prev)->next = tail;
            //first_time = 0;
            total_number_of_lines = number_of_deletes;
            return;
        }
        //se cancello dalla prima riga
        if(first_node == NULL){
            head = old_lines_starting_pointer;
            head -> next = old_lines_starting_pointer -> next;
            if(old_lines_starting_pointer -> next != NULL)
                (old_lines_starting_pointer -> next) -> prev = head;
            last_node -> prev = old_lines_ending_pointer;
            old_lines_ending_pointer -> next = last_node;
            if(tail == last_node){
                tail -> prev = old_lines_ending_pointer;
                old_lines_ending_pointer -> next = tail;
            }
            total_number_of_lines = total_number_of_lines + number_of_deletes;
            return;
        }
        //se cancello l'ultima riga
        if(last_node == NULL){
            tail = old_lines_ending_pointer;
            tail -> prev = old_lines_ending_pointer -> prev;
            if(old_lines_ending_pointer ->prev != NULL)
                (old_lines_ending_pointer->prev)->next = tail;
            first_node -> next = old_lines_starting_pointer;
            old_lines_starting_pointer -> prev = first_node;
            if(head == first_node){
                head -> next = old_lines_starting_pointer;
                old_lines_starting_pointer -> prev = head;
            }
            total_number_of_lines = total_number_of_lines + number_of_deletes;
            return;
        }
        //se cancello in mezzo
        if(first_node != NULL && last_node != NULL){
            first_node -> next = old_lines_starting_pointer;
            old_lines_starting_pointer -> prev = first_node;
            if(head == first_node) {
                head->next = old_lines_starting_pointer;
                old_lines_starting_pointer->prev = head;
            }
            last_node -> prev = old_lines_ending_pointer;
            old_lines_ending_pointer -> next = last_node;
            if(tail == last_node) {
                tail->prev = old_lines_ending_pointer;
                old_lines_ending_pointer->next = tail;
            }

            total_number_of_lines = total_number_of_lines + number_of_deletes;
            return;
        }
    }
    else{
        //se scrivo per la prima volta nella lista
        if(first_node == NULL && last_node == NULL && change_and_push != 1 && number_of_push != -1){
            head = tail = NULL;
            first_time = 1;
            total_number_of_lines = 0;
            return;
        }
        //se cambio tutte le stringhe nella lista
        if(first_node == NULL && last_node == NULL && change_and_push != 1 && number_of_push == -1){
            head = old_lines_starting_pointer;
            head -> next = old_lines_starting_pointer -> next;
            if(old_lines_starting_pointer -> next != NULL)
                (old_lines_starting_pointer -> next) -> prev = head;
            tail = old_lines_ending_pointer;
            tail -> prev = old_lines_ending_pointer -> prev;
            if(old_lines_ending_pointer ->prev != NULL)
                (old_lines_ending_pointer->prev)->next = tail;
            return;
        }
        //se cambio tutte le stringhe e faccio anche delle push
        if(first_node == NULL && last_node == NULL && change_and_push == 1 && number_of_push != -1){
            int i;
            head = old_lines_starting_pointer;
            head -> next = old_lines_starting_pointer -> next;
            if(old_lines_starting_pointer -> next != NULL)
                (old_lines_starting_pointer -> next) -> prev = head;
            tail = old_lines_ending_pointer;
            tail -> prev = old_lines_ending_pointer -> prev;
            /*int number_of_lines_with_only_change = (last_row - first_row +1)-number_of_push;
            for(ptr=head,i=0; i<number_of_lines_with_only_change;i++, ptr = ptr->next){
                if (ptr->next == NULL){
                    tail = ptr;
                    tail -> prev = ptr-> prev;
                }
            }*/
            if(old_lines_ending_pointer ->prev != NULL)
                (old_lines_ending_pointer->prev)->next = tail;
            //sgancio
            //tail -> next = NULL;
            total_number_of_lines = total_number_of_lines - number_of_push;
            return;
        }
        //se faccio una change senza modificare la coda
        if(first_node == NULL && last_node != NULL && change_and_push != 1 && number_of_push == -1){
            head = old_lines_starting_pointer;
            head -> next = old_lines_starting_pointer -> next;
            if(old_lines_starting_pointer -> next != NULL)
                (old_lines_starting_pointer -> next) -> prev = head;
            last_node -> prev = old_lines_ending_pointer;
            old_lines_ending_pointer -> next = last_node;
            if(tail == last_node) {
                tail->prev = old_lines_ending_pointer;
                old_lines_ending_pointer->next = tail;
            }
            //sgancio
            new_lines_ending_pointer -> next = NULL;
            return;
        }
        //se faccio una change modificando la coda ma niente push
        if(last_node == NULL && first_node != NULL &&change_and_push != 1 && number_of_push == -1){
            tail = old_lines_ending_pointer;
            tail -> prev = old_lines_ending_pointer -> prev;
            if(old_lines_ending_pointer ->prev != NULL)
                (old_lines_ending_pointer->prev)->next = tail;
            first_node -> next = old_lines_starting_pointer;
            old_lines_starting_pointer -> prev = first_node;
            if(head == first_node) {
                head->next = old_lines_starting_pointer;
                old_lines_starting_pointer->prev = head;
            }
            //sgancio
            new_lines_starting_pointer -> prev = NULL;
            return;
        }
        //se faccio solo push
        if(last_node == NULL && first_node != NULL && change_and_push != 1 && number_of_push != -1){
            tail = first_node;
            tail -> prev = first_node -> prev;
            if(first_node ->prev != NULL)
                (first_node->prev)->next = tail;
            if(head == first_node) {
                tail = head;
            }
            //sgancio
            first_node -> next = NULL;
            new_lines_starting_pointer -> prev = NULL;

            total_number_of_lines = total_number_of_lines - number_of_push;
            return;
        }
        //se modifico la coda e faccio anche delle push
        if(last_node == NULL && first_node != NULL && change_and_push == 1){
            tail = old_lines_ending_pointer;
            tail -> prev = old_lines_ending_pointer -> prev;
            if(old_lines_ending_pointer ->prev != NULL)
                (old_lines_ending_pointer->prev)->next = tail;
            first_node -> next = old_lines_starting_pointer;
            old_lines_starting_pointer -> prev = first_node;
            if(head == first_node) {
                head->next = old_lines_starting_pointer;
                old_lines_starting_pointer->prev = head;
            }
            tail -> next = NULL;
            //sgancio
            new_lines_starting_pointer -> prev = NULL;
            new_lines_ending_pointer -> next = NULL;

            total_number_of_lines = total_number_of_lines - number_of_push;
            return;
        }
        //se faccio delle change dentro la struttura senza avere delle push
        if(last_node != NULL && first_node != NULL && change_and_push != 1){
            first_node -> next = old_lines_starting_pointer;
            old_lines_starting_pointer -> prev = first_node;
            if(head == first_node) {
                head->next = old_lines_starting_pointer;
                old_lines_starting_pointer->prev = head;
            }
            last_node -> prev = old_lines_ending_pointer;
            old_lines_ending_pointer -> next = last_node;
            if(tail == last_node) {
                tail->prev = old_lines_ending_pointer;
                old_lines_ending_pointer->next = tail;
            }
            //sgancio
            new_lines_starting_pointer -> prev = NULL;
            new_lines_ending_pointer -> next = NULL;
            return;
        }

    }

}

void insert_for_redo() {
    char command = tail_redo->action->command;
    int first_row = tail_redo->action->first_row;
    int last_row = tail_redo->action->last_row;
    node_t *first_node = tail_redo->action->first_node;
    node_t *last_node = tail_redo->action->last_node;
    node_t *new_lines_starting_pointer = tail_redo->action->new_lines_starting_pointer;
    node_t *new_lines_ending_pointer = tail_redo->action->new_lines_ending_pointer;
    node_t *old_lines_starting_pointer = tail_redo->action->old_lines_starting_pointer;
    node_t *old_lines_ending_pointer = tail_redo->action->old_lines_starting_pointer;
    int number_of_deletes = tail_redo->action->number_of_deletes;
    int number_of_lines = last_row - first_row + 1;
    int number_of_push = tail_redo->action->number_of_push;
    int change_and_push = tail_redo->action->was_change_and_push;

    if (command == 'd') {
        if (tail_redo->action->was_delete_not_necessary == 1) {
            return;
        }
        if (first_node == NULL && last_node == NULL) {
            head = tail = NULL;
            first_time = 1;
            total_number_of_lines = 0;
            return;
        }
        if (first_node == NULL) {
            head = last_node;
            last_node->prev = NULL;
            head->next = last_node->next;
            if (last_node->next != NULL)
                (last_node->next)->prev = head;
            old_lines_ending_pointer->next = NULL;
            total_number_of_lines = total_number_of_lines - number_of_deletes;
            return;
        }
        if (last_node == NULL) {
            tail = first_node;
            tail->prev = first_node->prev;
            if (first_node->prev != NULL)
                (first_node->prev)->next = tail;
            first_node->next = NULL;
            old_lines_starting_pointer->prev = NULL;
            total_number_of_lines = total_number_of_lines - number_of_deletes;
            return;
        }
        if (first_node != NULL && last_node != NULL) {
            first_node->next = last_node;
            last_node->prev = first_node;
            if (head == first_node && tail != last_node) {
                head->next = last_node;
                last_node->prev = head;
            }
            if (head != first_node && tail == last_node) {
                tail->prev = first_node;
                first_node->next = tail;
            }
            if (head == first_node && tail == last_node) {
                head->next = tail;
                tail->prev = head;
            }
            old_lines_starting_pointer->prev = NULL;
            old_lines_ending_pointer->next = NULL;
            total_number_of_lines = total_number_of_lines - number_of_deletes;
            return;
        }
    } else {
        //se scrivo per la prima volta nella lista
        if (first_node == NULL && last_node == NULL && change_and_push != 1 && number_of_push != -1) {
            head = new_lines_starting_pointer;
            head->next = new_lines_starting_pointer->next;
            if (new_lines_starting_pointer->next != NULL)
                (new_lines_starting_pointer->next)->prev = head;
            tail = new_lines_ending_pointer;
            tail->prev = new_lines_ending_pointer->prev;
            if (new_lines_ending_pointer->prev != NULL)
                (new_lines_ending_pointer->prev)->next = tail;
            first_time = 0;
            total_number_of_lines = number_of_push;
            return;
        }
        //se cambio tutte le stringhe nella lista
        if (first_node == NULL && last_node == NULL && change_and_push != 1 && number_of_push == -1) {
            head = new_lines_starting_pointer;
            head->next = new_lines_starting_pointer->next;
            if (new_lines_starting_pointer->next != NULL)
                (new_lines_starting_pointer->next)->prev = head;
            tail = new_lines_ending_pointer;
            tail->prev = new_lines_ending_pointer->prev;
            if (new_lines_ending_pointer->prev != NULL)
                (new_lines_ending_pointer->prev)->next = tail;
            return;
        }
        //se cambio tutte le stringhe e faccio anche delle push
        if (first_node == NULL && last_node == NULL && change_and_push == 1 && number_of_push != -1) {
            int i;
            head = new_lines_starting_pointer;
            head->next = new_lines_starting_pointer->next;
            if (new_lines_starting_pointer->next != NULL)
                (new_lines_starting_pointer->next)->prev = head;
            tail = new_lines_ending_pointer;
            tail->prev = new_lines_ending_pointer->prev;
            if (new_lines_ending_pointer->prev != NULL)
                (new_lines_ending_pointer->prev)->next = tail;
            /*for(i = 0, ptr = tail; i< number_of_lines; i++, ptr = ptr -> prev){
                if(ptr -> prev == NULL){
                    head = ptr;
                    head -> next = ptr -> next;
                }
            }*/
            total_number_of_lines = total_number_of_lines + number_of_push;
            old_lines_ending_pointer->next = NULL;
            old_lines_starting_pointer->prev = NULL;
            return;
        }
        //se faccio una change senza modificare la coda
        if (first_node == NULL && last_node != NULL && change_and_push != 1 && number_of_push == -1) {
            head = new_lines_starting_pointer;
            head->next = new_lines_starting_pointer->next;
            if (new_lines_starting_pointer->next != NULL)
                (new_lines_starting_pointer->next)->prev = head;
            last_node->prev = new_lines_ending_pointer;
            new_lines_ending_pointer->next = last_node;
            if (tail == last_node) {
                tail->prev = new_lines_ending_pointer;
                new_lines_ending_pointer->next = tail;
            }
            //sgancio
            old_lines_ending_pointer->next = NULL;
            return;
        }
        //se faccio una change modificando la coda ma niente push
        if (last_node == NULL && first_node != NULL && change_and_push != 1 && number_of_push == -1) {
            tail = new_lines_ending_pointer;
            tail->prev = new_lines_ending_pointer->prev;
            if (new_lines_ending_pointer->prev != NULL)
                (new_lines_ending_pointer->prev)->next = tail;
            first_node->next = new_lines_starting_pointer;
            new_lines_starting_pointer->prev = first_node;
            if (head == first_node) {
                head->next = new_lines_starting_pointer;
                new_lines_starting_pointer->prev = head;
            }
            //sgancio
            old_lines_starting_pointer->prev = NULL;
            return;
        }
        //se faccio solo push
        if (last_node == NULL && first_node != NULL && change_and_push != 1 && number_of_push != -1) {
            tail = new_lines_ending_pointer;
            tail->prev = new_lines_ending_pointer->prev;
            if (new_lines_ending_pointer->prev != NULL)
                (new_lines_ending_pointer->prev)->next = tail;
            first_node->next = new_lines_starting_pointer;
            new_lines_starting_pointer->prev = first_node;
            if (head == first_node) {
                head->next = new_lines_starting_pointer;
                new_lines_starting_pointer->prev = head;
            }
            total_number_of_lines = total_number_of_lines + number_of_push;
            return;
        }
        //se modifico la coda e faccio anche delle push
        if (last_node == NULL && first_node != NULL && change_and_push == 1) {
            first_node->next = new_lines_starting_pointer;
            new_lines_starting_pointer->prev = first_node;
            if (head == first_node) {
                head->next = new_lines_starting_pointer;
                new_lines_starting_pointer->prev = head;
            }
            tail = new_lines_ending_pointer;
            tail->prev = new_lines_ending_pointer->prev;
            if (new_lines_ending_pointer->prev != NULL)
                (new_lines_ending_pointer->prev)->next = tail;
            //stacco
            old_lines_starting_pointer->prev = NULL;

            total_number_of_lines = total_number_of_lines + number_of_push;
            return;
        }
        //se faccio delle change dentro la struttura senza avere delle push
        if (last_node != NULL && first_node != NULL && change_and_push != 1) {
            first_node->next = new_lines_starting_pointer;
            new_lines_starting_pointer->prev = first_node;
            if (head == first_node) {
                head->next = new_lines_starting_pointer;
                new_lines_starting_pointer->prev = head;
            }
            last_node->prev = new_lines_ending_pointer;
            new_lines_ending_pointer->next = last_node;
            if (tail == last_node) {
                tail->prev = new_lines_ending_pointer;
                new_lines_ending_pointer->next = tail;
            }
            //sgancio
            old_lines_starting_pointer->prev = NULL;
            old_lines_ending_pointer->next = NULL;
            return;
        }
    }
}

    void insert_new_line() {
        int i;
        line *ask_line = NULL;
        line *tmp = NULL;
        int number_of_push = 0;
        int lines;
        int number_of_lines = current_action.last_row - current_action.first_row + 1;

        if (current_action.first_row <= total_number_of_lines && current_action.last_row > total_number_of_lines)
            tail_undo->action->was_change_and_push = 1;
        if(number_of_lines > total_number_of_lines)
            lines = total_number_of_lines;
        if(number_of_lines <= total_number_of_lines)
            lines = number_of_lines;
        //trovo i vecchi puntatori
        if (head != NULL && current_action.first_row <= total_number_of_lines) {
            for (i = 1, ptr = head; i <= total_number_of_lines; i++, ptr = ptr->next) {
                if (i == current_action.first_row) {
                    assign_starting_pointer(ptr, 0);
                    //se non sto modificando anche la testa
                    if (ptr->prev != NULL){
                        assign_border_pointers(ptr->prev, 1);
                        current_action.first_ptr = ptr->prev;
                    }
                    else{
                        //se sto modificando la testa
                        assign_border_pointers(NULL, 1);
                        current_action.first_ptr = NULL;
                    }

                }
                if (i == current_action.last_row || i == total_number_of_lines) {
                    assign_ending_pointer(ptr, 0);
                    //se non sto modificando anche la coda
                    if (ptr->next != NULL) {
                        if (i == current_action.last_row){
                            assign_border_pointers(ptr->next, 0);
                            current_action.last_ptr = ptr->next;
                            break;
                        }
                    } else{
                        assign_border_pointers(NULL, 0);
                        current_action.last_ptr =NULL;
                        break;
                    }
                }

            }
        }

        //chiedo le nuove linee e li segno come nuovi puntatori
        for (i = 0; i < number_of_lines; i++) {
            char current_line[INPUT_LINE];
            char *new_line = ask_for_line(current_line);
            if (ask_line != NULL) {
                //tmp->prev = ask_line->prev;
                //tmp -> next = ask_line -> next;
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
                assign_starting_pointer(ask_line, 1);
                current_action.starting_ptr = ask_line;
            }
            if (i == number_of_lines - 1) {
                assign_ending_pointer(ask_line, 1);
                current_action.ending_ptr = ask_line;
            }
        }
        //se ancora non c'è la testa
        if (head == NULL) {
            assign_border_pointers(NULL, 1);
            tail_undo->action->last_node = NULL;
            assign_ending_pointer(NULL, 0);
            assign_starting_pointer(NULL, 0);
            head = current_action.starting_ptr;
            head->next = current_action.starting_ptr->next;
            tail = current_action.ending_ptr;
            tail->prev = current_action.ending_ptr->prev;
            head -> prev = NULL;
            total_number_of_lines = number_of_lines;
            number_of_push = total_number_of_lines;
            tail_undo->action->number_of_push = number_of_push;
        }
            //se già esiste la lista
        else {
            //se sto modificando la testa
            if (current_action.first_row == 1) {
                head = current_action.starting_ptr;
                head->next =current_action.starting_ptr->next;
                head -> prev = NULL;
                //se ho anche righe dopo la coda
                if(current_action.last_row>=total_number_of_lines){
                    tail = current_action.ending_ptr;
                    tail->prev = current_action.ending_ptr->prev;
                    number_of_push = current_action.last_row - total_number_of_lines;
                    total_number_of_lines = total_number_of_lines + number_of_push;
                    tail_undo->action->number_of_push = number_of_push;
                }
                //se la modifica si ferma prima della coda
                else if(current_action.last_row < total_number_of_lines){
                    current_action.last_ptr -> prev = current_action.ending_ptr;
                    current_action.ending_ptr -> next = current_action.last_ptr;
                }
                if(tail_undo->action->old_lines_starting_pointer!= NULL )
                    tail_undo->action->old_lines_starting_pointer->prev = NULL;
                if(tail_undo->action->old_lines_ending_pointer!= NULL )
                    tail_undo->action->old_lines_ending_pointer->next = NULL;
            }
                //se NON sto modificando la testa ma sto modificando la coda o dopo la coda
            else if(current_action.last_row >= total_number_of_lines && current_action.first_row != 1) {
                if(current_action.first_row <= total_number_of_lines)
                    tail = current_action.first_ptr;
                tail->next = current_action.starting_ptr;
                current_action.starting_ptr->prev = tail;
                tail = current_action.ending_ptr;
                tail->prev = current_action.ending_ptr->prev;
                tail->next = NULL;
                if (current_action.first_row > total_number_of_lines) {
                    assign_starting_pointer(NULL,0);
                    assign_ending_pointer(NULL,0);
                    assign_border_pointers(NULL,0);
                    tail_undo->action->first_node = tail_undo->action->new_lines_starting_pointer->prev;
                    total_number_of_lines = total_number_of_lines + number_of_lines;
                    number_of_push = number_of_lines;
                    tail_undo->action->number_of_push = number_of_push;
                } else if (current_action.first_row <= total_number_of_lines &&
                           current_action.last_row > total_number_of_lines) {
                    number_of_push = current_action.last_row - total_number_of_lines;
                    total_number_of_lines = total_number_of_lines + number_of_push;
                    tail_undo->action->number_of_push = number_of_push;
                }
                if(tail_undo->action->old_lines_starting_pointer!= NULL )
                    tail_undo->action->old_lines_starting_pointer->prev = NULL;
                if(tail_undo->action->old_lines_ending_pointer!= NULL )
                    tail_undo->action->old_lines_ending_pointer->next = NULL;
            }
            //se la modifica è interna
            else if(current_action.first_row != 1 && current_action.first_row < total_number_of_lines && current_action.last_row < total_number_of_lines){
                current_action.first_ptr-> next = current_action.starting_ptr;
                current_action.starting_ptr->prev = current_action.first_ptr;
                current_action.last_ptr -> prev = current_action.ending_ptr;
                current_action.ending_ptr -> next = current_action.last_ptr;
                tail_undo->action->old_lines_starting_pointer -> prev = NULL;
                tail_undo->action->old_lines_ending_pointer -> next = NULL;
                tail_undo->action->old_lines_starting_pointer -> prev = NULL;
                tail_undo->action->old_lines_ending_pointer -> next = NULL;
            }

        }
    }