#include "updownport.h"

int main(int c,char *argv[])
{
  char *hostname="sinastorage.com";
  char *project="s3xp.s3watch";
  char *remotepath="/4.c";
  char *localpath="/tmp/new1.c";
  char *kid="SAE0000000M414ZO0X30";
  char *secretkey="hm2hzhim3k52jlh43hxh0l3ix00mzmh3h1k0hm11";

  upload(hostname,project,remotepath,localpath,kid,secretkey);
  return 0;
}
