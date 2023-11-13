#include <stdio.h>
#include <conio.h>
#include <stdlib.h>

union _4byte
{
    float f;
    int i;
    unsigned u;
    short si[2];
    unsigned short su[2];
    char c[4];
    unsigned char b[4];
};

_4byte buf;
FILE * in = 0, * out = 0;
int i;

int main()
{
    in = fopen("file.f", "rb");
    out = fopen("inf.txt", "wt");
    if(in == 0 || out == 0) return -1;
    while(!feof(in))
    {
        for(i = 0; i < 4 && !feof(in); i++) buf.b[i] = fgetc(in);
        for(i = 0; i < 4; i++) fprintf(out, "[%02X]", buf.b[i]);
        fputc('\t', out);
        for(i = 0; i < 4; i++) fprintf(out, "[%c]", buf.b[i]);
        fputc('\t', out);
        fprintf(out, "%+4.3e\t%+11d\t%10u", buf.f, buf.i, buf.u);
        fputc('\n', out);
    }
    fclose(in);
    fclose(out);
    return 0;
}
