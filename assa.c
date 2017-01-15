//-----------------------------------------------------------------------------
//// assa.c
////
//// Group Assignment
////
//// Group: Group 6, study assistant Promitzer 
////
//// Authors: Stefan Maritsch 1635076 
//// Blaz Mesarec 1414280
////---------------------------------------------------------------------------
////

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h> 

#define CLOSE_MESSAGE "Bye.\n"
#define OUT_OF_MEMORY "[ERR] Out of memory.\n"
#define COULD_NOT_READ_FILE "[ERR] Could not read file.\n"
#define SUCCESSFULLY_READ_FILE "File parsing successful...\n"
#define USAGE_EXCEPTION "Usage: ./ass [file-name]\n"

//add command
#define ADD_PERSONS_USAGE \
"[ERR] Wrong usage - add <namePerson1> [m/f] <relation> <namePerson2> [m/f].\n"
#define ADD_SEX_DOES_NOT_MATCH "[ERR] Sex does not match with relation.\n"
#define ADD_BOTH_PEOPLE_ARE_SAME "[ERR] Both people are the same.\n"
#define ADD_RELATION_NOT_POSSIBLE "[ERR] Relation not possible.\n"
//relationship command
#define RELATIONSHIP_USAGE \
"[ERR] Wrong usage - relationship <namePerson1> [m/f] <namePerson2> [m/f].\n"
#define RELATIONSHIP_INEXISTANT_PERSONS \
"[ERR] At least one person does not exist yet.\n"
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

#define ERROR (-1)
#define NORMAL (0)
#define WRONG_USAGE_EXCEPTION (1)
#define MEMORY_EXCEPTION (2)
#define FILE_UNREADABLE_EXCEPTION (3)


//----------------------------------------------------------------------------
//
//  enum that describes the gender of a person
//

enum _Gender_
{
  MALE,
  FEMALE
};

//----------------------------------------------------------------------------
//
//  enum that describes the relations used in the add command 
//

enum _Relations_
{
  RELATION_MOTHER,
  RELATION_FATHER,
  RELATION_MOTHER_GRANDFATHER,
  RELATION_MOTHER_GRANDMOTHER,
  RELATION_FATHER_GRANDFATHER,
  RELATION_FATHER_GRANDMOTHER
};


//----------------------------------------------------------------------------
//
//  enum that describes the relations used in the relationship command 
//

enum _Relationship_
{
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


struct _Person_
{
    struct _Person_* father_;
    struct _Person_* mother_;
    enum _Gender_ gender_;
    char* name_;
};


//----------------------------------------------------------------------------
//
// Struct that holds and manages a list of persons
//

struct _PersonList_
{
  struct _Person_** list_;
  int length_;
  bool data_is_allocated_;
};


//----------------------------------------------------------------------------
//
// Struct that holds and manages a list of lines (e.g an array of strings)
//

struct _LineBuffer_
{
  char** data_;
  int position_;
  int length_;
  bool data_is_allocated_;
};


//----------------------------------------------------------------------------
//
//  struct that holds a character buffer  
//

struct _CharacterBuffer_
{
  int length_;
  int position_;
  char* data_;
};

//----------------------------------------------------------------------------
//
//  Prints an error to the consolse
//
//  @param error -> the error that should be printed to the console
//

void printError(char* error)
{
  printf("%s", error);
}

//string helper functions

//----------------------------------------------------------------------------
//
//  Skips whitespace in a string. Overwrites argument pointer with next non-
//  whitespace position in string.
//
//  @param string -> the string in which whitespace should be skipped.
//

int skipWhitespace(char** string)
{
  while((*string)[0] == ' ')
  {
    (*string)++;
    if((*string)[0] == '\0')
    {
      return ERROR;
    }
  }
  return NORMAL;
}


//----------------------------------------------------------------------------
//
//  Strips leading and trailing whitespace from a string by moving all char-
//  acters to the beginning of the string and inserting \0 characters at the
//  end of it.
//
//  @param string -> string to strip Whitespace from
//

int stripWhitespace(char* string)
{
  unsigned int index = 0;
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
  return NORMAL;
}


//----------------------------------------------------------------------------
//
//  Sorting function used by qsort to sort the output of the list command. 
//
//  @param first_string_pointer -> first comparison argument
//  @param second_string_pointer -> second comparison argument
//

int linesComparisonFunction(const void* first_string_pointer, 
    const void* second_string_pointer)
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
  return difference;
}


//----------------------------------------------------------------------------
//
//  Sorting function used by qsort to sort dot output files generated by
//  draw or draw-all
//
//  @param first_string_pointer -> first comparison argument
//  @param second_string_pointer -> second comparison argument
//

int drawAllLinesComparisonFunction(const void* first_string_pointer, 
    const void* second_string_pointer)
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
    return difference;
  }

  char* first_string_second_person_gender =
    strchr(first_string + position, '[');
  char* second_string_second_person_gender =
    strchr(second_string + position, '[');
  
  if(first_string_second_person_gender != NULL && 
      second_string_second_person_gender != NULL)
  {
     return first_string_second_person_gender[1] - 
       second_string_second_person_gender[1]; 
  }
  else
  {
    return 1;
  }
}


//line buffer helper functions

//----------------------------------------------------------------------------
//
//  Function that initializes a new LineBuffer
//  
//  @param buffer -> pointer pointer to a _LineBuffer_ that should be created
//  @param allocate_data -> boolean that sets whether or not char* line
//  pointers should be free'd when struct is free'd
//
//  @return MEMORY_EXCEPTION on failure to allocate memory or NORMAL on
//  success
//

int initializeLineBuffer(struct _LineBuffer_** buffer, bool allocate_data)
{
  *buffer = malloc(sizeof(struct _LineBuffer_));
  if(*buffer == NULL)
  {
    return MEMORY_EXCEPTION;
  }
  (*buffer)->length_ = 1;
  (*buffer)->position_ = 0;
  (*buffer)->data_is_allocated_ = allocate_data;
  (*buffer)->data_ = malloc(sizeof(char*) * (*buffer)->length_);
  if((*buffer)->data_ == NULL)
  {
    return MEMORY_EXCEPTION;
  }
  return NORMAL;
}


