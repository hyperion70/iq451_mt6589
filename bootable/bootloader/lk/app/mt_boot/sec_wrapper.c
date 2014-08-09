/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2011
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE. 
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

#include <platform/mt_typedefs.h>
#include <platform/sec_status.h>
#include <mt_partition.h>

#ifndef MTK_EMMC_SUPPORT
extern int nand_erase(u64 offset, u64 size);
#endif

char* sec_dev_get_part_r_name(u32 index)
{
    return g_part_name_map[index].r_name;
}

int sec_dev_read_wrapper(char *part_name, u64 offset, u8* data, u32 size)
{
    part_dev_t *dev;
    int index;      /* partition table index */
    long len;
    
    index = partition_get_index(part_name);
    if (index == -1)
        return PART_GET_INDEX_FAIL;

    dev = mt_part_get_device();    
   	if (!dev)	
   	    return PART_GET_DEV_FAIL;

#ifdef MTK_EMMC_SUPPORT
    len = dev->read(dev, offset, (uchar *) data, size);
#else
    len = dev->read(dev, (u32)offset, (uchar *) data, size);
#endif
    if (len != (int)size)
    {
        return PART_READ_FAIL;
    }

    return B_OK;
}


int sec_dev_write_wrapper(char *part_name, u64 offset, u8* data, u32 size)
{
    part_dev_t *dev;
    int index;      /* partition table index */
    long len;
    
    index = partition_get_index(part_name);
    if (index == -1)
        return PART_GET_INDEX_FAIL;

    dev = mt_part_get_device();    
   	if (!dev)	
   	    return PART_GET_DEV_FAIL;

#ifndef MTK_EMMC_SUPPORT
    if(nand_erase(offset,(u64)size)!=0){
        return PART_ERASE_FAIL;
    }
#endif    

#ifdef MTK_EMMC_SUPPORT
    len = dev->write(dev, (uchar *) data, offset, size);
#else
    len = dev->write(dev, (uchar *) data, (u32)offset, size);
#endif
    if (len != (int)size)
    {
        return PART_WRITE_FAIL;
    }

    return B_OK;
}


