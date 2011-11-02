#include "sinastorage.h"


int main(int c,char *argv[])
{
  char *hostname="sinastorage.com";
  char *project="sandbox";
  char *remotepath="/ty/4.c";
  char *localpath="sp.c";
  char *kid="SYS0000001000SANDBOX";
  char *secretkey="1111111111111111111111111111111111111111";
  int timeout=900;

  download(hostname,project,remotepath,localpath,kid,secretkey,timeout);
  return 0;
}