//----------------------------------------------------------------------------
//
//  Function that frees a LineBuffer struct 
//
//  @param buffer -> pointer to LineBuffer that should be free'd 
//
//

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


//----------------------------------------------------------------------------
//
//  Function that adds a line to a LineBuffer struct
//
//  @param line -> line to add
//
//  @return MEMORY_EXCEPTION on failure to allocate memory or NORMAL on
//  success
//

int addLineToLineBuffer(struct _LineBuffer_* line_buffer, char* line)
{
  line_buffer->data_[line_buffer->position_++] = line;
  if(line_buffer->position_ >= line_buffer->length_)
  {
    line_buffer->length_ += 10;
    line_buffer->data_ = realloc(line_buffer->data_,
        sizeof(char*) * line_buffer->length_);
    if(line_buffer->data_ == NULL)
    {
      return MEMORY_EXCEPTION;
    }
  }
  return NORMAL;
}


//----------------------------------------------------------------------------
//
//  Function that initializes a CharacterBuffer struct 
//
//  @param buffer -> pointer to the buffer that should be initialized 
//
//  @return MEMORY_EXCEPTION on failure to allocate memory or NORMAL on
//  success
//
//

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


//----------------------------------------------------------------------------
//
//  Function that frees a CharacterBuffer struct 
//
//  @param buffer -> pointer to the CharacterBuffer struct that should be
//  free'd 
//

void freeCharacterBuffer(struct _CharacterBuffer_** buffer)
{
  free((*buffer)->data_);
  free(*buffer);
  *buffer = NULL;
}


//----------------------------------------------------------------------------
//
//  Function that writes a Character to a CharacterBuffer struct
//
//  @param buffer -> CharacterBuffer struct to which the character should be
//  written
//

int writeCharacterToCharacterBuffer(struct _CharacterBuffer_* buffer, 
    char character)
{
  buffer->data_[buffer->position_++] = character;
  if(buffer->position_ >= buffer->length_)
  {
    buffer->length_ += 10;
    buffer->data_ = realloc(buffer->data_, buffer->length_ * sizeof(char));
    if(buffer == NULL)
    {
      return MEMORY_EXCEPTION;
    }
  }
  return NORMAL;
}



//person helper functions


//----------------------------------------------------------------------------
//
//  Function that initializes a Person struct
//
//  @param person -> pointer to a Person struct that should be initialized
//
//  @return MEMORY_EXCEPTION on failure to allocate memory or NORMAL on
//  success
//

int initializePerson(struct _Person_** person)
{
  *person = malloc(sizeof(struct _Person_));
  if(*person == NULL)
  {
    return MEMORY_EXCEPTION; 
  }
  (*person)->name_ = malloc(sizeof(char));
  if((*person)->name_ == NULL)
  {
    return MEMORY_EXCEPTION;
  }
  (*person)->mother_ = NULL;
  (*person)->father_ = NULL;
  (*person)->gender_ = MALE;
  return NORMAL;
}


//----------------------------------------------------------------------------
//
//  Function that frees a Person struct
//
//  @param Person -> the Person struct that should be free'd 
//

void freePerson(struct _Person_** person)
{
  if(*person != NULL)
  {
    if((*person)->name_ != NULL)
    {
      free((*person)->name_);
    }
    free(*person);
  } 
  *person = NULL;
}


//----------------------------------------------------------------------------
//
//  Function that parses a person from a Person string (e.g Stefan [m])
//
//  @param person_string -> Pointer to beginning of string which contains the 
//  person. Is overwritten to first position after person in person_string 
//  when person was successfully parsed.
//  @param Person -> pointer to Person where parsed Person is stored 
//
//  @return MEMORY_EXCEPTION on failure to allocate memory, ERROR,
//  when no person is found within the string, otherwise NORMAL
//  
//

int parsePerson(char** person_string, struct _Person_** person)
{
  bool person_found = false;
  int length_of_string = strlen(*person_string);
  char* string_position = *person_string;

  while(!person_found)
  {
    string_position = strchr(string_position, ' ');
    if(string_position == NULL)
    {
      return ERROR;
    }
    if(string_position + 3 <= *person_string + length_of_string &&
      string_position[1] == '[' &&
      (string_position[2] == 'm' || string_position[2] == 'f') &&
      string_position[3] == ']')
    {
      person_found = true;
    }
    //increment *string_position by one to search for brackets 
    //after current bracket
    string_position++;
  }
  string_position--;

  (*person)->name_ = realloc((*person)->name_,
      (string_position - *person_string + 1) * sizeof(char));
  if((*person)->name_ == NULL)
  {
    return MEMORY_EXCEPTION;
  }
  (*person)->name_[string_position - *person_string] = '\0';
  strncpy((*person)->name_, *person_string, string_position - *person_string);

  //length of a persons name cannot be zero
  if(strlen((*person)->name_) == 0)
  {
    freePerson(person);
    return ERROR;
  }

  stripWhitespace((*person)->name_);

  (*person)->gender_ = string_position[2] == 'm' ? MALE : FEMALE;
  (*person)->mother_ = NULL;
  (*person)->father_ = NULL;
  string_position += 4;
  *person_string = string_position;
  return NORMAL;
}


//----------------------------------------------------------------------------
//
//  Function that copies an initialized person struct into another
//  initialized person struct
//
//  @param destination -> the Person struct where data get written
//  @param source -> the Person struct whose data is copied 
//
//  
//  @return MEMORY_EXCEPTION on failure to allocate memory or NORMAL on
//  success
//

