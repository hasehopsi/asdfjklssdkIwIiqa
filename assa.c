#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <ctype.h> //for isspace

#define CLOSE_MESSAGE "bye\n"
#define OUT_OF_MEMORY "[ERR] Out of memory.\n"
#define COULD_NOT_READ_FILE "[ERR] Could not read file.\n"
#define SUCCESSFULLY_READ_FILE "File parsing successful...\n"
#define USAGE_EXCEPTION "Usage: ./ass [file-name_]\n"

//add command
#define ADD_PERSONS_USAGE "[ERR] Wrong usage - add <namePerson1> [m/f] <relation> <namePerson2> [m/f].\n"
#define ADD_SEX_DOES_NOT_MATCH "[ERR] Sex does not match with relation.\n"
#define ADD_BOTH_PEOPLE_ARE_SAME "[ERR] Both people are the same.\n"
#define ADD_RELATION_NOT_POSSIBLE "[ERR] Relation not possible.\n"
//relationship command
#define RELATIONSHIP_USAGE "[ERR] Wrong usage - relationship <namePerson1> [m/f] <namePerson2> [m/f].\n"
#define RELATIONSHIP_INEXISTANT_PERSONS "[ERR] At least one person does not exist yet.\n"
#define RELATIONSHIP_BOTH_PEOPLE_ARE_SAME "[ERR] Both people are the same.\n" 
#define RELATIONSHIP_NO_RELATIONSHIP "There is no relationship between them.\n"
#define RELATIONSHIP_EXISTANT "The two people are related.\n"
//list command
#define LIST_WRONG_USAGE "[ERR] Wrong usage - list.\n"
#define LIST_NO_ENTRIES_AVAILABLE "[ERR] No entries available.\n"
//draw-all command
#define DRAW_ALL_USAGE "[ERR] Wrong usage - draw-all <file-name>.\n"
#define DRAW_ALL_NO_ENTRIES_AVAILABLE "[ERR] No entries available.\n"
#define DRAW_ALL_COULD_NOT_WRITE_FILE "[ERR] Could not write file.\n"
#define DRAW_ALL_SUCCESS "Creating DOT-file was successful.\n"
//draw command
#define DRAW_USAGE "[ERR] Wrong usage - draw <name> [m/f] <file-name>.\n"
#define DRAW_INEXISTANT_PERSON "[ERR] This person does not exist.\n"
#define DRAW_COULD_NOT_WRITE_FILE "[ERR] Could not write file.\n"
#define DRAW_SUCCESS "Creating DOT-file was successful.\n"

#define ERROR (1)
#define NORMAL (0)
#define WRONG_USAGE_EXCEPTION (-1)
#define MEMORY_EXCEPTION (-2)
#define FILE_UNREADABLE_EXCEPTION (-3)

enum _Gender_{
  MALE,
  FEMALE
};

enum _Relations_{
  RELATION_MOTHER,
  RELATION_FATHER,
  RELATION_MOTHER_GRANDFATHER,
  RELATION_MOTHER_GRANDMOTHER,
  RELATION_FATHER_GRANDFATHER,
  RELATION_FATHER_GRANDMOTHER
};

enum _Relationship_{
  NO_RELATION,
  SISTER,
  BROTHER,
  MOTHER,
  FATHER,
  AUNT,
  UNCLE,
  GRANDMOTHER,
  GRANDFATHER
};


//----------------------------------------------------------------------------
//
// Struct that holds the information about a person
//


struct _Person_{
    struct _Person_* father_;
    struct _Person_* mother_;
    enum _Gender_ gender_;
    char* name_;
};

struct _PersonList_{
  struct _Person_** list_;
  int length_;
  bool data_is_allocated_;
};

struct _LineBuffer_{
  char** data_;
  int position_;
  int length_;
  bool data_is_allocated_;
};

struct _TokenArray_{
    char** data_;
    int length_;
};

void debug(char* message){
  printf("%s\n", message);
}

void printError(char* error){
  printf("%s", error);
}

//string helper functions
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
         printError(OUT_OF_MEMORY);        
         return MEMORY_EXCEPTION;
      }
    }
    token = strtok(NULL, delimiter);
  }
  output_array->data_ = out_array;
  output_array->length_ = out_array_position - 1;
  return NORMAL;
}


int skipWhitespace(char** string)
{
  printf("string in skip whitespace is [%s]\n", *string);
  while((*string)[0] == ' ')
  {
    (*string)++;
    if((*string)[0] == '\0')
    {
      return ERROR;
    }
  }
  printf("string out of skip whitespace is [%s]\n", *string);
  return NORMAL;
}

int stripWhitespace(char* string)
{
  int index = 0;
  printf("string was: [%s]\n", string);
  while(isspace(string[0]))
  {
    for(index = 0; index < strlen(string); index++)
    {
      string[index] = string[index + 1];
    }
  }
  index = strlen(string) - 1;
  while(isspace(string[index]))
  {
    string[index] = '\0';
    index--;
  }
  printf("string is now: [%s]\n", string);
  return NORMAL;
}

int initializeLineBuffer(struct _LineBuffer_** buffer, bool allocate_data)
{
  *buffer = malloc(sizeof(struct _LineBuffer_));
  if(*buffer == NULL)
  {
    printError(OUT_OF_MEMORY);
    return MEMORY_EXCEPTION;
  }
  (*buffer)->length_ = 1;
  (*buffer)->position_ = 0;
  (*buffer)->data_is_allocated_ = allocate_data;
  (*buffer)->data_ = malloc(sizeof(char*) * (*buffer)->length_);
  if((*buffer)->data_ == NULL)
  {
    printError(OUT_OF_MEMORY);
    return MEMORY_EXCEPTION;
  }
  return NORMAL;
}
void freeLineBuffer(struct _LineBuffer_** buffer)
{
  int index = 0;
  if((*buffer)->data_is_allocated_)
  {
    for(index = 0; index < (*buffer)->position_; index++)
    {
      free((*buffer)->data_[index]);
    }
  }
  free((*buffer)->data_);
  free(*buffer);
  *buffer = NULL;
}

//line buffer helper functions
int addLineToLineBuffer(struct _LineBuffer_* line_buffer, char* line)
{
  line_buffer->data_[line_buffer->position_++] = line;
  if(line_buffer->position_ >= line_buffer->length_)
  {
    line_buffer->length_ += 10;
    line_buffer->data_ = realloc(line_buffer->data_, sizeof(char*) * line_buffer->length_);
    if(line_buffer->data_ == NULL)
    {
      printError(OUT_OF_MEMORY);
      return MEMORY_EXCEPTION;
    }
  }
  return NORMAL;
}


