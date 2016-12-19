#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <ctype.h> //for isspace

#define CLOSE_MESSAGE "bye\n"
#define OUT_OF_MEMORY "[ERR] Out of memory.\n"
#define COULD_NOT_READ_FILE "[ERR] Could not read file.\n"
#define USAGE_EXCEPTION "Usage: ./ass [file-name_]\n"
//add command
#define ADD_PERSONS_USAGE "[ERR] Wrong usage - add <namePerson1> [m/f] <relation> <namePerson2> [m/f].\n"
#define SEX_DOES_NOT_MATCH "[ERR] Sex does not match with relation.\n"
#define BOTH_PEOPLE_ARE_SAME "[ERR] Both people are the same.\n"
#define RELATION_NOT_POSSIBLE "[ERR] Relation not possible.\n"
//relationship command
#define RELATIONSHIP_USAGE "[ERR] Wrong usage - relationship <namePerson1> [m/f] <namePerson2> [m/f].\n"
#define INEXISTANT_PERSONS "[ERR] At least one person does not exist yet.\n"
//#define BOTH_PEOPLE_ARE_SAME "[ERR] Both people are the same.\n" //already defined
#define NO_RELATIONSHIP "There is no relationship between them.\n"
#define RELATIONSHIP_EXISTANT "The two people are related.\n"

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

int stripWhitespace(char* string)
{
  int index = 0;
  printf("string was: [%s]", string);
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
  printf("string is now: [%s]", string);
}

//add command

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

  person->name_ = malloc((string_position - *person_string + 1) * sizeof(char));
  if(person->name_ == NULL)
  {
    printError(OUT_OF_MEMORY);
    return MEMORY_EXCEPTION;
  }
  person->name_[string_position - *person_string] = '\0';
  strncpy(person->name_, *person_string, string_position - *person_string);
  stripWhitespace(person->name_);
  person->gender_ = string_position[1] == 'm' ? MALE : FEMALE;
  person->mother_ = NULL;
  person->father_ = NULL;
  string_position += 3;
  *person_string = string_position;
  return NORMAL;
}

