#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <curl/curl.h>
#include <iconv.h>
#include <openssl/sha.h>
#include <openssl/md5.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <malloc.h>
#include <time.h>
#include <unistd.h>
#include "sinastorage.h"

int base64(const unsigned char *input,int length,char *result,int reslen);
void urlencode(const char *s,int len,char *result);
void processkid(const char *kid,char *result);
void prockidauth(const char *kid,char *result);
static char *hexstr(unsigned char *buf,int len);


size_t writefunc(void *ptr,size_t size,size_t nmemb, void *stream)
{
    return fwrite(ptr,size,nmemb,stream);
}


int download_t(CURL *curlhandle, const char *hostname,const char *project,const char *remotepath,
const char *localpath,const char *kid,const char *secretkey,int timeout)
{
    FILE *f;
    curl_off_t local_file_len=-1;
    CURLcode r=CURLE_GOT_NOTHING;
    struct stat file_info;
    const EVP_MD *evp_md;
    unsigned int md_len;
    unsigned char md[1024*8];
    char stringtosignbuf[1024*8],kidbuf[50];
    char  urlbuf[1024*8],tmpbuf[1024*8];
    char  expires[expires_len],ssig[ssig_len];
    int i,j,k,reslen;
    time_t timer;
    reslen=1024*8;

    timer=time(NULL);
    j=time(&timer);
 
    k=j+timeout;
    sprintf(expires,"%d",k);

    sprintf(tmpbuf,"%s%s%s",slash_str,project,remotepath);

    processkid(kid,kidbuf);          

    sprintf(stringtosignbuf,"%s%s%s%s",stringtosign_get,expires,enter_str,tmpbuf);

    evp_md=EVP_sha1();
    HMAC(evp_md,secretkey,strlen(secretkey),stringtosignbuf,
            strlen(stringtosignbuf),md,&md_len);
    i=base64(md,md_len,stringtosignbuf,reslen);
    if(i!=0)
    return ERROR_BASE64_TOOLONG;

    for(i=5;i<15;i++)
    {
        ssig[i-5]=stringtosignbuf[i];
    }
    ssig[10]='\0';

    urlencode(ssig,strlen(ssig),stringtosignbuf);    

    sprintf(urlbuf,"%s%s%s%s%s%s%s%s",hostname,tmpbuf,ssig_str,stringtosignbuf,
expires_str,expires,kid_str,kidbuf);

    if(stat(localpath,&file_info)==0)
    {
        local_file_len=file_info.st_size;
    }

    f=fopen(localpath,"wb+");
    if(f==NULL)
    {
        return ERROR_OPEN_LOCALFILE;
    }

    curl_easy_setopt(curlhandle,CURLOPT_URL,urlbuf);
    curl_easy_setopt(curlhandle,CURLOPT_VERBOSE,1);
    curl_easy_setopt(curlhandle,CURLOPT_WRITEDATA,f);
    curl_easy_setopt(curlhandle,CURLOPT_WRITEFUNCTION,writefunc);

    r=curl_easy_perform(curlhandle);
    fclose(f);
    if(r==CURLE_OK)
        return 0;
    else
    {
        return ERROR_CURL_PERFORM;
    }
}


int upload_t(CURL *curlhandle, const char *hostname,const char *project,const char *remotepath,
const char *localpath,const char *kid,const char *secretkey,int timeout)
{
    FILE *f;
    CURLcode r=CURLE_GOT_NOTHING;
    const EVP_MD *evp_md;
    unsigned int md_len;
    unsigned char md[1024*8];
    char  stringtosignbuf[1024*8],kidbuf[50];
    char  urlbuf[1024*8],tmpbuf[1024*8];
    char  expires[expires_len],ssig[ssig_len];
    int i,j,k,reslen;
    time_t timer;
    reslen=1024*8;

    timer=time(NULL);
    j=time(&timer);
 
    k=j+timeout;
    sprintf(expires,"%d",k);

    sprintf(tmpbuf,"%s%s%s",slash_str,project,remotepath);

    processkid(kid,kidbuf);

    sprintf(stringtosignbuf,"%s%s%s%s",stringtosign_put,expires,enter_str,tmpbuf);

    evp_md=EVP_sha1();
    HMAC(evp_md,secretkey,strlen(secretkey),stringtosignbuf,
            strlen(stringtosignbuf),md,&md_len);
    i=base64(md,md_len,stringtosignbuf,reslen);
    if(i!=0)
    return ERROR_BASE64_TOOLONG;

    for(i=5;i<15;i++)
    {
        ssig[i-5]=stringtosignbuf[i];
    }
    ssig[10]='\0';

    urlencode(ssig,strlen(ssig),stringtosignbuf);

    sprintf(urlbuf,"%s%s%s%s%s%s%s%s",hostname,tmpbuf,ssig_str,stringtosignbuf,
expires_str,expires,kid_str,kidbuf);

    if(NULL == (f=fopen(localpath,"r")))
    {
      return ERROR_OPEN_LOCALFILE;
    }

    fseek(f,0,SEEK_END);
    int sendSize = ftell(f);
    if(sendSize < 0)
    {
        fclose(f);
        return ERROR_LOCALFILE_EMPTY;
    }

    fseek(f,0,SEEK_SET);

    curl_easy_setopt(curlhandle,CURLOPT_URL,urlbuf);
    curl_easy_setopt(curlhandle,CURLOPT_VERBOSE,1);
    curl_easy_setopt(curlhandle,CURLOPT_READDATA,f);
    curl_easy_setopt(curlhandle,CURLOPT_UPLOAD,1);
    curl_easy_setopt(curlhandle,CURLOPT_INFILESIZE,sendSize);

    r=curl_easy_perform(curlhandle);
    fclose(f);
    if(r==CURLE_OK)
        return 0;
    else
    {
        return ERROR_CURL_PERFORM;
    }

}

