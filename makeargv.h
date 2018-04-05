#include <errno.h>
#include <stdlib.h>
#include <string.h>
// Structure for every node
typedef struct node{
	char name[1024];
	char textFileName[1024];
	char inputFileLocation[1024];
	char outputFileLocation[1024];
	char outputDirectoryPath[1024];
	char
	int num_children;
	char childName[50][1024];
	char parentName[1024];
}node_t;

int makeargv(const char*s, const char *delimiters, char ***argvp){

	int error;
	int i;
	int numtokens;
	const char *snew;
	char *t;

	if ((s == NULL) || (delimiters == NULL) || (argvp == NULL)){

		errno = EINVAL;
		return -1;

	}

	*argvp = NULL; // already assigned as a new var, just blanking out

	snew = s + strspn(s, delimiters);

	if ((t = malloc(strlen(snew) + 1)) == NULL)
		return -1;

	strcpy(t, snew);

	numtokens = 0;

	if (strtok(t, delimiters) != NULL) // count number of tokens in s
		for (numtokens = 1; strtok(NULL, delimiters) != NULL; numtokens++);

	// create arg array for pointers to tokens
	if ((*argvp = malloc((numtokens + 1)*sizeof(char *))) == NULL){
		error = errno;
		free(t);
		errno = error;
		return -1;
	}

	// insert pointers to tokens into the arg array
	if (numtokens == 0)
		free(t);

	else{
		strcpy(t, snew);
		**argvp = strtok(t, delimiters);
		for(i = 1; i < numtokens; i++)
			*((*argvp) + i) = strtok(NULL, delimiters);
	}

	*((*argvp) + numtokens) = NULL; // put in final NULL pointer

	return numtokens;
}

char *trimwhitespace(char *str)
{
  char *end;
  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;

  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
}

node_t* findnode(node_t* start, char* tobefound){
	//Find the node in question
		node_t* temp = start;
		do {
			if( (strcmp(temp->name, tobefound)==0)){
				return temp;
			}

			temp++;
		} while(temp->id!=NULL);
		return NULL;
}

}