//person helper functions


int initializePerson(struct _Person_** person)
{
  *person = malloc(sizeof(struct _Person_));
  if(*person == NULL)
  {
    printError(OUT_OF_MEMORY);
    return MEMORY_EXCEPTION; 
  }
  (*person)->name_ = malloc(sizeof(char));
  if((*person)->name_ == NULL)
  {
    printError(OUT_OF_MEMORY);
    return MEMORY_EXCEPTION;
  }
  (*person)->mother_ = NULL;
  (*person)->father_ = NULL;
  (*person)->gender_ = MALE;
  return NORMAL;
}

void freePerson(struct _Person_** person)
{
  free((*person)->name_);
  free(*person);
  *person = NULL;
}

int parsePerson(char** person_string, struct _Person_* person)
{
  bool person_found = false;
  int length_of_string = strlen(*person_string);
  char* string_position = *person_string;

  while(!person_found){
    string_position = strchr(string_position, '[');
    if(string_position == NULL){
      return ERROR;
    }
    if(string_position + 2 <= *person_string + length_of_string &&
      (string_position[1] == 'm' || string_position[1] == 'f') &&
      string_position[2] == ']')
    {
      person_found = true;
    }
    //increment *string_position by one to search for brackets after current bracket
    string_position++;
  }
  string_position--;

  person->name_ = realloc(person->name_, (string_position - *person_string + 1) * sizeof(char));
  if(person->name_ == NULL)
  {
    printError(OUT_OF_MEMORY);
    return MEMORY_EXCEPTION;
  }
  person->name_[string_position - *person_string] = '\0';
  strncpy(person->name_, *person_string, string_position - *person_string);
  stripWhitespace(person->name_);

  //length of a persons name cannot be zero
  if(strlen(person->name_) == 0)
  {
    freePerson(&person);
    return ERROR;
  }

  person->gender_ = string_position[1] == 'm' ? MALE : FEMALE;
  person->mother_ = NULL;
  person->father_ = NULL;
  string_position += 3;
  *person_string = string_position;
  return NORMAL;
}

int copyPersonIntoAnotherPerson(struct _Person_* destination, struct _Person_* source)
{
  destination->name_ = realloc(destination->name_, (strlen(source->name_) + 1) * sizeof(char));
  if(destination->name_ == NULL)
  {
    printError(OUT_OF_MEMORY);
    return ERROR;
  }
  strncpy(destination->name_, source->name_, strlen(source->name_) + 1);
  destination->gender_ = source->gender_;
  destination->mother_ = source->mother_;
  destination->father_ = source->father_;
  return NORMAL;
}

int createQuestionMarkPerson(int* question_mark_person_counter, struct _Person_** pointer_to_person)
{
  struct _Person_* question_mark_person;
  int status = initializePerson(&question_mark_person);
  if(status != NORMAL)
  {
    printf(OUT_OF_MEMORY);
    return MEMORY_EXCEPTION;
  }

  question_mark_person->name_ = realloc(question_mark_person->name_, (10 + 1) * sizeof(char));
  if(question_mark_person->name_ == NULL)
  {
    printError(OUT_OF_MEMORY);
    return MEMORY_EXCEPTION;
  }

  snprintf(question_mark_person->name_, 10, "?%d", *question_mark_person_counter);
  (*question_mark_person_counter)++;
  *pointer_to_person = question_mark_person;
  return NORMAL;
}

//person list helper functions

int initializePersonList(struct _PersonList_** person_list, bool data_allocated)
{
  *person_list = malloc(sizeof(struct _PersonList_));
  if(*person_list == NULL)
  {
    printError(OUT_OF_MEMORY);
    return MEMORY_EXCEPTION;
  } 
  (*person_list)->length_= 0;
  (*person_list)->list_= malloc(sizeof(struct _Person_*));
  (*person_list)->data_is_allocated_ = data_allocated;
  if((*person_list)->list_ == NULL)
  {
    printError(OUT_OF_MEMORY);
    return MEMORY_EXCEPTION;
  } 
  return NORMAL;
}

void freePersonList(struct _PersonList_** person_list)
{
  int index = 0;
  if((*person_list)->data_is_allocated_)
  {
    printf("deleted Person data as flag was true\n");
    for(index = 0; index < (*person_list)->length_; index++)
    {
      freePerson(&((*person_list)->list_[index]));
    }
  }
  free((*person_list)->list_);
  free(*person_list);
  *person_list = NULL;
}

int addPersonToList(struct _Person_** input_person, struct _PersonList_* person_list){
  int index = 0;
  bool person_already_exists = false;
  struct _Person_* existing_person = NULL;
  struct _Person_* new_person = *input_person;
  for(index = 0; index < person_list->length_ ; index++)
  {
    if(strcmp(person_list->list_[index]->name_, new_person->name_) == 0 &&
    person_list->list_[index]->gender_ == new_person->gender_)
    {
      person_already_exists = true;
      existing_person = person_list->list_[index];
      break;
    }
  }
  if(!person_already_exists)
  {
    //add new_person to list, if it does not exist yet
    person_list->length_++;
    person_list->list_ = realloc(person_list->list_, person_list->length_ * sizeof(struct _Person_*));
    if(person_list->list_ == NULL)
    {
      printError(OUT_OF_MEMORY);
      return MEMORY_EXCEPTION;
    }
    person_list->list_[person_list->length_ - 1] = new_person;
    printf("added person %s \n", new_person->name_);
  }
  else
  {
    //if new_person already exists -> delete it
    free(new_person->name_);
    free(new_person);
    *input_person = existing_person;
  }
  printf("%p mother %p father\n", (*input_person)->mother_, (*input_person)->father_);
  if((*input_person)->mother_ != NULL)
  {
    printf("mother is %s\n", (*input_person)->mother_->name_);
  }
  if((*input_person)->father_ != NULL)
  {
    printf("father is %s\n", (*input_person)->father_->name_);
  }
  return NORMAL; 
}

int removePersonFromPersonList(struct _PersonList_* list_of_persons, struct _Person_* person)
{
  int index = 0;
  int position_of_person = -1;

  for(index = 0; index < list_of_persons->length_; index++)
  {
    if(list_of_persons->list_[index] == person)
    {
      //save the position of the freed person
      position_of_person = index;
      //free the person
      freePerson(&person);
      break;
    } 
  }

  if(position_of_person == -1)
  {
    return ERROR;
  }
  //move consecutive indexes to fill the gap
  for(index = position_of_person; index < list_of_persons->length_ - 1; index++)
  {
    list_of_persons->list_[index] = list_of_persons->list_[index + 1]; 
  }
  list_of_persons->length_--;

  return NORMAL;
}