int upload_relax_t(CURL *curlhandle,const char *hostname,const char *project,const char *remotepath,
const char *sourcepath,const char *kid,const char *secretkey,int timeout)
{
    FILE *f;
    CURLcode r=CURLE_GOT_NOTHING;
    struct curl_slist *headerlist=NULL;
    const EVP_MD *evp_md;
    unsigned int md_len;
    unsigned char md[1024*8];
    char  stringtosignbuf[1024*8],lengthbuf[50],sha1buf[80];
    char  urlbuf[1024*8],tmpbuf[1024*8],datebuf[50],kidbuf[50];
    char  ssig[ssig_len];
    int i,reslen;
    time_t timer;
    struct tm *tm;
    reslen=1024*8;


    timer=time(NULL);
    tm=localtime(&timer);
    strftime(datebuf,50,"%a, %d %b %Y %T",tm);
    sprintf(tmpbuf,"%s%s%s",slash_str,project,remotepath);

    prockidauth(kid,kidbuf);

    if(NULL == (f=fopen(sourcepath,"r")))
    {
      return ERROR_OPEN_LOCALFILE;
    }

    fseek(f,0,SEEK_END);
    int sendSize = ftell(f);
    if(sendSize < 0)
    {
        fclose(f);
        return ERROR_LOCALFILE_EMPTY;
    }

    fseek(f,0,SEEK_SET);
    fread(stringtosignbuf,sendSize,1,f);
    SHA1(stringtosignbuf,strlen(stringtosignbuf),sha1buf);

    sprintf(md,"%s",hexstr(sha1buf,20));
    sprintf(sha1buf,"%s%s",sha1_str,md);
    sprintf(stringtosignbuf,"%s%s%s%s%s%s%s%s%s",stringtosign_relax,md,enter_str,
  enter_str,datebuf,zero_str,enter_str,tmpbuf,relax_str);

    evp_md=EVP_sha1();
    HMAC(evp_md,secretkey,strlen(secretkey),stringtosignbuf,
            strlen(stringtosignbuf),md,&md_len);
    i=base64(md,md_len,stringtosignbuf,reslen);
    if(i!=0)
    return ERROR_BASE64_TOOLONG;

    for(i=5;i<15;i++)
    {
        ssig[i-5]=stringtosignbuf[i];
    }
    ssig[10]='\0';


    sprintf(urlbuf,"%s%s%s",hostname,tmpbuf,relax_str);
    sprintf(tmpbuf,"%s%s%s",author_str,kidbuf,ssig);
    sprintf(lengthbuf,"%s%d",length_str,sendSize);
    sprintf(md,"%s%s%s",date_str,datebuf,zero_str);
  
    headerlist=curl_slist_append(headerlist,sha1buf);
    headerlist=curl_slist_append(headerlist,lengthbuf);
    headerlist=curl_slist_append(headerlist,tmpbuf);
    headerlist=curl_slist_append(headerlist,md);    

    curl_easy_setopt(curlhandle,CURLOPT_URL,urlbuf);
    curl_easy_setopt(curlhandle,CURLOPT_VERBOSE,1);
    curl_easy_setopt(curlhandle,CURLOPT_UPLOAD,1);
    curl_easy_setopt(curlhandle,CURLOPT_INFILESIZE,0);
    curl_easy_setopt(curlhandle,CURLOPT_HTTPHEADER,headerlist);

    r=curl_easy_perform(curlhandle);
    if(r==CURLE_OK)
        return 0;
    else
    {
        return ERROR_CURL_PERFORM;
    }
}


