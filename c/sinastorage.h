#ifndef _SINASTORAGE_H_
#define _SINASTORAGE_H_

//#define -1    "Init libcurl failed, please insure the libcurl is included."
//#define -2    "Get an easy handle faild for libcurl."
//#define -3    "Open the localpath's file failed, please check the file."
//#define -4    "The file that you want to upload is empty, please check the file."
//#define -5    "When execute curl_easy_preform() failed."
//#define -6    "The strings are too long after base64() encoding."

#define stringtosign_get    "GET\n\n\n"
#define stringtosign_put    "PUT\n\n\n"
#define stringtosign_delete "DELETE\n\n\n"
#define enter_str           "\n"
#define ssig_str            "?ssig="
#define expires_str         "&Expires="
#define kid_str             "&KID="
#define slash_str           "/"
#define source_str          "x-amz-copy-source:"
#define copy_str            "?copy"

#define expires_len 15
#define ssig_len 11

int upload(const char *hostname,const char *project,const char *remotepath,
const char *localpath,const char *kid,const char *secretkey,int timeout);

int download(const char *hostname,const char *project,const char *remotepath,
const char *localpath,const char *kid,const char *secretkey,int timeout);

int delete(const char *hostname,const char *project,const char *remotepath,
const char *kid,const char *secretkey,int timeout);

int copy(const char *hostname,const char *project,const char *remotepath,
const char *sourcepath,const char *kid,const char *secretkey,int timeout);

#endif
