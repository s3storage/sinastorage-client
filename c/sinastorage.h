#ifndef _SINASTORAGE_H_
#define _SINASTORAGE_H_

#define EINITF    "Init libcurl failed, please insure the libcurl is included."
#define EHANDLEF  "Get an easy handle faild for libcurl."
#define EOPENF    "Open the localpath's file failed, please check the file."
#define EEMPTYF   "The file that you want to upload is empty, please check the file."
#define EPERFORMF "When execute curl_easy_preform() failed."

#define stringtosign_get "GET\n\n\n"
#define stringtosign_put "PUT\n\n\n"
#define enter_str        "\n"
#define ssig_str         "?ssig="
#define expires_str      "&Expires="
#define kid_str          "&KID="
#define slash_str        "/"

#define expires_len 15
#define ssig_len 11

int upload(const char *hostname,const char *project,const char *remotepath,
const char *localpath,const char *kid,const char *secretkey,int timeout);
int download(const char *hostname,const char *project,const char *remotepath,
const char *localpath,const char *kid,const char *secretkey,int timeout);
#endif