int delete_t(CURL *curlhandle,const char *hostname,const char *project,const char *remotepath,
const char *kid,const char *secretkey,int timeout)
{
    CURLcode r=CURLE_GOT_NOTHING;
    const EVP_MD *evp_md;
    unsigned int md_len;
    unsigned char md[1024*8];
    char  stringtosignbuf[1024*8],kidbuf[50];
    char  urlbuf[1024*8],tmpbuf[1024*8];
    char  expires[expires_len],ssig[ssig_len];
    int i,j,k,reslen;
    time_t timer;
    reslen=1024*8;

    timer=time(NULL);
    j=time(&timer);
 
    k=j+timeout;
    sprintf(expires,"%d",k);

    sprintf(tmpbuf,"%s%s%s",slash_str,project,remotepath);

    processkid(kid,kidbuf);

    sprintf(stringtosignbuf,"%s%s%s%s",stringtosign_delete,expires,enter_str,tmpbuf);

    evp_md=EVP_sha1();
    HMAC(evp_md,secretkey,strlen(secretkey),stringtosignbuf,
            strlen(stringtosignbuf),md,&md_len);
    i=base64(md,md_len,stringtosignbuf,reslen);
    if(i!=0)
    return ERROR_BASE64_TOOLONG;

    for(i=5;i<15;i++)
    {
        ssig[i-5]=stringtosignbuf[i];
    }
    ssig[10]='\0';

    urlencode(ssig,strlen(ssig),stringtosignbuf);

    sprintf(urlbuf,"%s%s%s%s%s%s%s%s",hostname,tmpbuf,ssig_str,stringtosignbuf,
expires_str,expires,kid_str,kidbuf);

    curl_easy_setopt(curlhandle,CURLOPT_URL,urlbuf);
    curl_easy_setopt(curlhandle,CURLOPT_VERBOSE,1);
    curl_easy_setopt(curlhandle,CURLOPT_CUSTOMREQUEST,"DELETE"); 

    r=curl_easy_perform(curlhandle);
    if(r==CURLE_OK)
        return 0;
    else
    {
        return ERROR_CURL_PERFORM;
    }


}

int copy_t(CURL *curlhandle,const char *hostname,const char *project,const char *remotepath,
const char *sourcepath,const char *kid,const char *secretkey,int timeout)
{
    CURLcode r=CURLE_GOT_NOTHING;
    struct curl_slist *headerlist=NULL;
    const EVP_MD *evp_md;
    unsigned int md_len;
    unsigned char md[1024*8];
    char  stringtosignbuf[1024*8],kidbuf[50];
    char  urlbuf[1024*8],tmpbuf[1024*8];
    char  expires[expires_len],ssig[ssig_len];
    int i,j,k,reslen;
    time_t timer;
    reslen=1024*8;

    timer=time(NULL);
    j=time(&timer);
 
    k=j+timeout;
    sprintf(expires,"%d",k);

    sprintf(tmpbuf,"%s%s%s",slash_str,project,remotepath);

    processkid(kid,kidbuf);
    
    sprintf(urlbuf,"%s%s",source_str,sourcepath);
    sprintf(stringtosignbuf,"%s%s%s%s%s%s",stringtosign_put,expires,
enter_str,urlbuf,enter_str,tmpbuf);

    evp_md=EVP_sha1();
    HMAC(evp_md,secretkey,strlen(secretkey),stringtosignbuf,
            strlen(stringtosignbuf),md,&md_len);
    i=base64(md,md_len,stringtosignbuf,reslen);
    if(i!=0)
    return ERROR_BASE64_TOOLONG;

    for(i=5;i<15;i++)
    {
        ssig[i-5]=stringtosignbuf[i];
    }
    ssig[10]='\0';

    urlencode(ssig,strlen(ssig),stringtosignbuf);

    sprintf(urlbuf,"%s%s%s%s%s%s%s%s",hostname,tmpbuf,ssig_str,stringtosignbuf,
expires_str,expires,kid_str,kidbuf);

    sprintf(tmpbuf,"%s%s",source_str,sourcepath);    
    headerlist=curl_slist_append(headerlist,tmpbuf);

    curl_easy_setopt(curlhandle,CURLOPT_URL,urlbuf);
    curl_easy_setopt(curlhandle,CURLOPT_VERBOSE,1);
    curl_easy_setopt(curlhandle,CURLOPT_UPLOAD,1);
    curl_easy_setopt(curlhandle,CURLOPT_INFILESIZE,0);
    curl_easy_setopt(curlhandle,CURLOPT_HTTPHEADER,headerlist);

    r=curl_easy_perform(curlhandle);
    if(r==CURLE_OK)
        return 0;
    else
    {
        return ERROR_CURL_PERFORM;
    }
}


