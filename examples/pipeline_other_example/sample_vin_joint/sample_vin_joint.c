/*
 * AXERA is pleased to support the open source community by making ax-samples available.
 *
 * Copyright (c) 2022, AXERA Semiconductor (Shanghai) Co., Ltd. All rights reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 *
 * https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software distributed
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 */

/*
 * Author: ZHEQIUSHUI
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include "ax_isp_api.h"
#include "common_sys.h"
#include "common_vin.h"
#include "common_cam.h"

#include "npu_common.h"
/* tuning server */
#ifdef TUNING_CTRL
#include "ax_nt_stream_api.h"
#include "ax_nt_ctrl_api.h"
#endif

#include "ax_interpreter_external_api.h"

#include "../sample_run_joint/sample_run_joint_post_process.h"
#include "../utilities/sample_log.h"
#include "../common/common_joint.h"

typedef enum
{
    SYS_CASE_NONE = -1,
    SYS_CASE_SINGLE_OS04A10 = 0,
    SYS_CASE_SINGLE_IMX334 = 1,
    SYS_CASE_SINGLE_GC4653 = 2,
    SYS_CASE_DUAL_OS04A10 = 3,
    SYS_CASE_SINGLE_OS08A20 = 4,
    SYS_CASE_SINGLE_OS04A10_ONLINE = 5,
    SYS_CASE_SINGLE_DVP = 6,
    SYS_CASE_SINGLE_BT601 = 7,
    SYS_CASE_SINGLE_BT656 = 8,
    SYS_CASE_SINGLE_BT1120 = 9,
    SYS_CASE_MIPI_YUV = 10,
    SYS_CASE_BUTT
} COMMON_SYS_CASE_E;

static COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleOs04a10Sdr[] = {
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_10BPP, 15}, /*vin raw10 use */
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_16BPP, 4},  /*vin raw16 use */
    {2688, 1520, 2688, AX_YUV420_SEMIPLANAR, 5},       /*vin nv21/nv21 use */
    {1920, 1080, 1920, AX_YUV420_SEMIPLANAR, 4},
    {1280, 720, 1280, AX_YUV420_SEMIPLANAR, 4},
};

static COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleOs04a10OnlineSdr[] = {
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_10BPP, 3}, /*vin raw10 use */
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_16BPP, 4}, /*vin raw16 use */
    {2688, 1520, 2688, AX_YUV420_SEMIPLANAR, 3},      /*vin nv21/nv21 use */
    {1920, 1080, 1920, AX_YUV420_SEMIPLANAR, 2},
    {1280, 720, 1280, AX_YUV420_SEMIPLANAR, 2},
};

static COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleOs04a10Hdr[] = {
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_10BPP, 17}, /*vin raw10 use */
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_16BPP, 5},  /*vin raw16 use */
    {2688, 1520, 2688, AX_YUV420_SEMIPLANAR, 6},       /*vin nv21/nv21 use */
    {1920, 1080, 1920, AX_YUV420_SEMIPLANAR, 6},
    {720, 576, 720, AX_YUV420_SEMIPLANAR, 6},
};

static COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleOs04a10OnlineHdr[] = {
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_10BPP, 6}, /*vin raw10 use */
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_16BPP, 4}, /*vin raw16 use */
    {2688, 1520, 2688, AX_YUV420_SEMIPLANAR, 3},      /*vin nv21/nv21 use */
    {1920, 1080, 1920, AX_YUV420_SEMIPLANAR, 2},
    {720, 576, 720, AX_YUV420_SEMIPLANAR, 2},
};

COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleImx334Sdr[] = {

    {3840, 2160, 3840, AX_FORMAT_BAYER_RAW_12BPP, 15}, /*vin raw10 use */
    {3840, 2160, 3840, AX_FORMAT_BAYER_RAW_16BPP, 5},  /*vin raw16 use */
    {3840, 2160, 3840, AX_YUV420_SEMIPLANAR, 6},       /*vin nv21/nv21 use */
    {1920, 1080, 1920, AX_YUV420_SEMIPLANAR, 3},
    {960, 540, 960, AX_YUV420_SEMIPLANAR, 3},

};

COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleImx334Hdr[] = {

    {3840, 2160, 3840, AX_FORMAT_BAYER_RAW_10BPP, 17}, /*vin raw10 use */
    {3840, 2160, 3840, AX_FORMAT_BAYER_RAW_16BPP, 5},  /*vin raw16 use */
    {3840, 2160, 3840, AX_YUV420_SEMIPLANAR, 6},       /*vin nv21/nv21 use */
    {1920, 1080, 1920, AX_YUV420_SEMIPLANAR, 6},
    {960, 540, 960, AX_YUV420_SEMIPLANAR, 6},

};

static COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleGc4653[] = {

    {2560, 1440, 2560, AX_FORMAT_BAYER_RAW_10BPP, 3}, /*vin raw10 use */
    {2560, 1440, 2560, AX_FORMAT_BAYER_RAW_16BPP, 4}, /*vin raw16 use */
    {2560, 1440, 2560, AX_YUV420_SEMIPLANAR, 2},      /*vin nv21/nv21 use */
    {1280, 720, 1280, AX_YUV420_SEMIPLANAR, 2},
    {640, 360, 640, AX_YUV420_SEMIPLANAR, 2},

};

static COMMON_SYS_POOL_CFG_T gtSysCommPoolDoubleOs04a10[] = {
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_10BPP, 15 * 2}, /*vin raw10 use */
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_16BPP, 5 * 2},  /*vin raw16 use */
    {2688, 1520, 2688, AX_YUV420_SEMIPLANAR, 6 * 2},       /*vin nv21/nv21 use */
    {1344, 760, 1344, AX_YUV420_SEMIPLANAR, 3 * 2},
    {1344, 760, 1344, AX_YUV420_SEMIPLANAR, 3 * 2},
};

COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleOs08a20Sdr[] = {

    {3840, 2160, 3840, AX_FORMAT_BAYER_RAW_12BPP, 15}, /*vin raw10 use */
    {3840, 2160, 3840, AX_FORMAT_BAYER_RAW_16BPP, 5},  /*vin raw16 use */
    {3840, 2160, 3840, AX_YUV420_SEMIPLANAR, 6},       /*vin nv21/nv21 use */
    {1920, 1080, 1920, AX_YUV420_SEMIPLANAR, 3},
    {960, 540, 960, AX_YUV420_SEMIPLANAR, 3},

};

COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleOs08a20Hdr[] = {

    {3840, 2160, 3840, AX_FORMAT_BAYER_RAW_10BPP, 17}, /*vin raw10 use */
    {3840, 2160, 3840, AX_FORMAT_BAYER_RAW_16BPP, 5},  /*vin raw16 use */
    {3840, 2160, 3840, AX_YUV420_SEMIPLANAR, 6},       /*vin nv21/nv21 use */
    {1920, 1080, 1920, AX_YUV420_SEMIPLANAR, 6},
    {960, 540, 960, AX_YUV420_SEMIPLANAR, 6},

};

static COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleDVP[] = {
    {1600, 300, 1600, AX_FORMAT_BAYER_RAW_8BPP, 40},  /*vin raw8 use */
    {1600, 300, 1600, AX_FORMAT_BAYER_RAW_16BPP, 5},  /*vin raw16 use */
    {1600, 300, 1600, AX_YUV422_INTERLEAVED_UYVY, 6}, /*vin nv21/nv21 use */
};

static COMMON_SYS_POOL_CFG_T gtSysCommPoolBT601[] = {
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_10BPP, 40}, /*vin raw10 use */
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_16BPP, 5},  /*vin raw16 use */
    {2688, 1520, 2688, AX_YUV422_INTERLEAVED_YUYV, 6}, /*vin nv21/nv21 use */
    {1920, 1080, 1920, AX_YUV422_INTERLEAVED_YUYV, 3},
    {1280, 720, 1280, AX_YUV422_INTERLEAVED_YUYV, 3},
};
static COMMON_SYS_POOL_CFG_T gtSysCommPoolBT656[] = {
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_10BPP, 40}, /*vin raw10 use */
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_16BPP, 5},  /*vin raw16 use */
    {2688, 1520, 2688, AX_YUV422_INTERLEAVED_YUYV, 6}, /*vin nv21/nv21 use */
    {1920, 1080, 1920, AX_YUV422_INTERLEAVED_YUYV, 3},
    {1280, 720, 1280, AX_YUV422_INTERLEAVED_YUYV, 3},
};
static COMMON_SYS_POOL_CFG_T gtSysCommPoolBT1120[] = {
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_10BPP, 40}, /*vin raw10 use */
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_16BPP, 5},  /*vin raw16 use */
    {2688, 1520, 2688, AX_YUV422_INTERLEAVED_YUYV, 6}, /*vin nv21/nv21 use */
    {1920, 1080, 1920, AX_YUV422_INTERLEAVED_YUYV, 3},
    {1280, 720, 1280, AX_YUV422_INTERLEAVED_YUYV, 3},
};

