#ifndef _UPDOWNPORT_H_
#define _UPDOWNPORT_H_

int upload(const char *hostname,const char *project,const char *remotepath,
const char *localpath,const char *kid,const char *secretkey);
int download(const char *hostname,const char *project,const char *remotepath,
const char *localpath,const char *kid,const char *secretkey);
#endif