int copyPersonIntoAnotherPerson(struct _Person_* destination,
    struct _Person_* source)
{
  destination->name_ = realloc(destination->name_,
      (strlen(source->name_) + 1) * sizeof(char));
  if(destination->name_ == NULL)
  {
    return MEMORY_EXCEPTION;
  }
  strncpy(destination->name_, source->name_, strlen(source->name_) + 1);
  destination->gender_ = source->gender_;
  destination->mother_ = source->mother_;
  destination->father_ = source->father_;
  return NORMAL;
}


//----------------------------------------------------------------------------
//
//  Function that creates a question mark person.
//
//  @param question_mark_person_counter -> global counter for question mark
//  persons
//  @param person -> pointer to store person in
//
//  @return MEMORY_EXCEPTION on failure to allocate memory or NORMAL on
//  success
//

int createQuestionMarkPerson(int* question_mark_person_counter,
    struct _Person_** person, enum _Gender_ question_mark_gender)
{
  struct _Person_* question_mark_person;
  int status = initializePerson(&question_mark_person);
  if(status != NORMAL)
  {
    printf(OUT_OF_MEMORY);
    return MEMORY_EXCEPTION;
  }

  question_mark_person->name_ = realloc(question_mark_person->name_,
      (10 + 1) * sizeof(char));
  if(question_mark_person->name_ == NULL)
  {
    return MEMORY_EXCEPTION;
  }

  snprintf(question_mark_person->name_, 10, "?%d",
      *question_mark_person_counter);
  question_mark_person->gender_ = question_mark_gender;
  (*question_mark_person_counter)++;
  *person = question_mark_person;
  return NORMAL;
}

//person list helper functions


//----------------------------------------------------------------------------
//
//  Function that initializes a PersonList struct
//
//  @param person_list -> pointer to store PersonList in
//  @param data_allocated -> if true, persons in personList are free'd when
//  PersonList is free'd
//
//  @return MEMORY_EXCEPTION on failure to allocate memory or NORMAL on
//  success
//

int initializePersonList(struct _PersonList_** person_list,
    bool data_allocated)
{
  *person_list = malloc(sizeof(struct _PersonList_));
  if(*person_list == NULL)
  {
    return MEMORY_EXCEPTION;
  } 
  (*person_list)->length_ = 0;
  (*person_list)->list_ = malloc(sizeof(struct _Person_*));
  (*person_list)->data_is_allocated_ = data_allocated;
  if((*person_list)->list_ == NULL)
  {
    return MEMORY_EXCEPTION;
  } 
  return NORMAL;
}


//----------------------------------------------------------------------------
//
//  Function that frees a PersonList struct
//
//  @param person_list-> the PersonList struct that should be free'd 
//

void freePersonList(struct _PersonList_** person_list)
{
  int index = 0;
  if((*person_list)->data_is_allocated_)
  {
    for(index = 0; index < (*person_list)->length_; index++)
    {
      freePerson(&((*person_list)->list_[index]));
    }
  }
  free((*person_list)->list_);
  free(*person_list);
  *person_list = NULL;
}


//----------------------------------------------------------------------------
//
//  Function that adds a Person struct to a PersonList struct 
//
//  @param Person -> the Person struct that should be added 
//  @param person_list -> the PersonList struct the Person should be added to
//

int addPersonToList(struct _Person_** input_person,
    struct _PersonList_* person_list)
{
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
    person_list->list_ = realloc(person_list->list_,
        person_list->length_ * sizeof(struct _Person_*));
    if(person_list->list_ == NULL)
    {
      return MEMORY_EXCEPTION;
    }
    person_list->list_[person_list->length_ - 1] = new_person;
  }
  else
  {
    //if new_person already exists -> delete it
    freePerson(&new_person);
    *input_person = existing_person;
  }
  return NORMAL; 
}


//----------------------------------------------------------------------------
//
//  Function that removes a Person struct from a PersonList struct 
//
//  @param list_of_persons -> PersonList from which Person struct should be 
//  removed
//  @param Person -> pointer the Person struct that should be removed from the 
//  list 
//

int removePersonFromPersonList(struct _PersonList_* list_of_persons,
    struct _Person_* person)
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
  for(index = position_of_person; index < list_of_persons->length_ - 1;
      index++)
  {
    list_of_persons->list_[index] = list_of_persons->list_[index + 1]; 
  }
  list_of_persons->length_--;

  return NORMAL;
}


//----------------------------------------------------------------------------
//
//  Function that gets a Person from a PersonList struct 
//
//  @param person_to_get -> the Person struct that should be searched 
//  @param existing_person -> the Person struct from person_list
//  @param person_list -> the PersonList in which person_to_get should be 
//  searched
//
//  @return ERROR on failure to find person_to_get in PersonList memory or 
//  NORMAL on success
//

int getExistingPerson(struct _Person_* person_to_get,
    struct _Person_** existing_person, struct _PersonList_* person_list)
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


//----------------------------------------------------------------------------
//
//  Function that creates a list of all ancestors of a person. (contains
//  person itself)
//
// 
//  @param person -> person whose ancestors should be searched.
//  @param person_list -> list to store results in.
//

int createAncestorList(struct _Person_* person,
    struct _PersonList_* person_list)
{
  int status = addPersonToList(&person, person_list);
  if(status != NORMAL)
  {
    return status;
  }
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

//----------------------------------------------------------------------------
//
//  Function that compares the ancestor lists of two persons in search for
//  matches
//
//  @param person1 -> first person to check
//  @param person2 -> second person to check
//  
//  @return NORMAL, if everything worked, MEMORY_EXCEPTION on failure to 
//  allocate memory
//

int checkAncestorListsOfPersonsForMatches(struct _Person_* person1,
    struct _Person_* person2, bool* people_are_related)
{
  //create ancestor lists of both persons
  struct _PersonList_* ancestors_of_person1;
  int status = initializePersonList(&ancestors_of_person1, false);
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

  status = createAncestorList(person1, ancestors_of_person1);
  if(status != NORMAL)
  {
    return status;
  }