static COMMON_SYS_POOL_CFG_T gtSysCommPoolMIPI_YUV[] = {
    {3840, 2160, 3840, AX_FORMAT_BAYER_RAW_16BPP, 40}, /*vin raw16 use */
};

static CAMERA_T gCams[MAX_CAMERAS] = {0};

static volatile AX_S32 gLoopExit = 0;

static AX_S32 g_isp_force_loop_exit = 0;

pthread_mutex_t g_result_mutex;
sample_run_joint_results g_result_disp;

int SAMPLE_MAJOR_STREAM_WIDTH;
int SAMPLE_MAJOR_STREAM_HEIGHT;

sample_run_joint_models gModels = {
    .bRunJoint = AX_FALSE,
    .mMajor.JointAttr = {0},
    .mMinor.JointAttr = {0},
    .mMajor.JointHandle = NULL,
    .mMinor.JointHandle = NULL,
    .mMajor.ModelType = MT_UNKNOWN,
    .mMinor.ModelType = MT_UNKNOWN,
    .ModelType_Main = MT_UNKNOWN,
    .SAMPLE_ALGO_FORMAT = AX_YUV420_SEMIPLANAR,
    .SAMPLE_IVPS_ALGO_WIDTH = 960,
    .SAMPLE_IVPS_ALGO_HEIGHT = 540,
};

void *IspRun(void *args)
{
    AX_U32 i = (AX_U32)args;

    ALOGN("cam %d is running...\n", i);

    while (!g_isp_force_loop_exit)
    {
        if (gCams[i].bOpen)
            AX_ISP_Run(gCams[i].nPipeId);
    }
    return NULL;
}

static void *getYuv(void *arg)
{
    AX_S32 timeOutMs = 200;
    int retval = -1;
    AX_IMG_INFO_T img_info = {0};
    AX_VOID *pSrcVirAddr = NULL;
    // char *yuvarr = NULL;
    AX_U32 i = (AX_U32)arg;

    while (gCams[i].bOpen)
    {
        if (gLoopExit == 1)
        {
            ALOGN("getYuv exit\n");
            break;
        }
        /* you can open here for get frame debug */

        retval = AX_VIN_GetYuvFrame(gCams[i].nPipeId, AX_YUV_SOURCE_ID_MAIN, &img_info, timeOutMs);
        if (retval != 0)
        {
            ALOGE("return error! gCams[i].nPipeId = %d, retval = 0x%x\n", gCams[i].nPipeId, retval);
            usleep(1000 * 1000);
            continue;
        }
        usleep(20 * 1000);

        ALOGN(" gCams[i].nPipeId = %d, w/h %d %d, enImgFormat %d, u64SeqNum = %lld, u64PTS = %lld, u32FrameSize: %d, Phy=%llu Vir=%llu\n",
              gCams[i].nPipeId,
              img_info.tFrameInfo.stVFrame.u32Width, img_info.tFrameInfo.stVFrame.u32Height,
              img_info.tFrameInfo.stVFrame.enImgFormat,
              img_info.tFrameInfo.stVFrame.u64SeqNum, img_info.tFrameInfo.stVFrame.u64PTS,
              img_info.tFrameInfo.stVFrame.u32FrameSize,
              img_info.tFrameInfo.stVFrame.u64PhyAddr[0],
              img_info.tFrameInfo.stVFrame.u64VirAddr[0]);

        pSrcVirAddr = AX_SYS_Mmap(img_info.tFrameInfo.stVFrame.u64PhyAddr[0], img_info.tFrameInfo.stVFrame.u32FrameSize);
        AX_SYS_Munmap(pSrcVirAddr, img_info.tFrameInfo.stVFrame.u32FrameSize);

        AX_NPU_CV_Image tSrcFrame = {0};
        tSrcFrame.nWidth = img_info.tFrameInfo.stVFrame.u32Width;
        tSrcFrame.nHeight = img_info.tFrameInfo.stVFrame.u32Height;
        tSrcFrame.eDtype = AX_NPU_CV_FDT_NV12;
        tSrcFrame.tStride.nW = (0 == img_info.tFrameInfo.stVFrame.u32PicStride[0]) ? tSrcFrame.nWidth : img_info.tFrameInfo.stVFrame.u32PicStride[0];
        tSrcFrame.nSize = img_info.tFrameInfo.stVFrame.u32FrameSize; // t.tStride.nW * t.nHeight * 3 / 2;
        tSrcFrame.pPhy = img_info.tFrameInfo.stVFrame.u64PhyAddr[0];
        tSrcFrame.pVir = (AX_U8 *)pSrcVirAddr;

        if (gModels.bRunJoint)
        {
            static sample_run_joint_results pResults;

            gModels.SAMPLE_RESTORE_WIDTH = SAMPLE_MAJOR_STREAM_WIDTH;
            gModels.SAMPLE_RESTORE_HEIGHT = SAMPLE_MAJOR_STREAM_HEIGHT;

            sample_run_joint_inference_single_func(&gModels, &tSrcFrame, &pResults);

            if (0 == retval)
            {
                for (AX_U8 i = 0; i < pResults.nObjSize; i++)
                {
                    printf("%2d %16s [%4.2f,%4.2f,%4.2f,%4.2f]\n", i, pResults.mObjects[i].objname,
                           pResults.mObjects[i].bbox.x, pResults.mObjects[i].bbox.y, pResults.mObjects[i].bbox.w, pResults.mObjects[i].bbox.h);
                }
            }
        }

        AX_VIN_ReleaseYuvFrame(gCams[i].nPipeId, AX_YUV_SOURCE_ID_MAIN, &img_info);

        usleep(200 * 1000);
    }

    ALOGN("getYuv thread gCams[i].nPipeId=%d exit!\n", gCams[i].nPipeId);
    return 0;
}

