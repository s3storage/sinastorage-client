#include "sinastorage.h"

int main(int c,char *argv[])
{
  char *hostname="sinastorage.com";
  char *project="sandbox";
  char *remotepath="/ty/9.c";
  char *sourcepath="/tmp/new2.c";
  char *kid="SYS0000000000SANDBOX";
  char *secretkey="1111111111111111111111111111111111111111";
  char *uploadid="2bca7b16e744424082cf4c392342f705";
  int partnum=2;
  int timeout=1800;

  upload_block(hostname,project,remotepath,sourcepath,kid,secretkey,uploadid,
  partnum,timeout);
  return 0;
}
