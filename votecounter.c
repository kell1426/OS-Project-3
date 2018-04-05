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

void outputDirectoryCreator(char *filename, char *outputDirectory);
int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf);
int rmrf(char *path);
void parseInputLine(char *buf, char *outputDirectory, int firstline);

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

void parseInputLine(char *buf, char *outputDirectory, int firstline)
{
  char **strings;
  char *topDirectory
  strcpy(rootNodeDirectory, outputDirectory);
  int numOfTokens = makeargv(buf, ":", &strings);
  if(line == 1) //First read_line
  {
      strcat(rootNodeDirectory, '/');
      strcat(rootNodeDirectory, strings[0]);
      mkdir(rootNodeDirectory);
      int i;
      for(i = 1; i < numOfTokens, i++)
      {
        char *newDirectory;
        strcpy(newDirectory, rootNodeDirectory);
        strcat(newDirectory, '/');
        strcat(newDirectory, strings[i]);
        mkdir(newDirectory);
      }


  }
}

void outputDirectoryCreator(char *filename, char *outputDirectory)
{
  DIR *dir = opendir(outputDirectory);
  if(dir) //Directory already exists
  {
    closedir(dir);
    rmrf(outputDirectory);
  }
  mkdir(outputDirectory, 0700);
  FILE *f = file_open(filename);
  char *buf = malloc(1024);
  int line = 1;
  while(1)
  {
    buf = read_line(buf, f);
    if(buf == NULL)								//Check for EOF. If so, break out of loop
    {
      break;
    }
    parseInputLine(buf, outputDirectory, line);			//Call parseInputLine on this line.
    line++;
  }
}


int main(int argc, char **argv){


	if (argc != 4){
		printf("Usage: %s Program\n", argv[0]);
		return -1;
	}

  outputDirectoryCreator(argv[1], argv[3]);