int getExistingPerson(struct _Person_* person_to_get, struct _Person_** existing_person, struct _PersonList_* person_list)
{
  int index = 0;
  for(index = 0; index < person_list->length_; index++)
  {
    if(strcmp(person_to_get->name_, person_list->list_[index]->name_) == 0 &&
        person_to_get->gender_ == person_list->list_[index]->gender_)
    {
      freePerson(&person_to_get);
      *existing_person = person_list->list_[index];
      return NORMAL;
    }
  }
  freePerson(&person_to_get);
  return ERROR;
}

int createAncestorList(struct _Person_* person, struct _PersonList_* person_list)
{
  int status = NORMAL;
  
  person_list->length_++;
  person_list->list_ = realloc(person_list->list_, person_list->length_ * sizeof(struct _Person_*));
  if(person_list->list_ == NULL)
  {
    printError(OUT_OF_MEMORY);
    return MEMORY_EXCEPTION;
  }
  person_list->list_[person_list->length_ - 1] = person;
  if(person->father_ != NULL)
  {
    status = createAncestorList(person->father_, person_list);
    if(status != NORMAL)
    {
      return status;
    }
  }
  if(person->mother_ != NULL)
  {
    status = createAncestorList(person->mother_, person_list);
    if(status != NORMAL)
    {
      return status;
    }
  }
  return NORMAL;
}

int detectCirclesRecursiveSearch(struct _Person_* person_to_check, struct _Person_* starting_person)
{
  int return_value = 0;
  if(person_to_check == starting_person)
  {
    return 1;
  }
  if(starting_person->father_ != NULL)
  {
    return_value = detectCirclesRecursiveSearch(person_to_check, starting_person->father_);
  }
  if(return_value != 0)
  {
    return return_value;
  }
  if(starting_person->mother_ != NULL)
  {
    return_value = detectCirclesRecursiveSearch(person_to_check, starting_person->mother_);
  }
  return return_value;
}

bool detectCircles(struct _Person_* person_to_check)
{
  int return_value = 0;  
  if(person_to_check->father_ != NULL)
  {
    return_value = detectCirclesRecursiveSearch(person_to_check, person_to_check->father_);
  }
  if(return_value != 0)
  {
    return return_value;
  }
  if(person_to_check->mother_ != NULL)
  {
    return_value = detectCirclesRecursiveSearch(person_to_check, person_to_check->mother_);
  }
  return return_value == 1 ? true : false;
}

int addPersonsWithRelationCommand(char* console_input, struct _PersonList_* all_persons, int* question_mark_person_counter)
{
  struct _Person_* person1 = malloc(sizeof(struct _Person_));
  person1->name_ = NULL;
  struct _Person_* person2 = malloc(sizeof(struct _Person_));
  person2->name_ = NULL;
  int status = NORMAL; 
  char* position = console_input;
  char* relation_input;

  if(console_input == NULL)
  {
    status = ERROR;  
  }

  //parse first person
  if(status == NORMAL)
  {
    status = parsePerson(&position, person1);
  }

  //skip whitespace in string
  if(status == NORMAL)
  {
    status = skipWhitespace(&position);
  }

  //parse relation
  if(status == NORMAL)
  {
    relation_input = strtok(position, " ");
    printf("relation input is %s\n", relation_input);
    if(relation_input == NULL)
    {
      status = ERROR;
    }
    if(status == NORMAL)
    {
      position += strlen(relation_input) + 1;
    }
  }

  //skip whitespace in string
  if(status == NORMAL)
  {
    status = skipWhitespace(&position);
  }

  //parse second person
  if(status == NORMAL)
  {
    status = parsePerson(&position, person2);
  }

  if(status != NORMAL)
  {
    if(status == ERROR)
    {
      if(person1->name_ != NULL)
      {
        free(person1->name_);
      }
      free(person1);
      if(person2->name_ != NULL)
      {
        free(person2->name_);
      }
      free(person2);
      printError(ADD_PERSONS_USAGE);
    }
    return status;
  }
  
  enum _Relations_ relation;

  if(strcmp(relation_input, "mother") == 0)
  {
    relation = RELATION_MOTHER;
  }
  else if(strcmp(relation_input, "father") == 0)
  {
    relation = RELATION_FATHER;
  }
  else if(strcmp(relation_input, "mgm") == 0)
  {
    relation = RELATION_MOTHER_GRANDMOTHER;
  }
  else if(strcmp(relation_input, "fgm") == 0)
  {
    relation = RELATION_FATHER_GRANDMOTHER; 
  } 
  else if(strcmp(relation_input, "mgf") == 0)
  {
    relation = RELATION_MOTHER_GRANDFATHER;
  }
  else if(strcmp(relation_input, "fgf") == 0)
  {
    relation = RELATION_FATHER_GRANDFATHER;
  }
  else
  {
    freePerson(&person1);
    freePerson(&person2);
    printError(ADD_PERSONS_USAGE);
    return ERROR;
  }

  printf("person1 gender is %d\n", person1->gender_);
  printf("person2 gender is %d\n", person2->gender_);

  if(((relation == RELATION_MOTHER ||
     relation == RELATION_MOTHER_GRANDMOTHER ||
     relation == RELATION_MOTHER_GRANDMOTHER) &&
     person1->gender_ == MALE) ||
     ((relation == RELATION_FATHER ||
     relation == RELATION_FATHER_GRANDFATHER ||
     relation == RELATION_FATHER_GRANDFATHER) &&
     person1->gender_ == FEMALE))
  {
    freePerson(&person1);
    freePerson(&person2);
    printError(ADD_SEX_DOES_NOT_MATCH);
    return ERROR;
  }

  if(strcmp(person1->name_, person2->name_) == 0 &&
    person1->gender_ == person2->gender_)
  {

    freePerson(&person1);
    freePerson(&person2);
    printError(ADD_BOTH_PEOPLE_ARE_SAME);
    return ERROR;
  }
  
  status = addPersonToList(&person1, all_persons);
  if(status != NORMAL)
  {
    return status;
  }
  status = addPersonToList(&person2, all_persons);
  if(status != NORMAL)
  {
    return status;
  } 
  printf("successfully added persons");

  struct _Person_** parent;
  struct _Person_** grandparent;

  if(relation == RELATION_FATHER || relation == RELATION_MOTHER)
  {
    if(relation == RELATION_MOTHER)
    {
      parent = &(person2->mother_);
    } else
    {
      parent = &(person2->father_);
    }
    printf("person2->father %p, person2->mother %p, parent %p", person2->father_, person2->mother_, *parent);

    if(*parent == NULL)
    {
      *parent = person1;
      printf("Normally added mother %s to %s\n", person1->name_, person2->name_);
    }
    else if((*parent)->name_[0] == '?' &&
        (*parent)->mother_ == person1->mother_ &&
        (*parent)->father_ == person1->father_)
    {
      status = copyPersonIntoAnotherPerson(*parent, person1);
      if(status != NORMAL)
      {
        return status;
      }
      removePersonFromPersonList(all_persons, person1);
    }
    else
    {
      printError(ADD_RELATION_NOT_POSSIBLE);
      return ERROR;
    }
    if(detectCircles(person1))
    {
      //delete all relations
      *parent = NULL;
      printError(ADD_RELATION_NOT_POSSIBLE);
      return ERROR;
    }
  }
  else
  {

    if(relation == RELATION_MOTHER_GRANDMOTHER || relation == RELATION_MOTHER_GRANDFATHER)
    {
      parent = &(person2->mother_);
    }
    else
    {
      parent = &(person2->father_);
    }

    if(*parent == NULL)
    {
      struct _Person_* question_mark;
      printf("before creating question mark");
      status = createQuestionMarkPerson(question_mark_person_counter, &question_mark);
      printf("after creating question mark");
      if(status != NORMAL)
      {
        return status; 
      }
      status = addPersonToList(&question_mark, all_persons);
      if(status != NORMAL)
      {
        return status;
      }
      *parent = question_mark;
    }

    if(relation == RELATION_MOTHER_GRANDMOTHER || relation == RELATION_FATHER_GRANDMOTHER)
    {
      grandparent = &((*parent)->mother_);
    }
    else
    {
      grandparent = &((*parent)->father_);
    }

    if(*grandparent == NULL)
    {
      //grandparent doesn't exist yet so make person new grandparent
      *grandparent = person1;
    }
    else if((*grandparent)->name_[0] == '?' &&
        (*grandparent)->mother_ == person1->mother_ &&
        (*grandparent)->father_ == person1->father_)
    {
      //grandparent is a question mark person, but we can replace it
      status = copyPersonIntoAnotherPerson(*grandparent, person1);
      if(status != NORMAL)
      {
        return status;
      }
      removePersonFromPersonList(all_persons, person1);
    }
    else
    {
      //grandparent is already defined and is no matching question mark
      printError(ADD_RELATION_NOT_POSSIBLE);
      return ERROR;
    }
    //detect circles
    if(detectCircles(person1))
    {
      printf("Detected a circle!\n");
      //delete all relations
      if((*parent)->name_[0] == '?')
      {
        //having a single question mark without relation would not make sense
        removePersonFromPersonList(all_persons, *parent);
      }
      *parent = NULL;
      *grandparent = NULL;
      printError(ADD_RELATION_NOT_POSSIBLE);
      return ERROR;
    }
  }
  return NORMAL;
}

