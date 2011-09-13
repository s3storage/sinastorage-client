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

char *base64(const unsigned char *input,int length);
char *urlencode(const char *s,int len);

size_t writefunc(void *ptr,size_t size,size_t nmemb, void *stream)
{
    return fwrite(ptr,size,nmemb,stream);
}


int download_t(CURL *curlhandle, const char *hostname,const char *project,const char *remotepath,const char *localpath,const char *kid,const char *secretkey)
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
    char* stringtosign="GET\n\n\n";
    char* expires,*ssig,*base64buf,*urlencodebuf;
    char* enter="\n";
    char* str1="?ssig=";
    char* str2="&Expires=";
    char* str3="&KID=";
    char* str4="/";
    int i,j,k;
    time_t timer;


    timer=time(NULL);
    j=time(&timer);
 
    k=j+60*15;
    expires=malloc(11);
    sprintf(expires,"%d",k);

    strcpy(tmpbuf2,str4);
    strcat(tmpbuf2,project);
    strcat(tmpbuf2,remotepath);

    for(i=0;i<strlen(kid)/2;i++)
    {
        if(kid[i]!='0')
        {
          if(kid[i]<'A'||kid[i]>'Z')  
          {
            tmpbuf1[i]=kid[i];       
          } 
          else
          {
            tmpbuf1[i]=kid[i]+32;
          } 
        }
        else
            break;
    }
    tmpbuf1[i]=',';
    j=i;

    for(i=strlen(kid)/2;i<strlen(kid);i++)
    {
      if(kid[i]!='0')
      break;   
    }

    for(k=i;k<strlen(kid);k++)
    {
       if(kid[k]<'A'||kid[k]>'Z')
        {
          tmpbuf1[j+1]=kid[k];
        }
       else
        {
          tmpbuf1[j+1]=kid[k]+32;
        }  
        j++;
    }
    tmpbuf1[j+1]='\0'; 

    strcpy(stringtosignbuf,stringtosign);
    strcat(stringtosignbuf,expires);
    strcat(stringtosignbuf,enter);
    strcat(stringtosignbuf,tmpbuf2);

    evp_md=EVP_sha1();
    HMAC(evp_md,secretkey,strlen(secretkey),stringtosignbuf,
            strlen(stringtosignbuf),md,&md_len);
    base64buf=base64(md,md_len);
    ssig=malloc(11);
    for(i=5;i<15;i++)
    {
        ssig[i-5]=base64buf[i];
    }
    ssig[10]='\0';

    urlencodebuf=(char*)urlencode(ssig,strlen(ssig));

    strcpy(urlbuf,hostname);
    strcat(urlbuf,tmpbuf2);

    strcat(urlbuf,str1);
    strcat(urlbuf,urlencodebuf);
    strcat(urlbuf,str2);
    strcat(urlbuf,expires);
    strcat(urlbuf,str3);
    strcat(urlbuf,tmpbuf1);
    free(expires);
    free(ssig);
    free(base64buf);
    free(urlencodebuf);

    if(stat(localpath,&file_info)==0)
    {
        local_file_len=file_info.st_size;
    }

    f=fopen(localpath,"ab+");
    if(f==NULL)
    {
        printf("error03:%s,%s\n",localpath,error03);
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
        fprintf(stderr, "%s\n", curl_easy_strerror(r));
        return -1;
    }
}


