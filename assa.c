#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define CLOSE_MESSAGE "bye\n"
#define OUT_OF_MEMORY "[ERR] Out of memory.\n"
#define COULD_NOT_READ_FILE "[ERR] Could not read file.\n"
#define USAGE_EXCEPTION "Usage: ./ass [file-name_]\n"

//----------------------------------------------------------------------------
//
// Struct that holds the information about a person
//

enum _ErrorType_{
  NORMAL,
  WRONG_USAGE,
  FILE_UNREADABLE_EXCEPTION,
  MEMORY_EXCEPTION
};

enum _Gender_{
  MALE,
  FEMALE
};

struct _Person_{
    struct _Person_* father_;
    struct _Person_* mother_;
    enum _Gender_ gender_;
    char* name_;
};

struct _PersonList_{
  struct _Person_* list_;
  int length_;
};

struct _TokenArray_{
    char** data_;
    int length_;
};

void debug(char* message){
  printf("%s\n", message);
}

void printError(char* error, enum _ErrorType_ type){
  printf("%s", error);
  switch(type)
  {
    case NORMAL:
      return;
    case WRONG_USAGE:
      exit(1);
    case MEMORY_EXCEPTION:
      exit(2);
    case FILE_UNREADABLE_EXCEPTION:
      exit(3);
  }
}

int tokenizeString(char* input_string, char* delimiter, struct _TokenArray_* output_array){
  int out_array_size = 1;
  int out_array_position = 0;
  char** out_array = malloc(sizeof(char*));

  char* token = strtok(input_string, delimiter); 
  while (token != NULL){
    
    out_array[out_array_position] = (char*) malloc(sizeof(char) * (strlen(token) + 1));
    strncpy(out_array[out_array_position], token, strlen(token) + 1);
    out_array_position += 1;
    if(out_array_position >= out_array_size)
    {
      out_array_size += 1;
      out_array = (char**) realloc(out_array, out_array_size * sizeof(char*));
      if(out_array == NULL)
      {
         printError(OUT_OF_MEMORY, MEMORY_EXCEPTION);        
      }
    }
    token = strtok(NULL, delimiter);
  }
  output_array->data_ = out_array;
  output_array->length_ = out_array_position - 1;
  return 0;
}

int addRelation(struct _Person_* person1, char* relation, struct _Person_* person2, struct _PersonList_ all_persons)
{
  if(strcmp(relation, "mother") == 0)
  {
    if(person1->gender_ == FEMALE)
    {
      person2->mother_ = person1;
    }
    else
    {
      return 2;
    }
  }
  else if(strcmp(relation, "father") == 0)
  {
    if(person1->gender_ == MALE)
    {
      person2->father_ = person1;
    }
    else
    {
      return 2;
    }
  }
  else
  {
    return 1;
  }
  return 0;
}

struct _Person_* addPersonToList(struct _Person_* person, struct _PersonList_* person_list){
  int index = 0;
  bool person_already_exists = false;
  struct _Person_* existing_person;
  for(index = 0; index < person_list->length_ ; index++)
  {
    if(strcmp(person_list->list_[index].name_, person->name_) == 0 &&
    person_list->list_[index].gender_ == person->gender_)
    {
      person_already_exists = true;
      existing_person = &(person_list->list_[index]);
      break;
    }
  }
  if(!person_already_exists)
  {
    //copy person
    person_list->length_++;
    person_list->list_ = realloc(person_list->list_, person_list->length_ * sizeof(struct _Person_));
    person_list->list_[person_list->length_ - 1] = *person;
    printf("added person %s\n", person->name_);
    existing_person = &(person_list->list_[person_list->length_ - 1]);
  }
  free(person);
  return existing_person; 
}

