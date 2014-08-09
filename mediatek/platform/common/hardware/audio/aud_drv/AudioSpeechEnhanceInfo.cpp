#include "AudioSpeechEnhanceInfo.h"
#include <utils/Log.h>
#include <utils/String16.h>
#include "AudioUtility.h"
#include "AudioMTKStreamIn.h"
#include "SpeechEnhancementController.h"

#define LOG_TAG "AudioSpeechEnhanceInfo"

namespace android
{

AudioSpeechEnhanceInfo *AudioSpeechEnhanceInfo::UniqueAudioSpeechEnhanceInfoInstance = NULL;

AudioSpeechEnhanceInfo *AudioSpeechEnhanceInfo::getInstance()
{
    if (UniqueAudioSpeechEnhanceInfoInstance == NULL) {
        ALOGD("+AudioSpeechEnhanceInfo");
        UniqueAudioSpeechEnhanceInfoInstance = new AudioSpeechEnhanceInfo();
        ALOGD("-AudioSpeechEnhanceInfo");
    }
    ALOGD("getInstance()");
    return UniqueAudioSpeechEnhanceInfoInstance;
}

void AudioSpeechEnhanceInfo::freeInstance()
{
    return;
}

AudioSpeechEnhanceInfo::AudioSpeechEnhanceInfo()
{
    ALOGD("AudioSpeechEnhanceInfo constructor");    
    mHdRecScene = -1;
    mIsLRSwitch = false;
    mUseSpecificMic = 0;
    mHDRecTunningEnable = false;
#ifndef DMNR_TUNNING_AT_MODEMSIDE    
    mAPDMNRTuningEnable = false;
#endif
#ifdef MTK_VOIP_ENHANCEMENT_SUPPORT
    mEnableNormalModeVoIP = false;
#endif 
}

AudioSpeechEnhanceInfo::~AudioSpeechEnhanceInfo()
{
    ALOGD("AudioSpeechEnhanceInfo destructor");
    mHdRecScene = -1;
}


void AudioSpeechEnhanceInfo::SetRecordLRChannelSwitch(bool bIsLRSwitch)
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("SetRecordLRChannelSwitch=%x",bIsLRSwitch);
    mIsLRSwitch = bIsLRSwitch;
}

bool AudioSpeechEnhanceInfo::GetRecordLRChannelSwitch(void)
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("GetRecordLRChannelSwitch=%x",mIsLRSwitch);
    return mIsLRSwitch;
}

void AudioSpeechEnhanceInfo::SetUseSpecificMIC(int32 UseSpecificMic)
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("SetUseSpecificMIC=%x",UseSpecificMic);
    mUseSpecificMic = UseSpecificMic;
}

int AudioSpeechEnhanceInfo::GetUseSpecificMIC(void)
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("GetUseSpecificMIC=%x",mUseSpecificMic);
    return mUseSpecificMic;
}

//----------------for HD Record Preprocess-----------------------------
void AudioSpeechEnhanceInfo::SetHDRecScene(int32 HDRecScene)
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("AudioSpeechEnhanceInfo SetHDRecScene=%d",HDRecScene);
    mHdRecScene = HDRecScene;
}

int32 AudioSpeechEnhanceInfo::GetHDRecScene()
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("AudioSpeechEnhanceInfo GetHDRecScene=%d",mHdRecScene);
    return mHdRecScene;
}

void AudioSpeechEnhanceInfo::ResetHDRecScene()
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("AudioSpeechEnhanceInfo ResetHDRecScene");
    mHdRecScene = -1;
}

//----------------for HDRec tunning --------------------------------
void AudioSpeechEnhanceInfo::SetHDRecTunningEnable(bool bEnable)
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("SetHDRecTunningEnable=%d",bEnable);
    mHDRecTunningEnable = bEnable;
}

bool AudioSpeechEnhanceInfo::IsHDRecTunningEnable(void)
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("IsHDRecTunningEnable=%d",mHDRecTunningEnable);
    return mHDRecTunningEnable;
}

status_t AudioSpeechEnhanceInfo::SetHDRecVMFileName(const char *fileName)
{
    Mutex::Autolock lock(mHDRInfoLock);
    if (fileName!=NULL && strlen(fileName)<128-1) {
        ALOGD("SetHDRecVMFileName file name:%s", fileName);
        memset(mVMFileName, 0, 128);
        strcpy(mVMFileName,fileName);
    }else {
        ALOGD("input file name NULL or too long!");
        return BAD_VALUE;
    }
    return NO_ERROR;
}
void AudioSpeechEnhanceInfo::GetHDRecVMFileName(char * VMFileName)
{
    Mutex::Autolock lock(mHDRInfoLock);
    memset(VMFileName, 0, 128);
    strcpy(VMFileName,mVMFileName);
    ALOGD("GetHDRecVMFileName mVMFileName=%s, VMFileName=%s",mVMFileName,VMFileName);
}