int upload_t(CURL *curlhandle, const char *hostname,const char *project,const char *remotepath,const char *localpath,const char *kid,const char *secretkey)
{
    FILE *f;
    CURLcode r=CURLE_GOT_NOTHING;
    const EVP_MD *evp_md;
    unsigned int md_len;
    unsigned char md[1024*8];
    char  stringtosignbuf[1024*8],tmpbuf1[50];
    char  urlbuf[1024*8],tmpbuf2[1024*8];
    char* stringtosign="PUT\n\n\n";
    char* expires,*ssig,*base64buf,*urlencodebuf;
    char* enter="\n";
    char* str1="?ssig=";
    char* str2="&Expires=";
    char* str3="&KID=";
    char* str4="/";
    int i,j,k;
    time_t timer;


    timer=time(NULL);
    j=time(&timer);
 
    k=j+60*30;
    expires=malloc(11);
    sprintf(expires,"%d",k);

    strcpy(tmpbuf2,str4);
    strcat(tmpbuf2,project);
    strcat(tmpbuf2,remotepath);

   for(i=0;i<strlen(kid)/2;i++)
    {
        if(kid[i]!='0')
        {
          if(kid[i]<'A'||kid[i]>'Z')  
          {
            tmpbuf1[i]=kid[i];       
          } 
          else
          {
            tmpbuf1[i]=kid[i]+32;
          } 
        }
        else
            break;
    }
    tmpbuf1[i]=',';
    j=i;

    for(i=strlen(kid)/2;i<strlen(kid);i++)
    {
       if(kid[i]!='0')
       break;
    }

    for(k=i;k<strlen(kid);k++)
    {
       if(kid[k]<'A'||kid[k]>'Z')
       {
         tmpbuf1[j+1]=kid[k];
       }
       else
       {
         tmpbuf1[j+1]=kid[k]+32;
       }  
        j++;
    }
    tmpbuf1[j+1]='\0';


    strcpy(stringtosignbuf,stringtosign);
    strcat(stringtosignbuf,expires);
    strcat(stringtosignbuf,enter);
    strcat(stringtosignbuf,tmpbuf2);

    evp_md=EVP_sha1();
    HMAC(evp_md,secretkey,strlen(secretkey),stringtosignbuf,
            strlen(stringtosignbuf),md,&md_len);
    base64buf=base64(md,md_len);
    ssig=malloc(11);
    for(i=5;i<15;i++)
    {
        ssig[i-5]=base64buf[i];
    }
    ssig[10]='\0';

    urlencodebuf=(char*)urlencode(ssig,strlen(ssig));

    strcpy(urlbuf,hostname);
    strcat(urlbuf,tmpbuf2);

    strcat(urlbuf,str1);
    strcat(urlbuf,urlencodebuf);
    strcat(urlbuf,str2);
    strcat(urlbuf,expires);
    strcat(urlbuf,str3);
    strcat(urlbuf,tmpbuf1);
    free(expires);
    free(ssig);
    free(base64buf);
    free(urlencodebuf);

    if(NULL == (f=fopen(localpath,"r")))
    {
      printf("error03:%s,%s\n",localpath,error03);
      return -1;
    }

    fseek(f,0,SEEK_END);
    int sendSize = ftell(f);
    if(sendSize < 0)
    {
        fclose(f);
        printf("error04:%s,%s",localpath,error04);
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
        fprintf(stderr, "%s\n", curl_easy_strerror(r));
        return -1;
    }

}

char *urlencode(const char *s,int len)
{
    const char *from,*end;
    char *start,*to;
    unsigned char hexchars[]="0123456789ABCDEF";
    char c;

    from=s;
    end=s+len;
    start=to=(unsigned char*)malloc(3*len+1);
    while(from<end)
    {
        c=*from++;
        if(c==' ')
        {
            *to++='+';
        }
        else if((c<'0'&& c!='-'&& c!='.')
                ||(c<'A'&& c>'9')
                ||(c>'Z'&& c<'a'&& c!='_')
                ||(c>'z')){
            to[0]='%';
            to[1]=hexchars[c>>4];
            to[2]=hexchars[c&15];
            to+=3;
        }else{
            *to++=c;
        }
    }
    *to=0;
    return (char*)start;
}

char *base64(const unsigned char *input, int length)
{
    BIO *bmem,*b64;
    BUF_MEM *bptr;

    b64=BIO_new(BIO_f_base64());
    bmem=BIO_new(BIO_s_mem());
    b64=BIO_push(b64,bmem);
    BIO_write(b64,input,length);
    BIO_flush(b64);
    BIO_get_mem_ptr(b64,&bptr);

    char *buff=(char*)malloc(bptr->length);
    memcpy(buff,bptr->data,bptr->length-1);
    buff[bptr->length-1]=0;

    BIO_free_all(b64);

    return buff;
}

int upload(const char *hostname,const char *project,const char *remotepath,const char *localpath,const char *kid,const char *secretkey)
{
    int res;
    CURLcode return_code;
    CURL *curlhandle=NULL;
    
    return_code=curl_global_init(CURL_GLOBAL_ALL);
    if(CURLE_OK!=return_code)
           {
      printf("error01:%s\n",error01);
      return -1;
           }
    
    curlhandle=curl_easy_init();
    if(NULL==curlhandle)
           {
      printf("error02:%s\n",error02);
      return -1;
           }
    
    res=upload_t(curlhandle,hostname,project,remotepath,localpath,kid,secretkey);

    curl_easy_cleanup(curlhandle);
    curl_global_cleanup();

    if(res==-1)
        return -1;
    else
        return 0;
}

int download(const char *hostname,const char *project,const char *remotepath,const char *localpath,const char *kid,const char *secretkey)
{
    int res;
    CURLcode return_code;
    CURL *curlhandle=NULL;

    return_code=curl_global_init(CURL_GLOBAL_ALL);
    if(CURLE_OK!=return_code)
    {
      printf("error01:%s\n",error01);
      return -1;
    }

    curlhandle=curl_easy_init();
    if(NULL==curlhandle)
           {
      printf("error02:%s\n",error02);
      return -1;
           }

    res=download_t(curlhandle,hostname,project,remotepath,localpath,kid,secretkey);
    
    curl_easy_cleanup(curlhandle);
    curl_global_cleanup();

    if(res==-1)
        return -1;
    else
        return 0;
}
