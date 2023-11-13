#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <stdint.h>

//=============================================================================================================

struct bmp_header_inf//don't forget add "BM" before this header
{
    uint32_t file_size;//bytes, including "BM"
    int16_t reserve_1;//=0?
    int16_t reserve_2;//=0?
    uint32_t pict_data_offset;//offset of pixel list from the beginning of file, don't forget "bm"
    uint32_t second_header_size;//this is start of second part of header, = 40, = 0x28
    uint32_t width;//pixels
    uint32_t height;//pixels
    uint16_t flat_count;//usually = 1
    uint16_t color_depth;//bits per pixel = 8|24|32, if( == 8) palette exists
    uint32_t compression_depth;//=0?
    uint32_t picture_size;//bytes of list of pixels, attention!!! if color depth == 24, size of line % 4 = 0
    int32_t width_scale;//=0?
    int32_t height_scale;//=0?
    uint32_t color_count;//if (color_depth != 8) =0?
    uint32_t main_color_count;//if (color_depth != 8) =0?
};

union bmp_header
{
    bmp_header_inf i;
    uint8_t b [sizeof(bmp_header_inf)];
};

struct color_24_inf//rgb
{
    uint8_t b;
    uint8_t g;
    uint8_t r;
};

union color_24
{
    color_24_inf i;
    uint8_t b [3];
};

struct color_32_inf//argb, palette: color_32 ... [0x100]
{
    uint8_t b;
    uint8_t g;
    uint8_t r;
    uint8_t a;//0- absolute transparency, 0xff - no transparency
};

union color_32
{
    color_32_inf i;
    uint8_t b [4];
    uint32_t u;
};

//=============================================================================================================

int bmp_create_interface ();
int bmp_create_do ();

//=============================================================================================================

FILE * picturefile = NULL, * infile = NULL, * outfile = NULL;
bmp_header header = {};
int i = 0, j = 0, t = 0;
uint8_t colordepth_t = 2, * picture;
uint32_t w = 0x400, h = 0x300;
color_32 stdcolor = {{0,0,0,0xff}};
const int headersize = sizeof(bmp_header), colordepth[3] = {8, 24, 32};
color_32 palette [0x100] = {};

//=============================================================================================================

int main()
{
    bmp_create_interface();
    bmp_create_do ();
    return 0;
}

//=============================================================================================================

int bmp_create_interface ()
{
    for (i = 0; i < headersize; i++)//filling header with 0
    {
        header.b[i] = 0;
    }
    t = ' ';
    while(t != 'y')//get parameters
    {
        printf("\n\
height         = %5d (0x%04x)\n\
width          = %5d (0x%04x)\n\
colordepth     =  %2d\n\
stdcolor, argb =  %08x\n\
use this parameters?\n\
use English keyboard, press [Y]/[N]\n", h, h, w, w, colordepth[colordepth_t], stdcolor.u);
        while(t != 'y' && t != 'n') t = getch();//ask if it's OK
        if(t == 'n')
        {
            printf("\nenter height:\n");
            while (scanf("%u", &h) == 0) scanf("%*s");
            printf("\nenter width:\n");
            while (scanf("%u", &w) == 0) scanf("%*s");
            printf("\nchoose colordepth, [1]/[2]/[3]\n");
            while(t != '\n' && t != '\r')
            {
                for(i = 0; i < 3; i++)
                {
                    if(i == colordepth_t) printf("[%02d]", colordepth[i]);
                    else                  printf(" %02d ", colordepth[i]);
                }
                t = getch();
                if (t >= '1' && t <= '3') colordepth_t = t - '1';
                for(i = 0; i < 12; i++) printf("\b \b");
            }
            printf("%2d", colordepth[colordepth_t]);
            printf("\nenter stdcolor, hex, argb:\n");
            while (scanf("%8x", &stdcolor.u) == 0) scanf("%*s");
            t = ' ';
        }
    }
    return 0;
}

int bmp_create_do ()
{
    if (picturefile != 0)
    {
        printf ("\n/* FILE* */ picturefile != 0, can't create bmp");
        return 1;
    }//check for variable is free
    else
    {

        outfile = fopen("picture.bmp","wb");
        if (outfile == 0)
        {
            printf("\n\nerror, can't create(recreate) file picture.bmp");
            return -1;
        }//check if file is opened
        else
        {
            fputc('B', outfile);
            fputc('M', outfile);//"BM"
            header.i.width = w;
            header.i.height = h;
            header.i.color_depth = colordepth[colordepth_t];
            header.i.flat_count = 1;
            header.i.second_header_size = 0x28;
            header.i.pict_data_offset = 2 + headersize;
            t = colordepth_t;
            if(t == 0)
            {
                header.i.pict_data_offset += 0x400;
                header.i.picture_size = h*w;
            }
            else if(t == 1)
            {
                header.i.picture_size = h*((w*3-1)/4+1)*4;
            }
            else //if(t == 2)
            {
                header.i.picture_size = h*w*4;
            }
            header.i.file_size = header.i.picture_size + header.i.pict_data_offset;
            for(i = 0; i < headersize; i++)
            {
                fputc(header.b[i],outfile);
            }
            if (colordepth_t == 0)
            {
                for(i = 0; i < 0x400; i++)
                {
                    fputc(stdcolor.b[i%4], outfile);
                }
                for(i = 0; i < header.i.picture_size; i++)
                {
                    fputc(0, outfile);
                }
            }
            else if(colordepth_t == 1) for (i = 0; i < h; i++)
            {
                for(j = 0; j < w; j++) for (char k = 0; k < 3; k++) fputc(stdcolor.b[k],outfile);
                for(j = 0; j < (4-(w*3)%4)%4; j++) fputc (0, outfile);
            }
            else /*if(colordepth_t == 2)*/ for (i = 0; i < header.i.picture_size; i++)
            {
                fputc(stdcolor.b[i%4], outfile);
            }
        }
        fclose(outfile);
    }
    return 0;
}
