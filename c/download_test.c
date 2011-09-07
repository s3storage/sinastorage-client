#include "updownport.h"


int main(int c,char *argv[])
{
  char *hostname="sinastorage.com";
  char *project="s3xp.s3watch";
  char *remotepath="/1.c";
  char *localpath="/tmp/xp.c";
  char *kid="SAE0000000M414ZO0X30";
  char *secretkey="hm2hzhim3k52jlh43hxh0l3ix00mzmh3h1k0hm11";

  download(hostname,project,remotepath,localpath,kid,secretkey);
  return 0;
}

