#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SEGMENT 5000 //approximate target size of small file

long file_size(char *name);//function definition below

int main(void)
{
    int segments=0, i, len, accum;
    FILE *fp1, *fp2;
    
    char filename[260]={"c:\\Users\\brown\\OneDrive\\Desktop\\smallFileName_"};//base name for small files.
    char largeFileName[]={"c:\\Users\\brown\\OneDrive\\Desktop\\largeFileName.txt"};//change to your path
    char smallFileName[260];
    char line[1080];
    long sizeFile = file_size(largeFileName);
    segments = sizeFile/SEGMENT + 1;//ensure end of file
    printf("number of segments will be %d", segments);
    fp1 = fopen(largeFileName, "r");
    if(fp1)
    {
        for(i=0;i<segments;i++)
        {
            accum = 0;
            sprintf(smallFileName, "%s%d.txt", filename, i);
            printf("%s%d.txt\n", filename, i);
            fp2 = fopen(smallFileName, "w");
            if(fp2)
            {
                while(fgets(line, 1080, fp1) && accum <= SEGMENT)
                {
                    accum += strlen(line);//track size of growing file
                    fputs(line, fp2);
                }
                fclose(fp2);
            }
        }
        fclose(fp1);
    }
    return 0;
}

long file_size(char *name)
{
    FILE *fp = fopen(name, "rb"); //must be binary read to get bytes

    long size=-1;
    if(fp)
    {
        fseek (fp, 0, SEEK_END);
        size = ftell(fp)+1;
        fclose(fp);
    }
    return size;
}