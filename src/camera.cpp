/***************************************************************************************************
版权：2023-2024，Climber 
文件名：camera.cpp
介绍:打开设备以及相机初始化的相关操作
作者：
版本：2023.1.20.01
完成时间：2023.1.20
***************************************************************************************************/
/*--------------include----------------*/
#include "camera.h"
/*-------------------------------------*/

/*--------------define-----------------*/
#define MAX_PATH 255
/*-------------------------------------*/

/*--------------变量区-----------------*/
unsigned int g_nPayloadSize = 0;
void* handle = NULL;                //相机句柄
cv::Mat srcImageA, srcImageB;       //图像缓存区
mutex mutexBufferA, mutexBufferB;   //互斥锁
bool bufferAFull = false;           //缓存区满标志位
bool bufferBFull = false;
bool bufferRefreshA = false;        //缓存区刷新标志位
/*-------------------------------------*/

/***************************************************************************************************
版权：2023-2024，Climber 
函数名：RGB2BGR
介绍:色彩空间RGB转为BGR
参数：pRgbData（RGB格式图像数据）、nWidth（图像宽度）、nHeight（图像高度）
作者：
版本：2023.1.20.01
完成时间：2023.1.20
***************************************************************************************************/
int RGB2BGR(unsigned char* pRgbData, unsigned int nWidth, unsigned int nHeight){
    if (NULL == pRgbData){
        return MV_E_PARAMETER;
    }
    for (unsigned int j = 0; j < nHeight; j++){
        for (unsigned int i = 0; i < nWidth; i++){
            unsigned char red = pRgbData[j * (nWidth * 3) + i * 3];
            pRgbData[j * (nWidth * 3) + i * 3] = pRgbData[j * (nWidth * 3) + i * 3 + 2];
            pRgbData[j * (nWidth * 3) + i * 3 + 2] = red;
        }
    }
    return MV_OK;
}

/***************************************************************************************************
版权：2023-2024，Climber 
函数名：Convert2Mat
介绍: conver类型转为Mat类型
参数：pstImageInfo（输出帧的信息）、pData（图像数据）
作者：
版本：2023.1.20.01
完成时间：2023.1.20
***************************************************************************************************/
cv::Mat Convert2Mat(MV_FRAME_OUT_INFO_EX* pstImageInfo, unsigned char* pData){
    cv::Mat srcImage;
    if (pstImageInfo->enPixelType == PixelType_Gvsp_BayerRG8){
        srcImage = cv::Mat(pstImageInfo->nHeight, pstImageInfo->nWidth, CV_8UC1, pData);
        cv::cvtColor(srcImage,srcImage,cv::COLOR_BayerRG2RGB);
    }
    else if (pstImageInfo->enPixelType == PixelType_Gvsp_RGB8_Packed){
        RGB2BGR(pData, pstImageInfo->nWidth, pstImageInfo->nHeight);
        srcImage = cv::Mat(pstImageInfo->nHeight, pstImageInfo->nWidth, CV_8UC3, pData);
    }
    else if (pstImageInfo->enPixelType == PixelType_Gvsp_YUV422_YUYV_Packed){
        RGB2BGR(pData, pstImageInfo->nWidth, pstImageInfo->nHeight);
        srcImage = cv::Mat(pstImageInfo->nHeight, pstImageInfo->nWidth, CV_8UC3, pData);
    }
    else{
        printf("unsupported pixel format\n");
        CloseDevice();
        exit(-1);
    }
    if (NULL == srcImage.data){
    }
    else
        return srcImage;
}

/***************************************************************************************************
版权：2023-2024，Climber 
函数名：SetExposureTime
介绍: 设置相机曝光时间
参数：dfFloatValue（曝光时间）
作者：
版本：2023.1.20.01
完成时间：2023.1.20
***************************************************************************************************/
bool SetExposureTime(float dfFloatValue){
    // ch:调节这两个曝光模式，才能让曝光时间生效 | en:Set exposure mode to valid exposure time
    int nRet = MV_CC_SetEnumValue(handle, "ExposureMode", MV_EXPOSURE_MODE_TIMED);
    if (MV_OK != nRet){
        return false;
    }
    nRet = MV_CC_SetEnumValue(handle, "ExposureAuto", MV_EXPOSURE_AUTO_MODE_OFF);
    nRet = MV_CC_SetFloatValue(handle, "ExposureTime", dfFloatValue);
    if (MV_OK != nRet){
        return false;
    }
    return true;
}

