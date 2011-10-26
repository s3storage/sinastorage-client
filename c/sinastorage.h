#ifndef _SINASTORAGE_H_
#define _SINASTORAGE_H_

#define ERROR_CURL_INIT         (-1)
#define ERROR_CURL_GETHANDLE    (-2)
#define ERROR_OPEN_LOCALFILE    (-3)
#define ERROR_LOCALFILE_EMPTY   (-4)
#define ERROR_CURL_PERFORM      (-5)
#define ERROR_BASE64_TOOLONG    (-6)

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
#define content_str         "application/xml"
#define contenttype_str     "Content-Type:"
#define partnum_str         "?partNumber="
#define uploadid_str        "&uploadId="
#define uploadid1_str       "&amp;uploadId="
#define uploadid_com_str    "?uploadId="
#define expect_str          "Expect:"
#define uploadid_tmpfile    "uploadid.xml"

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
const char *kid,const char *secretkey,char *uploadid,
int timeout);

int upload_block(const char *hostname,const char *project,const char *remotepath,
const char *sourcepath,const char *kid,const char *secretkey,const char *uploadid,
int partnum,int timeout);

int upload_complete(const char *hostname,const char *project,const char *remotepath,
const char *sourcepath,const char *kid,const char *secretkey,const char *uploadid,
int partnum,int timeout);
#endif
