#include "sinastorage.h"
#include <stdio.h>
int main(int c,char *argv[])
{
  char *hostname="sinastorage.com";
  char *project="sandbox";
  char *remotepath="/ty/8.c";
  char *kid="SYS0000000000SANDBOX";
  char *secretkey="1111111111111111111111111111111111111111";
  char uploadid[50];
  int timeout=1800;

  upload_init(hostname,project,remotepath,kid,secretkey,uploadid,timeout);
  return 0;
}