//relation

int checkIfPeopleAreRelated(char* console_input, struct _PersonList_* all_persons)
{
  struct _Person_* person1;
  int status = initializePerson(&person1);
  if(status != NORMAL)
  {
    return status;
  }

  struct _Person_* person2;
  status = initializePerson(&person2);
  if(status != NORMAL)
  {
    return status;
  }

  char* position = console_input;
  if(console_input == NULL)
  {
    status = ERROR;
  }
  
  if(status == NORMAL)
  {
    status = parsePerson(&position, person1);
  }

  if(status == NORMAL)
  {
    status = parsePerson(&position, person2);
  }

  if(status != NORMAL)
  {
    if(status == ERROR)
    {
      freePerson(&person1);
      freePerson(&person2);
      printError(RELATIONSHIP_USAGE);
    }
    return status;
  }

  struct _Person_* existing_person1 = NULL;
  struct _Person_* existing_person2 = NULL;
  status = getExistingPerson(person1, &existing_person1, all_persons);
  if(status != NORMAL)
  {
    printf("first person inexistant\n");
    if(status == ERROR)
    {
      freePerson(&person2);
      printError(RELATIONSHIP_INEXISTANT_PERSONS);
    }
    return status;
  }

  status = getExistingPerson(person2, &existing_person2, all_persons);
  if(status != NORMAL)
  {
    printf("second person inexistant\n");
    if(status == ERROR)
    {
      printError(RELATIONSHIP_INEXISTANT_PERSONS);
    }
    return status;
  }

  if(strcmp(existing_person1->name_, existing_person2->name_) == 0 &&
    existing_person1->gender_ == existing_person2->gender_)
  {
    printError(RELATIONSHIP_BOTH_PEOPLE_ARE_SAME);
    return ERROR;
  }
  
  //create ancestor lists of both persons
  struct _PersonList_* ancestors_of_person1;
  status = initializePersonList(&ancestors_of_person1, false);
  if(status != NORMAL)
  {
    return status;
  }
  
  struct _PersonList_* ancestors_of_person2;
  status = initializePersonList(&ancestors_of_person2, false);
  if(status != NORMAL)
  {
    return status;
  }

  status = createAncestorList(existing_person1, ancestors_of_person1);
  if(status != NORMAL)
  {
    return status;
  }

  status = createAncestorList(existing_person2, ancestors_of_person2);
  if(status != NORMAL)
  {
    return status;
  }

  //compare every entry of both lists with every entry of other list

  int index_ancestors1 = 0;
  int index_ancestors2 = 0;
  
  /*
  printf("ancestorlist1 length: %d", ancestors_of_person1->length_);
  printf("ancestorlist2 length: %d", ancestors_of_person2->length_);
  for(int i = 0; i < ancestors_of_person1->length_; i++)
  {
    printf("ancestor person 1: %p\n", ancestors_of_person1->list_[i]);
  }
  for(int i = 0; i < ancestors_of_person2->length_; i++)
  {
    printf("ancestor person 2: %p\n", ancestors_of_person2->list_[i]);
  }
  */

  bool people_are_related = false;
  for(index_ancestors1 = 0; index_ancestors1 < ancestors_of_person1->length_; index_ancestors1++)
  {
    for(index_ancestors2 = 0; index_ancestors2 < ancestors_of_person2->length_; index_ancestors2++)
    {
      if(ancestors_of_person1->list_[index_ancestors1] == ancestors_of_person2->list_[index_ancestors2])
      {
        people_are_related = true;
        break;
      }
    }
    if(people_are_related)
    {
      break;
    }
  }

  if(people_are_related)
  {
    printf("%s", RELATIONSHIP_EXISTANT);
  }
  else
  {
    printf("%s", RELATIONSHIP_NO_RELATIONSHIP); 
  }

  freePersonList(&ancestors_of_person1);
  freePersonList(&ancestors_of_person2);

  enum _Relationship_ relationship = NO_RELATION;

  if(existing_person1->gender_ == MALE)
  {
    if(existing_person1->father_!= NULL && existing_person1->mother_!= NULL &&
       existing_person2->father_!= NULL && existing_person2->mother_!= NULL &&
      existing_person1->father_ == existing_person2->father_ &&
      existing_person1->mother_ == existing_person2->mother_)
    {
      relationship = BROTHER;
    }
    else if(existing_person2->father_ != NULL && existing_person2->father_ == existing_person1)
    {
      relationship = FATHER; 
    } 
    else if(((existing_person2->father_ != NULL && existing_person2->father_->father_ != NULL &&
            existing_person2->father_->mother_ != NULL &&
            existing_person2->father_->father_ == existing_person1->father_ &&
            existing_person2->father_->mother_ == existing_person1->mother_) ||
           (existing_person2->mother_ != NULL && existing_person2->mother_->father_ != NULL &&
            existing_person2->mother_->mother_ != NULL &&
            existing_person2->mother_->father_ == existing_person1->father_ &&
            existing_person2->mother_->mother_ == existing_person1->mother_)))
    {
      relationship = UNCLE;
    }
    else if((existing_person2->mother_ != NULL && existing_person2->mother_->father_ == existing_person1) ||
            (existing_person2->father_ != NULL && existing_person2->father_->father_ == existing_person1))
    {
      relationship = GRANDFATHER;  
    }
  }
  else
  {
    if(existing_person1->father_!= NULL && existing_person1->mother_!= NULL &&
       existing_person2->father_!= NULL && existing_person2->mother_!= NULL &&
      existing_person1->father_ == existing_person2->father_ &&
      existing_person1->mother_ == existing_person2->mother_)
    {
      relationship = SISTER;
    }
    else if(existing_person2->mother_ != NULL && existing_person2->mother_ == existing_person1)
    {
      relationship = MOTHER;
    }
    else if(((existing_person2->father_ != NULL && existing_person2->father_->father_ != NULL &&
            existing_person2->father_->mother_ != NULL &&
            existing_person2->father_->father_ == existing_person1->father_ &&
            existing_person2->father_->mother_ == existing_person1->mother_) ||
           (existing_person2->mother_ != NULL && existing_person2->mother_->father_ != NULL &&
            existing_person2->mother_->mother_ != NULL &&
            existing_person2->mother_->father_ == existing_person1->father_ &&
            existing_person2->mother_->mother_ == existing_person1->mother_)))
    {
      relationship = AUNT;
    }
    else if((existing_person2->mother_ != NULL && existing_person2->mother_->mother_ == existing_person1) ||
            (existing_person2->father_ != NULL && existing_person2->father_->mother_ == existing_person1))
    {
      relationship = GRANDMOTHER;  
    }
    //printf("existing_person2 mother %p, existing_person2 grandmother %p, existing_person1 %p", existing_person2->mother_, existing_person2->mother_->mother_, existing_person1);
  }

  char* identifier = NULL;

  switch(relationship)
  {
    case SISTER:
      identifier = "sister";
      break;
    case BROTHER:
      identifier = "brother";
      break;
    case MOTHER:
      identifier = "mother";
      break;
    case FATHER:
      identifier = "father";
      break;
    case AUNT:
      identifier = "aunt";
      break;
    case UNCLE:
      identifier = "uncle";
      break;
    case GRANDMOTHER:
      identifier = "grandmother";
      break;
    case GRANDFATHER:
      identifier = "grandfather";
      break;
    default:
      break;
  }
  
  if(relationship != NO_RELATION)
  {
    char gender_person1 = existing_person1->gender_ == MALE ? 'm' : 'f';
    char gender_person2 = existing_person2->gender_ == MALE ? 'm' : 'f';
    printf("%s [%c] is the %s of %s [%c].\n", existing_person1->name_, gender_person1, identifier, existing_person2->name_, gender_person2);
  }
  return NORMAL;
}





