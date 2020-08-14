#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define INPUT_COMMAND  20
#define INPUT_LINE 1023
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
    node_t* new_lines_starting_pointer;
    node_t* new_lines_ending_pointer;
    node_t* old_lines_starting_pointer;
    node_t* old_lines_ending_pointer;
}History;

int buffer_dimension = 50;
History** all_history = NULL;
int number_of_total_actions = 0;

typedef struct node line;

int ask_for_action(char *input);
void find_command(char *input);
void create_action(char command,char *input);
char* ask_for_line(char *line);
void push(int starting_or_ending);
line* create_node(char *new_line);
void add_line(int is_first_and_only);
void search_and_delete(int number_of_line, int starting_or_ending);
void search_and_print(int number_of_line);
char *save_string(char *current_line);
void search_and_update(int number_of_line, int starting_or_ending);
void undo();
History* create_history_object(char command,int first_row, int last_row);
void add_space_to_buffer();
void assign_starting_pointer(node_t* pointer, int new_or_old);
void assign_ending_pointer(node_t* pointer, int new_or_old);

Action current_action;
//is it the first time we have to save a string? -> if we need to create the first node of the list
int first_time = 1;
//the total number of rows that can be found in the database. This number is updated as soon as a line is deleted
int total_number_of_lines = 0;
//if an undo is waiting to be processed
int is_undo_waiting = 0;
//how many actions we need to undo
int number_of_undo = 0;

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
    int is_push_at_the_tails = 3;
    if(is_undo_waiting == 1) undo();
    number_of_total_actions++;
    History* current_change = create_history_object(current_action.command,current_action.first_row,current_action.last_row);
    //how many lines we need to work with
    int number_of_lines = current_action.last_row - current_action.first_row + 1;
    //which is the current line we are working with
    int current_line_number = current_action.first_row;
    //how many lines that we need to work with are left
    int how_many_lines_left = 0;

    if (first_time == 1){
        is_push_at_the_tails = 2;
        int is_first_and_only = 0;
        if(current_action.first_row == current_action.last_row) is_first_and_only = 1;
        add_line(is_first_and_only);
        current_line_number++;
        total_number_of_lines ++;
        how_many_lines_left ++;
    }
    first_time = 0;
    if (first_time ==0){
        while( how_many_lines_left< number_of_lines){
            //if the line is new -> push
            if(is_push_at_the_tails != 2 && current_action.first_row != current_action.last_row) is_push_at_the_tails = 0;
            else if(how_many_lines_left == 1) is_push_at_the_tails = 1;
            else if(current_action.first_row == current_action.last_row) is_push_at_the_tails = 4;
            else is_push_at_the_tails = 3;
            if(current_line_number > total_number_of_lines ){
                push(is_push_at_the_tails);
                total_number_of_lines ++;
            }
            //if the line already exists and needs an update -> update
            else{
                search_and_update(current_line_number,is_push_at_the_tails);
            }
            current_line_number ++;
            how_many_lines_left ++;
        }
    }
    //the last input will always be a dot
    char dot[1] = "" ;
    scanf("%c",dot);
    printf(current_change->new_lines_starting_pointer->string);
    printf(current_change->new_lines_ending_pointer->string);
}

/**
 * Handles the delete command.
 * If we have a 0,0d command we can resolve it immediately. Otherwise searches for the row that needs
 * to be deleted and deletes it if it exists.
 * If is_undo_waiting == 1 then the undo command needs to be processed before the delete
 */
void handle_delete(){
    if(is_undo_waiting == 1) undo();
    int is_delete_at_the_tails = 3;
    number_of_total_actions++;
    History* current_delete = create_history_object(current_action.command,current_action.first_row,current_action.last_row);
    if(current_action.first_row == 0 && current_action.last_row == 0 || current_action.first_row > total_number_of_lines) {
        assign_starting_pointer(NULL,0);
        assign_ending_pointer(NULL,0);
        assign_starting_pointer(NULL,1);
        assign_ending_pointer(NULL,1);
        number_of_total_actions++;
        return;
    }
    //which is the current line we are working with
    int current_line_number= current_action.first_row;
    //how many lines we need to work with
    int number_of_lines = current_action.last_row - current_action.first_row + 1;

    for (int i = 0; i < number_of_lines; i ++){
        if(current_action.first_row == current_line_number) is_delete_at_the_tails  = 0;
        else if(current_action.last_row == current_line_number) is_delete_at_the_tails = 1;
        else is_delete_at_the_tails = 3;
        //if the current row that needs to be deleted doesnt exist then no action can be taken
        if(current_line_number > total_number_of_lines) return; //TODO maybe different for undo
        //otherwise deletes the row
        else search_and_delete(current_line_number,is_delete_at_the_tails);
    }

}