  status = createAncestorList(person2, ancestors_of_person2);
  if(status != NORMAL)
  {
    return status;
  }

  //compare every entry of both lists with every entry of other list

  int index_ancestors1 = 0;
  int index_ancestors2 = 0;

  for(index_ancestors1 = 0; index_ancestors1 < ancestors_of_person1->length_;
      index_ancestors1++)
  {
    for(index_ancestors2 = 0; index_ancestors2 < ancestors_of_person2->length_;
        index_ancestors2++)
    {
      if(ancestors_of_person1->list_[index_ancestors1] == 
          ancestors_of_person2->list_[index_ancestors2])
      {
        *people_are_related = true;
        break;
      }
    }
    if(*people_are_related)
    {
      break;
    }
  }

  freePersonList(&ancestors_of_person1);
  freePersonList(&ancestors_of_person2);

  return status;
}


//----------------------------------------------------------------------------
//
//  recursive Helper function for detectCircles.
//
//  @param person_to_check -> the Person struct that should be detected in 
//  ancestors
//  @param starting_persons -> recursively used parameter that holds ancestors
//  of person_to_check
//
//  @return 1 if person was found, 0 otherwise
//

int detectCirclesRecursiveSearch(struct _Person_* person_to_check,
    struct _Person_* starting_person)
{
  int return_value = 0;
  if(person_to_check == starting_person)
  {
    return 1;
  }
  if(starting_person->father_ != NULL)
  {
    return_value = detectCirclesRecursiveSearch(person_to_check,
        starting_person->father_);
  }
  if(return_value != 0)
  {
    return return_value;
  }
  if(starting_person->mother_ != NULL)
  {
    return_value = detectCirclesRecursiveSearch(person_to_check,
        starting_person->mother_);
  }
  return return_value;
}

//----------------------------------------------------------------------------
//
//  Function that detects circle relations. (checks if a person can reach
//  itself by propagating through its ancestors)
//
//  @param person_to_check -> the Person struct that should checked for circle
//  relations
//
//  @return true if circle relationship was detected, false otherwise
//

bool detectCircles(struct _Person_* person_to_check)
{
  int return_value = 0;  
  if(person_to_check->father_ != NULL)
  {
    return_value = detectCirclesRecursiveSearch(person_to_check,
        person_to_check->father_);
  }
  if(return_value != 0)
  {
    return return_value;
  }
  if(person_to_check->mother_ != NULL)
  {
    return_value = detectCirclesRecursiveSearch(person_to_check,
        person_to_check->mother_);
  }
  return return_value == 1 ? true : false;
}


//----------------------------------------------------------------------------
//
//  Function that Maps relation identifiers passed to the add function by the
//  user to an enum that is used internally.
//
//  @param relation_input -> the relation string passed by the user 
//  @param relation -> variable to store relation enum in
//
//  @return ERROR, if relation_input is not known, NORMAL otherwise
//

int mapRelationToRelationInput(char* relation_input,
    enum _Relations_* relation)
{
  if(strcmp(relation_input, "mother") == 0)
  {
    *relation = RELATION_MOTHER;
  }
  else if(strcmp(relation_input, "father") == 0)
  {
    *relation = RELATION_FATHER;
  }
  else if(strcmp(relation_input, "mgm") == 0)
  {
    *relation = RELATION_MOTHER_GRANDMOTHER;
  }
  else if(strcmp(relation_input, "fgm") == 0)
  {
    *relation = RELATION_FATHER_GRANDMOTHER; 
  } 
  else if(strcmp(relation_input, "mgf") == 0)
  {
    *relation = RELATION_MOTHER_GRANDFATHER;
  }
  else if(strcmp(relation_input, "fgf") == 0)
  {
    *relation = RELATION_FATHER_GRANDFATHER;
  }
  else
  {
    return ERROR;
  }
  return NORMAL;
}

//----------------------------------------------------------------------------
//
//  Function that creates compliant relationships between persons in 
//  a person list (no circles and question marks are replaced)
//
//  @param Person -> the Person struct that should be free'd 
//

int addRelationshipToPersons(struct _Person_* person1,
    struct _Person_* person2,
    struct _PersonList_* all_persons, int* question_mark_person_counter, 
    enum _Relations_ relation)
{
  struct _Person_** parent;
  struct _Person_** grandparent;
  int status = NORMAL;

  if(relation == RELATION_FATHER || relation == RELATION_MOTHER)
  {
    if(relation == RELATION_MOTHER)
    {
      parent = &(person2->mother_);
    }
    else
    {
      parent = &(person2->father_);
    }

    if(*parent == NULL)
    {
      *parent = person1;
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
      person1 = *parent;
    }
    else
    {
      return ERROR;
    }
    if(detectCircles(person1))
    {
      //delete all relations
      *parent = NULL;
      return ERROR;
    }
  }
  else
  {

    enum _Gender_ question_mark_gender;
    if(relation == RELATION_MOTHER_GRANDMOTHER || 
        relation == RELATION_MOTHER_GRANDFATHER)
    {
      parent = &(person2->mother_);
      question_mark_gender = FEMALE;
    }
    else
    {
      parent = &(person2->father_);
      question_mark_gender = MALE;
    }

    if(*parent == NULL)
    {
      struct _Person_* question_mark;
      status = createQuestionMarkPerson(question_mark_person_counter,
          &question_mark, question_mark_gender);
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

    if(relation == RELATION_MOTHER_GRANDMOTHER ||
        relation == RELATION_FATHER_GRANDMOTHER)
    {
      grandparent = &((*parent)->mother_);
    }
    else
    {
      grandparent = &((*parent)->father_);
    }

    if(*grandparent == NULL)
    {
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
      person1 = *parent;
    }
    else
    {
      //grandparent is already defined and is no matching question mark
      return ERROR;
    }
    if(detectCircles(person1))
    {
      //delete all relations
      if((*parent)->name_[0] == '?')
      {
        //having a single question mark without relation would not make sense
        removePersonFromPersonList(all_persons, *parent);
      }
      *parent = NULL;
      *grandparent = NULL;
      return ERROR;
    }
  }
  return NORMAL;
}

//----------------------------------------------------------------------------
//
//  Function that returns the relationship between two persons
//
//  @param person1 -> person to start searching from
//  @param person2 -> person that is father mother grandmother etc. of person1
//  
//  @return string that holds relationship identifier 
//

enum _Relationship_ getRelationshipBetweenPeople(struct _Person_* person1,
    struct _Person_* person2)
{
  enum _Relationship_ relationship = NO_RELATION;