int update_meta_t(CURL *curlhandle,const char *hostname,const char *project,const char *remotepath,
const char *sourcepath,const char *kid,const char *secretkey,int timeout)
{
    FILE *f;
    CURLcode r=CURLE_GOT_NOTHING;
    struct curl_slist *headerlist=NULL;
    const EVP_MD *evp_md;
    unsigned int md_len;
    unsigned char md[1024*8];
    char  stringtosignbuf[1024*8],sha1buf[80];
    char  urlbuf[1024*8],tmpbuf[1024*8],datebuf[50],kidbuf[50];
    char  ssig[ssig_len];
    int i,j,k,reslen;
    time_t timer;
    struct tm *tm;
    reslen=1024*8;


    timer=time(NULL);
    tm=localtime(&timer);
    j=time(&timer);
    k=j+timeout;
    strftime(datebuf,50,"%a, %d %b %Y %T",tm);
    sprintf(tmpbuf,"%s%s%s",slash_str,project,remotepath);

    prockidauth(kid,kidbuf);

    if(NULL == (f=fopen(sourcepath,"r")))
    {
      return ERROR_OPEN_LOCALFILE;
    }

    fseek(f,0,SEEK_END);
    int sendSize = ftell(f);
    if(sendSize < 0)
    {
        fclose(f);
        return ERROR_LOCALFILE_EMPTY;
    }

    fseek(f,0,SEEK_SET);
    fread(stringtosignbuf,sendSize,1,f);
    SHA1(stringtosignbuf,strlen(stringtosignbuf),sha1buf);

    sprintf(md,"%s",hexstr(sha1buf,20));
    sprintf(sha1buf,"%s%s",sha1_str,md);
    sprintf(stringtosignbuf,"%s%s%s%s%s%s%s%sfile is modified on %s%s%s%s",stringtosign_relax,
    md,enter_str,enter_str,datebuf,zero_str,enter_str,info_x_str,datebuf,enter_str,tmpbuf,meta_str);

    evp_md=EVP_sha1();
    HMAC(evp_md,secretkey,strlen(secretkey),stringtosignbuf,
            strlen(stringtosignbuf),md,&md_len);
    i=base64(md,md_len,stringtosignbuf,reslen);
    if(i!=0)
    return ERROR_BASE64_TOOLONG;

    for(i=5;i<15;i++)
    {
        ssig[i-5]=stringtosignbuf[i];
    }
    ssig[10]='\0';


    sprintf(urlbuf,"%s%s%s",hostname,tmpbuf,meta_str);
    sprintf(tmpbuf,"%s%s%s",author_str,kidbuf,ssig);
    sprintf(stringtosignbuf,"%s file is modified on %s",info_x_str,datebuf);
    sprintf(md,"%s%s%s",date_str,datebuf,zero_str);
  
    headerlist=curl_slist_append(headerlist,sha1buf);
    headerlist=curl_slist_append(headerlist,stringtosignbuf);
    headerlist=curl_slist_append(headerlist,tmpbuf);
    headerlist=curl_slist_append(headerlist,md);    

    curl_easy_setopt(curlhandle,CURLOPT_URL,urlbuf);
    curl_easy_setopt(curlhandle,CURLOPT_VERBOSE,1);
    curl_easy_setopt(curlhandle,CURLOPT_UPLOAD,1);
    curl_easy_setopt(curlhandle,CURLOPT_INFILESIZE,0);
    curl_easy_setopt(curlhandle,CURLOPT_HTTPHEADER,headerlist);

    r=curl_easy_perform(curlhandle);
    if(r==CURLE_OK)
        return 0;
    else
    {
        return ERROR_CURL_PERFORM;
    }
}

int upload_init_t(CURL *curlhandle,const char *hostname,const char *project,const char *remotepath,
const char *kid,const char *secretkey,char *uploadid,int timeout)
{
    FILE *f;
    CURLcode r=CURLE_GOT_NOTHING;
    struct curl_slist *headerlist=NULL;
    const EVP_MD *evp_md;
    unsigned int md_len;
    unsigned char md[1024*8];
    char  stringtosignbuf[1024*8],contentbuf[80];
    char  urlbuf[1024*8],tmpbuf[1024*8],datebuf[50],kidbuf[50];
    char  expires[expires_len],ssig[ssig_len];
    int i,j,k,reslen;
    time_t timer;
    struct tm *tm;
    reslen=1024*8;


    timer=time(NULL);
    tm=localtime(&timer);
    j=time(&timer);
    k=j+timeout;
    sprintf(expires,"%d",k);
    strftime(datebuf,50,"%a, %d %b %Y %T %Z",tm);
    sprintf(tmpbuf,"%s%s%s",slash_str,project,remotepath);

    prockidauth(kid,kidbuf);

    f=fopen(uploadid_tmpfile,"wb+");
    if(f==NULL)
    return ERROR_OPEN_LOCALFILE;

    sprintf(stringtosignbuf,"%s%s%s%s%s%s%s",stringtosign_init,content_str,enter_str,
    datebuf,enter_str,tmpbuf,upload_str);

    evp_md=EVP_sha1();
    HMAC(evp_md,secretkey,strlen(secretkey),stringtosignbuf,
            strlen(stringtosignbuf),md,&md_len);
    i=base64(md,md_len,stringtosignbuf,reslen);
    if(i!=0)
    return ERROR_BASE64_TOOLONG;

    for(i=5;i<15;i++)
    {
        ssig[i-5]=stringtosignbuf[i];
    }
    ssig[10]='\0';


    sprintf(urlbuf,"%s%s%s",hostname,tmpbuf,upload_str);
    sprintf(tmpbuf,"%s%s%s",author_str,kidbuf,ssig);
    sprintf(md,"%s%s",date_str,datebuf);
    sprintf(contentbuf,"%s%s",contenttype_str,content_str);  
    sprintf(stringtosignbuf,"%s",expect_str);

    headerlist=curl_slist_append(headerlist,contentbuf);
    headerlist=curl_slist_append(headerlist,stringtosignbuf);
    headerlist=curl_slist_append(headerlist,tmpbuf);
    headerlist=curl_slist_append(headerlist,md);    

    curl_easy_setopt(curlhandle,CURLOPT_URL,urlbuf);
    curl_easy_setopt(curlhandle,CURLOPT_VERBOSE,1);
    curl_easy_setopt(curlhandle,CURLOPT_POST,1);
    curl_easy_setopt(curlhandle,CURLOPT_POSTFIELDSIZE,0);
    curl_easy_setopt(curlhandle,CURLOPT_HTTPHEADER,headerlist);
    curl_easy_setopt(curlhandle,CURLOPT_WRITEFUNCTION,writefunc);
    curl_easy_setopt(curlhandle,CURLOPT_WRITEDATA,f);

    r=curl_easy_perform(curlhandle);
    if(r!=CURLE_OK)
        return ERROR_CURL_PERFORM;

    fseek(f,0,SEEK_END);
    int sendSize = ftell(f);
    if(sendSize < 0)
    {
        fclose(f);
        return ERROR_LOCALFILE_EMPTY;
    }

    fseek(f,0,SEEK_SET);
    fread(tmpbuf,sendSize,1,f);
    i=0;
    while(i<sendSize)
    {
        if(tmpbuf[i]=='U'&&tmpbuf[i+1]=='p'&&tmpbuf[i+2]=='l'&&tmpbuf[i+3]=='o'&&tmpbuf[i+4]=='a'
           &&tmpbuf[i+5]=='d'&&tmpbuf[i+6]=='I'&&tmpbuf[i+7]=='d')
        break;
        i++;
    }
    i+=9;
    for(j=0;j<32;j++)
    {
        uploadid[j]=tmpbuf[i];
        i++;
    }
    uploadid[j+1]='\0';

    fclose(f);
    return 0;
}