int linesComparisonFunction(const void* first_string_pointer, const void* second_string_pointer)
{
  int difference = 0;
  int position = 0;
  char* first_string = *(char**)first_string_pointer;
  char* second_string = *(char**)second_string_pointer;
  while(difference == 0)
  {
    char a = first_string[position];
    char b = second_string[position];
    //questionmarks always appear on top of the list
    if(position == 0)
    {
      if(a == '?')
      {
        a = ' ';
      }
      if(b == '?')
      {
        b = ' ';
      }
    }
    //remap to sort Sirius Jr. and Sirius the right way
    if(a == '[')
    {
      a = ' '; 
    }
    if(b == '[')
    {
      b = ' '; 
    }
    difference = a - b;
    position++;
  }
  printf("comparing %s and %s, difference is: %d\n", first_string, second_string, difference);
  return difference;
}


int listAllPersons(char* arguments, struct _PersonList_* all_persons)
{
  if(arguments != NULL)
  {
    printError(LIST_WRONG_USAGE);
    return ERROR;
  }
  if(all_persons->length_ == 0)
  {
    printError(LIST_NO_ENTRIES_AVAILABLE);
    return ERROR;
  }

  struct _LineBuffer_* lines_buffer;
  int status = initializeLineBuffer(&lines_buffer, true);
  if(status != NORMAL)
  {
    return status;
  }

  int index = 0;
  for(index = 0; index < all_persons->length_; index++)
  {
    struct _Person_* current_Person = all_persons->list_[index];
    int length_name = strlen(current_Person->name_);
    char gender = current_Person->gender_ == MALE ? 'm' : 'f';
    int length_string = length_name + 5 + 1;

    char* line = calloc(length_string, sizeof(char));
    if(line == NULL)
    {
      printError(OUT_OF_MEMORY);
      return MEMORY_EXCEPTION;
    }
    sprintf(line , "%s [%c]", current_Person->name_, gender);
    addLineToLineBuffer(lines_buffer, line);
  }

  //sort lines_buffer
  qsort(lines_buffer->data_, lines_buffer->position_, sizeof(char*), linesComparisonFunction);
  
  for(index = 0; index < lines_buffer->position_; index++)
  {
    printf("%s\n", lines_buffer->data_[index]);
  }

  freeLineBuffer(&lines_buffer);
  return NORMAL;
}