  if(person1->gender_ == MALE)
  {
    if((person1->father_!= NULL && person2->father_ != NULL &&
          person1->father_ == person2->father_) ||
        (person1->mother_ != NULL && person2->mother_ != NULL &&
         person1->mother_ == person2->mother_))
    {
      relationship = BROTHER;
    }
    else if(person2->father_ != NULL && person2->father_ == person1)
    {
      relationship = FATHER; 
    } 
    else if((person2->father_ != NULL && person2->father_->father_ != NULL &&
            person2->father_->father_ == person1->father_) ||
           (person2->father_ != NULL && person2->father_->mother_ != NULL &&
            person2->father_->mother_ == person1->mother_) || 
           (person2->mother_ != NULL && person2->mother_->father_ != NULL &&
            person2->mother_->father_ == person1->father_) ||
           (person2->mother_ != NULL && person2->mother_->mother_ != NULL &&
            person2->mother_->mother_ == person1->mother_))
    {
      relationship = UNCLE;
    }
    else if((person2->mother_ != NULL &&
           person2->mother_->father_ == person1) ||
          (person2->father_ != NULL &&
           person2->father_->father_ == person1))
    {
      relationship = GRANDFATHER;  
    }
  }
  else
  {
    if((person1->father_!= NULL && person2->father_ != NULL &&
         person1->father_ == person2->father_) ||
        (person1->mother_ != NULL && person2->mother_ != NULL &&
         person1->mother_ == person2->mother_))
    {
      relationship = SISTER;
    }
    else if(person2->mother_ != NULL && person2->mother_ == person1)
    {
      relationship = MOTHER;
    }
    else if((person2->father_ != NULL && person2->father_->father_ != NULL &&
            person2->father_->father_ == person1->father_) ||
           (person2->father_ != NULL && person2->father_->mother_ != NULL &&
            person2->father_->mother_ == person1->mother_) || 
           (person2->mother_ != NULL && person2->mother_->father_ != NULL &&
            person2->mother_->father_ == person1->father_) ||
           (person2->mother_ != NULL && person2->mother_->mother_ != NULL &&
            person2->mother_->mother_ == person1->mother_))
    {
      relationship = AUNT;
    }
    else if((person2->mother_ != NULL &&
          person2->mother_->mother_ == person1) ||
          (person2->father_ != NULL &&
          person2->father_->mother_ == person1))
    {
      relationship = GRANDMOTHER;  
    }
  }
  return relationship;
}


//----------------------------------------------------------------------------
//
//  Function that implements the add command. It parses the arguments that
//  have the form <person1> relationship <person2> and adds the persons and 
//  their relationship the the PersonList all_persons if the relationship 
//  between them is valid. Prints Errors to the console on failure.
//
//  @param console_input -> string of arguments that comes from the user 
//  @param all_persons -> PersonList struct that contains the tree to 
//  which the new persons should be added.
//  @param question_mark_person -> integer that tracks how many question
//  mark persons have been added yet
//
//  @return MEMORY_EXCEPTION on failure to allocate memory, ERROR on failure
//  to successfully add persons, NORMAL on success
//

int addPersonsWithRelationCommand(char* console_input,
    struct _PersonList_* all_persons, int* question_mark_person_counter)
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
    status = parsePerson(&position, &person1);
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
    status = parsePerson(&position, &person2);
  }

  if(status != NORMAL)
  {
    if(status == ERROR)
    {
      freePerson(&person1);
      freePerson(&person2);
      printError(ADD_PERSONS_USAGE);
    }
    return status;
  }
  
  enum _Relations_ relation;
  status = mapRelationToRelationInput(relation_input, &relation);
  if(status != NORMAL)
  {
    freePerson(&person1);
    freePerson(&person2);
    printError(ADD_PERSONS_USAGE);
    return status;
  }

  if(((relation == RELATION_MOTHER ||
     relation == RELATION_MOTHER_GRANDMOTHER ||
     relation == RELATION_FATHER_GRANDMOTHER) &&
     person1->gender_ == MALE) ||
     ((relation == RELATION_FATHER ||
     relation == RELATION_MOTHER_GRANDFATHER ||
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

  status = addRelationshipToPersons(person1, person2, all_persons,
      question_mark_person_counter, relation);
  if(status != NORMAL)
  {
    if(status == ERROR)
    {
      printError(ADD_RELATION_NOT_POSSIBLE);
    }
    return status;
  }
  return NORMAL;
}

//----------------------------------------------------------------------------
//
//  Function that returns a relationship string used by relationship command
//
//  @param relationship -> relationship to get identifier for 
//  
//  @return string that holds relationship identifier 
//

char* getRelationshipIdentifier(enum _Relationship_ relationship)
{
  char* identifier;

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
      identifier = "undefined";
      break;
  }

  return identifier;
}

//----------------------------------------------------------------------------
//
//  Function that implements the relation command. Checks if two people
//  have a relation in the tree.
//
//  @param console_input -> arguments from the user
//  @param all_persons -> Person list used to check relations of persons
//
//  @return MEMORY_EXCEPTION on failure to allocate memory, ERROR if any
//  of the defined input errors occured or NORMAL on success
//

int checkIfPeopleAreRelated(char* console_input,
    struct _PersonList_* all_persons)
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
    status = parsePerson(&position, &person1);
  }

