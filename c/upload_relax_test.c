#include "sinastorage.h"

int main(int c,char *argv[])
{
  char *hostname="sinastorage.com";
  char *project="sandbox";
  char *remotepath="/ty/9.c";
  char *sourcepath="/tmp/new2.c";
  char *kid="SYS0000000000SANDBOX";
  char *secretkey="1111111111111111111111111111111111111111";
  int timeout=1800;

  upload_relax(hostname,project,remotepath,sourcepath,kid,secretkey,timeout);
  return 0;
}
