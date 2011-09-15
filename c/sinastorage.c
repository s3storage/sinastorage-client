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

void base64(const unsigned char *input,int length,char *result);
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
    int i,j,k;
    time_t timer;


    timer=time(NULL);
    j=time(&timer);
 
    k=j+timeout;
    sprintf(expires,"%d",k);

    strcpy(tmpbuf2,slash_str);
    strcat(tmpbuf2,project);
    strcat(tmpbuf2,remotepath);

    processkid(kid,tmpbuf1);          

    strcpy(stringtosignbuf,stringtosign_get);
    strcat(stringtosignbuf,expires);
    strcat(stringtosignbuf,enter_str);
    strcat(stringtosignbuf,tmpbuf2);

    evp_md=EVP_sha1();
    HMAC(evp_md,secretkey,strlen(secretkey),stringtosignbuf,
            strlen(stringtosignbuf),md,&md_len);
    base64(md,md_len,stringtosignbuf);

    for(i=5;i<15;i++)
    {
        ssig[i-5]=stringtosignbuf[i];
    }
    ssig[10]='\0';

    urlencode(ssig,strlen(ssig),stringtosignbuf);    

    strcpy(urlbuf,hostname);
    strcat(urlbuf,tmpbuf2);

    strcat(urlbuf,ssig_str);
    strcat(urlbuf,stringtosignbuf);
    strcat(urlbuf,expires_str);
    strcat(urlbuf,expires);
    strcat(urlbuf,kid_str);
    strcat(urlbuf,tmpbuf1);

    if(stat(localpath,&file_info)==0)
    {
        local_file_len=file_info.st_size;
    }

    f=fopen(localpath,"ab+");
    if(f==NULL)
    {
        printf("EOPENF\n");
        return -1;
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
        printf("EPERFORMF\n");
        return -1;
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
    int i,j,k;
    time_t timer;
 

    timer=time(NULL);
    j=time(&timer);
 
    k=j+timeout;
    sprintf(expires,"%d",k);

    strcpy(tmpbuf2,slash_str);
    strcat(tmpbuf2,project);
    strcat(tmpbuf2,remotepath);

    processkid(kid,tmpbuf1);

    strcpy(stringtosignbuf,stringtosign_put);
    strcat(stringtosignbuf,expires);
    strcat(stringtosignbuf,enter_str);
    strcat(stringtosignbuf,tmpbuf2);

    evp_md=EVP_sha1();
    HMAC(evp_md,secretkey,strlen(secretkey),stringtosignbuf,
            strlen(stringtosignbuf),md,&md_len);
    base64(md,md_len,stringtosignbuf);

    for(i=5;i<15;i++)
    {
        ssig[i-5]=stringtosignbuf[i];
    }
    ssig[10]='\0';

    urlencode(ssig,strlen(ssig),stringtosignbuf);

    strcpy(urlbuf,hostname);
    strcat(urlbuf,tmpbuf2);

    strcat(urlbuf,ssig_str);
    strcat(urlbuf,stringtosignbuf);
    strcat(urlbuf,expires_str);
    strcat(urlbuf,expires);
    strcat(urlbuf,kid_str);
    strcat(urlbuf,tmpbuf1);

    if(NULL == (f=fopen(localpath,"r")))
    {
      printf("EOPENF\n");
      return -1;
    }

    fseek(f,0,SEEK_END);
    int sendSize = ftell(f);
    if(sendSize < 0)
    {
        fclose(f);
        printf("EEMPTYF\n");
        return -1;
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
        printf("EPERFORMF\n");
        return -1;
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

void base64(const unsigned char *input, int length,char *result)
{
    BIO *bmem,*b64;
    BUF_MEM *bptr;

    b64=BIO_new(BIO_f_base64());
    bmem=BIO_new(BIO_s_mem());
    b64=BIO_push(b64,bmem);
    BIO_write(b64,input,length);
    BIO_flush(b64);
    BIO_get_mem_ptr(b64,&bptr);

    memcpy(result,bptr->data,bptr->length-1);
    result[bptr->length-1]=0;

    BIO_free_all(b64);
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
      printf("EINITF\n");
      return -1;
           }
    
    curlhandle=curl_easy_init();
    if(NULL==curlhandle)
           {
      printf("EHANDLEF\n");
      return -1;
           }
    
    res=upload_t(curlhandle,hostname,project,remotepath,localpath,kid,secretkey,timeout);

    curl_easy_cleanup(curlhandle);
    curl_global_cleanup();

    if(res==-1)
        return -1;
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
      printf("EINITF\n");
      return -1;
    }

    curlhandle=curl_easy_init();
    if(NULL==curlhandle)
           {
      printf("EHANDLEF\n");
      return -1;
           }

    res=download_t(curlhandle,hostname,project,remotepath,localpath,kid,secretkey,timeout);
    
    curl_easy_cleanup(curlhandle);
    curl_global_cleanup();

    if(res==-1)
        return -1;
    else
        return 0;
}