int parsePeopleStrings(char* input_string, char** peopleList, int num_people)
{
  int index = 0;
  int entries_made = 0;
  bool in_parenthesis = false;  
  int length_= strlen(input_string);

  for(index = 0; index < length_; index++){
    if(input_string[index] == '\"')
    {
      in_parenthesis = !in_parenthesis;
      if(in_parenthesis && entries_made < num_people)
      {
        peopleList[entries_made++] = &(input_string[index + 1]);
      }
      if(!in_parenthesis){
        input_string[index] = '\0';
      }
    }
  }

  if(in_parenthesis)
  {
    return 1;
  }
  return 0;
}

char* parsePerson(char* person_string, struct _Person_* person)
{
  bool person_found = false;
  int length_= strlen(person_string);
  char* index = person_string;
  while(!person_found){
    index = strchr(index, '[');
    if(index == NULL){
      return NULL;
    }
    if(index + 2 <= person_string + length_&&
      (index[1] == 'm' || index[1] == 'f') &&
      index[2] == ']')
    {
      person_found = true;
    }
  }
  person->name_ = malloc((index - person_string + 1) * sizeof(char));
  person->name_[index - person_string] = '\0';
  strncpy(person->name_, person_string, index - person_string);
  person->gender_ = index[1] == 'm' ? MALE : FEMALE;
  return index + 3;
}

//-----------------------------------------------------------------------------
//
// Function that parses dot files
//
// @param command_buffer buffer to write input to
//
// @return void 
//
//

void parseDotFile(FILE* dot_file, struct _PersonList_* all_persons){
  int lines_size = 10;
  int lines_position = 0;
  char** lines = (char**) malloc(sizeof(char**) * lines_size);
  if(lines == NULL)
  {
    printError(OUT_OF_MEMORY, MEMORY_EXCEPTION);
  }
  
  int buffer_size = 10;
  int buffer_position = 0;
  char* buffer = (char*) malloc(sizeof(char) * buffer_size);
  if(buffer == NULL)
  {
    printError(OUT_OF_MEMORY, MEMORY_EXCEPTION);
  }

  int inputChar = '\0';

  while((inputChar = fgetc(dot_file)) != EOF)
  {
    buffer[buffer_position++] = (char) inputChar;
    if(buffer_position >= buffer_size)
    {
      buffer_size += 10;
      buffer = realloc(buffer, buffer_size * sizeof(char*));
      if(buffer == NULL)
      {
        printError(OUT_OF_MEMORY, MEMORY_EXCEPTION);
      }
    }
  }

  buffer[buffer_position] = '\0';

  char* line_pointer = strtok(buffer, "\n");
  if(line_pointer == NULL)
  {
    printError(COULD_NOT_READ_FILE, FILE_UNREADABLE_EXCEPTION);
  } 
  lines[lines_position++] = line_pointer;

  while((line_pointer = strtok(NULL, "\n")) != NULL)
  {
    lines[lines_position++] = line_pointer;
    if(lines_position >= lines_size)
    {
      lines_size += 10; 
      lines = realloc(lines, lines_size * sizeof(char*));
      if(lines == NULL)
      {
        printError(OUT_OF_MEMORY, MEMORY_EXCEPTION);
      }
    }
  } 

  lines_position--;

  if(strcmp("digraph FamilyTree", lines[0]) != 0)
  {
    printf("line is %s\n", lines[0]);
    debug("digraph familytree did not match");
    printError(COULD_NOT_READ_FILE, FILE_UNREADABLE_EXCEPTION);
  }

  if(lines[1][0] != '{')
  {
    debug("{ in first line not found");
    printf("line is %s\n", lines[0]);
    printError(COULD_NOT_READ_FILE, FILE_UNREADABLE_EXCEPTION);
  }

  if(lines[lines_position][0] != '}')
  {
    printf("line is [%s]\n", lines[lines_position]);
    debug("} in last line not found");
    printError(COULD_NOT_READ_FILE, FILE_UNREADABLE_EXCEPTION);
  }

  int line = 0;
  for(line=2; line < lines_position; line++)
  {
    
    if(strstr(lines[line], "->") != NULL)
    {
      char** peopleList = malloc(2 * sizeof(char**));     
      int status = parsePeopleStrings(lines[line], peopleList, 2);
      if(status == 1)
      {
        printError(COULD_NOT_READ_FILE, FILE_UNREADABLE_EXCEPTION);
      }

      struct _Person_* person1 = malloc(sizeof(struct _Person_));
      struct _Person_* person2 = malloc(sizeof(struct _Person_));
      parsePerson(peopleList[0], person1);
      parsePerson(peopleList[1], person2);
      person1 = addPersonToList(person1, all_persons);
      person2 = addPersonToList(person2, all_persons);
      if(person1->gender_ == MALE)
      {
        addRelation(person2, "father", person1, *all_persons);
      }
      else
      {
        addRelation(person2, "mother", person1, *all_persons);
      } 
      free(peopleList);
    }
    else
    {
      
      char** peopleList = malloc(sizeof(char**));
      int status = parsePeopleStrings(lines[line], peopleList, 1);
      if(status == 1)
      {
        printError(COULD_NOT_READ_FILE, FILE_UNREADABLE_EXCEPTION);
      }
      struct _Person_* person1 = malloc(sizeof(struct _Person_));
      parsePerson(peopleList[0], person1);
      person1 = addPersonToList(person1, all_persons);
      free(peopleList);
    }
  }
  free(lines);
  free(buffer);
}

