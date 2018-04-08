/*kell1426
*02/21/18
*Daniel Kelly
*4718021*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include "makeargv.h"
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/syscall.h>

#define MAX_NODES 100
#define MAX_CANDIDATES 100

pthread_mutex_t* listLock;
sem_t* listSem;
sem_t* memSem;

void inputDirectoryCreator(node_t* n, char *inputDirectory);
void outputDirectoryCreator(node_t* n, char *outputDirectory);
void DAGCreator(node_t* n, char *filename);
int initializeQueue(list_t* head, node_t* n);
// int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf);
// int rmrf(char *path);
void parseInputLine(char *buf, node_t* n, int line);
void threadFunction(void* arg);
list_t* dequeue(list_t* head);
char* decrypt(node_t* leafNode);
void leafCounter(char* leafFile, char** Candidates, int CandidatesVotes[MAX_CANDIDATES]);
void aggregateVotes(node_t* node, node_t* root, char **Candidates, int CandidatesVotes[MAX_CANDIDATES]);


// int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
// {
//     int rv = remove(fpath);
//
//     if (rv)
//         perror(fpath);
//
//     return rv;
// }
//
// int rmrf(char *path)
// {
//     return nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
// }

void parseInputLine(char *buf, node_t* n, int line)
{
  char **strings;
  int numOfTokens = makeargv(buf, ":", &strings);
  if(line == 1) //First read_line
  {
    int i;
    for(i = 0; i < numOfTokens; i++)
    {
      //n[i].name = strings[i];
      strcpy(n[i].name, strings[i]);
      strcpy(n[i].textFileName, n[i].name);
      strcat(n[i].textFileName, ".txt");
    }
    n[0].num_children = numOfTokens - 1;
    for(i = 1; i < numOfTokens; i++)
    {
      strcpy(n[0].childName[i - 1], n[i].name);
      strcpy(n[i].parentName, n[0].name);
    }
  }
  else
  {
    int i = 0;
    int parent = 0;
    while(n[i].name[0] != '\0')
    {
      char *name = &n[i].name;
      // if(strcmp(name, "0") == 0)
      // {
      //   break;
      // }
      if(strcmp(name, strings[0]) == 0)
      {
        parent = i;
      }
      i++;
    }
    n[parent].num_children = numOfTokens - 1;
    int j;
    for(j = 1; j < numOfTokens; j++)
    {
      strcpy(n[parent].childName[j - 1], strings[j]);
      strcpy(n[i + j - 1].name, strings[j]);
      strcpy(n[i + j - 1].textFileName, n[i + j - 1].name);
      strcat(n[i + j - 1].textFileName, ".txt");
      strcpy(n[i + j - 1].parentName, n[parent].name);
    }
  }
}

void DAGCreator(node_t* n, char *filename)
{
  FILE *DAG = fopen(filename, "r");
  char *buf = malloc(255);
  int line = 1;
  while(fgets(buf, 255, DAG) != NULL)
  {
    if(strcmp(buf, "\n") != 0)
    {
      char* p = strchr(buf, '\n');//Delete trailing \n character.
  		if(p)
  		{
  			*p = 0;
  		}
      parseInputLine(buf, n, line);			//Call parseInputLine on this line.
      line++;
    }
  }
}

void outputDirectoryCreator(node_t* n, char *outputDirectory)
{
  // DIR *dir = opendir(outputDirectory);
  // if(dir) //Directory already exists
  // {
  //   closedir(dir);
  //   rmrf(outputDirectory);
  // }
  // mkdir(outputDirectory, 0700);
  strcpy(n[0].outputDirectoryPath, outputDirectory);
  strcat(n[0].outputDirectoryPath, "/");
  strcat(n[0].outputDirectoryPath, n[0].name);
  strcpy(n[0].outputFileLocation, n[0].outputDirectoryPath);
  strcat(n[0].outputFileLocation, "/");
  strcat(n[0].outputFileLocation, n[0].textFileName);
  n[0].isLeafNode = 0;
  int i = 1;
  while(n[i].name[0] != '\0')
  {
    node_t* temp = findnode(n, n[i].parentName);
    strcpy(n[i].outputDirectoryPath, temp->outputDirectoryPath);
    strcat(n[i].outputDirectoryPath, "/");
    strcat(n[i].outputDirectoryPath, n[i].name);
    strcpy(n[i].outputFileLocation, n[i].outputDirectoryPath);
    strcat(n[i].outputFileLocation, "/");
    strcat(n[i].outputFileLocation, n[i].textFileName);
    if(n[i].num_children == 0)
    {
      n[i].isLeafNode = 1;
    }
    else
    {
      n[i].isLeafNode = 0;
    }
    i++;
  }
  i = 0;
  mkdir(outputDirectory, 0750);
  while(n[i].name[0] != '\0')
  {
    strcpy(n[i].logFile, outputDirectory);
    strcat(n[i].logFile, "/log.txt");
    char *path;
    sprintf(path, "%s", n[i].outputDirectoryPath);
    int direrr = mkdir(path, 0750);
    if(direrr == -1)
    {
      printf("Error is: %s\n", strerror(errno));
    }
    i++;
  }
}

void inputDirectoryCreator(node_t* n, char *inputDirectory)
{
  int i = 1;
  while(n[i].name[0] != '\0')
  {
    if(n[i].isLeafNode == 1)
    {
      strcpy(n[i].inputFileLocation, inputDirectory);
      strcat(n[i].inputFileLocation, "/");
      strcat(n[i].inputFileLocation, n[i].name);
    }
    i++;
  }
}

int initializeQueue(list_t* head, node_t* n)
{
  int i = 1;
  int numberOfFiles = 0;
  while(n[i].name[0] != '\0')
  {
    if(n[i].isLeafNode == 1)
    {
      list_t *newNode = malloc(sizeof(list_t));
      newNode->next = head->next;
      strcpy(newNode->fileLocation, n[i].inputFileLocation);
      strcpy(newNode->fileName, n[i].name);
      head->next = newNode;
      numberOfFiles++;
    }
    i++;
  }
  return numberOfFiles;
}

list_t* dequeue(list_t* head)
{
  list_t* temp = head->next;
  head->next = temp->next;
  temp->next = NULL;
  return temp;
}

char* decrypt(node_t* leafNode)
{
  FILE *inputFile = fopen(leafNode->inputFileLocation, "r");
  if(inputFile == NULL)
  {
    printf("Failed to open input File");
  }
  FILE *outputFile = fopen(leafNode->outputFileLocation, "w");
  if(outputFile == NULL)
  {
    printf("Failed to open output File");
  }
  int c;
  while(1)
  {
    c = fgetc(inputFile);
    if(feof(inputFile))
    {
      break;
    }
    if(c == 10)
    {
      fprintf(outputFile, "%c", c);
    }
    else
    {
      c = c + 2;
      fprintf(outputFile, "%c", c);
    }
  }
  fclose(inputFile);
  fclose(outputFile);
  return(leafNode->outputFileLocation);
}

void leafCounter(char* leafFile, char** Candidates, int CandidatesVotes[MAX_CANDIDATES])
{
  FILE *leaf = fopen(leafFile, "r");
  char *buf = malloc(70);
  while(fgets(buf, 256, leaf) != NULL)
  {
    //printf(buf);
    //printf("\n");
    if(strcmp(buf, "\n") != 0)
		{
      //printf(buf);
			char* p = strchr(buf, '\n');//Delete trailing \n character.
		  if(p)
		  {
			  *p = 0;
		  }
      int match = 0;
      int i;
      for(i = 0; i < MAX_CANDIDATES; i++)
      {
        if(Candidates[i][0] == 0) //Candidate not in array
        {
          break;
        }
        else if(strcmp(Candidates[i], buf) == 0) //Match found
        {
          match = 1;
          break;
        }
      }
      if(match == 1)
      {
        CandidatesVotes[i]++;
      }
      else
	    {
				int j = 0;
				while(buf[j] != 0)
				{
					Candidates[i][j] = buf[j];	//Store this Candidate into array
					j++;
				}
	      CandidatesVotes[i]++;	//Increment this candidates total votes
	    }
    }
  }
  fclose(leaf);
  free(buf);
}

void aggregateVotes(node_t* node, node_t* root, char **Candidates, int CandidatesVotes[MAX_CANDIDATES])
{
  node_t* parent = root;
  while(parent->id != node->parentid)
  {
    parent++;
  }
  //printf("Working in node %s and my parent node is %s\n", node->name, parent->name);
  sem_wait(&parent->nodeSem);
  char* filename = parent->outputFileLocation;
  int result = access(filename, F_OK);
  if(result != 0) //File does not exist yet
  {
    FILE *aggregateFile = fopen(filename, "w+");
    int i = 0;
    while(Candidates[i][0] != 0)
    {
      fprintf(aggregateFile, "%s:%d\n", Candidates[i], CandidatesVotes[i]);
      i++;
    }
    fclose(aggregateFile);
  }
  else
  {
    FILE *aggregateFile = fopen(filename, "r");
    char **readCandidates = NULL;
    readCandidates = malloc(MAX_CANDIDATES * sizeof(char*));
    int readCandidatesVotes[MAX_CANDIDATES];
    int i = 0;
    for(i = 0; i < MAX_CANDIDATES; i++)
    {
      readCandidates[i] = NULL;
      readCandidates[i] = malloc(90);
    }
    for(i = 0; i < MAX_CANDIDATES; i++)
    {
      readCandidatesVotes[i] = 0;
    }
    //printf("readCandidates made correctly\n");
    char *buf = NULL;
    buf = malloc(250);
    int j = 0;
    while(fgets(buf, 250, aggregateFile) != NULL)
    {
      if(strcmp(buf, "\n") != 0)
      {
        char* p = strchr(buf, '\n');//Delete trailing \n character.
        if(p)
        {
          *p = 0;
        }
        char **strings;
        int numOfTokens = makeargv(buf, ":", &strings);
        strcpy(readCandidates[j], strings[0]);
        int votes = atoi(strings[1]);
        readCandidatesVotes[j] = votes;
      }
      j++;
    }
    free(buf);
    fclose(aggregateFile);
    int match = 0;
    int a = 0;
    while(Candidates[a][0] != 0)
    {
      for(i = 0; i < MAX_CANDIDATES; i++)
      {
        if(readCandidates[i][0] == 0)
        {
          break;
        }
        else if(strcmp(readCandidates[i], Candidates[a]) == 0)
        {
          match = 1;
          break;
        }
      }
      if(match == 1)
      {
        readCandidatesVotes[i] += CandidatesVotes[a];
      }
      else
      {
        strcpy(readCandidates[j], Candidates[a]);
        readCandidatesVotes[j] = CandidatesVotes[a];
        j++;
      }
      a++;
    }
    // i = 0;
    // printf("I'm node %s\n", node->name);
    // printf("%s's new aggregated votes\n", parent->name);
    // while(readCandidates[i][0] != 0)
    // {
    //   printf("Candidate: %s, Votes: %d\n", readCandidates[i], readCandidatesVotes[i]);
    //   i++;
    // }
    FILE *newAggregateFile = fopen(filename, "w");
    i = 0;
    while(readCandidates[i][0] != 0)
    {
      fprintf(newAggregateFile, "%s:%d\n", readCandidates[i], readCandidatesVotes[i]);
      i++;
    }
    fclose(newAggregateFile);
    // i = 0;
    // printf("I'm node %s\n", node->name);
    // printf("%s's new aggregated votes\n", parent->name);
    // while(readCandidates[i][0] != 0)
    // {
    //   printf("Candidate: %s, Votes: %d\n", readCandidates[i], readCandidatesVotes[i]);
    //   i++;
    // }
  //   for(i = 0; i < MAX_CANDIDATES; i++)
  //   {
  //     free(readCandidates[i]);
  //   }
  //   free(readCandidates);
  }
  sem_post(&parent->nodeSem);
}

void threadFunction(void* arg)
{
  struct threadArgs *realArgs = arg;
  //usleep(rand() % 5000);
  sem_wait(&listSem);
  list_t* listNode = dequeue(realArgs->head);
  //printf("My input file is: %s\n", listNode->fileLocation);
  FILE *logFile = fopen(realArgs->n[0].logFile, "a");                         //Change to a gettid() call
  fprintf(logFile, "%s:%d:start\n", listNode->fileName, 100);
  fclose(logFile);

  node_t* leafNode = findnode(realArgs->n, listNode->fileName);
  sem_post(&listSem);
  //printf("My leafNode is: %s\n", listNode->fileName);
  // printf("My leafNode file location is: %s\n", listNode->fileLocation);

  char *decryptedFileLocation = decrypt(leafNode);
  //printf("My decrypted leafNode file location is: %s\n", decryptedFileLocation);
  sem_wait(&memSem);
  char **Candidates;
  int CandidatesVotes[MAX_CANDIDATES];
  Candidates = malloc(MAX_CANDIDATES * sizeof(char*));
  int i = 0;
  for(i = 0; i < MAX_CANDIDATES; i++)
  {
    Candidates[i] = malloc(50);
  }
  for(i = 0; i < MAX_CANDIDATES; i++)
  {
    CandidatesVotes[i] = 0;
  }
  leafCounter(decryptedFileLocation, Candidates, CandidatesVotes);
  // i = 0;
  // while(Candidates[i][0] != 0)
  // {
  //   printf("Candidate is :%s and their votes are: %d\n", Candidates[i], CandidatesVotes[i]);
  //   i++;
  // }
  //sem_post(&listSem);
  sem_post(&memSem);
  //printf("About to aggregate leaf node: %s\n", leafNode->name);
  aggregateVotes(leafNode, realArgs->n, Candidates, CandidatesVotes);
  return 0;
}

int main(int argc, char **argv){
  struct node* mainnodes=(struct node*)malloc(sizeof(struct node)*MAX_NODES);

	if (argc != 4){
		printf("Usage: %s Program\n", argv[0]);
		return -1;
	}

  DAGCreator(mainnodes, argv[1]);
  outputDirectoryCreator(mainnodes, argv[3]);
  inputDirectoryCreator(mainnodes, argv[2]);
  //printgraph(mainnodes);
  int i = 0;
  while(mainnodes[i].name[0] != '\0')
  {
    mainnodes[i].id = i;
    if(i != 1)
    {
      mainnodes[i].parentid = (findnode(mainnodes, mainnodes[i].parentName))->id;
    }
    i++;
  }
  //i = 0;
  // while(mainnodes[i].name[0] != '\0')
  // {
  //   printf("Node %s has id %d and parent name is %s with id %d\n", mainnodes[i].name, mainnodes[i].id, mainnodes[i].parentName, mainnodes[i].parentid);
  //   i++;
  // }
  list_t* head = NULL;
  head = malloc(sizeof(list_t));
  head->next = NULL;
  int numOfThreads = initializeQueue(head, mainnodes);
  char logFile[1024];
  strcpy(logFile, argv[3]);
  strcat(logFile, "/log.txt");
  FILE *logFP = fopen(logFile, "w");
  fclose(logFP);
  //printList(head);
  i = 0;
  while(mainnodes[i].name[0] != '\0')
  {
    sem_init(&mainnodes[i].nodeSem, 0, 1);
    i++;
  }
  if(pthread_mutex_init(&listLock, NULL) != 0)
  {
    printf("Mutex init failed\n");
    return 1;
  }
  sem_init(&listSem, 0, 1);
  sem_init(&memSem, 0, 1);
  pthread_t threads[numOfThreads];
  for(i = 0; i < numOfThreads; i++)
  {
    struct threadArgs* args = malloc(sizeof(struct threadArgs));
    args->head = head;
    args->n = mainnodes;
    pthread_create(&threads[i], NULL, threadFunction, (void*) args);
  }
  for(i = 0; i < numOfThreads; i++)
  {
    pthread_join(threads[i], NULL);
  }
}
