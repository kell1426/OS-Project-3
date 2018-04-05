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

void outputDirectoryCreator(struct node_t n, char *outputDirectory);
void DAGCreator(struct node_t n, char *filename, char *outputDirectory);
int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf);
int rmrf(char *path);
void parseInputLine(char *buf, struct node_t n, int line);

int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    int rv = remove(fpath);

    if (rv)
        perror(fpath);

    return rv;
}

int rmrf(char *path)
{
    return nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

void parseInputLine(char *buf, struct node_t n, int line)
{
  char **strings;
  int numOfTokens = makeargv(buf, ":", &strings);
  if(line == 1) //First read_line
  {
    int i;
    for(i = 0; i < numOfTokens; i++)
    {
      n[i].name = strings[i];
      strcpy(n[i].textFileName, n[i].name);
      strcat(n[i].textFileName, ".txt");
    }
    n[0].num_children = numOfTokens - 1;
    for(i = 1; i < numOfTokens; i++)
    {
      n[0].childName = n[i].name;
      n[i].parentName = n[0].name;
    }
  }
  else
  {
    int i = 0;
    int parent = 0;
    while(n[i].name != NULL)
    {
      if(n[i].name == strings[0])
      {
        parent = i;
      }
      i++
    }
    n[parent].num_children = numOfTokens - 1;
    int j;
    for(j = 1; j < numOfTokens; j++)
    {
      n[parent].childName = strings[j];
      n[i + j - 1].name = strings[j];
      strcpy(n[i + j - 1].textFileName, n[i + j - 1].name);
      strcat(n[i + j - 1].textFileName, ".txt");
      n[i + j - 1].parentName = n[parent].name;
    }
  }
}

void DAGCreator(struct node_t n, char *filename, char *outputDirectory)
{
  File *DAG = fopen(filename, "r");
  char *buf = malloc(1024);
  int line = 1;
  while(1)
  {
    buf = read_line(buf, f);
    if(buf == NULL)								//Check for EOF. If so, break out of loop
    {
      break;
    }
    parseInputLine(buf, n, line);			//Call parseInputLine on this line.
    line++;
  }
}

void outputDirectoryCreator(struct node_t n, char *outputDirectory)
{
  DIR *dir = opendir(outputDirectory);
  if(dir) //Directory already exists
  {
    closedir(dir);
    rmrf(outputDirectory);
  }
  mkdir(outputDirectory, 0700);
  int i = 0;
  while(n[i].name != NULL)
  {
    strcpy(n[i].outputDirectoryPath, outputDirectory);
    strcat(n[i].outputDirectoryPath, '/');
    i++
  }
}


int main(int argc, char **argv){
  //Initialize mainnodes

	if (argc != 4){
		printf("Usage: %s Program\n", argv[0]);
		return -1;
	}
  DAGCreator(mainnodes, argv[1], argv[3]);
  outputDirectoryCreator(argv[1], argv[3]);