#ifndef DMNR_TUNNING_AT_MODEMSIDE
//----------------for AP DMNR tunning --------------------------------
void AudioSpeechEnhanceInfo::SetAPDMNRTuningEnable(bool bEnable)
{
#ifdef MTK_DUAL_MIC_SUPPORT
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("SetAPDMNRTuningEnable=%d",bEnable);
    mAPDMNRTuningEnable = bEnable;
#else
    ALOGD("SetAPDMNRTuningEnable not Dual MIC, not set");
#endif
}

bool AudioSpeechEnhanceInfo::IsAPDMNRTuningEnable(void)
{
#ifdef MTK_DUAL_MIC_SUPPORT
    Mutex::Autolock lock(mHDRInfoLock);
    //ALOGD("IsAPDMNRTuningEnable=%d",mAPDMNRTuningEnable);
    return mAPDMNRTuningEnable;
#else
    return false;
#endif
}
#endif

#if defined(MTK_HANDSFREE_DMNR_SUPPORT) && defined(MTK_VOIP_ENHANCEMENT_SUPPORT)
//----------------Get MMI info for AP Speech Enhancement --------------------------------
bool AudioSpeechEnhanceInfo::GetDynamicSpeechEnhancementMaskOnOff(const sph_enh_dynamic_mask_t dynamic_mask_type)
{    
    bool bret = false;
    sph_enh_mask_struct_t mask;
    uint32_t dynamicMask;    
    mask = SpeechEnhancementController::GetInstance()->GetSpeechEnhancementMask();
    dynamicMask = mask.dynamic_func;    

    if(SpeechEnhancementController::GetInstance()->GetDynamicMask(dynamic_mask_type) == true)
        bret= true;
    ALOGD("%s(), %x, %x, bret=%d",__FUNCTION__,dynamicMask,dynamic_mask_type,bret);
    return bret;
}

void AudioSpeechEnhanceInfo::UpdateDynamicSpeechEnhancementMask()
{    
    Mutex::Autolock lock(mHDRInfoLock);
    sph_enh_mask_struct_t mask;
    mask = SpeechEnhancementController::GetInstance()->GetSpeechEnhancementMask();
    ALOGD("%s(), %x",__FUNCTION__,mask.dynamic_func);    
    
    if(mSPELayerVector.size())
    {                
        for (size_t i = 0; i < mSPELayerVector.size() ; i++)
        {
            AudioMTKStreamIn *pTempMTKStreamIn = (AudioMTKStreamIn *)mSPELayerVector.keyAt(i);                
            pTempMTKStreamIn->UpdateDynamicFunction();
        }
    }
}
#endif 

//----------------for Android Native Preprocess-----------------------------
void AudioSpeechEnhanceInfo::SetStreamOutPointer(void *pStreamOut)
{
    if (pStreamOut == NULL) {
        ALOGW(" SetStreamOutPointer pStreamOut = NULL");
    }
    else {
        mStreamOut = (AudioMTKStreamOut *)pStreamOut;
        ALOGW("SetStreamOutPointer mStreamOut=%p",mStreamOut);
    }
}

int AudioSpeechEnhanceInfo::GetOutputSampleRateInfo(void)
{
    int samplerate = 16000;
    samplerate = mStreamOut->GetSampleRate();
    ALOGD("AudioSpeechEnhanceInfo GetOutputSampleRateInfo=%d",samplerate);
    return samplerate;
}

int AudioSpeechEnhanceInfo::GetOutputChannelInfo(void)
{
    int chn = 1;
    chn = mStreamOut->GetChannel();
    ALOGD("AudioSpeechEnhanceInfo GetOutputChannelInfo=%d",chn);
    return chn;
}

bool AudioSpeechEnhanceInfo::IsOutputRunning(void)
{
    return mStreamOut->GetStreamRunning();
}

void AudioSpeechEnhanceInfo::add_echo_reference(struct echo_reference_itfe *reference)
{
    ALOGD("AudioSpeechEnhanceInfo add_echo_reference=%p",reference);
    mStreamOut->add_echo_reference(reference);
}
void AudioSpeechEnhanceInfo::remove_echo_reference(struct echo_reference_itfe *reference)
{
    ALOGD("AudioSpeechEnhanceInfo remove_echo_reference=%p",reference);
    mStreamOut->remove_echo_reference(reference);
}