/***************************************************************************************************
版权：2023-2024，Climber 
函数名：SetWidthtHeight
介绍: 设置采集图像的分辨率
参数：width（图像宽带）、height（图像高度）
作者：
版本：2023.1.20.01
完成时间：2023.1.20
***************************************************************************************************/
bool SetWidthtHeight(int width, int height){
    int nRet = MV_CC_SetIntValue(handle, "Width", width);
    if (MV_OK != nRet){
        printf("set Width fail!\n");
        return false;
    }
    nRet = MV_CC_SetIntValue(handle, "Height", height);
    if (MV_OK != nRet){
        printf("set Height fail!\n");
        return false;
    }
    return true;
}

/***************************************************************************************************
版权：2023-2024，Climber 
函数名：InitCamera
介绍: 相机初始化
参数：无
作者：
版本：2023.1.20.01
完成时间：2023.1.20
***************************************************************************************************/
bool InitCamera(){
    // Set trigger mode as off
    int nRet = MV_CC_SetEnumValue(handle, "TriggerMode", 0);
    if (MV_OK != nRet){
        printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet);
        CloseDevice();
        exit(-1);
    }
    nRet = MV_CC_SetPixelFormat(handle, PixelType_Gvsp_RGB8_Packed);
    if (nRet != MV_OK){
        printf("MV_CC_SetPixelFormat Failed!\n");
        printf("0x%x\n", nRet);
        CloseDevice();
        exit(-1);
    }
    if (!SetExposureTime(3000.0)){
        printf("SetExposureTime fail!\n");
        CloseDevice();
        exit(-1);
    }
    if (!SetWidthtHeight(1440, 1080)){
        printf("SetWidthtHeight Fail!\n");
        CloseDevice();
        exit(-1);
    }
    return true;
}

/***************************************************************************************************
版权：2023-2024，Climber 
函数名：OpenDevice
介绍: 打开设备
参数：无
作者：
版本：2023.1.20.01
完成时间：2023.1.20
***************************************************************************************************/
int OpenDevice(){
    int nRet = MV_OK;
    //void* handle = NULL;
    do
    {
        // Enum device
        MV_CC_DEVICE_INFO_LIST stDeviceList;
        memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
        nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList);
        if (MV_OK != nRet){
            printf("Enum Devices fail! nRet [0x%x]\n", nRet);
            break;
        }

        if (stDeviceList.nDeviceNum > 0){
            for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++){
                printf("[device %d]:\n", i);
                MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i];
                if (NULL == pDeviceInfo){
                    break;
                }
            }
        }
        else{
            printf("Find No Devices!\n");
            break;
        }

        // input the format to convert
        printf("[0] OpenCV_Mat\n");
        printf("[1] OpenCV_IplImage\n");
        printf("Please Input Format to convert:\n");
        unsigned int nFormat = 0;
        if (nFormat >= 2){
            printf("Input error!\n");
            return 0;
        }

        // select device to connect
        printf("Camera index(0-%d): 0\n", stDeviceList.nDeviceNum - 1);
        unsigned int nIndex = 0;
        if (nIndex >= stDeviceList.nDeviceNum){
            printf("Input error!\n");
            break;
        }

        // Select device and create handle
        nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]);
        if (MV_OK != nRet){
            printf("Create Handle fail! nRet [0x%x]\n", nRet);
            break;
        }

        // open device
        nRet = MV_CC_OpenDevice(handle);
        if (MV_OK != nRet){
            printf("Open Device fail! nRet [0x%x]\n", nRet);
            break;
        }

        // Detection network optimal package size(It only works for the GigE camera)
        if (stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE){
            int nPacketSize = MV_CC_GetOptimalPacketSize(handle);
            if (nPacketSize > 0){
                nRet = MV_CC_SetIntValue(handle, "GevSCPSPacketSize", nPacketSize);
                if (nRet != MV_OK){
                    printf("Warning: Set Packet Size fail nRet [0x%x]!", nRet);
                }
            }
            else{
                printf("Warning: Get Packet Size fail nRet [0x%x]!", nPacketSize);
            }
        }

        //相机参数初始化
       // InitCamera();

        // Get payload size
        MVCC_INTVALUE stParam;
        memset(&stParam, 0, sizeof(MVCC_INTVALUE));
        nRet = MV_CC_GetIntValue(handle, "PayloadSize", &stParam);
        if (MV_OK != nRet)
        {
            printf("Get PayloadSize fail! nRet [0x%x]\n", nRet);
            break;
        }
        g_nPayloadSize = stParam.nCurValue;
        // Start grab image
        nRet = MV_CC_StartGrabbing(handle);
        if (MV_OK != nRet)
        {
            printf("Start Grabbing fail! nRet [0x%x]\n", nRet);
        }
    }while (0);
}