//-----------------------------------------------------------------------------
//
// Function that displays command prompt to user and reads user input.
//
// @param command_buffer buffer to write input to
//
// @return void 
//
//
void commandPrompt(char** command_buffer)
{
  printf("esp> ");
  char input_character = '\0';
  char* input_buffer = (char*) malloc(sizeof(char) * 10);
  unsigned int input_buffer_position = 0;
  unsigned int input_buffer_length_= 10;
  
  input_character = getchar();
  while(input_character != '\n')
  {
    //printf("%c", input_character);
    input_buffer[input_buffer_position] = input_character;
    input_buffer_position++;
    if(input_buffer_position >= input_buffer_length_)
    {
      input_buffer_length_+= 10;
      input_buffer = (char*) realloc(input_buffer, input_buffer_length_* sizeof(char));
    }
    input_character = getchar();
  }
  //set last byte in buffer to zero
  input_buffer[input_buffer_position] = '\0';
  *command_buffer = input_buffer;
}



//------------------------------------------------------------------------------
//
// The main program.
// Runs the main program. Contains the infinite command-prompt-loop.
//
// @param argc used to count arguments from stdin
// @param argv used to receive arguments from stdin
//
// @return either zero on ordinary exit or one on error
//

int main(int argc, char *argv[])
{
  char* command_buffer = NULL;
  char* command;
  char* arguments;
  struct _PersonList_* all_persons = malloc(sizeof(struct _PersonList_));
  all_persons->length_= 0;
  all_persons->list_= malloc(sizeof(struct _Person_));
  if(argc > 1)
  {
    char* dot_inputfile_name = argv[1]; 
    FILE* dot_inputfile = fopen(dot_inputfile_name, "r");
    if(dot_inputfile == NULL)
    {
      printError(COULD_NOT_READ_FILE, FILE_UNREADABLE_EXCEPTION); 
    }
    parseDotFile(dot_inputfile, all_persons);
  }

  while(true)
  {
    command = NULL;
    arguments = NULL;
    commandPrompt(&command_buffer);
    int length_of_command_buffer = strlen(command_buffer);
    command = strtok(command_buffer, " ");
    if(command == NULL)
    {
      continue;
    }
    if(length_of_command_buffer > strlen(command)){
      arguments = command_buffer + strlen(command) + 1;
    }
    printf("command entered was %s\n", command);
    printf("arguemnts entered were %s\n", arguments);
    if(strcmp(command, "quit") == 0)
    {
      printf(CLOSE_MESSAGE);
      exit(0);
    }
    else
    {
      printf("cannot understand command %s\n", command);
    } 
    free(command_buffer); 
  } 
  return 0;
}
