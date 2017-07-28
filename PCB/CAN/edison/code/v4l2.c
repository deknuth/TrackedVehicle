#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/videodev2.h>
#include <linux/v4l2-mediabus.h>
#define CAMERA_DEVICE "/dev/video0"
#define CAPTURE_FILE "frame.bmp"

#define VIDEO_WIDTH  640
#define  VIDEO_HEIGHT  360
#define LEN (VIDEO_HEIGHT*VIDEO_WIDTH<<1)
#define BMP_SIZE (VIDEO_HEIGHT*VIDEO_WIDTH*sizeof(unsigned char)*3)
static int YUYVToRGB24(unsigned char* pYUV,unsigned char* pRGB24);
static void RGBToBMP(unsigned char* pdata, FILE* bmp_fd);
#define VIDEO_FORMAT V4L2_MBUS_FMT_YUYV8_2X8
#define BUFFER_COUNT 10

typedef struct VideoBuffer{
    void   *start;
    size_t  length;
}VideoBuffer;
int mask;
int main(void)
{
    int i, ret,count=0,fd,n_buffers,a,b;
    unsigned int total = 0;
    mask = (1<<((sizeof(int)<<3)-1));
    fd = open(CAMERA_DEVICE, O_RDWR, 0);
    if (fd < 0)
    {
        printf("Open %s failed\n", CAMERA_DEVICE);
        return -1;
    }
    struct v4l2_streamparm stream_parm;
    struct v4l2_capability cap;             // 获取驱动信息
    ioctl(fd, VIDIOC_G_PARM, &stream_parm);
    stream_parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(fd, VIDIOC_S_PARM, &stream_parm);

    if(ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0)
    {
        printf("VIDIOC_QUERYCAP failed (%d)\n", ret);
        return ret;
    }
    // Print capability infomations
    printf("Capability Informations:\n");
    printf(" driver: %s\n", cap.driver);
    printf(" card: %s\n", cap.card);
    printf(" bus_info: %s\n", cap.bus_info);
    printf(" version: %08X\n", cap.version);
    printf(" capabilities: %08X\n", cap.capabilities);
    printf("----------------------------\n");

    struct v4l2_format fmt;     // 设置视频格式
    memset(&fmt, 0, sizeof(fmt));
    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = VIDEO_WIDTH;
    fmt.fmt.pix.height      = VIDEO_HEIGHT;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field       = V4L2_FIELD_ALTERNATE;

    if (ioctl(fd, VIDIOC_S_FMT, &fmt) < 0)
    {
        printf("VIDIOC_S_FMT failed (%d)\n", ret);
        return ret;
    }

    if (ioctl(fd, VIDIOC_G_FMT, &fmt) < 0)  // 获取视频格式
    {
        printf("VIDIOC_G_FMT failed (%d)\n", ret);
        return ret;
    }

    printf("Stream Format Informations:\n");    // Print Stream Format
    printf(" type: %d\n", fmt.type);
    printf(" width: %d\n", fmt.fmt.pix.width);
    printf(" height: %d\n", fmt.fmt.pix.height);

    char fmtstr[8];
    memset(fmtstr, 0, 8);
    memcpy(fmtstr, &fmt.fmt.pix.pixelformat, 4);
    printf(" pixelformat: %s\n", fmtstr);
    printf(" field: %d\n", fmt.fmt.pix.field);
    printf(" bytesperline: %d\n", fmt.fmt.pix.bytesperline);
    printf(" sizeimage: %d\n", fmt.fmt.pix.sizeimage);
    printf(" colorspace: %d\n", fmt.fmt.pix.colorspace);
    printf(" priv: %d\n", fmt.fmt.pix.priv);
    printf(" raw_date: %s\n", fmt.fmt.raw_data);

    FILE *fp = fopen(CAPTURE_FILE, "wb");
    if (fp < 0)
    {
        printf("open frame data file failed\n");
        return -1;
    }
    // 请求分配内存
    struct v4l2_requestbuffers reqbuf;      // Memory Mapping模式
    reqbuf.count = BUFFER_COUNT;
    reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqbuf.memory = V4L2_MEMORY_MMAP;
    ret = ioctl(fd , VIDIOC_REQBUFS, &reqbuf);
    if(ret < 0)
    {
        printf("VIDIOC_REQBUFS failed (%d)\n", ret);
        return ret;
    }

    VideoBuffer*  framebuf = calloc(reqbuf.count, sizeof(VideoBuffer)); // 获取空间
    struct v4l2_buffer buf;
    for (n_buffers=0; n_buffers<reqbuf.count; n_buffers++)
    {
        buf.index = n_buffers;
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        if(ioctl(fd, VIDIOC_QUERYBUF,&buf) < 0)
        {
            printf("VIDIOC_QUERYBUF (%d) failed (%d)\n", n_buffers, ret);
            return ret;
        }
        // mmap buffer
        framebuf[n_buffers].length = buf.length;
        framebuf[n_buffers].start = (char *) mmap(0, buf.length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
        if(framebuf[n_buffers].start == MAP_FAILED)
        {
            printf("mmap (%d) failed: %s\n", n_buffers, strerror(errno));
            return -1;
        }
        // Queen buffer
        if (ioctl(fd , VIDIOC_QBUF, &buf) < 0)
        {
            printf("VIDIOC_QBUF (%d) failed (%d)\n", n_buffers, ret);
            return -1;
        }
        printf("Frame buffer %d: address=0x%x, length=%d\n", n_buffers, (unsigned int)framebuf[n_buffers].start, framebuf[n_buffers].length);
    }
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(fd, VIDIOC_STREAMON, &type);                //  开始捕捉  VIDIOC_STREAMOFF->结束捕捉
    if (ret < 0)
    {
        printf("VIDIOC_STREAMON failed (%d)\n", ret);
        return ret;
    }
    unsigned char *pRGB24 = malloc(BMP_SIZE);
    while(1)
    {
        fd_set fds;
        struct timeval tv;
        FD_ZERO(&fds);
        FD_SET(fd,&fds);
        tv.tv_sec = 2;
        tv.tv_usec = 0;
        ret = select(fd + 1,&fds, NULL, NULL, &tv);
        if (-1 == ret)
        {
            if (EINTR == errno)
                continue;
            else
                break;
        }
        if (0 == ret)
        {
            fprintf(stderr,"select timeout\n");
            break;
        }
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        if(ioctl(fd, VIDIOC_DQBUF, &buf) < 0)
        {
            printf("VIDIOC_DQBUF failed (%d)\n", ret);
            continue;
        }
        else        // 数据处理
        {
            total += buf.bytesused;
            a=clock();
            YUYVToRGB24(framebuf[buf.index].start,pRGB24);
    //        fwrite(framebuf[buf.index].start, 1, buf.bytesused, fp);   // Process the frame
   //         RGBToBMP(pRGB24, fp);
            b=clock();
            printf("se: %d\n",b-a);
      //      printf("write size[%d]: %d\n",buf.index,buf.bytesused);
      //      break;
        }
        if(ioctl(fd, VIDIOC_QBUF, &buf) < 0)   // Re-queen buffer
        {
            printf("VIDIOC_QBUF failed (%d)\n", ret);
            return ret;
        }
    }
    free(pRGB24);
    system("sync");
    printf("file size: %d\n",total);
    printf("Capture one frame saved in %s\n", CAPTURE_FILE);
    fclose(fp);
    close(fd);
    for(i=0; i<n_buffers; ++i)
        munmap(framebuf[i].start, framebuf[i].length);
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(fd, VIDIOC_STREAMOFF, &type);
    return 0;
}

 int V1[256]={-23060,-22880,-22700,-22519,-22339,-22159,-21979,-21799,-21619,-21439,-21258,-21078,-20898,-20718,-20538,-20358,-20177,-19997,-19817,-19637,-19457,-19277,-19096,-18916,-18736,-18556,-18376,-18196,-18015,-17835,-17655,-17475,-17295,-17115,-16935,-16754,-16574,-16394,-16214,-16034,-15854,-15673,-15493,-15313,-15133,-14953,-14773,-14592,-14412,-14232,-14052,-13872,-13692,-13511,-13331,-13151,-12971,-12791,-12611,-12431,-12250,-12070,-11890,-11710,-11530,-11350,-11169,-10989,-10809,-10629,-10449,-10269,-10088,-9908,-9728,-9548,-9368,-9188,-9007,-8827,-8647,-8467,-8287,-8107,-7927,-7746,-7566,-7386,-7206,-7026,-6846,-6665,-6485,-6305,-6125,-5945,-5765,-5584,-5404,-5224,-5044,-4864,-4684,-4503,-4323,-4143,-3963,-3783,-3603,-3423,-3242,-3062,-2882,-2702,-2522,-2342,-2161,-1981,-1801,-1621,-1441,-1261,-1080,-900,-720,-540,-360,-180,0,180,360,540,720,900,1080,1261,1441,1621,1801,1981,2161,2342,2522,2702,2882,3062,3242,3423,3603,3783,3963,4143,4323,4503,4684,4864,5044,5224,5404,5584,5765,5945,6125,6305,6485,6665,6846,7026,7206,7386,7566,7746,7927,8107,8287,8467,8647,8827,9007,9188,9368,9548,9728,9908,10088,10269,10449,10629,10809,10989,11169,11350,11530,11710,11890,12070,12250,12431,12611,12791,12971,13151,13331,13511,13692,13872,14052,14232,14412,14592,14773,14953,15133,15313,15493,15673,15854,16034,16214,16394,16574,16754,16935,17115,17295,17475,17655,17835,18015,18196,18376,18556,18736,18916,19096,19277,19457,19637,19817,19997,20177,20358,20538,20718,20898,21078,21258,21439,21619,21799,21979,22159,22339,22519,22700,22880};
 int V2[256]={-11745,-11653,-11562,-11470,-11378,-11286,-11195,-11103,-11011,-10919,-10828,-10736,-10644,-10552,-10461,-10369,-10277,-10185,-10093,-10002,-9910,-9818,-9726,-9635,-9543,-9451,-9359,-9268,-9176,-9084,-8992,-8901,-8809,-8717,-8625,-8533,-8442,-8350,-8258,-8166,-8075,-7983,-7891,-7799,-7708,-7616,-7524,-7432,-7341,-7249,-7157,-7065,-6974,-6882,-6790,-6698,-6606,-6515,-6423,-6331,-6239,-6148,-6056,-5964,-5872,-5781,-5689,-5597,-5505,-5414,-5322,-5230,-5138,-5046,-4955,-4863,-4771,-4679,-4588,-4496,-4404,-4312,-4221,-4129,-4037,-3945,-3854,-3762,-3670,-3578,-3487,-3395,-3303,-3211,-3119,-3028,-2936,-2844,-2752,-2661,-2569,-2477,-2385,-2294,-2202,-2110,-2018,-1927,-1835,-1743,-1651,-1559,-1468,-1376,-1284,-1192,-1101,-1009,-917,-825,-734,-642,-550,-458,-367,-275,-183,-91,0,91,183,275,367,458,550,642,734,825,917,1009,1101,1192,1284,1376,1468,1559,1651,1743,1835,1927,2018,2110,2202,2294,2385,2477,2569,2661,2752,2844,2936,3028,3119,3211,3303,3395,3487,3578,3670,3762,3854,3945,4037,4129,4221,4312,4404,4496,4588,4679,4771,4863,4955,5046,5138,5230,5322,5414,5505,5597,5689,5781,5872,5964,6056,6148,6239,6331,6423,6515,6606,6698,6790,6882,6974,7065,7157,7249,7341,7432,7524,7616,7708,7799,7891,7983,8075,8166,8258,8350,8442,8533,8625,8717,8809,8901,8992,9084,9176,9268,9359,9451,9543,9635,9726,9818,9910,10002,10093,10185,10277,10369,10461,10552,10644,10736,10828,10919,11011,11103,11195,11286,11378,11470,11562,11653};
 int U1[256]={-5660,-5616,-5572,-5527,-5483,-5439,-5395,-5351,-5306,-5262,-5218,-5174,-5129,-5085,-5041,-4997,-4953,-4908,-4864,-4820,-4776,-4731,-4687,-4643,-4599,-4555,-4510,-4466,-4422,-4378,-4333,-4289,-4245,-4201,-4157,-4112,-4068,-4024,-3980,-3935,-3891,-3847,-3803,-3759,-3714,-3670,-3626,-3582,-3537,-3493,-3449,-3405,-3361,-3316,-3272,-3228,-3184,-3139,-3095,-3051,-3007,-2963,-2918,-2874,-2830,-2786,-2741,-2697,-2653,-2609,-2564,-2520,-2476,-2432,-2388,-2343,-2299,-2255,-2211,-2166,-2122,-2078,-2034,-1990,-1945,-1901,-1857,-1813,-1768,-1724,-1680,-1636,-1592,-1547,-1503,-1459,-1415,-1370,-1326,-1282,-1238,-1194,-1149,-1105,-1061,-1017,-972,-928,-884,-840,-796,-751,-707,-663,-619,-574,-530,-486,-442,-398,-353,-309,-265,-221,-176,-132,-88,-44,0,44,88,132,176,221,265,309,353,398,442,486,530,574,619,663,707,751,796,840,884,928,972,1017,1061,1105,1149,1194,1238,1282,1326,1370,1415,1459,1503,1547,1592,1636,1680,1724,1768,1813,1857,1901,1945,1990,2034,2078,2122,2166,2211,2255,2299,2343,2388,2432,2476,2520,2564,2609,2653,2697,2741,2786,2830,2874,2918,2963,3007,3051,3095,3139,3184,3228,3272,3316,3361,3405,3449,3493,3537,3582,3626,3670,3714,3759,3803,3847,3891,3935,3980,4024,4068,4112,4157,4201,4245,4289,4333,4378,4422,4466,4510,4555,4599,4643,4687,4731,4776,4820,4864,4908,4953,4997,5041,5085,5129,5174,5218,5262,5306,5351,5395,5439,5483,5527,5572,5616};
 int U2[256]={-29147,-28919,-28691,-28463,-28236,-28008,-27780,-27553,-27325,-27097,-26870,-26642,-26414,-26186,-25959,-25731,-25503,-25276,-25048,-24820,-24592,-24365,-24137,-23909,-23682,-23454,-23226,-22998,-22771,-22543,-22315,-22088,-21860,-21632,-21404,-21177,-20949,-20721,-20494,-20266,-20038,-19810,-19583,-19355,-19127,-18900,-18672,-18444,-18216,-17989,-17761,-17533,-17306,-17078,-16850,-16622,-16395,-16167,-15939,-15712,-15484,-15256,-15028,-14801,-14573,-14345,-14118,-13890,-13662,-13435,-13207,-12979,-12751,-12524,-12296,-12068,-11841,-11613,-11385,-11157,-10930,-10702,-10474,-10247,-10019,-9791,-9563,-9336,-9108,-8880,-8653,-8425,-8197,-7969,-7742,-7514,-7286,-7059,-6831,-6603,-6375,-6148,-5920,-5692,-5465,-5237,-5009,-4781,-4554,-4326,-4098,-3871,-3643,-3415,-3187,-2960,-2732,-2504,-2277,-2049,-1821,-1593,-1366,-1138,-910,-683,-455,-227,0,227,455,683,910,1138,1366,1593,1821,2049,2277,2504,2732,2960,3187,3415,3643,3871,4098,4326,4554,4781,5009,5237,5465,5692,5920,6148,6375,6603,6831,7059,7286,7514,7742,7969,8197,8425,8653,8880,9108,9336,9563,9791,10019,10247,10474,10702,10930,11157,11385,11613,11841,12068,12296,12524,12751,12979,13207,13435,13662,13890,14118,14345,14573,14801,15028,15256,15484,15712,15939,16167,16395,16622,16850,17078,17306,17533,17761,17989,18216,18444,18672,18900,19127,19355,19583,19810,20038,20266,20494,20721,20949,21177,21404,21632,21860,22088,22315,22543,22771,22998,23226,23454,23682,23909,24137,24365,24592,24820,25048,25276,25503,25731,25959,26186,26414,26642,26870,27097,27325,27553,27780,28008,28236,28463,28691,28919};
 int Y[256]={0,128,256,384,512,640,768,896,1024,1152,1280,1408,1536,1664,1792,1920,2048,2176,2304,2432,2560,2688,2816,2944,3072,3200,3328,3456,3584,3712,3840,3968,4096,4224,4352,4480,4608,4736,4864,4992,5120,5248,5376,5504,5632,5760,5888,6016,6144,6272,6400,6528,6656,6784,6912,7040,7168,7296,7424,7552,7680,7808,7936,8064,8192,8320,8448,8576,8704,8832,8960,9088,9216,9344,9472,9600,9728,9856,9984,10112,10240,10368,10496,10624,10752,10880,11008,11136,11264,11392,11520,11648,11776,11904,12032,12160,12288,12416,12544,12672,12800,12928,13056,13184,13312,13440,13568,13696,13824,13952,14080,14208,14336,14464,14592,14720,14848,14976,15104,15232,15360,15488,15616,15744,15872,16000,16128,16256,16384,16512,16640,16768,16896,17024,17152,17280,17408,17536,17664,17792,17920,18048,18176,18304,18432,18560,18688,18816,18944,19072,19200,19328,19456,19584,19712,19840,19968,20096,20224,20352,20480,20608,20736,20864,20992,21120,21248,21376,21504,21632,21760,21888,22016,22144,22272,22400,22528,22656,22784,22912,23040,23168,23296,23424,23552,23680,23808,23936,24064,24192,24320,24448,24576,24704,24832,24960,25088,25216,25344,25472,25600,25728,25856,25984,26112,26240,26368,26496,26624,26752,26880,27008,27136,27264,27392,27520,27648,27776,27904,28032,28160,28288,28416,28544,28672,28800,28928,29056,29184,29312,29440,29568,29696,29824,29952,30080,30208,30336,30464,30592,30720,30848,30976,31104,31232,31360,31488,31616,31744,31872,32000,32128,32256,32384,32512,32640};
 int YUV[350]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255};