void movePersonIntoAnotherPerson(struct _Person_* destination, struct _Person_* source)
{
  free(destination->name_);
  destination->name_ = source->name_;
  destination->gender_ = source->gender_;
  destination->mother_ = source->mother_;
  destination->father_ = source->father_;
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

int removePersonFromList(struct _PersonList_* list_of_persons, struct _Person_* person)
{
  int index = 0;
  int position_of_person = -1;

  for(index = 0; index < list_of_persons->length_; index++)
  {
    if(list_of_persons->list_[index] == person)
    {
      //save the position of the freed person
      int position_of_person = index;
      //free the person
      free(person->name_);
      free(person);
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

int createQuestionMarkPerson(int* question_mark_person_counter, struct _Person_** pointer_to_person)
{
  struct _Person_* question_mark_person = malloc(sizeof(struct _Person_));
  if(question_mark_person == NULL)
  {
    printError(OUT_OF_MEMORY);
    return MEMORY_EXCEPTION;
  }
  question_mark_person->name_ = malloc(sizeof(char) * 10);
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

int addPersonsWithRelationCommand(char* console_input, struct _PersonList_* all_persons, int* question_mark_person_counter)
{
  //store position of \0 byte at the end of the string
  char* position_of_last_char = console_input + strlen(console_input);
  struct _Person_* person1 = malloc(sizeof(struct _Person_));
  struct _Person_* person2 = malloc(sizeof(struct _Person_));
  char* position = console_input;
  int status = parsePerson(&position, person1);
  if(status != NORMAL)
  {
    if(status == ERROR)
    {
      printError(ADD_PERSONS_USAGE);
    }
    return status;
  }

  //skip whitespace in string
  status = skipWhitespace(&position);
  if(status != NORMAL)
  {
    printError(ADD_PERSONS_USAGE);
    return status;
  }

  char* relation_input = strtok(position, " ");
  if(relation_input == NULL)
  {
    printError(ADD_PERSONS_USAGE);
    return ERROR;
  }

  position += strlen(relation_input) + 1;

  //skip whitespace in string
  status = skipWhitespace(&position);
  if(status != NORMAL)
  {
    printf("encountered error while skipping whitespace");
    printError(ADD_PERSONS_USAGE);
    return status;
  }

  if(position >= position_of_last_char)
  {
    printError(ADD_PERSONS_USAGE);
    return ERROR;
  }
  status = parsePerson(&position, person2);
  if(status != NORMAL)
  {
    if(status == ERROR)
    {
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
    printError(ADD_PERSONS_USAGE);
    return ERROR;
  }

  printf("person1 gender is %d\n", person1->gender_);
  printf("person2 gender is %d\n", person2->gender_);
  if(((relation == RELATION_MOTHER ||
     relation == RELATION_MOTHER_GRANDMOTHER ||
     relation == RELATION_MOTHER_GRANDFATHER) &&
     person1->gender_ == MALE) ||
     ((relation == RELATION_FATHER ||
     relation == RELATION_FATHER_GRANDMOTHER ||
     relation == RELATION_FATHER_GRANDFATHER) &&
     person1->gender_ == FEMALE))
  {
    printError(SEX_DOES_NOT_MATCH);
    return ERROR;
  }

  if(strcmp(person1->name_, person2->name_) == 0 &&
    person1->gender_ == person2->gender_)
  {
    printError(BOTH_PEOPLE_ARE_SAME);
    return ERROR;
  }
  
  status = addPersonToList(&person1, all_persons);
  status = addPersonToList(&person2, all_persons);
  
  struct _Person_** parent;
  struct _Person_** grandparent;

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
    printf("person2->father %p, person2->mother %p, parent %p", person2->father_, person2->mother_, *parent);

    if(*parent == NULL)
    {
      *parent = person1;
    }
    else if((*parent)->name_[0] == '?' &&
        (*parent)->mother_ == person1->mother_ &&
        (*parent)->father_ == person1->father_)
    {
      movePersonIntoAnotherPerson(*parent, person1);
      removePersonFromList(all_persons, person1);
    }
    else
    {
      printError(RELATION_NOT_POSSIBLE);
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
      status = createQuestionMarkPerson(question_mark_person_counter, parent);
      if(status != NORMAL)
      {
        return status; 
      }
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
      movePersonIntoAnotherPerson(*grandparent, person1);
      removePersonFromList(all_persons, person1);
    }
    else
    {
      //grandparent is already defined
      printError(RELATION_NOT_POSSIBLE);
      return ERROR;
    }

  }

}

//relation

int getExistingPerson(struct _Person_* person_to_get, struct _PersonList_ person_list)
{
  int index = 0;
  for(index = 0; index < person_list.length_; index++)
  {
    if(strcmp(person_to_get->name_, person_list.list_[index]->name_) == 0 &&
        person_to_get->gender_ == person_list.list_[index]->gender_)
    {
      *person_to_get = *(person_list.list_[index]);
      return NORMAL;
    }
  }
  return ERROR;
}

int createAncestorList(struct _Person_* person, struct _PersonList_* person_list)
{
  int status = NORMAL;
  if(person->father_ != NULL)
  {
    person_list->length_++;
    person_list->list_ = realloc(person_list->list_, person_list->length_ * sizeof(struct _Person_*));
    if(person_list->list_ == NULL)
    {
      printError(OUT_OF_MEMORY);
      return MEMORY_EXCEPTION;
    }
    person_list->list_[person_list->length_ - 1] = person->father_;
    status = createAncestorList(person->father_, person_list);
    if(status != NORMAL)
    {
      return status;
    }
  }
  if(person->mother_ != NULL)
  {
    person_list->length_++;
    person_list->list_ = realloc(person_list->list_, person_list->length_ * sizeof(struct _Person_*));
    if(person_list->list_ == NULL)
    {
      printError(OUT_OF_MEMORY);
      return MEMORY_EXCEPTION;
    }
    person_list->list_[person_list->length_ - 1] = person->mother_;
    status = createAncestorList(person->mother_, person_list);
    if(status != NORMAL)
    {
      return status;
    }
  }
  return status;
}

int checkIfPeopleAreRelated(char* console_input, struct _PersonList_* all_persons)
{
  
  struct _Person_* person1 = malloc(sizeof(struct _Person_));
  struct _Person_* person2 = malloc(sizeof(struct _Person_));
  char* position = console_input;
  int status = parsePerson(&position, person1);
  if(status != NORMAL)
  {
    if(status == ERROR)
    {
      printError(RELATIONSHIP_USAGE); 
    }
    return status;
  }
  status = parsePerson(&position, person2);
  if(status != NORMAL)
  {
    if(status == ERROR)
    {
      printError(RELATIONSHIP_USAGE);
    }
    return status;
  }

  status = getExistingPerson(person1, *all_persons);
  if(status != NORMAL)
  {
    if(status == ERROR)
    {
      printError(INEXISTANT_PERSONS);
    }
    return status;
  }

  status = getExistingPerson(person2, *all_persons);
  if(status != NORMAL)
  {
    if(status == ERROR)
    {
      printError(INEXISTANT_PERSONS);
    }
    return status;
  }

  if(strcmp(person1->name_, person2->name_) == 0 &&
    person1->gender_ == person2->gender_)
  {
    printError(BOTH_PEOPLE_ARE_SAME);
    return ERROR;
  }
  
  //create ancestor lists of both persons
  struct _PersonList_* ancestors_of_person1 = malloc(sizeof(struct _PersonList_));
  ancestors_of_person1->length_= 0;
  ancestors_of_person1->list_= malloc(sizeof(struct _Person_*));

  struct _PersonList_* ancestors_of_person2 = malloc(sizeof(struct _PersonList_));
  ancestors_of_person2->length_= 0;
  ancestors_of_person2->list_= malloc(sizeof(struct _Person_*));

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
  }
  if(people_are_related)
  {
    printf("%s", RELATIONSHIP_EXISTANT);
  }
  else
  {
    printf("%s", NO_RELATIONSHIP);
  }

  //sister

  enum _Relationship_ relationship = NO_RELATION;

  if(person1->gender_ == MALE)
  {
    if(person1->father_!= NULL && person1->mother_!= NULL &&
       person2->father_!= NULL && person2->mother_!= NULL &&
      person1->father_ == person2->father_ &&
      person1->mother_ == person2->mother_)
    {
      relationship = BROTHER;
    }
    else if(person2->father_ != NULL && person2->father_ == person1)
    {
      relationship = FATHER; } else if( person1->father_ != NULL && person1->mother_ != NULL &&
           ((person2->father_ != NULL && person2->father_->father_ != NULL &&
            person2->father_->mother_ != NULL &&
            person2->father_->father_ == person1->father_ &&
            person2->father_->mother_ == person1->mother_) ||
           (person2->mother_ != NULL && person2->mother_->father_ != NULL &&
            person2->mother_->mother_ != NULL &&
            person2->mother_->father_ == person1->father_ &&
            person2->mother_->mother_ == person1->mother_)))
             
    {
      relationship = UNCLE;
    }
    else if(person2->father_ != NULL && person2->father_->father_ != NULL &&
            person2->father_->father_ == person1)
    {
      relationship = GRANDFATHER;  
    }
  }
  else
  {
    if(person1->father_!= NULL && person1->mother_!= NULL &&
       person2->father_!= NULL && person2->mother_!= NULL &&
      person1->father_ == person2->father_ &&
      person1->mother_ == person2->mother_)
    {
      relationship = SISTER;
    }
    else if(person2->father_ != NULL && person2->father_ == person1)
    {
      relationship = MOTHER;
    }
    else if( person1->father_ != NULL && person1->mother_ != NULL &&
           ((person2->father_ != NULL && person2->father_->father_ != NULL &&
            person2->father_->mother_ != NULL &&
            person2->father_->father_ == person1->father_ &&
            person2->father_->mother_ == person1->mother_) ||
           (person2->mother_ != NULL && person2->mother_->father_ != NULL &&
            person2->mother_->mother_ != NULL &&
            person2->mother_->father_ == person1->father_ &&
            person2->mother_->mother_ == person1->mother_)))
             
    {
      relationship = AUNT;
    }
    else if(person2->father_ != NULL && person2->father_->father_ != NULL &&
            person2->father_->father_ == person1)
    {
      relationship = GRANDMOTHER;  
    }
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
    char gender_person1 = person1->gender_ == MALE ? 'm' : 'f';
    char gender_person2 = person2->gender_ == MALE ? 'm' : 'f';
    printf("%s [%c] is the %s of %s [%c].\n", person1->name_, gender_person1, identifier, person2->name_, gender_person2);
  }
  return NORMAL;
}

int parsePersonStringsFromInputFileLine(char* input_string, char** peopleList, int num_people)
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
    //reached end of string while in parenthesis
    return ERROR;
  }
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

int parseDotFile(char* dot_inputfile_name, struct _PersonList_* all_persons){
  FILE* dot_file = fopen(dot_inputfile_name, "r");
  if(dot_file == NULL)
  {
    printError(COULD_NOT_READ_FILE); 
    return FILE_UNREADABLE_EXCEPTION;
  }

  int lines_size = 10;
  int lines_position = 0;
  char** lines = (char**) malloc(sizeof(char**) * lines_size);
  if(lines == NULL)
  {
    printError(OUT_OF_MEMORY);
    return MEMORY_EXCEPTION;
  }
  
  int buffer_size = 10;
  int buffer_position = 0;
  char* buffer = (char*) malloc(sizeof(char) * buffer_size);
  if(buffer == NULL)
  {
    printError(OUT_OF_MEMORY);
    return MEMORY_EXCEPTION;
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
        printError(OUT_OF_MEMORY); 
        return MEMORY_EXCEPTION;
      }
    }
  }

  buffer[buffer_position] = '\0';

  char* line_pointer = strtok(buffer, "\n");
  if(line_pointer == NULL)
  {
    printError(COULD_NOT_READ_FILE);
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
        printError(OUT_OF_MEMORY);
        return MEMORY_EXCEPTION;
      }
    }
  } 

  lines_position--;

  if(strcmp("digraph FamilyTree", lines[0]) != 0)
  {
    printf("line is %s\n", lines[0]);
    debug("digraph familytree did not match");
    printError(COULD_NOT_READ_FILE);
    return FILE_UNREADABLE_EXCEPTION;
  }

  if(lines[1][0] != '{')
  {
    debug("{ in first line not found");
    printf("line is %s\n", lines[0]);
    printError(COULD_NOT_READ_FILE);
    return FILE_UNREADABLE_EXCEPTION;
  }

  if(lines[lines_position][0] != '}')
  {
    printf("line is [%s]\n", lines[lines_position]);
    debug("} in last line not found");
    printError(COULD_NOT_READ_FILE);
    return FILE_UNREADABLE_EXCEPTION;
  }

  int line = 0;
  for(line=2; line < lines_position; line++)
  {
    
    if(strstr(lines[line], "->") != NULL)
    {
      char** peopleList = malloc(2 * sizeof(char**));     
      int status = parsePersonStringsFromInputFileLine(lines[line], peopleList, 2);
      if(status == ERROR)
      {
        printError(COULD_NOT_READ_FILE);
        return FILE_UNREADABLE_EXCEPTION;
      }

      struct _Person_* person1 = malloc(sizeof(struct _Person_));
      struct _Person_* person2 = malloc(sizeof(struct _Person_));
      parsePerson(&(peopleList[0]), person1);
      parsePerson(&(peopleList[1]), person2);
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

      if(person1->gender_ == MALE){
        person2->father_ = person1;
        printf("added father [%s] to person [%s]", person1->name_, person2->name_);
        if(strcmp(person1->name_, "Arcturus ") == 0)
        {
          printf("mother and father of Arcturus are %p %p\n", person1->mother_, person1->father_);
        }
      }
      else{
        person2->mother_ = person1;
        printf("added mother %s to person %s", person1->name_, person2->name_);
      }
      free(peopleList);
    }
    else
    {
      
      char** peopleList = malloc(sizeof(char**));
      int status = parsePersonStringsFromInputFileLine(lines[line], peopleList, 1);
      if(status == ERROR)
      {
        printError(COULD_NOT_READ_FILE);
        return FILE_UNREADABLE_EXCEPTION;
      }
      struct _Person_* person1 = malloc(sizeof(struct _Person_));
      parsePerson(&(peopleList[0]), person1);
      status = addPersonToList(&person1, all_persons);
      if(status != NORMAL)
      {
        return status;
      }
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
  //set last byte in buffer to zero
  input_buffer[input_buffer_position] = '\0';
  *command_buffer = input_buffer;

  int total_length_of_command_buffer = strlen(*command_buffer);
  char* position_in_command_buffer = *command_buffer;
  //set arguments and command
  skipWhitespace(&position_in_command_buffer);
  *command = strtok(position_in_command_buffer, " ");

  position_in_command_buffer += strlen(*command) + 1;

  
  if(*command_buffer + total_length_of_command_buffer  > position_in_command_buffer){
    *arguments = *command_buffer + strlen(*command) + 1;
  }
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

  //initialize person list
  struct _PersonList_* all_persons = malloc(sizeof(struct _PersonList_));
  all_persons->length_= 0;
  all_persons->list_= malloc(sizeof(struct _Person_*));

  int question_mark_person_counter = 1;

  int return_status = 0;
  if(argc > 1)
  {
    char* dot_inputfile_name = argv[1]; 
    return_status = parseDotFile(dot_inputfile_name, all_persons);
  }

  while(return_status >= 0)
  {
    command = NULL;
    arguments = NULL;
    commandPrompt(&command_buffer, &command, &arguments);

    if(command == NULL)
    {
      continue;
    }

    printf("command entered was %s\n", command);
    printf("arguments entered were %s\n", arguments);
    if(strcmp(command, "quit") == 0)
    {
      printf(CLOSE_MESSAGE);
      exit(0);
    }
    else if(strcmp(command, "add") == 0)
    {
      printf("in add command\n");
      return_status = addPersonsWithRelationCommand(arguments, all_persons, &question_mark_person_counter);
    }
    else if(strcmp(command, "relationship") == 0)
    {
      printf("in relationship command\n");
      return_status = checkIfPeopleAreRelated(arguments, all_persons);
    }
    else
    {
      printf("cannot understand command %s\n", command);
    } 
    free(command_buffer); 
  } 
  free(all_persons->list_);
  free(all_persons);
  return return_status;
}
