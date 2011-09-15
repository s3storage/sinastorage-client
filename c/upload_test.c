#include "sinastorage.h"

int main(int c,char *argv[])
{
  char *hostname="sinastorage.com";
  char *project="sandbox";
  char *remotepath="/4.c";
  char *localpath="/tmp/new1.c";
  char *kid="SYS0000000000SANDBOX";
  char *secretkey="1111111111111111111111111111111111111111";
  int timeout=1800;

  upload(hostname,project,remotepath,localpath,kid,secretkey,timeout);
  return 0;
}