// R' = 1.164*(Y’-16) + 1.596*(Cr'-128) // R = Y+V1
// G' = 1.164*(Y’-16) - 0.813*(Cr'-128) - 0.392*(Cb'-128)   //  G = Y1-V2 - U1
// B' = 1.164*(Y’-16) + 2.017*(Cb'-128) // B = Y+U2

// R = Y + 1.4075 *（V-128）
// G = Y – 0.3455 *（U –128） – 0.7169 *（V –128）
// B = Y + 1.779 *（U – 128）


int YUYVToRGB24(unsigned char* pYUV,unsigned char* pRGB24)
{
    unsigned int i;
    int R1,G1,B1,R2,G2,B2;
    printf("pRGB24: %p\n",pRGB24);
    for(i=0; i<LEN; )
    {
        R1 = (Y[pYUV[i]]+V1[pYUV[i+3]])>>7;
        G1 = (Y[pYUV[i]]-U1[pYUV[i+1]]-V2[pYUV[i+3]])>>7;
        B1 = (Y[pYUV[i]]+U2[pYUV[i+1]])>>7;
        R2 = (Y[pYUV[i+2]]+V1[pYUV[i+3]])>>7;
        G2 = (Y[pYUV[i+2]]-U1[pYUV[i+1]]-V2[pYUV[i+3]])>>7;
        B2 = (Y[pYUV[i+2]]+U2[pYUV[i+1]])>>7;
        /*
        *(pRGB24++) = G1>255?255:(G1<0?0:G1);
        *(pRGB24++) = R1>255?255:(R1<0?0:R1);
        *(pRGB24++) = B1>255?255:(B1<0?0:B1);
        *(pRGB24++) = G2>255?255:(G2<0?0:G2);
        *(pRGB24++) = R2>255?255:(R2<0?0:R2);
        *(pRGB24++) = B2>255?255:(B2<0?0:B2);
        */
        *(pRGB24++) = (G1&mask)?0:255;//YUV[G1];
        *(pRGB24++) = (R1&mask)?0:255;//YUV[R1];
        *(pRGB24++) = (B1&mask)?0:255;//YUV[B1];
        *(pRGB24++) = (G2&mask)?0:YUV[G2];
        *(pRGB24++) = (R2&mask)?0:YUV[R2];
        *(pRGB24++) = (B2&mask)?0:YUV[B2];
        i += 4;
    }
    printf("pRGB24: %p\n",pRGB24);
    return 1;
}

