/*kell1426
*04/11/18
*Daniel Kelly
*4718021*/

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

// Structure for every node
typedef struct node{
	char name[1024];
	char textFileName[1024];
	char inputFileLocation[1024];
	char outputFileLocation[1024];
	char outputDirectoryPath[1024];
	char logFile[1024];
	int isLeafNode;
	int num_children;
	char childName[50][1024];
	char parentName[1024];
	sem_t* nodeSem;
	struct node *parent;
	int id;
	int parentid;
}node_t;

typedef struct list{
	char fileName[1024];
	char fileLocation[1024];
	struct list *next;
} list_t;

struct threadArgs{
	list_t* head;
	node_t* n;
	//char **Candidates;
	//int CandidatesVotes[MAX_CANDIDATES]
};

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

node_t* findnode(node_t* start, char* tobefound){
	//Find the node in question
		node_t* temp = start;
		do {
			if( (strcmp(temp->name, tobefound)==0)){
				return temp;
			}

			temp++;
		} while(temp->name!=NULL);
		return NULL;
}

void printgraph(node_t* n)
{
	int i = 0;
	while(n[i].name[0] != '\0')
	{
		printf("Node name is: %s\n", n[i].name);
		printf("Node textfile name is: %s\n", n[i].textFileName);
		printf("Node output directory path is: %s\n", n[i].outputDirectoryPath);
		printf("Node output file path is: %s\n", n[i].outputFileLocation);
		if(n[i].isLeafNode == 1)
		{
			printf("Node is a leaf node. Input file path is: %s\n", n[i].inputFileLocation);
		}
		printf("Node's parent name is: %s\n", n[i].parentName);
		printf("Node has %d children\n", n[i].num_children);
		int j;
		for(j = 0; j < n[i].num_children; j++)
		{
			printf("Child %d is: %s\n", j, n[i].childName[j]);
		}
		i++;
	}
}

void printList(list_t *head)
{
	list_t *current = head->next;
	while(current != NULL)
	{
		printf("Leaf Node input file name is: %s\n", current->fileName);
		printf("Leaf Node input file location is: %s\n", current->fileLocation);
		current = current->next;
	}
}
