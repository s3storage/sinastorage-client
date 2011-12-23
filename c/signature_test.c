#include <stdio.h>
#include "signature.h"

int main(int argc, char **argv)
{
   char *secrety="1111111111111111111111111111111111111111";
   char *stringtosign="GET\n\n\n1175139620\n/sinastorage.com/sandbox/123.jpg"; 
   char result[20];

   signature(secrety,stringtosign,result);
   printf("%s\n",result);
}
