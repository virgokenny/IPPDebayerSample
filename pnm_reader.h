#include <ipp.h>
#include <stdio.h>
#include <math.h>

typedef enum {
    P2,  // PGM, Grey, 8 bits per element (0-255), ASCII
    P3,  // PPM, Color, 8 bits per element, RGB triplets, ASCII
    P5,  // PGM, Grey, 8 bits per element, binary
    P6,  // PPM, Color, 8 bits per element, RGB triplets, binary
    PUNKNOWN
} PNM_magic;

typedef struct {
    IppiSize imgsize;
    PNM_magic imgtype;
    Ipp8u nChannels;
} PNM_info;

static IppStatus readPNMheader(char *fname, PNM_info *p_PNMinfo)
{
    FILE *f=fopen(fname,"r");
    char tmp[255];
    int w,h;
    IppStatus sts=ippStsNoErr;

    if (f!=NULL)
    {
        p_PNMinfo->imgsize.width=0;
        p_PNMinfo->imgsize.height=0;
        p_PNMinfo->imgtype=PUNKNOWN;
        fscanf(f,"%s\n",tmp);

        switch (tmp[1])
        {
        case '2':  //P2,P3,P5, and P6 supported
            p_PNMinfo->imgtype=P2;
            p_PNMinfo->nChannels=1;
            break;
        case '3':
            p_PNMinfo->imgtype=P3;
            p_PNMinfo->nChannels=3;
            break;
        case '5':
            p_PNMinfo->imgtype=P5;
            p_PNMinfo->nChannels=1;
            break;
        case '6':
            p_PNMinfo->imgtype=P6;
            p_PNMinfo->nChannels=3;
            break;
        case '1': // P1 and P4 unsupported
        case '4':
            printf("%s unsupported\n",tmp);
            sts=ippStsDataTypeErr;
            break;
        default: // Anything else unsupported
            printf("unsupported format\n");
            sts=ippStsErr;
        }

        if (ippStsNoErr==sts)
        {
            fscanf(f,"%d %d\n",&w,&h);
            p_PNMinfo->imgsize.width=w;
            p_PNMinfo->imgsize.height=h;
        }

        fscanf(f,"%d\n",&w);
        if (w!=255)
        {
            sts=ippStsSizeErr;
            printf("only 8 bit depth supported\n");
        }
        fclose(f);
    }
    else
    {
        sts=ippStsErr;
    }
    return sts;
}