bool personHasChildrenInList(struct _Person_* person, struct _PersonList_* list_of_persons)
{
  int index = 0; 
  for(index = 0; index < list_of_persons->length_; index++)
  {
    struct _Person_* person_from_list = list_of_persons->list_[index];
    if(person_from_list->father_ == person || person_from_list->mother_ == person)
    {
      printf("person %s has children\n", person->name_);
      return true;
    }
  }
  printf("person %s does not have children\n", person->name_);
  return false;
}


int drawAllLinesComparisonFunction(const void* first_string_pointer, const void* second_string_pointer)
{
  int difference = 0;
  int position = 0;
  int parenthesis_count = 0;
  bool first_char_of_name = false;

  char* first_string = *(char**)first_string_pointer;
  char* second_string = *(char**)second_string_pointer;
  while(difference == 0 && parenthesis_count < 2)
  {
    char a = first_string[position];
    char b = second_string[position];
    if(first_char_of_name)
    {
      if(a == '?')
      {
        a = ' ';
      }
      if(b == '?')
      {
        b = ' ';
      }
      first_char_of_name = false;
    }
    if(a == '[')
    {
      a = ' '; 
    }
    if(b == '[')
    {
      b = ' '; 
    }
    if(a == '"' && b == '"')
    {
      parenthesis_count++;
      first_char_of_name = true;
    } 
    difference = a - b;
    position++;
  }
  if(difference != 0)
  {
    printf("comparing %s and %s, difference is: %d\n", first_string, second_string, difference);
    return difference;
  }

  printf("first person is equal\n");
  //first person is equal -> check second person gender
  char* first_string_second_person_gender = strchr(first_string + position, '[');
  char* second_string_second_person_gender = strchr(second_string + position, '[');
  printf("assigned char*ers\n");
  
  if(first_string_second_person_gender != NULL && 
      second_string_second_person_gender != NULL)
  {
     return first_string_second_person_gender[1] - second_string_second_person_gender[1]; 
  }
  else
  {
    return 1;
  }
}

int parsePersonListIntoLineBuffer(struct _LineBuffer_* lines_buffer, struct _PersonList_* person_list)
{
  int index = 0;
  for(index = 0; index < person_list->length_; index++)
  {
    struct _Person_* current_Person = person_list->list_[index];
    struct _Person_* parent = NULL;
    int length_name = strlen(current_Person->name_);
    int length_parent_name = 0;
    char person_gender = current_Person->gender_ == MALE ? 'm' : 'f';
    char parent_gender = '\0';
    int length_string = 0;
    char* line;

    if(current_Person->father_ == NULL && current_Person->mother_ == NULL)
    {
      length_string = length_name + 10 + 1;   

      line = calloc(length_string, sizeof(char));
      if(line == NULL)
      {
        printError(OUT_OF_MEMORY);
        return MEMORY_EXCEPTION;
      }

      sprintf(line , "  \"%s [%c]\";", current_Person->name_, person_gender);
      addLineToLineBuffer(lines_buffer, line);
    }
    if(current_Person->mother_ != NULL) 
    {
      parent = current_Person->mother_;
      length_parent_name = strlen(parent->name_);
      parent_gender = 'f';
      length_string = length_name + length_parent_name + 19 + 1;   

      line = calloc(length_string, sizeof(char));
      if(line == NULL)
      {
        printError(OUT_OF_MEMORY);
        return MEMORY_EXCEPTION;
      }

      sprintf(line, "  \"%s [%c]\" -> \"%s [%c]\";", current_Person->name_, person_gender,
      parent->name_, parent_gender);
      addLineToLineBuffer(lines_buffer, line);
    }
    if(current_Person->father_ != NULL)
    {
      parent = current_Person->father_;
      length_parent_name = strlen(parent->name_);
      parent_gender = 'm';
      length_string = length_name + length_parent_name + 19 + 1;   

      line = calloc(length_string, sizeof(char));
      if(line == NULL)
      {
        printError(OUT_OF_MEMORY);
        return MEMORY_EXCEPTION;
      }

      sprintf(line, "  \"%s [%c]\" -> \"%s [%c]\";", current_Person->name_, person_gender,
      parent->name_, parent_gender);
      addLineToLineBuffer(lines_buffer, line);
    }
  }
  return NORMAL;
}

int openDotFileForWriting(FILE** file_pointer, char* filename)
{
  int filename_length = strlen(filename) + 4 + 1;
  char* filename_with_extension = malloc(filename_length * sizeof(char));

  sprintf(filename_with_extension, "%s.dot", filename);
  *file_pointer = fopen(filename_with_extension, "w");
  free(filename_with_extension);

  if(*file_pointer == NULL)
  {
    printError(DRAW_ALL_COULD_NOT_WRITE_FILE); 
    return ERROR;
  }
  return NORMAL;
}

int drawAllPersonsToFile(char* arguments, struct _PersonList_* all_persons)
{
  if(arguments == NULL)
  {
    printError(DRAW_ALL_USAGE);
    return ERROR;
  }

  char* last_char_of_arguments = arguments + strlen(arguments);
  char* input_filename = strtok(arguments, " ");

  if(input_filename == NULL || input_filename + strlen(input_filename) != last_char_of_arguments)
  {
    printError(DRAW_ALL_USAGE);
    return ERROR;
  } 
  if(all_persons->length_ == 0)
  {
    printError(DRAW_ALL_NO_ENTRIES_AVAILABLE);
    return ERROR;
  }
  
  FILE* output_file;

  int status = openDotFileForWriting(&output_file, input_filename);
  if(status != NORMAL)
  {
    if(status == ERROR)
    {
      printError(DRAW_COULD_NOT_WRITE_FILE); 
    }
    return status;
  } 

  struct _LineBuffer_* lines_buffer;
  status = initializeLineBuffer(&lines_buffer, true);
  if(status != NORMAL)
  {
    return status;
  }
  
  status = parsePersonListIntoLineBuffer(lines_buffer, all_persons);  
  if(status != NORMAL)
  {
    return status;
  } 

  //sort lines_buffer
  qsort(lines_buffer->data_, lines_buffer->position_, sizeof(char*), drawAllLinesComparisonFunction);

  //write lines_buffer to output file
  fprintf(output_file, "digraph FamilyTree\n"); fprintf(output_file, "{\n");

  int index = 0;
  for(index = 0; index < lines_buffer->position_; index++)
  {
    fprintf(output_file, "%s\n", lines_buffer->data_[index]);
  }
  fprintf(output_file, "}\n");
  
  printf("%s", DRAW_ALL_SUCCESS);
  freeLineBuffer(&lines_buffer);
  fclose(output_file);
  return NORMAL;
}