int upload_block_t(CURL *curlhandle, const char *hostname,const char *project,const char *remotepath,
const char *sourcepath,const char *kid,const char *secretkey,const char *uploadid,int partnum,int timeout)
{
    FILE *f;
    CURLcode r=CURLE_GOT_NOTHING;
    struct curl_slist *headerlist=NULL;
    const EVP_MD *evp_md;
    unsigned int md_len;
    unsigned char md[1024*8];
    char  stringtosignbuf[1024*8],contentbuf[80];
    char  urlbuf[1024*8],tmpbuf[1024*8],datebuf[50],kidbuf[50];
    char  expires[expires_len],ssig[ssig_len];
    int i,j,k,reslen;
    time_t timer;
    struct tm *tm;
    reslen=1024*8;


    timer=time(NULL);
    tm=localtime(&timer);
    j=time(&timer);
    k=j+timeout;
    sprintf(expires,"%d",k);
    strftime(datebuf,50,"%a, %d %b %Y %T %Z",tm);
    sprintf(tmpbuf,"%s%s%s",slash_str,project,remotepath);

    prockidauth(kid,kidbuf);

    if(NULL == (f=fopen(sourcepath,"rb")))
    {
      return ERROR_OPEN_LOCALFILE;
    }

    fseek(f,0,SEEK_END);
    int sendSize = ftell(f);
    if(sendSize < 0)
    {
        fclose(f);
        return ERROR_LOCALFILE_EMPTY;
    }

    fseek(f,0,SEEK_SET);


    sprintf(stringtosignbuf,"%s%s%s%s%s%s%s%s%d%s%s",stringtosign_relax,enter_str,content_str,
    enter_str,datebuf,enter_str,tmpbuf,partnum_str,partnum,uploadid_str,uploadid);
    printf("%s\n",stringtosignbuf);
    
    evp_md=EVP_sha1();
    HMAC(evp_md,secretkey,strlen(secretkey),stringtosignbuf,
            strlen(stringtosignbuf),md,&md_len);
    i=base64(md,md_len,stringtosignbuf,reslen);
    if(i!=0)
    return ERROR_BASE64_TOOLONG;

    for(i=5;i<15;i++)
    {
        ssig[i-5]=stringtosignbuf[i];
    }
    ssig[10]='\0';


    sprintf(urlbuf,"%s%s%s%d%s%s",hostname,tmpbuf,partnum_str,partnum,uploadid_str,
    uploadid);
    sprintf(tmpbuf,"%s%s%s",author_str,kidbuf,ssig);
    sprintf(stringtosignbuf,"%s",expect_str);
    sprintf(md,"%s%s",date_str,datebuf);
    sprintf(contentbuf,"%s%s",contenttype_str,content_str);  

    headerlist=curl_slist_append(headerlist,contentbuf);
    headerlist=curl_slist_append(headerlist,stringtosignbuf);
    headerlist=curl_slist_append(headerlist,tmpbuf);
    headerlist=curl_slist_append(headerlist,md);    

    curl_easy_setopt(curlhandle,CURLOPT_URL,urlbuf);
    curl_easy_setopt(curlhandle,CURLOPT_VERBOSE,1);
    curl_easy_setopt(curlhandle,CURLOPT_READDATA,f);
    curl_easy_setopt(curlhandle,CURLOPT_UPLOAD,1);
    curl_easy_setopt(curlhandle,CURLOPT_INFILESIZE,sendSize);
    curl_easy_setopt(curlhandle,CURLOPT_HTTPHEADER,headerlist);

    r=curl_easy_perform(curlhandle);
    fclose(f);
    if(r==CURLE_OK)
        return 0;
    else
    {
        return ERROR_CURL_PERFORM;
    }
}

