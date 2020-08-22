#ifndef XMU_COMMON_H
#define XMU_COMMON_H

#define SUCCESS 0
#define FAILED 1

#define H264                    96
#define AAC						97

enum mem_distribution{
    venc_mem = 100 * 1024,
    vdec_mem = 100 * 1024,
    aenc_mem = 5 * 1024,
    adec_mem = 5 * 1024
};

// video param


#endif