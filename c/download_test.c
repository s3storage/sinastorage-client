#include "updownport.h"


int main(int c,char *argv[])
{
  char *hostname="sinastorage.com";
  char *project="sandbox";
  char *remotepath="/4.c";
  char *localpath="/tmp/xp.c";
  char *kid="SYS0000001000SANDBOX";
  char *secretkey="1111111111111111111111111111111111111111";

  download(hostname,project,remotepath,localpath,kid,secretkey);
  return 0;
}

