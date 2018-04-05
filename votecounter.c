/*kell1426
*02/21/18
*Daniel Kelly
*4718021*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include "makeargv.h"
#include <sys/wait.h>
#include <sys/types.h>
#include <dirent.h>

#define MAX_NODES 100

void inputDirectoryCreator(node_t* n, char *inputDirectory);
void outputDirectoryCreator(node_t* n, char *outputDirectory);
void DAGCreator(node_t* n, char *filename);
// int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf);
// int rmrf(char *path);
void parseInputLine(char *buf, node_t* n, int line);

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
  char *buf = malloc(50);
  int line = 1;
  while(fgets(buf, 50, DAG) != NULL)
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
  strcat(n[0].outputFileLocation, n[0].name);
  strcat(n[0].outputFileLocation, ".txt");
  n[0].isLeafNode = 0;
  int i = 1;
  while(n[i].name != NULL)
  {
    node_t* temp = findnode(n, n[i].parentName);
    strcpy(n[i].outputDirectoryPath, temp->outputDirectoryPath);
    strcat(n[i].outputDirectoryPath, "/");
    strcat(n[i].outputDirectoryPath, n[i].name);
    strcpy(n[i].outputFileLocation, n[i].outputDirectoryPath);
    strcat(n[i].outputFileLocation, "/");
    strcat(n[i].outputFileLocation, n[i].name);
    strcat(n[i].outputFileLocation, ".txt");
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
}

void inputDirectoryCreator(node_t* n, char *inputDirectory)
{
  int i = 1;
  while(n[i].name != NULL)
  {
    if(n[i].isLeafNode == 1)
    {
      strcpy(n[i].inputFileLocation, inputDirectory);
      strcat(n[i].inputFileLocation, "/");
      strcat(n[i].inputFileLocation, n[i].name);
      strcat(n[i].inputFileLocation, ".txt");
    }
  }
}


int main(int argc, char **argv){
  struct node* mainnodes=(struct node*)malloc(sizeof(struct node)*MAX_NODES);

	if (argc != 4){
		printf("Usage: %s Program\n", argv[0]);
		return -1;
	}
  DAGCreator(mainnodes, argv[1]);
  printgraph(mainnodes);
  //outputDirectoryCreator(mainnodes, argv[3]);
  //inputDirectoryCreator(mainnodes, argv[2]);
}
