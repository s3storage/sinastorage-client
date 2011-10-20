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
#define stringtosign_relax  "PUT\n"
#define stringtosign_init   "POST\n\n"
#define stringtosign_block  "POST\n"
#define enter_str           "\n"
#define ssig_str            "?ssig="
#define ssig_after_str      "&ssig="
#define expires_str         "&Expires="
#define kid_str             "&KID="
#define slash_str           "/"
#define source_str          "x-amz-copy-source:"
#define copy_str            "?copy"
#define sha1_str            "s-sina-sha1:"
#define md5_str             "Content-MD5:"
#define length_str          "s-sina-length:"
#define relax_str           "?relax"
#define author_str          "Authorization:"
#define date_str            "Date:"
#define zero_str            " +0000"
#define meta_str            "?meta"
#define expires_x_str       "x-sina-expire:"
#define info_x_str          "x-sina-info:"
#define upload_str          "?uploads"
//#define content_str         "application/x-www-form-urlencoded"
#define content_str         "application/xml"
#define contenttype_str     "Content-Type:"
#define partnum_str         "?partNumber="
#define uploadid_str        "&uploadId="
#define uploadid1_str       "&amp;uploadId="
#define uploadid_com_str    "?uploadId="
#define expect_str          "Expect:"
#define test_str            "?ssig=9jFU2Hvt1i&Expires=1318839074&KID=sys,sandbox"

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

int upload_relax(const char *hostname,const char *project,const char *remotepath,
const char *sourcepath,const char *kid,const char *secretkey,int timeout);

int update_meta(const char *hostname,const char *project,const char *remotepath,
const char *sourcepath,const char *kid,const char *secretkey,int timeout);

int upload_init(const char *hostname,const char *project,const char *remotepath,
const char *sourcepath,const char *kid,const char *secretkey,int timeout);

int upload_block(const char *hostname,const char *project,const char *remotepath,
const char *sourcepath,const char *kid,const char *secretkey,const char *uploadid,
int partnum,int timeout);

int upload_complete(const char *hostname,const char *project,const char *remotepath,
const char *sourcepath,const char *kid,const char *secretkey,const char *uploadid,
int partnum,int timeout);
#endif
