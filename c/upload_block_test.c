#include "sinastorage.h"

int main(int c,char *argv[])
{
  char *hostname="sinastorage.com";
  char *project="sandbox";
  char *remotepath="/ty/9.c";
  char *sourcepath="/tmp/new1.c";
  char *kid="SYS0000000000SANDBOX";
  char *secretkey="1111111111111111111111111111111111111111";
  char *uploadid="6d22aaa189ec41999c77a2784befbcfb";
  int partnum=1;
  int timeout=1800;

  upload_block(hostname,project,remotepath,sourcepath,kid,secretkey,uploadid,
  partnum,timeout);
  return 0;
}