  if(status == NORMAL)
  {
    status = parsePerson(&position, &person2);
  }

  skipWhitespace(&position);

  if(position[0] != '\0')
  {
    status = ERROR;
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
 
  bool people_are_related = false;
  status = checkAncestorListsOfPersonsForMatches(existing_person1,
      existing_person2, &people_are_related);
  if(status != NORMAL)
  {
    return status;
  }

  if(people_are_related)
  {
    printf("%s", RELATIONSHIP_EXISTANT);
  }
  else
  {
    printf("%s", RELATIONSHIP_NO_RELATIONSHIP); 
  }

  enum _Relationship_ relationship =
    getRelationshipBetweenPeople(existing_person1, existing_person2);
  
  if(relationship != NO_RELATION)
  {
    char* identifier = getRelationshipIdentifier(relationship);
    char gender_person1 = existing_person1->gender_ == MALE ? 'm' : 'f';
    char gender_person2 = existing_person2->gender_ == MALE ? 'm' : 'f';
    printf("%s [%c] is the %s of %s [%c].\n",
        existing_person1->name_, gender_person1, identifier,
        existing_person2->name_, gender_person2);
  }
  return NORMAL;
}


//----------------------------------------------------------------------------
//
//  Function that implements the list command 
//
//  @param console_input -> input from the user
//  @param all_persons -> list of persons that should be listed
//  
//  @return MEMORY_EXCEPTION on failure to allocate memory or ERROR if any
//  of the defined errors occured, or NORMAL on success
//

int listAllPersons(char* console_input, struct _PersonList_* all_persons)
{
  if(console_input != NULL)
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
      return MEMORY_EXCEPTION;
    }
    sprintf(line , "%s [%c]", current_Person->name_, gender);
    addLineToLineBuffer(lines_buffer, line);
  }

  //sort lines_buffer
  qsort(lines_buffer->data_, lines_buffer->position_,
      sizeof(char*), linesComparisonFunction);
  
  for(index = 0; index < lines_buffer->position_; index++)
  {
    printf("%s\n", lines_buffer->data_[index]);
  }

  freeLineBuffer(&lines_buffer);
  return NORMAL;
}


//----------------------------------------------------------------------------
//
//  Function that creates a LineBuffer from a person_list which contains the
//  person_list in dot format.
//
//  @param lines_buffer -> the buffer within which lines should be stored 
//  @param person_list -> the list of persons used to create the buffer
//   
//  @return MEMORY_EXCEPTION on failure to allocate memory or NORMAL on
//  success
//

int parsePersonListIntoLineBuffer(struct _LineBuffer_* lines_buffer,
    struct _PersonList_* person_list)
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
        return MEMORY_EXCEPTION;
      }

      sprintf(line, "  \"%s [%c]\" -> \"%s [%c]\";",
          current_Person->name_, person_gender, parent->name_, parent_gender);
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
        return MEMORY_EXCEPTION;
      }

      sprintf(line, "  \"%s [%c]\" -> \"%s [%c]\";",
          current_Person->name_, person_gender, parent->name_, parent_gender);
      addLineToLineBuffer(lines_buffer, line);
    }
  }
  return NORMAL;
}


//----------------------------------------------------------------------------
//
//  Function that opens a file for reading
//
//  @param file_pointer -> file pointer that is being assigned in the Function 
//  @param filename -> name of the file to open 
//
//  @return ERROR if file cannot be opened,  NORMAL otherwise
//

int openDotFileForReading(FILE** file_pointer, char* filename)
{
  *file_pointer = fopen(filename, "r");
  if(*file_pointer == NULL)
  {
    return ERROR;
  }
  return NORMAL;
}


//----------------------------------------------------------------------------
//
//  Function that creates/overwrites a dot file for writing. (automatically 
//  appends .dot to the filename)
//
//  @param file_pointer -> file pointer that is being assigned in the Function
//  @param filename -> name of the file to create/overwrite
//
//  @return ERROR if file cannot be created/overwritten, NORMAL otherwise
//

int openDotFileForWriting(FILE** file_pointer, char* filename)
{
  int filename_length = strlen(filename) + 4 + 1;
  char* filename_with_extension = malloc(filename_length * sizeof(char));

  sprintf(filename_with_extension, "%s.dot", filename);
  *file_pointer = fopen(filename_with_extension, "w");
  free(filename_with_extension);

  if(*file_pointer == NULL)
  {
    return ERROR;
  }
  return NORMAL;
}

//----------------------------------------------------------------------------
//
//  Function that implements the draw-all command.
//  Filename is parsed out of user arguments, and used to create new dot file
//  with all person in it.
//
//  @param arguments -> user supplied arguments
//  @param all_persons -> list of all persons
//  
//  @return MEMORY_EXCEPTION on failure to allocate memory, ERROR if one
//  of the defined errors is triggered, or NORMAL on success
//