#ifdef MTK_VOIP_ENHANCEMENT_SUPPORT
void AudioSpeechEnhanceInfo::SetOutputStreamRunning(bool bRun)
{

    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("SetOutputStreamRunning %d, SPELayer %d",bRun, mSPELayerVector.size());

    if (mSPELayerVector.size())
    {
        for (size_t i = 0; i < mSPELayerVector.size() ; i++)
        {
            SPELayer *pTempSPELayer = (SPELayer *)mSPELayerVector.valueAt(i);
            pTempSPELayer->SetOutputStreamRunning(bRun, true);
        }
    }
}
void AudioSpeechEnhanceInfo::SetSPEPointer(AudioMTKStreamIn * pMTKStreamIn, SPELayer *pSPE)
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("AudioSpeechEnhanceInfo SetSPEPointer %p, %p",pMTKStreamIn,pSPE);
    //mStreamOut->SetSPEPointer(pSPE);
    if (mSPELayerVector.size())
    {
        for (size_t i = 0; i < mSPELayerVector.size() ; i++)
        {
            if(pMTKStreamIn == mSPELayerVector.keyAt(i))
            {
                ALOGD("SetSPEPointer already add this before, not add it again");
                return;
            }
        }
    }
    pSPE->SetDownLinkLatencyTime(mStreamOut->latency());
    mSPELayerVector.add(pMTKStreamIn, pSPE);
    ALOGD("SetSPEPointer size %d",mSPELayerVector.size());
}

void AudioSpeechEnhanceInfo::ClearSPEPointer(AudioMTKStreamIn * pMTKStreamIn)
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("ClearSPEPointer %p, size=%d",pMTKStreamIn, mSPELayerVector.size());
    //mStreamOut->ClearSPEPointer();
    if (mSPELayerVector.size())
    {
        for (size_t i = 0; i < mSPELayerVector.size() ; i++)
        {
            if(pMTKStreamIn == mSPELayerVector.keyAt(i))
            {
                ALOGD("find and remove it ++");
                mSPELayerVector.removeItem(pMTKStreamIn);
                ALOGD("find and remove it --");
            }
        }
    }
}

bool AudioSpeechEnhanceInfo::IsInputStreamAlive(void)
{
    Mutex::Autolock lock(mHDRInfoLock);
    if(mSPELayerVector.size())
    {
        return true;
    }    
    return false;
}

//no argument, check if there is VoIP running input stream
//MTKStreamIn argument, check if the dedicated MTKStreamIn is VoIP running stream
bool AudioSpeechEnhanceInfo::IsVoIPActive(AudioMTKStreamIn * pMTKStreamIn)
{
    Mutex::Autolock lock(mHDRInfoLock);
    if(mSPELayerVector.size())
    {
        if(pMTKStreamIn == NULL)
        {
            //ALOGD("IsVoIPActive!");
            for (size_t i = 0; i < mSPELayerVector.size() ; i++)
            {
                AudioMTKStreamIn *pTempMTKStreamIn = (AudioMTKStreamIn *)mSPELayerVector.keyAt(i);                
                if(pTempMTKStreamIn->GetVoIPRunningState())
                    return true;                
            }
            return false;
        }
        else
        {
            for (size_t i = 0; i < mSPELayerVector.size() ; i++)
            {
                if(pMTKStreamIn == mSPELayerVector.keyAt(i))
                {
                    if(pMTKStreamIn->GetVoIPRunningState())
                        return true;
                }
            }
            return false;
        }
    }
    return false;
}

void AudioSpeechEnhanceInfo::GetDownlinkIntrStartTime(void)
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("GetDownlinkIntrStartTime %d",mSPELayerVector.size());
    if (mSPELayerVector.size())
    {
        for (size_t i = 0; i < mSPELayerVector.size() ; i++)
        {
            SPELayer *pTempSPELayer = (SPELayer *)mSPELayerVector.valueAt(i);
            pTempSPELayer->GetDownlinkIntrStartTime();
        }
    }
}

void AudioSpeechEnhanceInfo::WriteReferenceBuffer(struct InBufferInfo *Binfo)
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("WriteReferenceBuffer %d",mSPELayerVector.size());
    if (mSPELayerVector.size())
    {
        for (size_t i = 0; i < mSPELayerVector.size() ; i++)
        {
            SPELayer *pTempSPELayer = (SPELayer *)mSPELayerVector.valueAt(i);
            pTempSPELayer->WriteReferenceBuffer(Binfo);
        }
    }
}

void AudioSpeechEnhanceInfo::NeedUpdateVoIPParams(void)
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("NeedUpdateVoIPParams %d",mSPELayerVector.size());    
    if (mSPELayerVector.size())
    {
        for (size_t i = 0; i < mSPELayerVector.size() ; i++)
        {
            AudioMTKStreamIn *pTempMTKStreamIn = (AudioMTKStreamIn *)mSPELayerVector.keyAt(i);
            pTempMTKStreamIn->NeedUpdateVoIPParams();
        }
    }
}

void AudioSpeechEnhanceInfo::SetEnableNormalModeVoIP(bool bEnable)
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("SetEnableNormalModeVoIP=%d",bEnable);
    mEnableNormalModeVoIP = bEnable;    
}

bool AudioSpeechEnhanceInfo::GetEnableNormalModeVoIP(void)
{
    Mutex::Autolock lock(mHDRInfoLock);
    ALOGD("GetEnableNormalModeVoIP=%x",mEnableNormalModeVoIP);
    return mEnableNormalModeVoIP;
}
#endif

}