static IppStatus readPNM_8u(char *fname, Ipp8u *pImg, int stride, PNM_info *p_PNMinfo)
{
    FILE *f;
    IppStatus sts=ippStsNoErr;
    char tmp[255];
    int w1,h1;

    switch (p_PNMinfo->imgtype)
    {
    case P2:
        f=fopen(fname,"r");
        fscanf(f,"%s\n",tmp);
        if (tmp[1]!='2') sts=ippStsBadArgErr;
        break;
    case P3:
        f=fopen(fname,"r");
        fscanf(f,"%s\n",tmp);
        if (tmp[1]!='3') sts=ippStsBadArgErr;
        break;
    case P5:
        f=fopen(fname,"rb");
        fscanf(f,"%s\n",tmp);
        if (tmp[1]!='5') sts=ippStsBadArgErr;
        break;
    case P6:
        f=fopen(fname,"rb");
        fscanf(f,"%s\n",tmp);
        if (tmp[1]!='6') sts=ippStsBadArgErr;
        break;
    default:
        printf("unsupported image type\n");
        sts=ippStsBadArgErr;
    }


    // check that w & h match
    fscanf(f,"%d %d\n",&w1,&h1); 
    if ((p_PNMinfo->imgsize.width!=w1) || (p_PNMinfo->imgsize.height!=h1)) sts=ippStsBadArgErr;


    fscanf(f,"%d\n",&w1);
    if (w1!=255)
    {
        sts=ippStsSizeErr;
        printf("only 8 bit depth supported\n");
    }



    if (ippStsNoErr==sts)
    {

        switch (p_PNMinfo->imgtype)
        {
        case P2:
            for (int r=0;r<p_PNMinfo->imgsize.height;r++)
            {
                for (int c=0; c<p_PNMinfo->imgsize.width; c++)
                {
                    int valtmp;
                    fscanf(f,"%03d ", &valtmp);
                    pImg[r*stride+c]=valtmp;
                }
                fscanf(f,"\n");
            }
            break;
        case P3:
            for (int r=0;r<p_PNMinfo->imgsize.height;r++)
            {
                for (int c=0; c<p_PNMinfo->imgsize.width; c++)
                {
                    Ipp8u *pImgTmp=&pImg[r*stride+c*3];
                    int v0,v1,v2;
                    fscanf(f,"%03d %03d %03d ", &v0,&v1,&v2);
                    pImgTmp[0]=v0; pImgTmp[1]=v1; pImgTmp[2]=v2;
                }
                fscanf(f,"\n");
            }
            break;
        case P5:
            for (int r=0;r<p_PNMinfo->imgsize.height;r++)
            {
                fread(pImg+(r*stride),1,p_PNMinfo->imgsize.width,f);
            }
            break;
        case P6:
            for (int r=0;r<p_PNMinfo->imgsize.height;r++)
            {
                fread(pImg+(r*stride),1,p_PNMinfo->imgsize.width*3,f);
            }
            break;
        default:
            printf("unsupported format\n");
        }


        fclose(f);
    }
    return sts;
}

static IppStatus writePNM_8u(char *fname, Ipp8u *pImg, int stride, PNM_info *p_PNMinfo)
{
    FILE *f;
    IppStatus sts=ippStsNoErr;

    switch (p_PNMinfo->imgtype)
    {
    case P2:
        f=fopen(fname,"w");
        fprintf(f,"P2\n");
        break;
    case P3:
        f=fopen(fname,"w");
        fprintf(f,"P3\n");
        break;
    case P5:
        f=fopen(fname,"wb");
        fprintf(f,"P5\n");
        break;
    case P6:
        f=fopen(fname,"wb");
        fprintf(f,"P6\n");
        break;
    default:
        printf("unsupported image type\n");
        sts=ippStsBadArgErr;
    }


    if (ippStsNoErr==sts)
    {
        fprintf(f,"%d %d\n",p_PNMinfo->imgsize.width,p_PNMinfo->imgsize.height);
        fprintf(f,"255\n");  // only 8 bit depth supported


        switch (p_PNMinfo->imgtype)
        {
        case P2:
            for (int r=0;r<p_PNMinfo->imgsize.height;r++)
            {
                for (int c=0; c<p_PNMinfo->imgsize.width; c++)
                {
                    fprintf(f,"%03d ", pImg[r*stride+c]);
                }
                fprintf(f,"\n");
            }
            break;
        case P3:
            for (int r=0;r<p_PNMinfo->imgsize.height;r++)
            {
                for (int c=0; c<p_PNMinfo->imgsize.width; c++)
                {
                    Ipp8u *pImgTmp=&pImg[r*stride+c*3];
                    fprintf(f,"%03d %03d %03d ", pImgTmp[0],pImgTmp[1],pImgTmp[2]);
                }
                fprintf(f,"\n");
            }
            break;
        case P5:
            for (int r=0;r<p_PNMinfo->imgsize.height;r++)
            {
                fwrite(pImg+(r*stride),1,p_PNMinfo->imgsize.width,f);
            }
            break;
        case P6:
            for (int r=0;r<p_PNMinfo->imgsize.height;r++)
            {
                fwrite(pImg+(r*stride),1,p_PNMinfo->imgsize.width*3,f);
            }
            break;
        default:
            printf("unsupported format\n");
        }


        fclose(f);
    }
    return sts;
}