int upload_complete_t(CURL *curlhandle, const char *hostname,const char *project,const char *remotepath,
const char *sourcepath,const char *kid,const char *secretkey,const char *uploadid,int partnum,int timeout)
{
    FILE *f;
    CURLcode r=CURLE_GOT_NOTHING;
    struct curl_slist *headerlist=NULL;
    const EVP_MD *evp_md;
    unsigned int md_len;
    unsigned char md[1024*8];
    char  stringtosignbuf[1024*8],contentbuf[80];
    char  urlbuf[1024*8],tmpbuf[1024*8],datebuf[50],kidbuf[50];
    char  expires[expires_len],ssig[ssig_len];
    int i,j,k,reslen;
    time_t timer;
    struct tm *tm;
    reslen=1024*8;



    timer=time(NULL);
    tm=localtime(&timer);
    j=time(&timer);
    k=j+timeout;
    sprintf(expires,"%d",k);
    strftime(datebuf,50,"%a, %d %b %Y %T %Z",tm);
    sprintf(tmpbuf,"%s%s%s",slash_str,project,remotepath);

    prockidauth(kid,kidbuf);

    if(NULL == (f=fopen(sourcepath,"r")))
    {
       return ERROR_OPEN_LOCALFILE;
    }

    fseek(f,0,SEEK_END);
    int sendSize = ftell(f);
    if(sendSize < 0)
    {
        fclose(f);
        return ERROR_LOCALFILE_EMPTY;
    }

    fseek(f,0,SEEK_SET);

    sprintf(stringtosignbuf,"%s%s%s%s%s%s%s%s",stringtosign_init,content_str,enter_str,
    datebuf,enter_str,tmpbuf,uploadid_com_str,uploadid);

    evp_md=EVP_sha1();
    HMAC(evp_md,secretkey,strlen(secretkey),stringtosignbuf,
            strlen(stringtosignbuf),md,&md_len);
    i=base64(md,md_len,stringtosignbuf,reslen);
    if(i!=0)
    return ERROR_BASE64_TOOLONG;

    for(i=5;i<15;i++)
    {
        ssig[i-5]=stringtosignbuf[i];
    }
    ssig[10]='\0';


    sprintf(urlbuf,"%s%s%s%s",hostname,tmpbuf,uploadid_com_str,uploadid);
    sprintf(tmpbuf,"%s%s%s",author_str,kidbuf,ssig);
    sprintf(stringtosignbuf,"%s",expect_str);    
    sprintf(contentbuf,"%s%s",contenttype_str,content_str); 
    sprintf(md,"%s%s",date_str,datebuf);
  
    headerlist=curl_slist_append(headerlist,contentbuf);
    headerlist=curl_slist_append(headerlist,stringtosignbuf);
    headerlist=curl_slist_append(headerlist,tmpbuf);
    headerlist=curl_slist_append(headerlist,md);    

    curl_easy_setopt(curlhandle,CURLOPT_URL,urlbuf);
    curl_easy_setopt(curlhandle,CURLOPT_VERBOSE,1);
    curl_easy_setopt(curlhandle,CURLOPT_READDATA,f);
    curl_easy_setopt(curlhandle,CURLOPT_POST,1);
    curl_easy_setopt(curlhandle,CURLOPT_POSTFIELDSIZE,sendSize);
    curl_easy_setopt(curlhandle,CURLOPT_HTTPHEADER,headerlist);

    r=curl_easy_perform(curlhandle);
    if(r==CURLE_OK)
        return 0;
    else
    {
        return ERROR_CURL_PERFORM;
    }
}


void processkid(const char *kid,char *result)
{
  int i,j,k,m;  
    
  k=strlen(kid);
  for(i=0;i<k/2;i++)
  {
     if(kid[i]!='0')
     {
       if(kid[i]<'A'||kid[i]>'Z')  
       {
         result[i]=kid[i];       
       } 
       else
       {
         result[i]=kid[i]+32;
       } 
     }
     else
     break;
  }
  result[i]=',';
  j=i;

  for(i=k/2;i<k;i++)
  {
     if(kid[i]!='0')
     break;
  }

  for(m=i;m<k;m++)
  {
     if(kid[m]<'A'||kid[m]>'Z')
     {
       result[j+1]=kid[m];
     }
     else
     {
       result[j+1]=kid[m]+32;
     }  
     j++;
  }
  result[j+1]='\0';

}

