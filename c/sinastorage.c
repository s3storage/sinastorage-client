#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <curl/curl.h>
#include <iconv.h>
#include <openssl/sha.h>
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
    char stringtosignbuf[1024*8],tmpbuf1[50];
    char  urlbuf[1024*8],tmpbuf2[1024*8];
    char  expires[expires_len],ssig[ssig_len];
    int i,j,k,reslen;
    time_t timer;
    reslen=1024*8;

    timer=time(NULL);
    j=time(&timer);
 
    k=j+timeout;
    sprintf(expires,"%d",k);

    sprintf(tmpbuf2,"%s%s%s",slash_str,project,remotepath);

    processkid(kid,tmpbuf1);          

    sprintf(stringtosignbuf,"%s%s%s%s",stringtosign_get,expires,enter_str,tmpbuf2);

    evp_md=EVP_sha1();
    HMAC(evp_md,secretkey,strlen(secretkey),stringtosignbuf,
            strlen(stringtosignbuf),md,&md_len);
    i=base64(md,md_len,stringtosignbuf,reslen);
    if(i!=0)
    return -6;

    for(i=5;i<15;i++)
    {
        ssig[i-5]=stringtosignbuf[i];
    }
    ssig[10]='\0';

    urlencode(ssig,strlen(ssig),stringtosignbuf);    

    sprintf(urlbuf,"%s%s%s%s%s%s%s%s",hostname,tmpbuf2,ssig_str,stringtosignbuf,
expires_str,expires,kid_str,tmpbuf1);

    if(stat(localpath,&file_info)==0)
    {
        local_file_len=file_info.st_size;
    }

    f=fopen(localpath,"ab+");
    if(f==NULL)
    {
        return -3;
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
        return -5;
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
    char  stringtosignbuf[1024*8],tmpbuf1[50];
    char  urlbuf[1024*8],tmpbuf2[1024*8];
    char  expires[expires_len],ssig[ssig_len];
    int i,j,k,reslen;
    time_t timer;
    reslen=1024*8;

    timer=time(NULL);
    j=time(&timer);
 
    k=j+timeout;
    sprintf(expires,"%d",k);

    sprintf(tmpbuf2,"%s%s%s",slash_str,project,remotepath);

    processkid(kid,tmpbuf1);

    sprintf(stringtosignbuf,"%s%s%s%s",stringtosign_put,expires,enter_str,tmpbuf2);

    evp_md=EVP_sha1();
    HMAC(evp_md,secretkey,strlen(secretkey),stringtosignbuf,
            strlen(stringtosignbuf),md,&md_len);
    i=base64(md,md_len,stringtosignbuf,reslen);
    if(i!=0)
    return -6;

    for(i=5;i<15;i++)
    {
        ssig[i-5]=stringtosignbuf[i];
    }
    ssig[10]='\0';

    urlencode(ssig,strlen(ssig),stringtosignbuf);

    sprintf(urlbuf,"%s%s%s%s%s%s%s%s",hostname,tmpbuf2,ssig_str,stringtosignbuf,
expires_str,expires,kid_str,tmpbuf1);

    if(NULL == (f=fopen(localpath,"r")))
    {
      return -3;
    }

    fseek(f,0,SEEK_END);
    int sendSize = ftell(f);
    if(sendSize < 0)
    {
        fclose(f);
        return -4;
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
        return -5;
    }

}