typedef struct {
    unsigned short  bfType;
    unsigned int    bfSize;
    unsigned short  bfReserved1;
    unsigned short  bfReserved2;
    unsigned int    bfOffBits;
}BMPFILEHEADER_T;

typedef struct{
    unsigned int    biSize;
    long            biWidth;
    long            biHeight;
    unsigned short  biPlanes;
    unsigned short  biBitCount;
    unsigned int    biCompression;
    unsigned int    biSizeImage;
    long            biXPelsPerMeter;
    long            biYPelsPerMeter;
    unsigned int    biClrUsed;
    unsigned int    biClrImportant;
} BMPINFOHEADER_T;

void RGBToBMP(unsigned char* pdata, FILE* bmp_fd)
{
    // 位图第一部分，文件信息
    BMPFILEHEADER_T bfh;
    bfh.bfType = (unsigned short)0x4d42;  //bm
    bfh.bfSize = BMP_SIZE  // data size
            + sizeof( BMPFILEHEADER_T ) // first section size
            + sizeof( BMPINFOHEADER_T ) // second section size
            ;
    bfh.bfReserved1 = 0; // reserved
    bfh.bfReserved2 = 0; // reserved
    bfh.bfOffBits = sizeof( BMPFILEHEADER_T )+ sizeof( BMPINFOHEADER_T );//真正的数据的位置
    //    printf("bmp_head== %ld\n", bfh.bfOffBits);
    // 位图第二部分，数据信息
    BMPINFOHEADER_T bih;
    bih.biSize = sizeof(BMPINFOHEADER_T);
    bih.biWidth = VIDEO_WIDTH;
    bih.biHeight = -VIDEO_HEIGHT;//BMP图片从最后一个点开始扫描，显示时图片是倒着的，所以用-height，这样图片就正了
    bih.biPlanes = 1;//为1，不用改
    bih.biBitCount = 24;
    bih.biCompression = 0;//不压缩
    bih.biSizeImage = BMP_SIZE;
    bih.biXPelsPerMeter = 0;//像素每米
    bih.biYPelsPerMeter = 0;
    bih.biClrUsed = 0;//已用过的颜色，为0,与bitcount相同
    bih.biClrImportant = 0;//每个像素都重要
    fwrite( &bfh, 8, 1, bmp_fd);
    fwrite(&bfh.bfReserved2, sizeof(bfh.bfReserved2), 1, bmp_fd);
    fwrite(&bfh.bfOffBits, sizeof(bfh.bfOffBits), 1, bmp_fd);
    fwrite(&bih, sizeof(BMPINFOHEADER_T), 1, bmp_fd);
    fwrite(pdata, BMP_SIZE, 1, bmp_fd);
}

