#include <ipp.h>
#include <stdio.h>
#include <math.h>

#include "tbb/tbb.h"
#include "tbb/parallel_for.h"
#include "tbb/task_scheduler_init.h"
#include "tbb/tick_count.h"
#include "tbb/scalable_allocator.h"
#include "pnm_reader.h"


#if defined( _WIN32 ) || defined ( _WIN64 )
#pragma warning(disable : 4996)
#endif


#define BENCHLOOPS 50

using namespace tbb;

int main(int argc, char* argv[])
{
    Ipp64u cycle_start, cycle_end;
    tick_count tbb_start, tbb_end;

    PNM_info pnminfo_src,pnminfo_dst;

    IppiPoint dstOffset = {0, 0}; 

    IppStatus sts;
    int stridesrc_8u,  stridedst_8u;
    Ipp8u *pSrc=NULL,*pDst=NULL;

    IppiBayerGrid ippiCFAFormat = ippiBayerBGGR;

    if (argc!=5)
    {
        printf("usage: %s infile cfa outfile threads\n",argv[0]);
        printf("  infile : input file, PNM format only\n");
        printf("  cfa : CFA format, rggb/bggr/gbrg/grbg\n");
        printf("  outfile: output file, same PNM type as input\n");
        printf("  threads: # of threads (0=default)\n");

        exit(1);
    }

    // parse command line
    char inFileName[1024];
    strcpy(inFileName,argv[1]);

    char cfaFormatName[8];
    strcpy(cfaFormatName, argv[2]);

    if (strcmp(cfaFormatName, "rggb") == 0) ippiCFAFormat = ippiBayerRGGB;
    else if (strcmp(cfaFormatName, "bggr") == 0) ippiCFAFormat = ippiBayerBGGR;
    else if (strcmp(cfaFormatName, "grbg") == 0) ippiCFAFormat = ippiBayerGRBG;
    else if (strcmp(cfaFormatName, "gbrg") == 0) ippiCFAFormat = ippiBayerGBRG;
    else { puts("invalid cfa format value.  Exiting."); exit(1); }

    char outFileName[1024]; 
    strcpy(outFileName,argv[3]);

    Ipp32s nthreads=atoi(argv[4]);

    if (nthreads<0) {puts("invalid nthreads value.  Exiting."); exit(1);}

    // Initialize IPP (set the dispatcher)
    ippInit();

    const IppLibraryVersion* lib = ippiGetLibVersion();
    printf("Version info: %s %s %d.%d.%d.%d\n", lib->Name, lib->Version,
        lib->major, lib->minor, lib->majorBuild, lib->build);

    // --------------------------
    // read the source image header and allocate image buffers
    // --------------------------
    sts=readPNMheader(inFileName, &pnminfo_src);
    if (ippStsNoErr!=sts)
    {
        printf("Error opening file\n");
        exit(1);
    }

    // ---------------------------
    // Set output image infromation
    // ---------------------------
    pnminfo_dst.imgtype= P6;  //output image type is color format
    pnminfo_dst.nChannels= 3; // output image # color format int channels
    pnminfo_dst.imgsize.width = pnminfo_src.imgsize.width;
    pnminfo_dst.imgsize.height = pnminfo_src.imgsize.height;

    // ---------------------------
    // Malloc source, temparary and dstination image buffer
    // ---------------------------
    pSrc = ippiMalloc_8u_C1(pnminfo_src.imgsize.width, pnminfo_src.imgsize.height, &stridesrc_8u); 
    pDst = ippiMalloc_8u_C3(pnminfo_dst.imgsize.width, pnminfo_dst.imgsize.height, &stridedst_8u);

    // ---------------------------
    // read the source image into the source image buffer
    // ---------------------------
    sts=readPNM_8u(inFileName, pSrc, stridesrc_8u, &pnminfo_src);                       
    if (ippStsNoErr != sts) {puts("Error: could not read source file. Exiting."); exit(1);}

    // ---------------------------
    // Resize init
    // ---------------------------
    tbb_start=tick_count::now();
    cycle_start = ippGetCpuClocks();

    printf(" [Src] width: %d, height: %d, bytesStep: %d\n", pnminfo_src.imgsize.width, pnminfo_src.imgsize.height, stridesrc_8u);
    printf(" [Dst] width: %d, height: %d, bytesStep: %d\n", pnminfo_dst.imgsize.width, pnminfo_dst.imgsize.height, stridedst_8u);

    IppiRect roi;
    roi.x = 0;
    roi.y = 0;
    roi.width = pnminfo_src.imgsize.width;
    roi.height = pnminfo_src.imgsize.height;
   
    cycle_end = ippGetCpuClocks();
    tbb_end=tick_count::now();

    printf(" init seconds: %f\n",(tbb_end-tbb_start).seconds());
    printf(" init clocks:  %f\n",double(cycle_end-cycle_start));

    // for most cases explicitly setting the number of threads is discouraged
    // the purpose here is simply to allow for scaling experiments
    // For general use these next lines are unnecessary
    if (0==nthreads)
        nthreads = task_scheduler_init::default_num_threads();
    task_scheduler_init init(nthreads);

    // ---------------------------
    // Debayer the image
    // ---------------------------
    tbb_start=tick_count::now();
    cycle_start = ippGetCpuClocks();
    
    //for (int bloop=0;bloop<BENCHLOOPS;bloop++)
    {
        int grainsize = 128;  // This is arbitrary.  Best setting is system and resolution dependent.
        parallel_for( blocked_range<int>( 0, pnminfo_dst.imgsize.height, grainsize ),
            [pSrc, pDst, stridesrc_8u, stridedst_8u, roi, ippiCFAFormat,
            pnminfo_src, pnminfo_dst]( const blocked_range<int>& range )
        {
            int strideTemp_8u;
            Ipp8u *pTempT;
            Ipp8u *pSrcT,*pDstT;
            IppiPoint dstOffset = {0, 0};

            // region is the full width of the image,
            // The height is set by TBB via range.size() 
            IppiSize  imageSizeT = {pnminfo_dst.imgsize.width,(int)range.size()};

            pTempT = ippiMalloc_8u_C1(imageSizeT.width + 6, 30, &strideTemp_8u);

            // given the destination offset, calculate the offset in the source image
            dstOffset.y = range.begin(); 

            // pointers to the starting points within the buffers that this thread
            // will read from/write to
            pSrcT=pSrc+(dstOffset.y*stridesrc_8u);
            pDstT=pDst+(dstOffset.y*stridedst_8u);

            // do the debayer from greyscale to color
            ippiDemosaicAHD_8u_C1C3R(
                pSrcT, roi, imageSizeT, stridesrc_8u,
                pDstT, stridedst_8u, 
                ippiCFAFormat,
                pTempT, strideTemp_8u);

            if (pTempT)     ippFree(pTempT);
        });
    }

    cycle_end = ippGetCpuClocks();
    tbb_end=tick_count::now();

    Ipp64u cycle_elapsed=(cycle_end-cycle_start)/*/(Ipp64u)BENCHLOOPS*/;

    printf(" debayer seconds: %f\n",(tbb_end-tbb_start).seconds() /*/ (BENCHLOOPS)*/);
    printf(" debayer clocks:  %f\n", (double)cycle_elapsed);
    printf(" debayer CpE:     %f\n", (double)cycle_elapsed / (double)(pnminfo_dst.imgsize.width*pnminfo_dst.imgsize.height*pnminfo_dst.nChannels));

    // write the output image to a file 
    sts=writePNM_8u(outFileName, pDst, stridedst_8u, &pnminfo_dst);
    if (ippStsNoErr != sts) {puts("Error writing output file. Exiting."); exit(1);}

    // clean up and exit
    if (pSrc)     ippFree(pSrc);
    if (pDst)     ippFree(pDst);

    return 0;
}