void prockidauth(const char *kid,char *result)
{
  int i,j,k,m;  
    
  k=strlen(kid);
  for(i=0;i<k/2;i++)
  {
     if(kid[i]!='0')
     {
       if((kid[i]>='A')&&(kid[i]<='Z'))  
       {
         result[i]=kid[i];       
       } 
       else
       {
         result[i]=kid[i]-32;
       } 
     }
     else
     break;
  }
  result[i]=' ';
  j=i;

  for(i=k/2;i<k;i++)
  {
     if(kid[i]!='0')
     break;
  }

  for(m=i;m<k;m++)
  {
     if(kid[m]<'A'||kid[m]>'Z')
     {
       result[j+1]=kid[m];
     }
     else
     {
       result[j+1]=kid[m]+32;
     }  
     j++;
  }
  result[j+1]=':';
  result[j+2]='\0';
}


void urlencode(const char *s,int len,char *result)
{
    const char *from,*end;
    unsigned char hexchars[]="0123456789ABCDEF";
    char c;

    from=s;
    end=s+len;
    while(from<end)
    {
        c=*from++;
        if(c==' ')
        {
            *result++='+';
        }
        else if((c<'0'&& c!='-'&& c!='.')
                ||(c<'A'&& c>'9')
                ||(c>'Z'&& c<'a'&& c!='_')
                ||(c>'z')){
            result[0]='%';
            result[1]=hexchars[c>>4];
            result[2]=hexchars[c&15];
            result+=3;
        }else{
            *result++=c;
        }
    }
    *result=0;
}

int base64(const unsigned char *input, int length,char *result,int reslen)
{
    BIO *bmem,*b64;
    BUF_MEM *bptr;

    b64=BIO_new(BIO_f_base64());
    bmem=BIO_new(BIO_s_mem());
    b64=BIO_push(b64,bmem);
    BIO_write(b64,input,length);
    BIO_flush(b64);
    BIO_get_mem_ptr(b64,&bptr);

    if((bptr->length)>=reslen)
    return ERROR_BASE64_TOOLONG;
    memcpy(result,bptr->data,bptr->length-1);
    result[bptr->length-1]=0;

    BIO_free_all(b64);
    return 0;
}


static char *hexstr(unsigned char *buf,int len)
{
    const char *set ="0123456789abcdef";
    static char str[80],*tmp;
    unsigned char *end;
    
    end=buf+len; 
    tmp=&str[0];
    while(buf<end)
          {
      *tmp++=set[(*buf)>>4];
      *tmp++=set[(*buf)&0x0F];
      buf++;
           }

    *tmp='\0';
    return str;
}

int upload(const char *hostname,const char *project,const char *remotepath,const char *localpath,
const char *kid,const char *secretkey,int timeout)
{
    int res;
    CURLcode return_code;
    CURL *curlhandle=NULL;
    
    return_code=curl_global_init(CURL_GLOBAL_ALL);
    if(CURLE_OK!=return_code)
    {
      return ERROR_CURL_INIT;
    }
    
    curlhandle=curl_easy_init();
    if(NULL==curlhandle)
    {
      return ERROR_CURL_GETHANDLE;
    }
    
    res=upload_t(curlhandle,hostname,project,remotepath,localpath,kid,secretkey,timeout);

    curl_easy_cleanup(curlhandle);
    curl_global_cleanup();

    if(res!=0)
        return res;
    else
        return 0;
}

int upload_relax(const char *hostname,const char *project,const char *remotepath,const char *sourcepath,
const char *kid,const char *secretkey,int timeout)
{
    int res;
    CURLcode return_code;
    CURL *curlhandle=NULL;

    return_code=curl_global_init(CURL_GLOBAL_ALL);
    if(CURLE_OK!=return_code)
    {
      return ERROR_CURL_INIT;
    }

    curlhandle=curl_easy_init();
    if(NULL==curlhandle)
    {
      return ERROR_CURL_GETHANDLE;
    }

    res=upload_relax_t(curlhandle,hostname,project,remotepath,sourcepath,kid,secretkey,timeout);
    
    curl_easy_cleanup(curlhandle);
    curl_global_cleanup();

    if(res!=0)
        return res;
    else
        return 0;
}

int download(const char *hostname,const char *project,const char *remotepath,const char *localpath,
const char *kid,const char *secretkey,int timeout)
{
    int res;
    CURLcode return_code;
    CURL *curlhandle=NULL;

    return_code=curl_global_init(CURL_GLOBAL_ALL);
    if(CURLE_OK!=return_code)
    {
      return ERROR_CURL_INIT;
    }

    curlhandle=curl_easy_init();
    if(NULL==curlhandle)
    {
      return ERROR_CURL_GETHANDLE;
    }

    res=download_t(curlhandle,hostname,project,remotepath,localpath,kid,secretkey,timeout);
    
    curl_easy_cleanup(curlhandle);
    curl_global_cleanup();

    if(res!=0)
        return res;
    else
        return 0;
}