/***************************************************************************************************
版权：2023-2024，Climber 
函数名：GetImgThread
介绍: 采集图像线程
参数：无
作者：
版本：2023.1.20.01
完成时间：2023.1.20
***************************************************************************************************/
bool GetImgThread(){
    cv::Mat img;
    MV_FRAME_OUT_INFO_EX stImageInfo = { 0 };
    memset(&stImageInfo, 0, sizeof(MV_FRAME_OUT_INFO_EX));
    unsigned char* pData = (unsigned char*)malloc(sizeof(unsigned char) * (g_nPayloadSize));
    if (pData == NULL){
        printf("Allocate memory failed.\n");
        return false;
    }
    
    // get one frame from camera with timeout=1000ms
    while (1){
        int nRet = MV_CC_GetOneFrameTimeout(handle, pData, g_nPayloadSize, &stImageInfo, 1000);
        
        if (nRet == MV_OK){
            /*printf("Get One Frame: Width[%d], Height[%d], nFrameNum[%d]\n",
                stImageInfo.nWidth, stImageInfo.nHeight, stImageInfo.nFrameNum);*/
        }
        else{
            free(pData);
            pData = NULL;
        }
        if (bufferAFull == false){
            mutexBufferA.lock();
            srcImageA = Convert2Mat(&stImageInfo, pData);
            bufferRefreshA = false;
            bufferAFull = true;
            mutexBufferA.unlock();
        }
        else{
            if (bufferBFull == false){
                mutexBufferB.lock();
                srcImageB = Convert2Mat(&stImageInfo, pData);
                bufferRefreshA = true;
                bufferBFull = true;
                mutexBufferB.unlock();
            }
            else{
                if (bufferRefreshA){
                    bufferAFull == false;
                    mutexBufferA.lock();
                    srcImageA = Convert2Mat(&stImageInfo, pData);
                    bufferRefreshA = false;
                    bufferAFull = true;
                    mutexBufferA.unlock();
                }
                else{
                    bufferBFull == false;
                    mutexBufferB.lock();
                    srcImageB = Convert2Mat(&stImageInfo, pData);
                    bufferRefreshA = true;
                    bufferBFull = true;
                    mutexBufferB.unlock();
                }
            }
        }
    }
    CloseDevice();
    return true;
}

/***************************************************************************************************
版权：2023-2024，Climber 
函数名：ReadImgBuffer
介绍: 读取图像缓存区
参数：srcImage（图像读取位置）
作者：
版本：2023.1.20.01
完成时间：2023.1.20
***************************************************************************************************/
bool ReadImgBuffer(cv::Mat& srcImage){
    int waitTimeCount = 0;
    while (true){
        if (bufferAFull == true){
            mutexBufferA.lock();
            srcImage = srcImageA;
            bufferAFull = false;
            mutexBufferA.unlock();
            break;
        }
        else{
            if (bufferBFull == true){
                mutexBufferB.lock();
                srcImage = srcImageB;
                bufferBFull = false;
                mutexBufferB.unlock();
                break;
            }
            else{
                cv::waitKey(10);
                waitTimeCount++;
                if (waitTimeCount == 100){
                    printf("ReadBuffer Over Time, waiting........\n");
                    return false;
                }
            }
        }
    }
    return true;
}

/***************************************************************************************************
版权：2023-2024，Climber 
函数名：CloseDevice
介绍: 关闭设备
参数：无
作者：
版本：2023.1.20.01
完成时间：2023.1.20
***************************************************************************************************/
int CloseDevice(){
    // Stop grab image
    int nRet = MV_CC_StopGrabbing(handle);
    if (MV_OK != nRet){
        printf("Stop Grabbing fail! nRet [0x%x]\n", nRet);
        return -1;
    }

    // Close device
    nRet = MV_CC_CloseDevice(handle);
    if (MV_OK != nRet){
        printf("ClosDevice fail! nRet [0x%x]\n", nRet);
        return -1;
    }

    // Destroy handle
    nRet = MV_CC_DestroyHandle(handle);
    if (MV_OK != nRet){
        printf("Destroy Handle fail! nRet [0x%x]\n", nRet);
        return -1;
    }


    if (nRet != MV_OK){
        if (handle != NULL){
            MV_CC_DestroyHandle(handle);
            handle = NULL;
        }
    }

    printf("Press a key to exit.\n");

    return 0;
}