int delete_t(CURL *curlhandle,const char *hostname,const char *project,const char *remotepath,
const char *kid,const char *secretkey,int timeout)
{
    FILE *f;
    CURLcode r=CURLE_GOT_NOTHING;
    const EVP_MD *evp_md;
    unsigned int md_len;
    unsigned char md[1024*8];
    char  stringtosignbuf[1024*8],tmpbuf1[50];
    char  urlbuf[1024*8],tmpbuf2[1024*8];
    char  expires[expires_len],ssig[ssig_len];
    int i,j,k,reslen;
    time_t timer;
    reslen=1024*8;

    timer=time(NULL);
    j=time(&timer);
 
    k=j+timeout;
    sprintf(expires,"%d",k);

    sprintf(tmpbuf2,"%s%s%s",slash_str,project,remotepath);

    processkid(kid,tmpbuf1);

    sprintf(stringtosignbuf,"%s%s%s%s",stringtosign_delete,expires,enter_str,tmpbuf2);

    evp_md=EVP_sha1();
    HMAC(evp_md,secretkey,strlen(secretkey),stringtosignbuf,
            strlen(stringtosignbuf),md,&md_len);
    i=base64(md,md_len,stringtosignbuf,reslen);
    if(i!=0)
    return -6;

    for(i=5;i<15;i++)
    {
        ssig[i-5]=stringtosignbuf[i];
    }
    ssig[10]='\0';

    urlencode(ssig,strlen(ssig),stringtosignbuf);

    sprintf(urlbuf,"%s%s%s%s%s%s%s%s",hostname,tmpbuf2,ssig_str,stringtosignbuf,
expires_str,expires,kid_str,tmpbuf1);

    curl_easy_setopt(curlhandle,CURLOPT_URL,urlbuf);
    curl_easy_setopt(curlhandle,CURLOPT_VERBOSE,1);
    curl_easy_setopt(curlhandle,CURLOPT_CUSTOMREQUEST,"DELETE"); 

    r=curl_easy_perform(curlhandle);
    if(r==CURLE_OK)
        return 0;
    else
    {
        return -5;
    }


}

int copy_t(CURL *curlhandle,const char *hostname,const char *project,const char *remotepath,
const char *sourcepath,const char *kid,const char *secretkey,int timeout)
{
    FILE *f;
    CURLcode r=CURLE_GOT_NOTHING;
    struct curl_slist *headerlist=NULL;
    const EVP_MD *evp_md;
    unsigned int md_len;
    unsigned char md[1024*8];
    char  stringtosignbuf[1024*8],tmpbuf1[50];
    char  urlbuf[1024*8],tmpbuf2[1024*8];
    char  expires[expires_len],ssig[ssig_len];
    int i,j,k,reslen;
    time_t timer;
    reslen=1024*8;

    timer=time(NULL);
    j=time(&timer);
 
    k=j+timeout;
    sprintf(expires,"%d",k);

    sprintf(tmpbuf2,"%s%s%s",slash_str,project,remotepath);

    processkid(kid,tmpbuf1);
    
    sprintf(urlbuf,"%s%s",source_str,sourcepath);
    sprintf(stringtosignbuf,"%s%s%s%s%s%s",stringtosign_put,expires,
enter_str,urlbuf,enter_str,tmpbuf2);

    evp_md=EVP_sha1();
    HMAC(evp_md,secretkey,strlen(secretkey),stringtosignbuf,
            strlen(stringtosignbuf),md,&md_len);
    i=base64(md,md_len,stringtosignbuf,reslen);
    if(i!=0)
    return -6;

    for(i=5;i<15;i++)
    {
        ssig[i-5]=stringtosignbuf[i];
    }
    ssig[10]='\0';

    urlencode(ssig,strlen(ssig),stringtosignbuf);

    sprintf(urlbuf,"%s%s%s%s%s%s%s%s",hostname,tmpbuf2,ssig_str,stringtosignbuf,
expires_str,expires,kid_str,tmpbuf1);

    sprintf(tmpbuf2,"%s%s",source_str,sourcepath);    
    headerlist=curl_slist_append(headerlist,tmpbuf2);

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
        return -5;
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

void urlencode(const char *s,int len,char *result)
{
    const char *from,*end;
    char *start,*to;
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
    return -6;
    memcpy(result,bptr->data,bptr->length-1);
    result[bptr->length-1]=0;

    BIO_free_all(b64);
    return 0;
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
      return -1;
    }
    
    curlhandle=curl_easy_init();
    if(NULL==curlhandle)
    {
      return -2;
    }
    
    res=upload_t(curlhandle,hostname,project,remotepath,localpath,kid,secretkey,timeout);

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
      return -1;
    }

    curlhandle=curl_easy_init();
    if(NULL==curlhandle)
    {
      return -2;
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
      return -1;
    }

    curlhandle=curl_easy_init();
    if(NULL==curlhandle)
    {
      return -2;
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
      return -1;
    }

    curlhandle=curl_easy_init();
    if(NULL==curlhandle)
    {
      return -2;
    }

    res=copy_t(curlhandle,hostname,project,remotepath,sourcepath,kid,secretkey,timeout);
    
    curl_easy_cleanup(curlhandle);
    curl_global_cleanup();

    if(res!=0)
        return res;
    else
        return 0;
}