int drawPersonsFromRootToFile(char* arguments, struct _PersonList_* all_persons)
{
  if(arguments == NULL)
  {
    printError(DRAW_USAGE);
    return ERROR;
  }
  char* position = arguments;
  char* last_char_of_arguments = arguments + strlen(arguments); 


  struct _Person_* person;
  int status = initializePerson(&person);
  if(status != NORMAL)
  {
    return status;
  }
  
  status = parsePerson(&position, person);
  if(status != NORMAL)
  {
    if(status == ERROR)
    {
      printError(DRAW_USAGE);
    }
    return status;
  }
  
  char* input_filename = strtok(position, " ");
  if(input_filename == NULL || input_filename + strlen(input_filename) != last_char_of_arguments)
  {
    free(person);
    printError(DRAW_USAGE);
    return ERROR;
  }

  struct _Person_* existing_person = NULL;
  status = getExistingPerson(person, &existing_person, all_persons);
  if(status != NORMAL)
  {
    if(status == ERROR)
    {
      printError(DRAW_INEXISTANT_PERSON); 
    } 
    return status;
  }

  FILE* output_file;

  status = openDotFileForWriting(&output_file, input_filename);
  if(status != NORMAL)
  {
    if(status == ERROR)
    {
      printError(DRAW_COULD_NOT_WRITE_FILE);
    }
    return status;
  }
  
  struct _LineBuffer_* lines_buffer;

  status = initializeLineBuffer(&lines_buffer, true);
  if(status != NORMAL)
  {
    return status;
  } 

  struct _PersonList_* ancestor_list;

  status = initializePersonList(&ancestor_list, false);
  if(status != NORMAL)
  {
    return status;
  }

  status = createAncestorList(existing_person, ancestor_list);
  if(status != NORMAL)
  {
    return status;
  }

  status = parsePersonListIntoLineBuffer(lines_buffer, ancestor_list);  
  if(status != NORMAL)
  {
    return status;
  } 

  //sort lines_buffer
  qsort(lines_buffer->data_, lines_buffer->position_, sizeof(char*), drawAllLinesComparisonFunction);

  //write lines_buffer to output file
  fprintf(output_file, "digraph FamilyTree\n"); fprintf(output_file, "{\n");
  
  int index = 0;
  for(index = 0; index < lines_buffer->position_; index++)
  {
    fprintf(output_file, "%s\n", lines_buffer->data_[index]);
  }
  fprintf(output_file, "}\n");
  
  printf("%s", DRAW_SUCCESS);
  freeLineBuffer(&lines_buffer);
  fclose(output_file);
  return NORMAL;
}

//-----------------------------------------------------------------------------
//
// Function that parses dot files
//
// @param command_buffer buffer to write input to
//
// @return int
//
//

struct _CharacterBuffer_{
  int length_;
  int position_;
  char* data_;
};

int initializeCharacterBuffer(struct _CharacterBuffer_** buffer)
{
  (*buffer) = malloc(sizeof(struct _CharacterBuffer_));
  if((*buffer) == NULL)
  {
    printf(OUT_OF_MEMORY);
    return MEMORY_EXCEPTION;
  }
  (*buffer)->length_ = 10;
  (*buffer)->position_ = 0;
  (*buffer)->data_ = malloc((*buffer)->length_ * sizeof(char));
  if((*buffer)->data_ == NULL)
  {
    printf(OUT_OF_MEMORY);
    return MEMORY_EXCEPTION;
  }
  return NORMAL;
}

int writeCharacterToCharacterBuffer(struct _CharacterBuffer_* buffer, char character)
{
  buffer->data_[buffer->position_++] = character;
  if(buffer->position_ >= buffer->length_)
  {
    buffer->length_ += 10;
    buffer->data_ = realloc(buffer->data_, buffer->length_ * sizeof(char));
    if(buffer == NULL)
    {
      printError(OUT_OF_MEMORY); 
      return MEMORY_EXCEPTION;
    }
  }
  return NORMAL;
}

void freeCharacterBuffer(struct _CharacterBuffer_** buffer)
{
  free((*buffer)->data_);
  free(*buffer);
  *buffer = NULL;
}

int openDotFileForReading(FILE** file_pointer, char* filename)
{
  *file_pointer = fopen(filename, "r");
  if(file_pointer == NULL)
  {
    return ERROR;
  }
  return NORMAL;
}

int parsePersonsFromInputFileLine(char* input_string, struct _PersonList_* peopleList, int num_people)
{
  int index = 0;
  int entries_made = 0;
  bool in_parenthesis = false;  
  int length_= strlen(input_string);
  struct _Person_* person;

  for(index = 0; index < length_; index++){
    if(input_string[index] == '\"')
    {
      in_parenthesis = !in_parenthesis;
      if(in_parenthesis && entries_made < num_people)
      {
        char* position_in_string = input_string + index + 1;
        int status = initializePerson(&person);
        if(status != NORMAL)
        {
          return status;
        }
        status = parsePerson(&(position_in_string), person);
        if(status != NORMAL)
        {
          return status;
        }
        status = addPersonToList(&person, peopleList);
        if(status != NORMAL)
        {
          return status;
        }
      }
      if(!in_parenthesis){
        input_string[index] = '\0';
      }
    }
  }

  if(in_parenthesis)
  {
    //reached end of string while in parenthesis
    return ERROR;
  }
  return NORMAL;
}

