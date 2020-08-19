#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define INPUT_COMMAND  20
#define INPUT_LINE 1024
#define BUFFER_DIMENSION 50

//ACTION saves the data of requested actions
typedef struct{
    char command;
    int first_row;
    int last_row;
    int number;
} Action;

//the node of the LIST for database with the the strings
typedef struct node {
    char* string;
    struct node *next;
    struct node *prev;
}node_t;

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

int buffer_dimension = BUFFER_DIMENSION;
History** all_history = NULL;
int number_of_total_actions = 0;
int history_tmp = 0;
int edit_history = -1;

typedef struct node line;

int ask_for_action(char *input);
void find_command(char *input);
void create_action(char command,char *input);
char* ask_for_line(char *line);
void push(int starting_or_ending);
line* create_node(char *new_line);
void add_line(int is_first_and_only);
void search_and_delete(int number_of_line, int starting_or_ending, int end_delete);
void search_and_print(int number_of_line);
char *save_string(char *current_line);
void search_and_update(int number_of_line, int starting_or_ending, int end_change, int start_change);
void undo();
History* create_history_object(char command,int first_row, int last_row);
void add_space_to_buffer();
void assign_starting_pointer(node_t* pointer, int new_or_old);
void assign_ending_pointer(node_t* pointer, int new_or_old);
void assign_border_pointers(node_t* pointer, int first_or_last);
void assign_all_history_pointers_to_null();
void delete_assign_data_of_history(int array_position_for_data);
void redo();
void insert_for_undo();
void insert_for_redo();

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
    int number_push = 0;
    int is_push_at_the_tails = 3;
    int end_change = 0;
    int start_change = 1;

    if(is_undo_waiting == 1){
        undo();
        number_of_undo = 0;
        is_undo_waiting = 0;
        if(history_tmp != number_of_total_actions){
            int number_of_frees = number_of_total_actions - history_tmp;
            int action = history_tmp + 1;
            for (i = 0; i< number_of_frees; i++){
                delete_assign_data_of_history(action);
                action ++;
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
            int action = history_tmp + 1;
            for (i = 0; i< number_of_frees; i++){
                delete_assign_data_of_history(action);
                action ++;
            }
        }
        number_of_total_actions = history_tmp;
        edit_history = -1;

    }

    if(edit_history != -1){
        edit_history = -1;
        if(history_tmp != number_of_total_actions){
            int number_of_frees = number_of_total_actions - history_tmp;
            int action = history_tmp + 1;
            for (i = 0; i< number_of_frees; i++){
                delete_assign_data_of_history(action);
                action ++;
            }
        }
        number_of_total_actions = history_tmp;
    }

    number_of_total_actions++;
    history_tmp ++;
    History* current_change = create_history_object(current_action.command,current_action.first_row,current_action.last_row);
    //how many lines we need to work with
    int number_of_lines = current_action.last_row - current_action.first_row + 1;
    //which is the current line we are working with
    int current_line_number = current_action.first_row;
    //how many lines that we need to work with are left
    int how_many_lines_left = 0;
    int last_line = number_of_lines - how_many_lines_left;

    if (first_time == 1){
        is_push_at_the_tails = 2;
        int is_first_and_only = 0;
        if(current_action.first_row == current_action.last_row) is_first_and_only = 1;
        add_line(is_first_and_only);
        current_line_number++;
        total_number_of_lines ++;
        how_many_lines_left ++;
        number_push++;
        last_line = number_of_lines - how_many_lines_left;
    }
    first_time = 0;
    if (first_time ==0){
        while( how_many_lines_left< number_of_lines){
            //if the line is new -> push
            if(how_many_lines_left == 0 && current_action.first_row != current_action.last_row) is_push_at_the_tails = 0; //first c
            else if(last_line == 1 && current_action.first_row != current_action.last_row) is_push_at_the_tails = 1; //last c
            else if(current_action.first_row == current_action.last_row) is_push_at_the_tails = 4; //just one c
            else is_push_at_the_tails = 3;
            if(current_line_number > total_number_of_lines ){
                push(is_push_at_the_tails);
                number_push ++;
                total_number_of_lines ++;
                is_push_at_the_tails = 3;
                if(end_change == 1) current_change->was_change_and_push = 1;
            }
            //if the line already exists and needs an update -> update
            else{
                if(current_line_number == total_number_of_lines) end_change = 1;
                search_and_update(current_line_number,is_push_at_the_tails,end_change,start_change);
                start_change = 0;
            }
            current_line_number ++;
            how_many_lines_left ++;
            last_line = number_of_lines - how_many_lines_left;
        }
    }
    current_change->number_of_push = number_push;
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
            int action = history_tmp + 1;
            for (i = 0; i< number_of_frees; i++){
                delete_assign_data_of_history(action);
                action ++;
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
            int action = history_tmp + 1;
            for (i = 0; i< number_of_frees; i++){
                delete_assign_data_of_history(action);
                action ++;
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
    History* current_delete = create_history_object(current_action.command,current_action.first_row,current_action.last_row);
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

    for (int i = 0; i < number_of_lines; i ++){
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
    else{
        int current_line_number= current_action.first_row;
        int number_of_lines = current_action.last_row - current_action.first_row + 1;
        for (int i = 0; i < number_of_lines; i ++){
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
    all_history = malloc(BUFFER_DIMENSION * sizeof(History*));
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
    if(command == 'u' || command == 'r'){
        current_action.number = atoi(strtok(input,&command));
        return;
    }
    char delim[] = ",";
    current_action.first_row =  atoi(strtok(input,delim));
    current_action.last_row = atoi(strtok(NULL,&command));
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
 * Pushes a new node at the end of the list
 */
void push(int starting_or_ending){
    //asks for the new line and stores it in memory
    char current_line [INPUT_LINE];
    char* new_line = ask_for_line(current_line);
    //creates the new node with the new line
    new = create_node(new_line);


    assign_border_pointers(NULL,0);
    //if the list does not exist
    if (head == tail && head == NULL){
        head = tail = new;
        head -> next = tail -> next = NULL;
        head -> prev = tail -> prev = NULL;
    }
    else {
        //puts the node at the end
        tail -> next = new;
        new -> prev = tail;
        tail = new;
        //head -> prev = tail;
        //tail -> next = head;
        //printf("%s\n","i pushed a line");
    }
    if(starting_or_ending == 0){
        assign_starting_pointer(new,1);
        assign_starting_pointer(NULL,0);
        assign_border_pointers(new->prev,1);
        assign_ending_pointer(NULL,0);
    }
    else if(starting_or_ending == 1)assign_ending_pointer(new,1);
    else if(starting_or_ending == 4){
        assign_starting_pointer(new,1);
        assign_ending_pointer(new,1);
        assign_starting_pointer(NULL,0);
        assign_border_pointers(new->prev,1);
        assign_ending_pointer(NULL,0);
    }
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

void add_line(int is_first_and_only){
    //printf("i'll save the line\n");
    char current_line [INPUT_LINE];
    char* new_line = ask_for_line(current_line);
    new = create_node(new_line);

    if (head == tail && head == NULL){
        head = tail = new;
        head -> next = tail -> next = NULL;
        head -> prev = tail -> prev = NULL;
        assign_starting_pointer(new,1);
        if(is_first_and_only == 1){
            assign_ending_pointer(new,1);
            assign_border_pointers(NULL,0);
        }
        assign_starting_pointer(NULL,0);
        assign_ending_pointer(NULL,0);
        assign_border_pointers(NULL,1);

    }
    else{
        tail ->next = new;
        new -> prev = tail;
        tail = new;
        tail -> next = head;
        head -> prev = tail;

    }
}

/**
 * Searches for the needed row and prints it
 * @param number_of_line the row number we want to print
 */
void search_and_print(int number_of_line){
    int count_head = 1,i;
    int count_tail = total_number_of_lines;
    //if no Lists exists just print a dot
    if( head == NULL){
        printf(". \n");
        return;
    }
    //if the number is lower than the total number of rows/2 the search can start at the head of the list
    else if(number_of_line <= (total_number_of_lines/2)){
        for (ptr = head, i = 0; i < number; i ++, ptr = ptr -> next){
            if(count_head == number_of_line){
                //if the string exist it can be printed
                if (ptr->string != NULL){
                    printf("%s",ptr->string);
                    return;
                }
                //otherwise a dot
                else{
                    printf("%c",'.' );
                    return;
                }
            }
            count_head++;
        }
    }
    //if the number is higher than the total of number of rows/2 the search can start at the tail
    else if(number_of_line > (total_number_of_lines/2)){
        for (ptr = tail, i = 0; i < number; i ++, ptr = ptr -> prev){
            if(count_tail == number_of_line){
                //if the string exist it can be printed
                if (ptr->string != NULL){
                    printf("%s",ptr->string);
                    return;
                }
                    //otherwise a dot
                else{
                    printf("%c\n",'.' );
                    return;
                }
            }
            //if the row is a higher number than the the total number of rows then it does not exist and prints a dot
            else if(count_tail < number_of_line){
                printf("%c\n",'.' );
                return;
            }
            count_tail--;
        }
    }

}

/**
 * Searches and deletes the node (but not the string) of the row that needs to be deleted
 * @param number_of_line the row that needs to be deleted
 */
void search_and_delete(int number_of_line,int starting_or_ending, int end_delete){
    line *prevnode,*nextnode;
    int count_head = 1,i;
    int count_tail = total_number_of_lines;
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
        if(ptr->next != NULL)
            head -> next = (ptr -> next) -> next;
        if(ptr->prev != NULL)
            head -> prev = (ptr->prev) -> prev;
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
    //if the number is lower than the total number of rows/2 the search can start at the head of the list
    else if(number_of_line <= (total_number_of_lines/2)){
        for (ptr = head, i = 0; i < number; i ++, ptr = ptr -> next){
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
    //if the number is higher than the total of number of rows/2 the search can start at the tail
    else if(number_of_line > (total_number_of_lines/2)){
        for (ptr = tail, i = 0; i < number; i ++, ptr = ptr -> prev){
            if(count_tail == number_of_line){
                nextnode = ptr -> next;
                prevnode = ptr -> prev;
                prevnode -> next = nextnode;
                nextnode -> prev = prevnode;
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
            count_tail--;
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
/**
 * Searches for a row and updated the string saved
 * @param number_of_line
 */
void search_and_update(int number_of_line,int starting_or_ending, int end_change,int start_change) {
    int i, count_head = 1;
    int head_or_tail = 2;
    int count_tail = total_number_of_lines;
    //asks for the new line
    char current_line[INPUT_LINE];
    char *new_line = ask_for_line(current_line);
    new = create_node(new_line);

    //if the head list does not exists creates a new one
    if (head == tail && head == NULL) {
        if (number_of_line == 1) {
            head = tail = new;
            head->next = tail->next = NULL;
            head->prev = tail->prev = NULL;
        }

    }
    else {
        //if the number is lower than the total number of rows/2 the search can start at the head of the list
        if (number_of_line < (total_number_of_lines / 2)) {
            for (ptr = head, i = 1; i <= number; i++) {

                if (count_head == number_of_line) {
                    new -> prev = ptr -> prev;
                    new -> next = ptr -> next;
                    if(ptr != head)
                        (ptr->prev)->next = new;
                    if(ptr!= tail)
                        (ptr->next)->prev = new;
                    if(ptr == head){
                        head = new;
                        head_or_tail = 1;
                    }
                    if(ptr == tail){
                        tail = new;
                        head_or_tail = 0;
                    }
                    if(end_change == 1 && starting_or_ending != 1 && starting_or_ending != 4){
                        assign_ending_pointer(ptr,0);
                        ptr->next = NULL;
                    }
                    if(start_change == 1){
                        if(ptr->prev != NULL)
                            assign_border_pointers(ptr->prev,1);
                        if(head_or_tail == 1){
                            assign_border_pointers(NULL,1);
                        }
                        ptr -> prev = NULL;
                        assign_starting_pointer(new,1);
                        assign_starting_pointer(ptr,0);
                    }
                    if(total_number_of_lines == 1){
                        assign_border_pointers(NULL,1);
                        assign_border_pointers(NULL,0);
                    }
                    if(starting_or_ending == 1){
                        assign_ending_pointer(ptr,0);
                        assign_ending_pointer(new,1);
                        assign_border_pointers(ptr->next,0);
                        if(head_or_tail == 0)
                            assign_border_pointers(NULL,0);
                        if(end_change == 1)
                            ptr->next = NULL;
                    }
                    else if(starting_or_ending == 4) {
                        assign_starting_pointer(ptr,0);
                        assign_ending_pointer(ptr,0);
                        assign_ending_pointer(new,1);
                        assign_border_pointers(ptr->next,0);
                        if(end_change == 1)
                            ptr->next = NULL;

                    }
                    return;
                }
                ptr = ptr->next;
                count_head++;
            }
        }
        //if the number is higher than the total of number of rows/2 the search can start at the tail
        else if (number_of_line >= (total_number_of_lines / 2)) {
            for (ptr = tail, i = 1; i <= number; i++) {
                if (count_tail == number_of_line) {
                    new -> prev = ptr -> prev;
                    new -> next = ptr -> next;
                    if(ptr != head)
                        (ptr->prev)->next = new;
                    if(ptr!= tail)
                        (ptr->next)->prev = new;
                    if(ptr == head){
                        head = new;
                        head_or_tail = 1;
                    }
                    if(ptr == tail){
                        tail = new;
                        head_or_tail = 0;
                    };
                    if(end_change == 1 && starting_or_ending != 1 && starting_or_ending != 4){
                        assign_ending_pointer(ptr,0);
                        ptr->next = NULL;
                    }
                    if(start_change == 1){
                        if(ptr->prev != NULL)
                            assign_border_pointers(ptr->prev,1);
                        if(head_or_tail == 1){
                            assign_border_pointers(NULL,1);
                        }
                        ptr->prev = NULL;
                        assign_starting_pointer(new,1);
                        assign_starting_pointer(ptr,0);
                    }
                    if(total_number_of_lines == 1){
                        assign_border_pointers(NULL,1);
                        assign_border_pointers(NULL,0);
                    }
                    if(starting_or_ending == 1){
                        assign_ending_pointer(ptr,0);
                        assign_ending_pointer(new,1);
                        assign_border_pointers(ptr->next,0);
                        if(head_or_tail == 0)
                            assign_border_pointers(NULL,0);
                        if(end_change == 1)
                            ptr->next = NULL;
                    }
                    else if(starting_or_ending == 4) {
                        assign_starting_pointer(ptr,0);
                        assign_ending_pointer(ptr,0);
                        assign_ending_pointer(new,1);
                        assign_border_pointers(ptr->next,0);
                        if(end_change == 1)
                            ptr->next = NULL;
                    }
                    return;
                }
                count_tail--;
                ptr = ptr->prev;

            }
        }
    }
}



History* create_history_object(char command,int first_row, int last_row){
    History* current_history = malloc(sizeof(History));
    if(number_of_total_actions > buffer_dimension) add_space_to_buffer();
    all_history[number_of_total_actions] = current_history;
    all_history[number_of_total_actions]->command = command;
    all_history[number_of_total_actions]->first_row = first_row;
    all_history[number_of_total_actions]->last_row = last_row;
    return current_history;
}

void add_space_to_buffer(){
    buffer_dimension = buffer_dimension * 2;
    History** tmp = realloc(all_history,buffer_dimension * sizeof(History*));
    assert(tmp != NULL);
    all_history = tmp;
}

/**
 *
 * @param pointer
 * @param new_or_old 1 for new, 0 for old
 */
void assign_starting_pointer(node_t* pointer, int new_or_old){
    if(new_or_old == 1)
        all_history[number_of_total_actions]->new_lines_starting_pointer = pointer;
    else
        all_history[number_of_total_actions]->old_lines_starting_pointer = pointer;

}

/**
 *
 * @param pointer
 * @param new_or_old 1 for new, 0 for old
 */
void assign_ending_pointer(node_t* pointer, int new_or_old){
    if(new_or_old ==1)
        all_history[number_of_total_actions]->new_lines_ending_pointer = pointer;
    else
        all_history[number_of_total_actions]->old_lines_ending_pointer = pointer;
}

/**
 *
 * @param pointer
 * @param first_or_last 1 for first, 0 for last
 */
void assign_border_pointers(node_t* pointer, int first_or_last) {
    if (first_or_last == 1)
        all_history[number_of_total_actions]->first_node = pointer;
    else
        all_history[number_of_total_actions]->last_node = pointer;
}

void assign_all_history_pointers_to_null(){
    assign_starting_pointer(NULL,0);
    assign_ending_pointer(NULL,0);
    assign_starting_pointer(NULL,1);
    assign_ending_pointer(NULL,1);
    assign_border_pointers(NULL,1);
    assign_border_pointers(NULL,0);
}
void delete_assign_data_of_history(int array_position_for_data){
    int i;
    int number_of_strings = all_history[array_position_for_data]->last_row - all_history[array_position_for_data]->first_row + 1;
    /*for (i = 0; i < number_of_strings; i ++){
        if(all_history[array_position_for_data]->old_lines_starting_pointer != NULL){
            free(all_history[array_position_for_data]->old_lines_starting_pointer->string);
            all_history[array_position_for_data]->old_lines_starting_pointer = all_history[array_position_for_data]->old_lines_starting_pointer->next;
        }
    }*/
    free(all_history[array_position_for_data]);
    all_history[array_position_for_data] = NULL;
    number_of_undo = 0;
}

void undo(){
    int i;
    if(history_tmp == 0) return;
    for(i = 0; i<number_of_undo; i++){
        insert_for_undo();
        history_tmp--;
    }
}

void redo(){
    int i;
    if(history_tmp == number_of_total_actions) return;
    for(i = 0; i<number_of_redo; i++){
        insert_for_redo();
        history_tmp ++;
    }
}

void insert_for_undo(){
    char command = all_history[history_tmp]->command;
    int first_row = all_history[history_tmp]->first_row;
    int last_row = all_history[history_tmp]->last_row;
    node_t* first_node = all_history[history_tmp]->first_node;
    node_t* last_node = all_history[history_tmp]->last_node;
    node_t* new_lines_starting_pointer = all_history[history_tmp] -> new_lines_starting_pointer;
    node_t* new_lines_ending_pointer = all_history[history_tmp] -> new_lines_ending_pointer;
    node_t* old_lines_starting_pointer = all_history[history_tmp] -> old_lines_starting_pointer;
    node_t* old_lines_ending_pointer = all_history[history_tmp] -> old_lines_ending_pointer;
    int number_of_deletes = all_history[history_tmp] ->number_of_deletes;
    int number_of_push = all_history[history_tmp] -> number_of_push;
    int change_and_push = all_history[history_tmp] -> was_change_and_push;

    if(command == 'd'){
        //se cancello niente
        if(all_history[history_tmp]->was_delete_not_necessary == 1){
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
            first_time = 0;
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
            total_number_of_lines = total_number_of_lines + number_of_deletes;
            return;
        }
        //se cancello in mezzo
        if(first_node != NULL && last_node != NULL){
            first_node -> next = old_lines_starting_pointer;
            old_lines_starting_pointer -> prev = first_node;
            last_node -> prev = old_lines_ending_pointer;
            old_lines_ending_pointer -> next = last_node;
            total_number_of_lines = total_number_of_lines + number_of_deletes;
            return;
        }
    }
    else{
        //se scrivo per la prima volta nella lista
        if(first_node == NULL && last_node == NULL && change_and_push != 1 && number_of_push != 0){
            head = tail = NULL;
            first_time = 1;
            total_number_of_lines = 0;
            return;
        }
        //se cambio tutte le stringhe nella lista
        if(first_node == NULL && last_node == NULL && change_and_push != 1 && number_of_push == 0){
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
        if(first_node == NULL && last_node == NULL && change_and_push == 1 && number_of_push != 0){
            head = old_lines_starting_pointer;
            head -> next = old_lines_starting_pointer -> next;
            if(old_lines_starting_pointer -> next != NULL)
                (old_lines_starting_pointer -> next) -> prev = head;
            tail = old_lines_ending_pointer;
            tail -> prev = old_lines_ending_pointer -> prev;
            if(old_lines_ending_pointer ->prev != NULL)
                (old_lines_ending_pointer->prev)->next = tail;
            //sgancio
            tail -> next = NULL;
            total_number_of_lines = total_number_of_lines - number_of_push;
            return;
        }
        //se faccio una change senza modificare la coda
        if(first_node == NULL && last_node != NULL && change_and_push != 1 && number_of_push == 0){
            head = old_lines_starting_pointer;
            head -> next = old_lines_starting_pointer -> next;
            if(old_lines_starting_pointer -> next != NULL)
                (old_lines_starting_pointer -> next) -> prev = head;
            last_node -> prev = old_lines_ending_pointer;
            old_lines_ending_pointer -> next = last_node;
            //sgancio
            new_lines_ending_pointer -> next = NULL;
            return;
        }
        //se faccio una change modificando la coda ma niente push
        if(last_node == NULL && first_node != NULL &&change_and_push != 1 && number_of_push == 0){
            tail = old_lines_ending_pointer;
            tail -> prev = old_lines_ending_pointer -> prev;
            if(old_lines_ending_pointer ->prev != NULL)
                (old_lines_ending_pointer->prev)->next = tail;
            first_node -> next = old_lines_starting_pointer;
            old_lines_starting_pointer -> prev = first_node;
            //sgancio
            new_lines_starting_pointer -> prev = NULL;
            return;
        }
        //se faccio solo push
        if(last_node == NULL && first_node != NULL && change_and_push != 1 && number_of_push != 0){
            tail = first_node;
            tail -> prev = first_node -> prev;
            if(first_node ->prev != NULL)
                (first_node->prev)->next = tail;
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
            last_node -> prev = old_lines_ending_pointer;
            old_lines_ending_pointer -> next = last_node;
            //sgancio
            new_lines_starting_pointer -> prev = NULL;
            new_lines_ending_pointer -> next = NULL;
            return;
        }

    }

}

void insert_for_redo() {
    char command = all_history[history_tmp +1]->command;
    int first_row = all_history[history_tmp +1 ]->first_row;
    int last_row = all_history[history_tmp +1]->last_row;
    node_t *first_node = all_history[history_tmp +1 ]->first_node;
    node_t *last_node = all_history[history_tmp +1]->last_node;
    node_t *new_lines_starting_pointer = all_history[history_tmp +1]->new_lines_starting_pointer;
    node_t *new_lines_ending_pointer = all_history[history_tmp +1]->new_lines_ending_pointer;
    node_t *old_lines_starting_pointer = all_history[history_tmp +1]->old_lines_starting_pointer;
    node_t *old_lines_ending_pointer = all_history[history_tmp +1]->old_lines_starting_pointer;
    int number_of_deletes = all_history[history_tmp +1]->number_of_deletes;
    int number_of_lines = last_row - first_row + 1;
    int number_of_push = all_history[history_tmp+1] -> number_of_push;
    int change_and_push = all_history[history_tmp+1] -> was_change_and_push;

    if (command == 'd') {
        if (all_history[history_tmp + 1]->was_delete_not_necessary == 1) {
            return;
        }
        if(first_node == NULL && last_node == NULL){
            head = tail = NULL;
            first_time = 1;
            total_number_of_lines = 0;
            return;
        }
        if(first_node == NULL){
            head = last_node;
            last_node -> prev = NULL;
            head -> next = last_node -> next;
            if(last_node -> next != NULL)
                (last_node -> next) -> prev = head;
            old_lines_ending_pointer -> next = NULL;
            total_number_of_lines = total_number_of_lines - number_of_deletes;
            return;
        }
        if(last_node == NULL){
            tail = first_node;
            tail -> prev = first_node -> prev;
            if(first_node ->prev != NULL)
                (first_node->prev)->next = tail;
            first_node -> next = NULL;
            old_lines_starting_pointer -> prev = NULL;
            total_number_of_lines = total_number_of_lines - number_of_deletes;
            return;
        }
        if(first_node != NULL && last_node != NULL){
            first_node -> next = last_node;
            last_node -> prev = first_node;
            old_lines_starting_pointer -> prev = NULL;
            old_lines_ending_pointer -> next = NULL;
            total_number_of_lines = total_number_of_lines - number_of_deletes;
            return;
        }
    } else {
        //se scrivo per la prima volta nella lista
        if(first_node == NULL && last_node == NULL && change_and_push != 1 && number_of_push != 0){
            head = new_lines_starting_pointer;
            head -> next = new_lines_starting_pointer -> next;
            if(new_lines_starting_pointer -> next != NULL)
                (new_lines_starting_pointer -> next) -> prev = head;
            tail = new_lines_ending_pointer;
            tail -> prev = new_lines_ending_pointer -> prev;
            if(new_lines_ending_pointer ->prev != NULL)
                (new_lines_ending_pointer->prev)->next = tail;
            first_time = 0;
            total_number_of_lines = number_of_push;
            return;
        }
        //se cambio tutte le stringhe nella lista
        if(first_node == NULL && last_node == NULL && change_and_push != 1 && number_of_push == 0){
            head = new_lines_starting_pointer;
            head -> next = new_lines_starting_pointer -> next;
            if(new_lines_starting_pointer -> next != NULL)
                (new_lines_starting_pointer -> next) -> prev = head;
            tail = new_lines_ending_pointer;
            tail -> prev = new_lines_ending_pointer -> prev;
            if(new_lines_ending_pointer ->prev != NULL)
                (new_lines_ending_pointer->prev)->next = tail;
            return;
        }
        //se cambio tutte le stringhe e faccio anche delle push
        if(first_node == NULL && last_node == NULL && change_and_push == 1 && number_of_push != 0){
            head = new_lines_starting_pointer;
            head -> next = new_lines_starting_pointer -> next;
            if(new_lines_starting_pointer -> next != NULL)
                (new_lines_starting_pointer -> next) -> prev = head;
            tail = new_lines_ending_pointer;
            tail -> prev = new_lines_ending_pointer -> prev;
            if(new_lines_ending_pointer ->prev != NULL)
                (new_lines_ending_pointer->prev)->next = tail;
            total_number_of_lines = total_number_of_lines + number_of_push;
            return;
        }
        //se faccio una change senza modificare la coda
        if(first_node == NULL && last_node != NULL && change_and_push != 1 && number_of_push == 0){
            head = new_lines_starting_pointer;
            head -> next = new_lines_starting_pointer -> next;
            if(new_lines_starting_pointer -> next != NULL)
                (new_lines_starting_pointer -> next) -> prev = head;
            last_node -> prev = new_lines_ending_pointer;
            new_lines_ending_pointer -> next = last_node;
            //sgancio
            old_lines_ending_pointer -> next = NULL;
            return;
        }
        //se faccio una change modificando la coda ma niente push
        if(last_node == NULL && first_node != NULL && change_and_push != 1 && number_of_push == 0){
            tail = new_lines_ending_pointer;
            tail -> prev = new_lines_ending_pointer -> prev;
            if(new_lines_ending_pointer ->prev != NULL)
                (new_lines_ending_pointer->prev)->next = tail;
            first_node -> next = new_lines_starting_pointer;
            new_lines_starting_pointer -> prev = first_node;
            //sgancio
            old_lines_starting_pointer -> prev = NULL;
            return;
        }
        //se faccio solo push
        if(last_node == NULL && first_node != NULL && change_and_push != 1 && number_of_push != 0){
            tail = new_lines_ending_pointer;
            tail -> prev = new_lines_ending_pointer -> prev;
            if(new_lines_ending_pointer ->prev != NULL)
                (new_lines_ending_pointer->prev)->next = tail;
            first_node -> next = new_lines_starting_pointer;
            new_lines_starting_pointer -> prev = first_node;

            total_number_of_lines = total_number_of_lines + number_of_push;
            return;
        }
        //se modifico la coda e faccio anche delle push
        if(last_node == NULL && first_node != NULL && change_and_push == 1){
            first_node -> next = new_lines_starting_pointer;
            new_lines_starting_pointer -> prev = first_node;
            tail = new_lines_ending_pointer;
            tail -> prev = new_lines_ending_pointer -> prev;
            if(new_lines_ending_pointer ->prev != NULL)
                (new_lines_ending_pointer->prev)->next = tail;
            //stacco
            old_lines_starting_pointer -> prev = NULL;

            total_number_of_lines = total_number_of_lines + number_of_push;
            return;
        }
        //se faccio delle change dentro la struttura senza avere delle push
        if(last_node != NULL && first_node != NULL && change_and_push != 1){
            first_node -> next = new_lines_starting_pointer;
            new_lines_starting_pointer -> prev = first_node;
            last_node -> prev = new_lines_ending_pointer;
            new_lines_ending_pointer -> next = last_node;
            //sgancio
            old_lines_starting_pointer -> prev = NULL;
            old_lines_ending_pointer-> next = NULL;
            return;
        }
    }
}