int drawAllPersonsToFile(char* arguments, struct _PersonList_* all_persons)
{
  if(arguments == NULL)
  {
    printError(DRAW_ALL_USAGE);
    return ERROR;
  }

  stripWhitespace(arguments);
  char* last_char_of_arguments = arguments + strlen(arguments);
  char* input_filename = strtok(arguments, " ");

  if(input_filename == NULL ||
    input_filename + strlen(input_filename) != last_char_of_arguments)
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
      printError(DRAW_ALL_COULD_NOT_WRITE_FILE); 
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
  qsort(lines_buffer->data_, lines_buffer->position_,
      sizeof(char*), drawAllLinesComparisonFunction);

  //write lines_buffer to output file
  fprintf(output_file, "digraph FamilyTree\n"); 
  fprintf(output_file, "{\n");

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

int drawPersonsFromRootToFile(char* arguments,
    struct _PersonList_* all_persons)
{
  if(arguments == NULL)
  {
    printError(DRAW_USAGE);
    return ERROR;
  }

  stripWhitespace(arguments);
  char* position = arguments;
  char* last_char_of_arguments = arguments + strlen(arguments); 


  struct _Person_* person;
  int status = initializePerson(&person);
  if(status != NORMAL)
  {
    return status;
  }
  
  status = parsePerson(&position, &person);
  if(status != NORMAL)
  {
    if(status == ERROR)
    {
      printError(DRAW_USAGE);
    }
    return status;
  }
  
  char* input_filename = strtok(position, " ");
  if(input_filename == NULL ||
      input_filename + strlen(input_filename) != last_char_of_arguments)
  {
    freePerson(&person);
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
  qsort(lines_buffer->data_, lines_buffer->position_,
      sizeof(char*), drawAllLinesComparisonFunction);

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
  freePersonList(&ancestor_list);
  fclose(output_file);
  return NORMAL;
}



//----------------------------------------------------------------------------
//
//  Function that parses a line from a dot_file. It extracts and initializes
//  the persons in the line. It returns an error if both persons are the
//  same person.
//
//  @param input_string -> one line of the file that should be parsed
//  @param peopleList -> struct to store persons in the line
//  @param num_people -> number of people that should be in the line
//
//  @return MEMORY_EXCEPTION on failure to allocate memory, ERROR, if a 
//  parsing error is encountered, or NORMAL on success
//

int parsePersonsFromInputFileLine(char* input_string,
    struct _PersonList_* people_list, int num_people)
{
  int index = 0;
  int entries_made = 0;
  bool in_parenthesis = false;  
  int length_ = strlen(input_string);
  int number_of_parsed_persons = 0;
  struct _LineBuffer_* person_string_buffer;
  char* begin_of_person_string;
  int status = initializeLineBuffer(&person_string_buffer, false);
  if(status != NORMAL)
  {
    return MEMORY_EXCEPTION;
  }

  for(index = 0; index < length_; index++)
  {
    if(input_string[index] == '\"')
    {
      in_parenthesis = !in_parenthesis;
      if(in_parenthesis && entries_made < num_people)
      {
        begin_of_person_string = input_string + index + 1;
      }
      if(!in_parenthesis)
      {
        addLineToLineBuffer(person_string_buffer, begin_of_person_string);
        if(status != NORMAL)
        {
          return status;
        }
        number_of_parsed_persons++;
        if(number_of_parsed_persons >= num_people)
        {
          break;
        }
      }
    }
  }

  if(in_parenthesis || number_of_parsed_persons < num_people)
  {
    freeLineBuffer(&person_string_buffer);
    return ERROR;
  }

  char* person1_position = person_string_buffer->data_[0];
  char* person2_position;
  if(num_people == 2)
  {
    person2_position = person_string_buffer->data_[1];
  }
  freeLineBuffer(&person_string_buffer);

  struct _Person_* person1;
  struct _Person_* person2;
  
  status = initializePerson(&person1);
  if(status != NORMAL)
  {
    return status;
  }
  status = parsePerson(&person1_position, &person1);
  if(status != NORMAL)
  {
    return status;
  }
  status = addPersonToList(&person1, people_list);
  if(status != NORMAL)
  {
    return status;
  }

  if(num_people == 2)
  {
    status = initializePerson(&person2);
    if(status != NORMAL)
    {
      return status;
    }
    status = parsePerson(&person2_position, &person2);
    if(status != NORMAL)
    {
      return status;
    }
    status = addPersonToList(&person2, people_list);
    if(status != NORMAL)
    {
      return status;
    }
    if(person1 == person2)
    {
      return ERROR;
    }
  }
  
  if(num_people == 1)
  {
    if(strstr(input_string, "  \"") != input_string || 
       strstr(input_string, "\";") != person1_position)
    {
      return ERROR;
    }
  }
  if(num_people == 2)
  {
    if(strstr(input_string, "  \"") != input_string || 
       strstr(input_string, "\" -> \"") != person1_position || 
       strstr(input_string, "\";") != person2_position)
    {
      return ERROR;
    }
  }
  return NORMAL;
}


//----------------------------------------------------------------------------
//
//  Function that extracts the lines of a file and stores them in a line
//  buffer
//
//  @param filename -> filename of the file from which lines should be 
//  extracted
//  @param lines -> struct that contains lines of file, when function
//  finishes
//  
//  @return NORMAL if everything worked, MEMORY_EXCEPTION on failure to
//  allocate memory
//

int parseLinesFromFile(char *filename, struct _LineBuffer_** lines, 
    struct _CharacterBuffer_** character_buffer)
{
  FILE* dot_file;
  int status = openDotFileForReading(&dot_file, filename);
  if(status == ERROR)
  {
    return FILE_UNREADABLE_EXCEPTION;
  }
  
  //initialize lines and character buffer
  status = initializeLineBuffer(lines, false);
  if(status != NORMAL)
  {
    return status;
  }

  status = initializeCharacterBuffer(character_buffer);
  if(status != NORMAL)
  {
    return status;
  }

  //read file into character buffer
  char input_char = '\0';

  while((input_char = fgetc(dot_file)) != EOF)
  {
    writeCharacterToCharacterBuffer(*character_buffer, input_char);
  }

  writeCharacterToCharacterBuffer(*character_buffer, '\0');

  //separate lines in character buffer and write them to lines buffer
  char* line_pointer = strtok((*character_buffer)->data_, "\n");
  if(line_pointer == NULL)
  {
    return FILE_UNREADABLE_EXCEPTION;
  } 

  status = addLineToLineBuffer(*lines, line_pointer);
  if(status != NORMAL)
  {
    return status;
  }

  while((line_pointer = strtok(NULL, "\n")) != NULL)
  {
    status = addLineToLineBuffer(*lines, line_pointer);
    if(status != NORMAL)
    {
      return status;
    }
  } 

  fclose(dot_file);

  return status;
}

//----------------------------------------------------------------------------
//
//  Function that parses persons from a line of a dot file and adds them to 
//  a person list
//
//  @param line -> line to parse persons from 
//  
//  @return FILE_UNREADABLE_EXCEPTION on failure to interpretate line,
//  MEMORY_EXCEPTION on allocation failure, otherwise NORMAL
//

int parseDotFileLine(char* line, struct _PersonList_* all_persons)
{
  if(strstr(line, " -> ") != NULL)
  {
    //parse two persons from the file in this if
    //create a person list to store persons
    struct _PersonList_* input_persons;
    int status = initializePersonList(&input_persons, true);
    if(status == ERROR)
    {
      return status;
    }

    //parse line
    status = parsePersonsFromInputFileLine(line, input_persons, 2);
    if(status != NORMAL)
    {
      return FILE_UNREADABLE_EXCEPTION;
    }
   
    //initialize two persons and copy the persons from dot file into them
    struct _Person_* person1;
    status = initializePerson(&person1);
    if(status != NORMAL)
    {
      return status;
    }

    status = copyPersonIntoAnotherPerson(person1, input_persons->list_[0]);
    if(status != NORMAL)
    {
      return status;
    }

    status = addPersonToList(&person1, all_persons);
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

    status = copyPersonIntoAnotherPerson(person2, input_persons->list_[1]);
    if(status != NORMAL)
    {
      return status;
    }

    status = addPersonToList(&person2, all_persons);
    if(status != NORMAL)
    {
      return status;
    }

    //determine their relationship
    if(person2->gender_ == MALE)
    {
      if(person1->father_ != NULL)
      {
        return FILE_UNREADABLE_EXCEPTION;
      }
      person1->father_ = person2;
    }
    else
    {
      if(person1->father_ != NULL)
      {
        return FILE_UNREADABLE_EXCEPTION;
      }
      person1->mother_ = person2;
    }

    //detect if any circles are created by adding them
    if(detectCircles(person1))
    {
      return FILE_UNREADABLE_EXCEPTION;
    }

    freePersonList(&input_persons);
  }
  else
  {
    //parse one person from file
    //initialize person list
    struct _PersonList_* input_persons;
    int status = initializePersonList(&input_persons, true);
    if(status == ERROR)
    {
      return status;
    }
      
    //parse persons from input file line
    status = parsePersonsFromInputFileLine(line, input_persons, 1);
    if(status == ERROR)
    {
      return FILE_UNREADABLE_EXCEPTION;
    }

    //initialize one person
    struct _Person_* person1;
    status = initializePerson(&person1);
    if(status != NORMAL)
    {
      return status;
    }

    //copy person from line into person1
    status = copyPersonIntoAnotherPerson(person1, input_persons->list_[0]);
    if(status != NORMAL)
    {
      return status;
    }

    //add the person to the list
    status = addPersonToList(&person1, all_persons);
    if(status != NORMAL)
    {
      return status;
    }
    freePersonList(&input_persons);
  }
  return NORMAL;
}

//----------------------------------------------------------------------------
//
//  Function that parses dot file and adds persons in it to the list of
//  all persons
//
//  @param dot_inputfile_name -> name of the file
//  
//

int parseDotFile(char* dot_inputfile_name, struct _PersonList_* all_persons)
{
  struct _CharacterBuffer_* character_buffer = NULL;
  struct _LineBuffer_* lines = NULL;
  int status = parseLinesFromFile(dot_inputfile_name,
      &lines, &character_buffer);
  if(status != NORMAL)
  {
    return status;
  }
  //check overall integrity of dot file
  if(strcmp("digraph FamilyTree", lines->data_[0]) != 0)
  {
    return FILE_UNREADABLE_EXCEPTION;
  }

  if(lines->data_[1][0] != '{')
  {
    return FILE_UNREADABLE_EXCEPTION;
  }

  if(lines->data_[lines->position_ - 1][0] != '}')
  {
    return FILE_UNREADABLE_EXCEPTION;
  }

  //parse each line of data in the dot file
  int line = 0;
  for(line = 2; line < lines->position_ - 1; line++)
  {
    char* currentLine = lines->data_[line];
    status = parseDotFileLine(currentLine, all_persons);
    if(status != NORMAL)
    {
      return status;
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
// @param command_buffer -> buffer to write input to
// @param command -> pointer into command buffer to the start of command
// @param arguments -> pointer into command buffer to start of arguments
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
  unsigned int input_buffer_length_ = 10;
  
  input_character = getchar();
  while(input_character != '\n')
  {
    input_buffer[input_buffer_position] = input_character;
    input_buffer_position++;
    if(input_buffer_position >= input_buffer_length_)
    {
      input_buffer_length_ += 10;
      input_buffer = (char*) realloc(input_buffer,
          input_buffer_length_* sizeof(char));
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
  
  if(*command_buffer + total_length_of_command_buffer > 
      position_in_command_buffer)
  {
    *arguments = *command_buffer + strlen(*command) + 1;
  }
}

//-----------------------------------------------------------------------------
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

  while(return_status <= 0)
  {
    command = NULL;
    arguments = NULL;
    commandPrompt(&command_buffer, &command, &arguments);

    if(command == NULL)
    {
      free(command_buffer);
      continue;
    }

    if(strcmp(command, "quit") == 0)
    {
      printf(CLOSE_MESSAGE);
      free(command_buffer);
      return_status = NORMAL;
      break;
    }
    else if(strcmp(command, "EOF") == 0)
    {
      free(command_buffer);
      return_status = NORMAL;
      break;
    }
    else if(strcmp(command, "add") == 0)
    {
      return_status = addPersonsWithRelationCommand(arguments,
          all_persons, &question_mark_person_counter);
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

  if(return_status == MEMORY_EXCEPTION)
  {
    printError(OUT_OF_MEMORY);
  }
  if(return_status == FILE_UNREADABLE_EXCEPTION)
  {
    printError(COULD_NOT_READ_FILE);
  }

  return return_status;
}