/**
 * Handles the print command.
 * If we have 0,0p we can print a dot immediately.
 * Otherwise it prints the rows requested
 * If is_undo_waiting == 1 then the undo command needs to be processed before the print
 */
void handle_print(){
    if(is_undo_waiting == 1) undo();
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
    //the number of actions we need to undo is updated
    number_of_undo = current_action.number;
    if(is_undo_waiting ==1 ){
        number_of_undo = number_of_undo + current_action.number;
    }
    is_undo_waiting =1;
}

/**
 * Handles the redo command
 */
void handle_redo(){
    if(is_undo_waiting == 0) return;
    else{
        //handles if redo is needed or if undo is needed
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
        current_action.number = atoi(strtok(&input,&command));
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
    if(starting_or_ending == 0)assign_starting_pointer(new,1);
    else if(starting_or_ending == 1)assign_ending_pointer(new,1);
    else if(starting_or_ending == 4){
        assign_starting_pointer(new,1);
        assign_ending_pointer(new,1);
    }
    assign_starting_pointer(NULL,0);
    assign_ending_pointer(NULL,0);
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
        head -> prev = tail;
        tail -> next = head;
        //printf("%s\n","i pushed a line");
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
        if(is_first_and_only == 1)
            assign_ending_pointer(new,1);
        assign_starting_pointer(NULL,0);
        assign_ending_pointer(NULL,0);
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
                    printf("%s\n",ptr->string);
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
    //if the number is higher than the total of number of rows/2 the search can start at the tail
    else if(number_of_line > (total_number_of_lines/2)){
        for (ptr = tail, i = 0; i < number; i ++, ptr = ptr -> prev){
            if(count_tail == number_of_line){
                //if the string exist it can be printed
                if (ptr->string != NULL){
                    printf("%s\n",ptr->string);
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
void search_and_delete(int number_of_line,int starting_or_ending){
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
            if(starting_or_ending == 0){
                assign_starting_pointer(ptr,0);
                assign_ending_pointer(ptr,0);
            }
            else if(starting_or_ending == 1)
                assign_ending_pointer(ptr,0);
            head = tail = NULL;
            first_time = 1;
            total_number_of_lines--;
            return;
        }
        if(starting_or_ending == 0)
            assign_starting_pointer(ptr,0);
        //the position of the head is shifted
        head = ptr -> next;
        head -> next = (ptr -> next) -> next;
        head -> prev = (ptr->prev) -> prev;
        total_number_of_lines--;
        return;
    }
    //if the row is the last node
    else if(number_of_line > 1 && number_of_line == total_number_of_lines){
        //the tail is shifted
        ptr = tail;
        if(starting_or_ending == 0){
            assign_starting_pointer(ptr,0);
            assign_ending_pointer(ptr,0);
        }
        else if(starting_or_ending == 1){
            assign_ending_pointer(ptr,0);
        }
        tail = ptr -> prev;
        tail -> next = head;
        total_number_of_lines--;
        return;
    }
    //if the number is lower than the total number of rows/2 the search can start at the head of the list
    else if(number_of_line <= (total_number_of_lines/2)){
        for (ptr = head, i = 0; i < number; i ++, ptr = ptr -> next){
            if(count_head == number_of_line){
                if(starting_or_ending == 1)
                    assign_ending_pointer(ptr,0);
                else if(starting_or_ending == 0)
                    assign_starting_pointer(ptr,0);
                prevnode = ptr -> prev;
                nextnode = ptr-> next;
                prevnode -> next = nextnode;
                nextnode -> prev = prevnode;

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
                if(starting_or_ending == 1)
                    assign_ending_pointer(ptr,0);
                else if(starting_or_ending == 0)
                    assign_starting_pointer(ptr,0);
                nextnode = ptr -> next;
                prevnode = ptr -> prev;
                prevnode -> next = nextnode;
                nextnode -> prev = prevnode;
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
void search_and_update(int number_of_line,int starting_or_ending) {
    int i, count_head = 1;
    int count_tail = total_number_of_lines;
    //asks for the new line
    char current_line[INPUT_LINE];
    char *new_line = ask_for_line(current_line);

    //if the head list does not exists creates a new one
    if (head == tail && head == NULL) {
        if (number_of_line == 1) {
            new = create_node(new_line);
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
                    if(starting_or_ending == 0) assign_starting_pointer(ptr,0);
                    else if(starting_or_ending == 1)assign_ending_pointer(ptr,0);
                    ptr->string = new_line;
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
                    if(starting_or_ending == 0) assign_starting_pointer(ptr,0);
                    else if(starting_or_ending == 1)assign_ending_pointer(ptr,0);
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

void undo(){
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
    History** tmp = realloc(all_history,buffer_dimension);
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