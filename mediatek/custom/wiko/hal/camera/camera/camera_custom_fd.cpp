#include "camera_custom_fd.h"

void get_fd_CustomizeData(FD_Customize_PARA  *FDDataOut)
{    
    FDDataOut->FDThreadNum = 1;
    FDDataOut->FDThreshold = 32;
    FDDataOut->MajorFaceDecision = 1;
    FDDataOut->OTRatio = 900;
    FDDataOut->SmoothLevel = 1;
    FDDataOut->FDSkipStep = 4;
    FDDataOut->FDRefresh = 60;
}