int parseDotFile(char* dot_inputfile_name, struct _PersonList_* all_persons){
  FILE* dot_file;
  int status = openDotFileForReading(&dot_file, dot_inputfile_name);
  if(status == ERROR)
  {
    printError(COULD_NOT_READ_FILE);
    return ERROR;
  }

  struct _LineBuffer_* lines;
  status = initializeLineBuffer(&lines, false);
  if(status != NORMAL)
  {
    return status;
  }

  struct _CharacterBuffer_* character_buffer;
  status = initializeCharacterBuffer(&character_buffer);
  if(status != NORMAL)
  {
    return status;
  }

  char input_char = '\0';

  while((input_char = fgetc(dot_file)) != EOF)
  {
    writeCharacterToCharacterBuffer(character_buffer, input_char);
  }

  writeCharacterToCharacterBuffer(character_buffer, '\0');

  char* line_pointer = strtok(character_buffer->data_, "\n");
  if(line_pointer == NULL)
  {
    printError(COULD_NOT_READ_FILE);
  } 

  status = addLineToLineBuffer(lines, line_pointer);
  if(status != NORMAL)
  {
    return status;
  }

  while((line_pointer = strtok(NULL, "\n")) != NULL)
  {
    status = addLineToLineBuffer(lines, line_pointer);
    if(status != NORMAL)
    {
      return status;
    }
  } 

  if(strcmp("digraph FamilyTree", lines->data_[0]) != 0)
  {
    printf("line is %s\n", lines->data_[0]);
    debug("digraph familytree did not match");
    printError(COULD_NOT_READ_FILE);
    return FILE_UNREADABLE_EXCEPTION;
  }

  if(lines->data_[1][0] != '{')
  {
    debug("{ in first line not found");
    printf("line is %s\n", lines->data_[0]);
    printError(COULD_NOT_READ_FILE);
    return FILE_UNREADABLE_EXCEPTION;
  }

  if(lines->data_[lines->position_ - 1][0] != '}')
  {
    printf("line is [%s]\n", lines->data_[lines->position_ - 1]);
    debug("} in last line not found");
    printError(COULD_NOT_READ_FILE);
    return FILE_UNREADABLE_EXCEPTION;
  }

  int line = 0;
  for(line=2; line < lines->position_ - 1; line++)
  {

    printf("%s\n", lines->data_[line]);
    if(strstr(lines->data_[line], "->") != NULL)
    {
      struct _PersonList_* input_persons;
      status = initializePersonList(&input_persons, true);
      if(status == ERROR)
      {
        return status;
      }

      status = parsePersonsFromInputFileLine(lines->data_[line], input_persons, 2);
      if(status == ERROR)
      {
        printError(COULD_NOT_READ_FILE);
        return FILE_UNREADABLE_EXCEPTION;
      }
      
      struct _Person_* person1;
      status = initializePerson(&person1);
      if(status != NORMAL)
      {
        return status;
      }

      copyPersonIntoAnotherPerson(person1, input_persons->list_[0]);

      struct _Person_* person2;
      status = initializePerson(&person2);
      if(status != NORMAL)
      {
        return status;
      }

      copyPersonIntoAnotherPerson(person2, input_persons->list_[1]);


      status = addPersonToList(&person1, all_persons);
      if(status != NORMAL)
      {
        return status;
      }
      status = addPersonToList(&person2, all_persons);
      if(status != NORMAL)
      {
        return status;
      }

      printf("person1 gender: %d, person2 gender: %d\n", person1->gender_, person2->gender_);
      if(person2->gender_ == MALE)
      {
        person1->father_ = person2;
        printf("added father [%s] to person [%s]\n", person2->name_, person1->name_);
        if(strcmp(person1->name_, "Arcturus ") == 0)
        {
          printf("mother and father of Arcturus are %p %p\n", person2->mother_, person2->father_);
        }
      }
      else
      {
        person1->mother_ = person2;
        printf("added mother %s to person %s\n", person2->name_, person1->name_);
      }
      freePersonList(&input_persons);
    }
    else
    {
      struct _PersonList_* input_persons;
      status = initializePersonList(&input_persons, true);
      if(status == ERROR)
      {
        return status;
      }

      int status = parsePersonsFromInputFileLine(lines->data_[line], input_persons, 1);
      if(status == ERROR)
      {
        printError(COULD_NOT_READ_FILE);
        return FILE_UNREADABLE_EXCEPTION;
      }

      struct _Person_* person1;
      status = initializePerson(&person1);
      if(status != NORMAL)
      {
        return status;
      }

      copyPersonIntoAnotherPerson(person1, input_persons->list_[0]);
      status = addPersonToList(&person1, all_persons);
      if(status != NORMAL)
      {
        return status;
      }
      freePersonList(&input_persons);
    }
  }

  listAllPersons(NULL, all_persons);
  printf(SUCCESSFULLY_READ_FILE);
  freeLineBuffer(&lines);
  freeCharacterBuffer(&character_buffer);
  return NORMAL;
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

void commandPrompt(char** command_buffer, char** command, char** arguments)
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
  input_buffer[input_buffer_position] = '\0';
  *command_buffer = input_buffer;

  int total_length_of_command_buffer = strlen(*command_buffer);
  char* position_in_command_buffer = *command_buffer;
  //set arguments and command
  skipWhitespace(&position_in_command_buffer);
  *command = strtok(position_in_command_buffer, " ");
  if(*command == NULL)
  {
    return;
  }

  position_in_command_buffer += strlen(*command) + 1;
  
  if(*command_buffer + total_length_of_command_buffer > position_in_command_buffer){
    *arguments = *command_buffer + strlen(*command) + 1;
  }
}

//------------------------------------------------------------------------------
//
// The main program.
// Runs the main program. Contains the infinite command-prompt-loop. and the 
// logic to read in dot files from the console.
//
// @param argc used to count arguments from stdin
// @param argv used to receive arguments from stdin
//
// @return either zero on ordinary exit or a negative Number on error 
//

int main(int argc, char *argv[])
{
  int question_mark_person_counter = 1;

  struct _PersonList_* all_persons;
  int return_status = initializePersonList(&all_persons, true);
  if(return_status != NORMAL)
  {
    return return_status;
  }

  if(argc == 2)
  {
    char* dot_inputfile_name = argv[1]; 
    return_status = parseDotFile(dot_inputfile_name, all_persons);
    if(return_status != NORMAL)
    {
      return return_status;
    }
  }
  if(argc > 2)
  {
    printError(USAGE_EXCEPTION);
    return WRONG_USAGE_EXCEPTION;
  }

  char* command_buffer = NULL;
  char* command;
  char* arguments;

  while(return_status >= 0)
  {
    command = NULL;
    arguments = NULL;
    commandPrompt(&command_buffer, &command, &arguments);

    if(command == NULL)
    {
      continue;
    }

    if(strcmp(command, "quit") == 0)
    {
      printf(CLOSE_MESSAGE);
      free(command_buffer);
      break;
    }
    else if(strcmp(command, "EOF") == 0)
    {
      free(command_buffer);
      break;
    }
    if(strcmp(command, "add") == 0)
    {
      return_status = addPersonsWithRelationCommand(arguments, all_persons, &question_mark_person_counter);
    }
    else if(strcmp(command, "relationship") == 0)
    {
      return_status = checkIfPeopleAreRelated(arguments, all_persons);
    }
    else if(strcmp(command, "list") == 0)
    {
      return_status = listAllPersons(arguments, all_persons);
    }
    else if(strcmp(command, "draw-all") == 0)
    {
      return_status = drawAllPersonsToFile(arguments, all_persons);
    }
    else if(strcmp(command, "draw") == 0)
    {
      return_status = drawPersonsFromRootToFile(arguments, all_persons);
    }
    free(command_buffer); 
  } 
  freePersonList(&all_persons);
  return return_status;
}