AX_S32 SysRun()
{
    pthread_t tgetYuvThread;
    AX_S32 s32Ret = 0, i;
    g_isp_force_loop_exit = 0;
    for (i = 0; i < MAX_CAMERAS; i++)
    {
        if (gCams[i].bOpen)
        {
            pthread_create(&gCams[i].tIspProcThread, NULL, IspRun, (AX_VOID *)i);
            pthread_create(&tgetYuvThread, NULL, getYuv, (AX_VOID *)i);
        }
    }

    while (!gLoopExit)
    {
        sleep(1);
    }

    g_isp_force_loop_exit = 1;

    for (i = 0; i < MAX_CAMERAS; i++)
    {
        if (gCams[i].bOpen)
        {
            pthread_cancel(gCams[i].tIspProcThread);
            pthread_cancel(tgetYuvThread);
            s32Ret = pthread_join(gCams[i].tIspProcThread, NULL);
            if (s32Ret < 0)
            {
                ALOGE(" isp run thread exit failed, ret=0x%x.\n", s32Ret);
            }
            s32Ret = pthread_join(tgetYuvThread, NULL);
            if (s32Ret < 0)
            {
                ALOGE(" getyuv thread exit failed, ret=0x%x.\n", s32Ret);
            }
        }
    }

    return 0;
}

AX_VOID PrintHelp()
{
    printf("command:\n");
    printf("\t-p: yolov5 param file path\n");
    printf("\t-m: Joint model path\n");
    printf("\t-c: ISP Test Case:\n");
    printf("\t\t0: Single OS04A10\n");
    printf("\t\t1: Single IMX334\n");
    printf("\t\t2: Single GC4653\n");
    printf("\t\t3: DUAL OS04A10\n");
    printf("\t\t4: Single OS08A20\n");
    printf("\t\t5: Single OS04A10 Online\n");
    printf("\t\t6: Single DVP\n");
    printf("\t\t7: Single BT601\n");
    printf("\t\t8: Single BT656\n");
    printf("\t\t9: Single BT1120\n");
    printf("\t\t10: MIPI YUV\n");

    printf("\t-e: SDR/HDR Mode:\n");
    printf("\t\t1: SDR\n");
    printf("\t\t2: HDR 2DOL\n");
}