int delete(const char *hostname,const char *project,const char *remotepath,
const char *kid,const char *secretkey,int timeout)
{
    int res;
    CURLcode return_code;
    CURL *curlhandle=NULL;

    return_code=curl_global_init(CURL_GLOBAL_ALL);
    if(CURLE_OK!=return_code)
    {
      return ERROR_CURL_INIT;
    }

    curlhandle=curl_easy_init();
    if(NULL==curlhandle)
    {
      return ERROR_CURL_GETHANDLE;
    }

    res=delete_t(curlhandle,hostname,project,remotepath,kid,secretkey,timeout);
    
    curl_easy_cleanup(curlhandle);
    curl_global_cleanup();

    if(res!=0)
        return res;
    else
        return 0;
}

int copy(const char *hostname,const char *project,const char *remotepath,const char *sourcepath,
const char *kid,const char *secretkey,int timeout)
{
    int res;
    CURLcode return_code;
    CURL *curlhandle=NULL;

    return_code=curl_global_init(CURL_GLOBAL_ALL);
    if(CURLE_OK!=return_code)
    {
      return ERROR_CURL_INIT;
    }

    curlhandle=curl_easy_init();
    if(NULL==curlhandle)
    {
      return ERROR_CURL_GETHANDLE;
    }

    res=copy_t(curlhandle,hostname,project,remotepath,sourcepath,kid,secretkey,timeout);
    
    curl_easy_cleanup(curlhandle);
    curl_global_cleanup();

    if(res!=0)
        return res;
    else
        return 0;
}

int update_meta(const char *hostname,const char *project,const char *remotepath,const char *sourcepath,
const char *kid,const char *secretkey,int timeout)
{
    int res;
    CURLcode return_code;
    CURL *curlhandle=NULL;

    return_code=curl_global_init(CURL_GLOBAL_ALL);
    if(CURLE_OK!=return_code)
    {
      return ERROR_CURL_INIT;
    }

    curlhandle=curl_easy_init();
    if(NULL==curlhandle)
    {
      return ERROR_CURL_GETHANDLE;
    }

    res=update_meta_t(curlhandle,hostname,project,remotepath,sourcepath,kid,secretkey,timeout);
    
    curl_easy_cleanup(curlhandle);
    curl_global_cleanup();

    if(res!=0)
        return res;
    else
        return 0;
}

int upload_init(const char *hostname,const char *project,const char *remotepath,
const char *kid,const char *secretkey,char *uploadid, int timeout)
{
    int res;
    CURLcode return_code;
    CURL *curlhandle=NULL;

    return_code=curl_global_init(CURL_GLOBAL_ALL);
    if(CURLE_OK!=return_code)
    {
      return ERROR_CURL_INIT;
    }

    curlhandle=curl_easy_init();
    if(NULL==curlhandle)
    {
      return ERROR_CURL_GETHANDLE;
    }

    res=upload_init_t(curlhandle,hostname,project,remotepath,kid,secretkey,uploadid,timeout);
    
    curl_easy_cleanup(curlhandle);
    curl_global_cleanup();

    if(res!=0)
        return res;
    else
        return 0;
}

int upload_block(const char *hostname,const char *project,const char *remotepath,const char *sourcepath,
const char *kid,const char *secretkey,const char *uploadid,int partnum,int timeout)
{
    int res;
    CURLcode return_code;
    CURL *curlhandle=NULL;

    return_code=curl_global_init(CURL_GLOBAL_ALL);
    if(CURLE_OK!=return_code)
    {
      return ERROR_CURL_INIT;
    }

    curlhandle=curl_easy_init();
    if(NULL==curlhandle)
    {
      return ERROR_CURL_GETHANDLE;
    }

    res=upload_block_t(curlhandle,hostname,project,remotepath,sourcepath,kid,secretkey,uploadid,partnum,timeout);
    
    curl_easy_cleanup(curlhandle);
    curl_global_cleanup();

    if(res!=0)
        return res;
    else
        return 0;
}

int upload_complete(const char *hostname,const char *project,const char *remotepath,const char *sourcepath,
const char *kid,const char *secretkey,const char *uploadid,int partnum,int timeout)
{
    int res;
    CURLcode return_code;
    CURL *curlhandle=NULL;

    return_code=curl_global_init(CURL_GLOBAL_ALL);
    if(CURLE_OK!=return_code)
    {
      return ERROR_CURL_INIT;
    }

    curlhandle=curl_easy_init();
    if(NULL==curlhandle)
    {
      return ERROR_CURL_GETHANDLE;
    }

    res=upload_complete_t(curlhandle,hostname,project,remotepath,sourcepath,kid,secretkey,uploadid,partnum,timeout);
    
    curl_easy_cleanup(curlhandle);
    curl_global_cleanup();

    if(res!=0)
        return res;
    else
        return 0;
}
