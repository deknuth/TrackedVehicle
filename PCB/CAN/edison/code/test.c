#include <stdio.h>
float k;
void V1(void)
{
    int i ,j ;
    printf("const int V1[256]={");
    for(i=0;i<=255;i++)
    {
        k = i;
        j = (k-128)*1.4075*128;
        printf("%d,",j);
    }
    printf("};");
    printf("\n");
}

void V2(void)
{
    int i ,j ;
    printf("const int V2[256]={");
    for(i=0;i<=255;i++)
    {
        k = i;
        j = (k-128)*0.7169*128;
        printf("%d,",j);
    }
    printf("};");
    printf("\n");
}


void U1(void)
{
    int i ,j ;
    printf("const int U1[256]={");
    for(i=0;i<=255;i++)
    {
        k = i;
        j = (k-128)*0.3455*128;
        printf("%d,",j);
    }
    printf("};");
    printf("\n");
}


void U2(void)
{
    int i ,j ;
    printf("const int U2[256]={");
    for(i=0;i<=255;i++)
    {
        k = i;
        j = (k-128)*1.779*128;
        printf("%d,",j);
    }
    printf("};");
    printf("\n");
}


void Y(void)
{
    int i ,j ;
    printf("const int Y[256]={");
    for(i=0;i<=255;i++)
    {
        k = i;
        printf("%d,",i*128);
    }
    printf("};");
    printf("\n");
}

void YUV(void)
{
    int i ,j ;
    printf("const int YUV[256]={");
    for(i=0;i<350;i++)
    {
        if(i > 255)
            printf("255,");
        else
            printf("%d,",i);
    }
    printf("};");
    printf("\n");
}

int main(void)
{
    V1();
    V2();
    U1();
    U2();
    Y();
    YUV();
    printf("%d",sizeof(int));
}