//允许外部调用
AX_VOID __sigExit(int iSigNo)
{
    // ALOGN("Catch signal %d!\n", iSigNo);
    gLoopExit = 1;
    return;
}
int main(int argc, char *argv[])
{
    optind = 0;
    gLoopExit = 0;
    g_isp_force_loop_exit = 0;
    memset(&gModels, 0, sizeof(gModels));
    memset(&g_result_disp, 0, sizeof(g_result_disp));
    memset(&gCams, 0, sizeof(gCams));

    gModels.bRunJoint = AX_FALSE;
    gModels.mMajor.JointHandle = NULL;
    gModels.mMinor.JointHandle = NULL;
    gModels.mMajor.ModelType = MT_UNKNOWN;
    gModels.mMinor.ModelType = MT_UNKNOWN;
    gModels.ModelType_Main = MT_UNKNOWN;
    gModels.SAMPLE_ALGO_FORMAT = AX_YUV420_SEMIPLANAR;
    gModels.SAMPLE_IVPS_ALGO_WIDTH = 960;
    gModels.SAMPLE_IVPS_ALGO_HEIGHT = 540;

    int c;
    int isExit = 0, i;
    COMMON_SYS_CASE_E eSysCase = SYS_CASE_NONE;
    COMMON_SYS_ARGS_T tCommonArgs = {0};
    AX_SNS_HDR_MODE_E eHdrMode = AX_SNS_LINEAR_MODE;
    SAMPLE_SNS_TYPE_E eSnsType = OMNIVISION_OS04A10;
    AX_S32 s32Ret = 0;

    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, __sigExit);

    if (argc < 2)
    {
        PrintHelp();
        exit(0);
    }

    while ((c = getopt(argc, argv, "p:m:c:e:h")) != -1)
    {
        isExit = 0;
        switch (c)
        {
        case 'm':
            strcpy(gModels.MODEL_PATH, optarg);
            gModels.bRunJoint = AX_TRUE;
            break;
        case 'p':
        {
            int ret = sample_run_joint_parse_param(optarg, &gModels);
            if (ret != 0)
            {
                ALOGE("sample_parse_param_det failed");
                isExit = 1;
            }
            break;
        }
        case 'c':
            eSysCase = (COMMON_SYS_CASE_E)atoi(optarg);
            break;
        case 'e':
            eHdrMode = (AX_SNS_HDR_MODE_E)atoi(optarg);
            break;
        case 'h':
            isExit = 1;
            break;
        default:
            isExit = 1;
            break;
        }
    }

    if (isExit)
    {
        PrintHelp();
        exit(0);
    }

    if (eSysCase >= SYS_CASE_BUTT || eSysCase <= SYS_CASE_NONE)
    {
        ALOGE("error case type\n");
        exit(0);
    }

    if (eSysCase == SYS_CASE_SINGLE_OS04A10)
    {
        tCommonArgs.nCamCnt = 1;
        eSnsType = OMNIVISION_OS04A10;
        COMMON_ISP_GetSnsConfig(OMNIVISION_OS04A10, &gCams[0].stSnsAttr, &gCams[0].stSnsClkAttr, &gCams[0].stDevAttr, &gCams[0].stPipeAttr, &gCams[0].stChnAttr);
        if (eHdrMode == AX_SNS_LINEAR_MODE)
        {
            tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolSingleOs04a10Sdr) / sizeof(gtSysCommPoolSingleOs04a10Sdr[0]);
            tCommonArgs.pPoolCfg = gtSysCommPoolSingleOs04a10Sdr;
        }
        else if (eHdrMode == AX_SNS_HDR_2X_MODE)
        {
            tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolSingleOs04a10Hdr) / sizeof(gtSysCommPoolSingleOs04a10Hdr[0]);
            tCommonArgs.pPoolCfg = gtSysCommPoolSingleOs04a10Hdr;
        }
    }
    else if (eSysCase == SYS_CASE_SINGLE_OS04A10_ONLINE)
    {
        tCommonArgs.nCamCnt = 1;
        eSnsType = OMNIVISION_OS04A10;
        COMMON_ISP_GetSnsConfig(OMNIVISION_OS04A10, &gCams[0].stSnsAttr, &gCams[0].stSnsClkAttr, &gCams[0].stDevAttr, &gCams[0].stPipeAttr, &gCams[0].stChnAttr);
        if (eHdrMode == AX_SNS_LINEAR_MODE)
        {
            tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolSingleOs04a10OnlineSdr) / sizeof(gtSysCommPoolSingleOs04a10OnlineSdr[0]);
            tCommonArgs.pPoolCfg = gtSysCommPoolSingleOs04a10OnlineSdr;
        }
        else if (eHdrMode == AX_SNS_HDR_2X_MODE)
        {
            tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolSingleOs04a10OnlineHdr) / sizeof(gtSysCommPoolSingleOs04a10OnlineHdr[0]);
            tCommonArgs.pPoolCfg = gtSysCommPoolSingleOs04a10OnlineHdr;
        }
        gCams[0].stPipeAttr.ePipeDataSrc = AX_PIPE_SOURCE_DEV_ONLINE;
        gCams[0].stChnAttr.tChnAttr[0].nDepth = 1;
        gCams[0].stChnAttr.tChnAttr[1].nDepth = 1;
        gCams[0].stChnAttr.tChnAttr[2].nDepth = 1;
    }
    else if (eSysCase == SYS_CASE_SINGLE_IMX334)
    {
        tCommonArgs.nCamCnt = 1;
        eSnsType = SONY_IMX334;
        COMMON_ISP_GetSnsConfig(SONY_IMX334, &gCams[0].stSnsAttr, &gCams[0].stSnsClkAttr, &gCams[0].stDevAttr, &gCams[0].stPipeAttr,
                                &gCams[0].stChnAttr);
        if (eHdrMode == AX_SNS_LINEAR_MODE)
        {
            tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolSingleImx334Sdr) / sizeof(gtSysCommPoolSingleImx334Sdr[0]);
            tCommonArgs.pPoolCfg = gtSysCommPoolSingleImx334Sdr;
            gCams[0].stSnsAttr.eRawType = AX_RT_RAW12;
            gCams[0].stDevAttr.ePixelFmt = AX_FORMAT_BAYER_RAW_12BPP;
            gCams[0].stPipeAttr.ePixelFmt = AX_FORMAT_BAYER_RAW_12BPP;
        }
        else
        {
            tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolSingleImx334Hdr) / sizeof(gtSysCommPoolSingleImx334Hdr[0]);
            tCommonArgs.pPoolCfg = gtSysCommPoolSingleImx334Hdr;
        }
    }
    else if (eSysCase == SYS_CASE_SINGLE_GC4653)
    {
        tCommonArgs.nCamCnt = 1;
        eSnsType = GALAXYCORE_GC4653;
        tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolSingleGc4653) / sizeof(gtSysCommPoolSingleGc4653[0]);
        tCommonArgs.pPoolCfg = gtSysCommPoolSingleGc4653;
        COMMON_ISP_GetSnsConfig(GALAXYCORE_GC4653, &gCams[0].stSnsAttr, &gCams[0].stSnsClkAttr, &gCams[0].stDevAttr, &gCams[0].stPipeAttr,
                                &gCams[0].stChnAttr);
    }
    else if (eSysCase == SYS_CASE_DUAL_OS04A10)
    {
        tCommonArgs.nCamCnt = 2;
        eSnsType = OMNIVISION_OS04A10;
        COMMON_ISP_GetSnsConfig(OMNIVISION_OS04A10, &gCams[0].stSnsAttr, &gCams[0].stSnsClkAttr, &gCams[0].stDevAttr, &gCams[0].stPipeAttr,
                                &gCams[0].stChnAttr);
        COMMON_ISP_GetSnsConfig(OMNIVISION_OS04A10, &gCams[1].stSnsAttr, &gCams[1].stSnsClkAttr, &gCams[1].stDevAttr, &gCams[1].stPipeAttr,
                                &gCams[1].stChnAttr);
        tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolDoubleOs04a10) / sizeof(gtSysCommPoolDoubleOs04a10[0]);
        tCommonArgs.pPoolCfg = gtSysCommPoolDoubleOs04a10;

        gCams[0].stSnsClkAttr.nSnsClkIdx = 0; /* mclk0 only by AX DEMO board, User defined */
        gCams[1].stSnsClkAttr.nSnsClkIdx = 2; /* mclk2 only by AX DEMO board, User defined */
    }
    else if (eSysCase == SYS_CASE_SINGLE_OS08A20)
    {
        tCommonArgs.nCamCnt = 1;
        eSnsType = OMNIVISION_OS08A20;
        COMMON_ISP_GetSnsConfig(OMNIVISION_OS08A20, &gCams[0].stSnsAttr, &gCams[0].stSnsClkAttr, &gCams[0].stDevAttr, &gCams[0].stPipeAttr,
                                &gCams[0].stChnAttr);
        if (eHdrMode == AX_SNS_LINEAR_MODE)
        {
            tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolSingleOs08a20Sdr) / sizeof(gtSysCommPoolSingleOs08a20Sdr[0]);
            tCommonArgs.pPoolCfg = gtSysCommPoolSingleOs08a20Sdr;
            gCams[0].stSnsAttr.eRawType = AX_RT_RAW12;
            gCams[0].stDevAttr.ePixelFmt = AX_FORMAT_BAYER_RAW_12BPP;
            gCams[0].stPipeAttr.ePixelFmt = AX_FORMAT_BAYER_RAW_12BPP;
        }
        else
        {
            tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolSingleOs08a20Hdr) / sizeof(gtSysCommPoolSingleOs08a20Hdr[0]);
            tCommonArgs.pPoolCfg = gtSysCommPoolSingleOs08a20Hdr;
        }
    }
    else if (eSysCase == SYS_CASE_SINGLE_DVP)
    {
        tCommonArgs.nCamCnt = 1;
        gCams[0].eSnsType = SENSOR_DVP;
        COMMON_ISP_GetSnsConfig(SENSOR_DVP, &gCams[0].stSnsAttr, &gCams[0].stSnsClkAttr, &gCams[0].stDevAttr,
                                &gCams[0].stPipeAttr, &gCams[0].stChnAttr);

        tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolSingleDVP) / sizeof(gtSysCommPoolSingleDVP[0]);
        tCommonArgs.pPoolCfg = gtSysCommPoolSingleDVP;
    }
    else if (eSysCase == SYS_CASE_SINGLE_BT601)
    {
        tCommonArgs.nCamCnt = 1;
        gCams[0].eSnsType = SENSOR_BT601;
        COMMON_ISP_GetSnsConfig(SENSOR_BT601, &gCams[0].stSnsAttr, &gCams[0].stSnsClkAttr, &gCams[0].stDevAttr, &gCams[0].stPipeAttr,
                                &gCams[0].stChnAttr);

        tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolBT601) / sizeof(gtSysCommPoolBT601[0]);
        tCommonArgs.pPoolCfg = gtSysCommPoolBT601;
    }
    else if (eSysCase == SYS_CASE_SINGLE_BT656)
    {
        tCommonArgs.nCamCnt = 1;
        gCams[0].eSnsType = SENSOR_BT656;
        COMMON_ISP_GetSnsConfig(SENSOR_BT656, &gCams[0].stSnsAttr, &gCams[0].stSnsClkAttr, &gCams[0].stDevAttr, &gCams[0].stPipeAttr,
                                &gCams[0].stChnAttr);

        tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolBT656) / sizeof(gtSysCommPoolBT656[0]);
        tCommonArgs.pPoolCfg = gtSysCommPoolBT656;
    }
    else if (eSysCase == SYS_CASE_SINGLE_BT1120)
    {
        tCommonArgs.nCamCnt = 1;
        gCams[0].eSnsType = SENSOR_BT1120;
        COMMON_ISP_GetSnsConfig(SENSOR_BT1120, &gCams[0].stSnsAttr, &gCams[0].stSnsClkAttr, &gCams[0].stDevAttr, &gCams[0].stPipeAttr,
                                &gCams[0].stChnAttr);

        tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolBT1120) / sizeof(gtSysCommPoolBT1120[0]);
        tCommonArgs.pPoolCfg = gtSysCommPoolBT1120;
    }
    else if (eSysCase == SYS_CASE_MIPI_YUV)
    {
        tCommonArgs.nCamCnt = 1;
        eSnsType = MIPI_YUV;
        COMMON_ISP_GetSnsConfig(MIPI_YUV, &gCams[0].stSnsAttr, &gCams[0].stSnsClkAttr, &gCams[0].stDevAttr, &gCams[0].stPipeAttr,
                                &gCams[0].stChnAttr);
        tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolMIPI_YUV) / sizeof(gtSysCommPoolMIPI_YUV[0]);
        tCommonArgs.pPoolCfg = gtSysCommPoolMIPI_YUV;
    }

    for (i = 0; i < tCommonArgs.nCamCnt; i++)
    {
        gCams[i].eSnsType = eSnsType;
        gCams[i].stSnsAttr.eSnsMode = eHdrMode;
        gCams[i].stDevAttr.eSnsMode = eHdrMode;
        gCams[i].stPipeAttr.eSnsMode = eHdrMode;

        if (i == 0)
        {
            gCams[i].nDevId = 0;
            gCams[i].nRxDev = AX_MIPI_RX_DEV_0;
            gCams[i].nPipeId = 0;
        }
        else if (i == 1)
        {
            gCams[i].nDevId = 2;
            gCams[i].nRxDev = AX_MIPI_RX_DEV_2;
            gCams[i].nPipeId = 2;
        }
    }

    SAMPLE_MAJOR_STREAM_WIDTH = gCams[0].stChnAttr.tChnAttr[AX_YUV_SOURCE_ID_MAIN].nWidth;
    SAMPLE_MAJOR_STREAM_HEIGHT = gCams[0].stChnAttr.tChnAttr[AX_YUV_SOURCE_ID_MAIN].nHeight;
    printf("%d %d\n", SAMPLE_MAJOR_STREAM_WIDTH, SAMPLE_MAJOR_STREAM_HEIGHT);

    AX_NPU_SDK_EX_ATTR_T sNpuAttr;
    sNpuAttr.eHardMode = AX_NPU_VIRTUAL_1_1;
    s32Ret = AX_NPU_SDK_EX_Init_with_attr(&sNpuAttr);
    if (0 != s32Ret)
    {
        ALOGE("AX_NPU_SDK_EX_Init_with_attr failed, ret=0x%x.\n", s32Ret);
        return -1;
    }

    s32Ret = COMMON_SYS_Init(&tCommonArgs);
    if (s32Ret)
    {
        ALOGE("isp sys init fail\n");
        goto EXIT;
    }

    s32Ret = COMMON_JOINT_Init(&gModels, SAMPLE_MAJOR_STREAM_WIDTH, SAMPLE_MAJOR_STREAM_HEIGHT);
    if (0 != s32Ret)
    {
        ALOGE("COMMON_JOINT_Init failed,s32Ret:0x%x\n", s32Ret);
        goto EXIT;
    }

    COMMON_CAM_Init();

    for (i = 0; i < tCommonArgs.nCamCnt; i++)
    {
        if (eSysCase == SYS_CASE_SINGLE_DVP || eSysCase == SYS_CASE_SINGLE_BT601 || eSysCase == SYS_CASE_SINGLE_BT656 || eSysCase == SYS_CASE_SINGLE_BT1120 || eSysCase == SYS_CASE_MIPI_YUV)
        {
            s32Ret = COMMON_CAM_DVP_Open(&gCams[i]);
        }
        else
        {
            s32Ret = COMMON_CAM_Open(&gCams[i]);
        }
        if (s32Ret)
            goto EXIT;
        gCams[i].bOpen = AX_TRUE;
        ALOGN("camera %d is open\n", i);
    }

