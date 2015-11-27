#include <yaos/types.h>
void bcopy (const void *, void *, size_t);
void malloc()
{
}

int copyin(void *d,void *s,size_t len)
{
  bcopy(d,s,len);
  return 0;
}
int copyout(void *d,void *s,size_t len)
{
  bcopy(d,s,len);
  return 0;
}
void _dummy_()
{
int i;
size_t strlen (const char *);
char *strdup (const char *);

char *strstr (const char *, const char *);
int strcmp (const char *, const char *);


void bcopy (const void *, void *, size_t);
int bcmp (const void *, const void *, size_t);
void bzero (void *, size_t);
bzero(&i,0);
bcmp(&i,&i,0);
bcopy(&i,&i,0);
strlen("");
strdup("");
strstr("","");
strcmp("","");
}
