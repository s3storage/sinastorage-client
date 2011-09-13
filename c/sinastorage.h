#ifndef _UPDOWNPORT_H_
#define _UPDOWNPORT_H_

#define error01 "Init libcurl failed!"
#define error02 "Get a easy handle faild!"
#define error03 "file open faild!"
#define error04 "upload file is empty!"

int upload(const char *hostname,const char *project,const char *remotepath,
const char *localpath,const char *kid,const char *secretkey);
int download(const char *hostname,const char *project,const char *remotepath,
const char *localpath,const char *kid,const char *secretkey);
#endif