#ifdef TUNING_CTRL
    /* Net Preview */
    ALOGN("Start the service on the tuning device side.\n");

    s32Ret = AX_NT_StreamInit(6000);
    if (0 != s32Ret)
    {
        ALOGE("AX_NT_StreamInit failed, ret=0x%x.\n", s32Ret);
        return -1;
    }
    s32Ret = AX_NT_CtrlInit(8082);
    if (0 != s32Ret)
    {
        ALOGE("AX_NT_CtrlInit failed, ret=0x%x.\n", s32Ret);
        return -1;
    }

    for (i = 0; i < tCommonArgs.nCamCnt; i++)
    {
        AX_NT_SetStreamSource(gCams[i].nPipeId);
    }
    ALOGN("tuning runing.\n");
#endif

    SysRun();

EXIT:
    COMMON_JOINT_Deinit(&gModels);
    for (i = 0; i < MAX_CAMERAS; i++)
    {
        if (!gCams[i].bOpen)
            continue;
        if (eSysCase == SYS_CASE_SINGLE_DVP || eSysCase == SYS_CASE_SINGLE_BT601 || eSysCase == SYS_CASE_SINGLE_BT656 || eSysCase == SYS_CASE_SINGLE_BT1120)
        {
            COMMON_CAM_DVP_Close(&gCams[i]);
        }
        else
        {
            COMMON_CAM_Close(&gCams[i]);
        }
    }

#ifdef TUNING_CTRL
    AX_NT_CtrlDeInit();
    AX_NT_StreamDeInit();
#endif

    COMMON_CAM_Deinit();
    COMMON_SYS_DeInit();

    /* exit */
    exit(0);
}
