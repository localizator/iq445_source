/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#ifndef __MDP_ELEMENT_H__
#define __MDP_ELEMENT_H__

//Data Types
#include "mdp_datatypes.h"
//Register Base
#include "mdp_reg.h"
//Kernel Includes
#include "mt_mdp.h" //For MDP Element ID
//mdp driver functions(include here for backward compatible)
#include "mdp_drv.h"


#define MDP_MAX_RINGBUFFER_CNT  16

#define RESZ_CRZ_MAX_MINWIDTH   (3328)
#define RESZ_PRZ0_MAX_MINWIDTH  (1280)
#define RESZ_PRZ1_MAX_MINWIDTH  (960)



/*/////////////////////////////////////////////////////////////////////////////
    Global MDP Function
  /////////////////////////////////////////////////////////////////////////////*/
  /*
    MdpDrvInit();

        MdpDrvXXX();    //MdpDrv Operation family between MdpDrvInit() & MdpDrvRelease();

        MdpBegin();
            .   //MDP element operation (reg r/w) between MdpBegin() & MdpEnd()
            .   //MDP element operation (reg r/w) between MdpBegin() & MdpEnd()
        MdpEnd();

        MdpBegin();
            .   //MDP element operation (reg r/w) between MdpBegin() & MdpEnd()
            .   //MDP element operation (reg r/w) between MdpBegin() & MdpEnd()
        MdpEnd();

    MdpDrvRelease();
   */




/***MDP Element ctor/dtor. Embrace "MDP Element Register R/W Operation" ***/
class MDPELEMENT_I;
int MdpBegin( MDPELEMENT_I* mdp_element_list[] , int mdp_element_count, const char* mdp_path_name );       //Begin a series Mdp Element operation
int MdpEnd( MDPELEMENT_I* mdp_element_list[] , int mdp_element_count );         //End a series Mdp Element









/*/////////////////////////////////////////////////////////////////////////////
    MDPELEMENT_I
  /////////////////////////////////////////////////////////////////////////////*/
class MDPELEMENT_I
{
private:
    static unsigned long    m_mmsys1_reg_base_addr;   //maybe virtual address
    unsigned long           m_reg_base_addr;          //maybe virtual address
    unsigned long           m_descript_base_addr;     //Dynamic alloc
    
public:
    MDPELEMENT_I()
        {
            /*m_mmsys1_reg_base_addr = mmsys1_reg_base_addr_pa();*/
            m_reg_base_addr         = 0x0;
            m_descript_base_addr    = 0x0;
        }
    
    virtual ~MDPELEMENT_I(){};

public:
    virtual unsigned long   id( void ) = 0;
    unsigned long           dec_id( void );         //Decimal ID number
    virtual const char*     name_str( void ) = 0;   //Name string of module

    static unsigned long    mmsys1_reg_base_addr( void )    {   return m_mmsys1_reg_base_addr;   }
    static unsigned long    mmsys1_reg_base_addr_pa( void ) {   return MT6575_MMSYS1_BASE;  }//Phy reg base address, usually a fixed value
    static unsigned long    mmsys1_reg_range( void )        {   return MT6575_MMSYS1_REG_RANGE;  }
    
    unsigned long           reg_base_addr( void )           {   return m_reg_base_addr;   }
    virtual unsigned long   reg_base_addr_pa( void ) = 0;      //Phy reg base address, usually a fixed value
    virtual unsigned long   reg_range( void ) = 0; 

    unsigned long           descript_base_addr( void )      {   return m_descript_base_addr;   }
    virtual int             is_bypass( void ) = 0;


public:
    static void Remap_mmsys1_reg_base_addr( unsigned long new_addr )   {   m_mmsys1_reg_base_addr = new_addr;  }
    void        Remap_reg_base_addr( unsigned long new_addr )          {   m_reg_base_addr = new_addr;         }
    void        Remap_descript_base_addr( unsigned long new_addr )     {   m_descript_base_addr = new_addr;    }

public:
    /*
        Config()/Enable()/Disable() will call below functions sequencly 
            1. _ConfigPre( void );  //virtual
            2. _Config( void );
            3. _ConfigPost( void ); //virtual
    */
    int Config( void );
    int Enable( void );
    int Disable( void );

    int DumpRegister( int mode );   //mode:  0: dump mdp element register   1: dump mmsys1 register

    int WaitIntDone( unsigned long timeout_ms );        //Return 0: Done -1:timeout (Wait event trigger by interrupt)
    virtual int CheckBusy( unsigned long* param ) = 0;  //Return 1: Busy 0:Free ( Check busy bit by means of polling )

    /*Special Register Read/Write function*/
    static void             RegisterWrite32_Mmsys1( unsigned long offset, unsigned long value );
    static unsigned long    RegisterRead32_Mmsys1( unsigned long offset );
    void            RegisterWrite32( unsigned long offset, unsigned long value );
    unsigned long   RegisterRead32( unsigned long offset );


protected:
    virtual int _ConfigPre( void ) = 0;     //Instance Config (Pre)
    virtual int _Config( void ) = 0;        //Core Config
    virtual int _ConfigPost( void ) = 0;    //Instance Config (Post)
    
    virtual int _EnablePre( void ) = 0;
    virtual int _Enable( void ) = 0;
    virtual int _EnablePost( void ) = 0;
    
    virtual int _DisablePre( void ) = 0;
    virtual int _Disable( void ) = 0;
    virtual int _DisablePost( void ) = 0;

    virtual int Reset( void ) = 0;

    virtual int DumpRegisterCustom( int mode ) = 0;   //mode:  0: dump mdp element register   1: dump mmsys1 register
    
public:
    virtual int HardReset( void ) = 0;
    
    virtual int DumpDebugInfo( void );

private:
    int Mmsys1DumpRegisterCustom( void );
    
    
};


/*/////////////////////////////////////////////////////////////////////////////
    RDMA
  /////////////////////////////////////////////////////////////////////////////*/
#define MT6575RDMA_MAX_RINGBUFFER_CNT MDP_MAX_RINGBUFFER_CNT

class RDMA_I:public MDPELEMENT_I
{
public:/*[member veriable]*/
    unsigned long   uInBufferCnt:8;
    MdpYuvAddr      src_img_yuv_addr[MT6575RDMA_MAX_RINGBUFFER_CNT];
    MdpSize         src_img_size;
    MdpRect         src_img_roi;
    MdpColorFormat  src_color_format;

    //unsigned long   bContinuousHWTrigger:1;// HW trigger + continous + framesync
    unsigned long   bHWTrigger:1;// HW trigger + continous + framesync
    unsigned long   bContinuous:1;// HW trigger + continous + framesync
    unsigned long   bCamIn:1;// HW trigger + continous + framesync


    unsigned long   bEISEn:1;
    unsigned long   u4EISCON;//EIS setting

private:
    /*Internal Use Only*/
    MdpYuvAddr      src_img_yuv_adapt_addr[MT6575RDMA_MAX_RINGBUFFER_CNT];
    unsigned long   adapt_m4u_flag_bit_;
    unsigned long   alloc_mva_flag_bit_;
    unsigned long   src_img_buffer_size_total_;

private:
    MdpMemBuffer    sysram_;    //Allocated sysram for this element usage


private:/*For descriptor*/
    MdpYuvAddr      desc_src_img_yuv_addr_[MT6575RDMA_MAX_RINGBUFFER_CNT];
    unsigned long   desc_sw_write_index_;
    
    

public: /*ctor/dtor*/
    RDMA_I():
        uInBufferCnt(1),
        bHWTrigger(0),
        bContinuous(0),
        bCamIn(0),
        bEISEn(0),
        u4EISCON(0),
        adapt_m4u_flag_bit_(0),
        alloc_mva_flag_bit_(0),
        src_img_buffer_size_total_(0),
        desc_sw_write_index_(0)
        {};


public: /*[MDPELEMENT_I]*/
    virtual int is_bypass( void )                   {   return 0;       }
    virtual int CheckBusy( unsigned long* param );

private:/*[MDPELEMENT_I]*/
    virtual int _Config( void );
    virtual int _Enable( void );
    virtual int _Disable( void );

    virtual int Reset( void );
    
    virtual int DumpRegisterCustom( int mode );   //mode:  0: dump mdp element register   1: dump mmsys1 register
        
public:
    virtual int HardReset( void );
    virtual int DumpDebugInfo( void );

public: /*[RDMA_I]*/
    unsigned long   desc_sw_write_index( void ) {   return desc_sw_write_index_;    }
    int             DescQueueGetFreeList( unsigned long *p_StartIndex, unsigned long *p_Count );/*Equel to De Queue Buffer in 73*/
    int             DescQueueRefill( void );


private: /*[RDMA_I]*/
    inline int DescWriteWaitBusy( unsigned long reg_base );

public:
    int     CalcBufferSize( void );


};
/*/////////////////////////////////////////////////////////////////////////////
    RDMA0
  /////////////////////////////////////////////////////////////////////////////*/

class RDMA0:public RDMA_I
{
public:/*[member veriable]*/
    /*output sel*/
    unsigned long to_cam:1;
    unsigned long to_ovl:1;
    unsigned long to_mout:1;
    
    /*trigger sel*/
    unsigned long trigger_src:2;    /*0:VDO_ROT0 1:RGB_ROT0 2:RGB_ROT1 3:VDO_ROT1*/    

public:/*ctor/dtor*/
    RDMA0():
        to_cam(0),
        to_ovl(0),
        to_mout(0),
        trigger_src(0)
        {  }

public:/*[MDPELEMENT_I]*/
    virtual unsigned long id( void )                {    return     MID_R_DMA0;  } 
    virtual const char*   name_str( void )          {    return     "MID_R_DMA0";}
    virtual unsigned long reg_base_addr_pa( void )  {    return     MT6575_RDMA0_BASE;     }
    virtual unsigned long reg_range( void )         {    return     MT6575_RDMA0_REG_RANGE;     }


protected:/*[RDMA_I]*/
    virtual int _ConfigPre( void );
    virtual int _ConfigPost( void );
    virtual int _EnablePre( void );
    virtual int _EnablePost( void );
    virtual int _DisablePre( void );
    virtual int _DisablePost( void );
    
};

/*/////////////////////////////////////////////////////////////////////////////
    RDMA1
  /////////////////////////////////////////////////////////////////////////////*/

class RDMA1:public RDMA_I
{
public:/*[member veriable]*/
    /*Nothing to add*/

public:/*ctor/dtor*/
    RDMA1()     {     }


public:/*[MDPELEMENT_I]*/
    virtual unsigned long id( void )                {   return  MID_R_DMA1;  }
    virtual const char*   name_str( void )          {   return  "MID_R_DMA1";}
    virtual unsigned long reg_base_addr_pa( void )  {   return  MT6575_RDMA1_BASE;     }
    virtual unsigned long reg_range( void )         {   return  MT6575_RDMA1_REG_RANGE;     }


protected:/*[RDMA_I]*/
    virtual int _ConfigPre( void );
    virtual int _ConfigPost( void );
    virtual int _EnablePre( void );
    virtual int _EnablePost( void );
    virtual int _DisablePre( void );
    virtual int _DisablePost( void );


};


/*/////////////////////////////////////////////////////////////////////////////
    MOUT
  /////////////////////////////////////////////////////////////////////////////*/
class MOUT:public MDPELEMENT_I
{
public:/*[member veriable]*/
    /*input sel*/
    unsigned long src_sel:1;// 0-R_DMA0_MOUT, 1-OVL_DMA_MIMO

    /*output sel*/
    unsigned long to_jpg_dma:1;//bit[0] : JPEG_DMA, bit[1] : PRZ0, bit[2] : VRZ,bit[3] : VDO_ROT1
    unsigned long to_prz0:1;
    unsigned long to_vrz0:1;
    unsigned long to_prz1:1;

    unsigned long bCamIn:1;

public:/*ctor/dtor*/
    MOUT():
        src_sel(0),
        to_jpg_dma(0),
        to_prz0(0),
        to_vrz0(0),
        to_prz1(0),
        bCamIn(0)
        {     }

public: /*[MDPELEMENT_I]*/
    virtual int is_bypass( void )                   {   return 0;       }
    virtual int CheckBusy( unsigned long* param );


public:/*[MDPELEMENT_I]*/
    virtual unsigned long id( void )                    {   return MID_MOUT;  } 
    virtual const char*   name_str( void )              {   return "MID_MOUT";}
    virtual unsigned long reg_base_addr_pa( void )      {   return MT6575_MMSYS1_BASE;  }
    virtual unsigned long reg_range( void )             {   return MT6575_MMSYS1_REG_RANGE;  }

protected:/*[MDPELEMENT_I]*/
    virtual int _ConfigPre( void )  {   return 0; } //Instance Config (Pre)
    virtual int _Config( void );                    //Core Config
    virtual int _ConfigPost( void ) {   return 0; } //Instance Config (Post)
    
    virtual int _EnablePre( void )      {   return 0; }
    virtual int _Enable( void );
    virtual int _EnablePost( void )     {   return 0; }
    
    virtual int _DisablePre( void )     {   return 0; }
    virtual int _Disable( void );
    virtual int _DisablePost( void )    {   return 0; }

    virtual int Reset( void );
    virtual int DumpRegisterCustom( int mode );   //mode:  0: dump mdp element register   1: dump mmsys1 register
    
public:
    virtual int HardReset( void );
    

};


/*/////////////////////////////////////////////////////////////////////////////
    OVL
  /////////////////////////////////////////////////////////////////////////////*/

class OVL:public MDPELEMENT_I
{
public:/*[member veriable]*/
    unsigned long u4HeaderBaseAddr[16];
    unsigned long u4MaskBaseAddr[16];
    unsigned short u2SrcWidthInPixel;
    unsigned short u2SrcHeightInLine;
    unsigned short u2MaskOffsetX[16];
    unsigned short u2MaskOffsetY[16];
    unsigned short u2MaskSrcWidthInPixel[16];
    unsigned short u2MaskSrcHeightInLine[16];
    unsigned short u2MaskDestWidthInPixel[16];
    unsigned short u2MaskDestHeightInLine[16];

    unsigned char uColorKey;
    unsigned char uReplaceColorY;
    unsigned char uReplaceColorU;
    unsigned char uReplaceColorV;

    unsigned long bMaskEn:1;
    unsigned long bIsCompressMask:1;
    unsigned long bContinuous:1;
    unsigned long bCamIn:1;
    unsigned long uMaskCnt:8;
    unsigned long bSource:1;// 0-RDMA0 ,1-CRZ
    unsigned long bToIPP:1;
    unsigned long bToMOUT:1;
    unsigned long bToVRZ:1;
    unsigned long bOverlayIPP:2;//0 : no overlay, 1:overlay , 2:single color overlay
    unsigned long bOverlayMOUT:2;//0 : no overlay, 1:overlay , 2:single color overlay
    unsigned long bOverlayVRZ:2;//0 : no overlay, 1:overlay , 2:single color overlay
    unsigned long bBypass:1;//
    unsigned long u4OVLPalette[256];

public:/*ctor/dtor*/
    OVL():
        uColorKey(0),
        uReplaceColorY(0),
        uReplaceColorU(0),
        uReplaceColorV(0),
        bMaskEn(0),
        bIsCompressMask(0),
        bContinuous(0),
        bCamIn(0),
        uMaskCnt(0),
        bSource(0),
        bToIPP(0),
        bToMOUT(0),
        bToVRZ(0),
        bOverlayIPP(0),
        bOverlayMOUT(0),
        bOverlayVRZ(0),
        bBypass(0)
        {     }

public: /*[MDPELEMENT_I]*/
    virtual int CheckBusy( unsigned long* param );


public:/*[MDPELEMENT_I]*/
    virtual unsigned long id( void )                    {   return MID_OVL;  }
    virtual const char*   name_str( void )              {   return "MID_OVL";}
    virtual unsigned long reg_base_addr_pa( void )      {   return MT6575_OVL_DMA_BASE;  }
    virtual unsigned long reg_range( void )             {   return MT6575_OVL_DMA_REG_RANGE;  }
    virtual int is_bypass( void )                       {   return bBypass;       }

protected:/*[MDPELEMENT_I]*/
    virtual int _ConfigPre( void )  {   return 0; } //Instance Config (Pre)
    virtual int _Config( void );                    //Core Config
    virtual int _ConfigPost( void ) {   return 0; } //Instance Config (Post)
    
    virtual int _EnablePre( void )      {   return 0; }
    virtual int _Enable( void );
    virtual int _EnablePost( void )     {   return 0; }
    
    virtual int _DisablePre( void )     {   return 0; }
    virtual int _Disable( void );
    virtual int _DisablePost( void )    {   return 0; }

    virtual int Reset( void );
    virtual int DumpRegisterCustom( int mode );   //mode:  0: dump mdp element register   1: dump mmsys1 register
    
public:
    virtual int HardReset( void );
    

};



/*/////////////////////////////////////////////////////////////////////////////
    IPP
  /////////////////////////////////////////////////////////////////////////////*/
typedef struct
{
    unsigned char uColorAdj[21];
} ColorAdjCoeff;

class IPP:public MDPELEMENT_I
{
public:/*[member veriable]*/
    unsigned char uHueRotMatrix[4];//[0] C11, [1] C12, [2] C21, [3] C22 , 64*cos(theta),2'comp
    unsigned char uSatAdj;//unity:0x20, 0~127
    unsigned char uBriYAdj;//0~255,Y axis
    unsigned char uBriXAdj;//0~255,X axis
    unsigned char uContrastAdj;//unity:0x20, 0~255
    unsigned char uColorizeU;//-128~127,2's comp
    unsigned char uColorizeV;//-128~127,2's comp
    ColorAdjCoeff * puColorAdjCoeff;
    unsigned char uRGBDetectR;
    unsigned char uRGBDetectG;
    unsigned char uRGBDetectB;
    unsigned char uRGBReplaceR;
    unsigned char uRGBReplaceG;
    unsigned char uRGBReplaceB;
    
    /*enable flag*/
    unsigned long bEnRGBReplace:1;
    unsigned long bEnColorInverse:1;
    unsigned long bEnColorAdj:1;
    unsigned long bEnColorize:1;
    unsigned long bEnSatAdj:1;
    unsigned long bEnHueAdj:1;
    unsigned long bEnContractBrightAdj:1;

    /*output sel*/
    unsigned long to_jpg_dma:1;
    unsigned long to_vdo_rot0:1;
    unsigned long to_prz0:1;
    unsigned long to_vrz0:1;
    unsigned long to_rgb_rot0:1;

    /*input sel*/
    unsigned long src_sel:1;//0 : OVL, 1:CRZ

    unsigned long bCamIn:1;
    unsigned long bBypass:1;


public:/*ctor/dtor*/
    IPP():
        uSatAdj(0),
        uBriYAdj(0),
        uBriXAdj(0),
        uContrastAdj(0),
        uColorizeU(0),
        uColorizeV(0),
        puColorAdjCoeff(0),
        uRGBDetectR(0),
        uRGBDetectG(0),
        uRGBDetectB(0),
        uRGBReplaceR(0),
        uRGBReplaceG(0),
        uRGBReplaceB(0),
        bEnRGBReplace(0),
        bEnColorInverse(0),
        bEnColorAdj(0),
        bEnColorize(0),
        bEnSatAdj(0),
        bEnHueAdj(0),
        bEnContractBrightAdj(0),
        to_jpg_dma(0),
        to_vdo_rot0(0),
        to_prz0(0),
        to_vrz0(0),
        to_rgb_rot0(0),
        src_sel(0),
        bCamIn(0),
        bBypass(0)
        {     }

public: /*[MDPELEMENT_I]*/
    virtual int is_bypass( void )                       {   return bBypass;       }
    virtual int CheckBusy( unsigned long* param );


public:/*[MDPELEMENT_I]*/
    virtual unsigned long id( void )                    {   return MID_IPP;  }
    virtual const char*   name_str( void )              {   return "MID_IPP";}
    virtual unsigned long reg_base_addr_pa( void )      {   return MT6575_IPP_BASE;  }
    virtual unsigned long reg_range( void )             {   return MT6575_IPP_REG_RANGE;  }

protected:/*[MDPELEMENT_I]*/
    virtual int _ConfigPre( void )  {   return 0; } //Instance Config (Pre)
    virtual int _Config( void );                    //Core Config
    virtual int _ConfigPost( void ) {   return 0; } //Instance Config (Post)
    
    virtual int _EnablePre( void )      {   return 0; }
    virtual int _Enable( void );
    virtual int _EnablePost( void )     {   return 0; }
    
    virtual int _DisablePre( void )     {   return 0; }
    virtual int _Disable( void );
    virtual int _DisablePost( void )    {   return 0; }

    virtual int Reset( void );
    virtual int DumpRegisterCustom( int mode );   //mode:  0: dump mdp element register   1: dump mmsys1 register
    
public:
    virtual int HardReset( void );

    

};




/*/////////////////////////////////////////////////////////////////////////////
    RESZ_I
  /////////////////////////////////////////////////////////////////////////////*/
class RESZ_I:public MDPELEMENT_I
{
public:/*[member veriable]*/
    MdpSize         src_img_size;
    MdpRect         src_img_roi;
    MdpSize         dst_img_size;

    unsigned char uUpScaleCoeff;//Coefficient for edge enhancement
    unsigned char uDnScaleCoeff;

    /*coefficient table for resizing.1:most blur 19:sharpest 0:linear interpolation rather than cubic*/
    unsigned char uEEHCoeff;
    unsigned char uEEVCoeff;

    unsigned long bContinuous:1;//0: single shot, 1: continuous, set to continuous only under camera input case (CLS:sensor must be configured to continuous as well)
    unsigned long bCamIn:1;
    unsigned long bBypass:1;
    
public: /*ctor/dtor*/
    RESZ_I():
        uUpScaleCoeff(0),
        uDnScaleCoeff(0),
        uEEHCoeff(0),
        uEEVCoeff(0),
        bContinuous(0),
        bCamIn(0),
        bBypass(0)
        {};
    
public: /*[MDPELEMENT_I]*/
    virtual int is_bypass( void )                   {   return bBypass;       }
    virtual int CheckBusy( unsigned long* param );

private:/*[MDPELEMENT_I]*/
    virtual int _Config( void );
    virtual int _Enable( void );
    virtual int _Disable( void );

    virtual int Reset( void );
    virtual int DumpRegisterCustom( int mode );   //mode:  0: dump mdp element register   1: dump mmsys1 register
public:
    virtual int HardReset( void );
    virtual int DumpDebugInfo( void );

public: /*[RESZ_I]*/
    int ConfigZoom( MdpRect crop_size , unsigned long linked_mdp_id_set );
    
public:/*[RESZ_I]*/
    typedef struct
    {
        unsigned long LBMAX;
        unsigned long reg_ORIGSZ;
        unsigned long reg_SRCSZ;
        unsigned long reg_TARSZ;
        unsigned long reg_CROPLR;
        unsigned long reg_CROPTB;
        unsigned long reg_HRATIO;
        unsigned long reg_VRATIO;
        
    } ReszZsdZoomParam;
    
    int ConfigZsdZoom_RegWrite( ReszZsdZoomParam param );


public: /*[RESZ_I]*/
    unsigned long CalcLBMAX( MdpSize src_size, MdpSize dst_size );
    unsigned long CalcHRATIO( unsigned long src_w, unsigned long dst_w );
    unsigned long CalcVRATIO( unsigned long src_h, unsigned long dst_h );






};
/*/////////////////////////////////////////////////////////////////////////////
    CRZ
  /////////////////////////////////////////////////////////////////////////////*/

class CRZ:public RESZ_I
{
public:/*[member veriable]*/
    /*input sel*/
    unsigned long src_sel;// 0:RDMA0 , 1:BRZ, 2:Camera

public:/*ctor/dtor*/
    CRZ():
        src_sel(0)
        {     }

public:/*[MDPELEMENT_I]*/
    virtual unsigned long id( void )                    {   return MID_CRZ;  }
    virtual const char*   name_str( void )              {   return "MID_CRZ";}
    virtual unsigned long reg_base_addr_pa( void )      {   return MT6575_CRZ_BASE;    }
    virtual unsigned long reg_range( void )             {   return MT6575_CRZ_REG_RANGE;    }

protected:/*[RESZ_I]*/
    virtual int _ConfigPre( void );
    virtual int _ConfigPost( void );
    virtual int _EnablePre( void );
    virtual int _EnablePost( void );
    virtual int _DisablePre( void );
    virtual int _DisablePost( void );

    

};

/*/////////////////////////////////////////////////////////////////////////////
    PRZ0
  /////////////////////////////////////////////////////////////////////////////*/

class PRZ0:public RESZ_I
{
public:/*[member veriable]*/
    /*input sel*/
    unsigned long src_sel;      //0-MOUT, 1-IPP_MOUT,2-CAM(Sensor),3-BRZ_MOUT,6-CAM(RDMA0),10-CAM(BRZ)

    /*output sel*/
    unsigned long to_vdo_rot0:1;
    unsigned long to_rgb_rot0:1;
    unsigned long to_vrz0:1;


public:/*ctor/dtor*/
    PRZ0():
        src_sel(0),//0-MOUT, 1-IPP_MOUT,2-CAM(Sensor),3-BRZ_MOUT,6-CAM(RDMA0),10-CAM(BRZ)
        /*output sel*/
        to_vdo_rot0(0),
        to_rgb_rot0(0),
        to_vrz0(0)
        {    }

public:/*[MDPELEMENT_I]*/
    virtual unsigned long id( void )                    {   return MID_PRZ0;  }
    virtual const char*   name_str( void )              {   return "MID_PRZ0";}
    virtual unsigned long reg_base_addr_pa( void )      {   return MT6575_PRZ0_BASE;    }
    virtual unsigned long reg_range( void )             {   return MT6575_PRZ0_REG_RANGE;    }

protected:/*[RESZ_I]*/
    virtual int _ConfigPre( void );
    virtual int _ConfigPost( void );
    virtual int _EnablePre( void );
    virtual int _EnablePost( void );
    virtual int _DisablePre( void );
    virtual int _DisablePost( void );

};
/*/////////////////////////////////////////////////////////////////////////////
    PRZ1
  /////////////////////////////////////////////////////////////////////////////*/

class PRZ1:public RESZ_I
{
public:/*[member veriable]*/
    /*N/A*/

public:/*ctor/dtor*/
    PRZ1()     {     }

public:/*[MDPELEMENT_I]*/
    virtual unsigned long id( void )                    {   return MID_PRZ1;  }
    virtual const char*   name_str( void )              {   return "MID_PRZ1";}
    virtual unsigned long reg_base_addr_pa( void )      {   return MT6575_PRZ1_BASE;    }
    virtual unsigned long reg_range( void )             {   return MT6575_PRZ1_REG_RANGE;    }

protected:/*[RESZ_I]*/
    virtual int _ConfigPre( void );
    virtual int _ConfigPost( void );
    virtual int _EnablePre( void );
    virtual int _EnablePost( void );
    virtual int _DisablePre( void );
    virtual int _DisablePost( void );

};


/*/////////////////////////////////////////////////////////////////////////////
    VRZ_I
  /////////////////////////////////////////////////////////////////////////////*/
class VRZ_I:public MDPELEMENT_I
{
public:/*[member veriable]*/
    MdpSize         src_img_size;
    MdpSize         dst_img_size;

    unsigned long bContinuous:1;//0: single shot, 1: continuous, set to continuous only under camera input case (CLS:sensor must be configured to continuous as well)
    unsigned long bCamIn:1;
    unsigned long bBypass:1;

private:
        MdpMemBuffer    sysram_;    //Allocated sysram for this element usage
        

public: /*ctor/dtor*/
    VRZ_I():
        bContinuous(0),
        bCamIn(0),
        bBypass(0)
        {};
    
public: /*[MDPELEMENT_I]*/
    virtual int is_bypass( void )                       {   return bBypass;       }
    virtual int CheckBusy( unsigned long* param );

private:/*[MDPELEMENT_I]*/
    virtual int _Config( void );
    virtual int _Enable( void );
    virtual int _Disable( void );

    virtual int Reset( void );
    virtual int DumpRegisterCustom( int mode );   //mode:  0: dump mdp element register   1: dump mmsys1 register
public:
    virtual int HardReset( void );
    virtual int DumpDebugInfo( void );

public: /*[VRZ_I]*/
    typedef struct
    {
        unsigned long reg_SRCSZ;
        unsigned long reg_TARSZ;
        unsigned long reg_HRATIO;
        unsigned long reg_VRATIO;
        unsigned long reg_HRES;
        unsigned long reg_VRES;
    } VrzZsdZoomParam;
    
    int ConfigZsdZoom_RegWrite( VrzZsdZoomParam param );
    



};

/*/////////////////////////////////////////////////////////////////////////////
    VRZ0
  /////////////////////////////////////////////////////////////////////////////*/
/* 
 *   VRZ0 can output to one of RGB_ROT1 & VDO_ROT1, depend on which ROT is enabled
 */
class VRZ0:public VRZ_I
{
public:/*[member veriable]*/
    /*input sel*/
    unsigned long src_sel:3;// 0-MOUT, 1-BRZ_MOUT, 2-IPP_MOUT, 3-OVL_DMA_MIMO, 4-PRZ0_MOUT

public:/*ctor/dtor*/
    VRZ0():
        src_sel(0)
        {     }

public:/*[MDPELEMENT_I]*/
    virtual unsigned long id( void )                    {   return MID_VRZ0;  }
    virtual const char*   name_str( void )              {   return "MID_VRZ0";}
    virtual unsigned long reg_base_addr_pa( void )      {   return MT6575_VRZ0_BASE;    }
    virtual unsigned long reg_range( void )             {   return MT6575_VRZ0_REG_RANGE;    }

protected:/*[VRZ_I]*/
    virtual int _ConfigPre( void );
    virtual int _ConfigPost( void );
    virtual int _EnablePre( void );
    virtual int _EnablePost( void );
    virtual int _DisablePre( void );
    virtual int _DisablePost( void );

};

/*/////////////////////////////////////////////////////////////////////////////
    VRZ1
  /////////////////////////////////////////////////////////////////////////////*/
class VRZ1:public VRZ_I
{
public:/*[member veriable]*/
    /*input sel*/
    unsigned long src_sel:1;// 0-R_DMA1, 1-BRZ_MOUT

public:/*ctor/dtor*/
    VRZ1():
        src_sel(0)
        {     }

public:/*[MDPELEMENT_I]*/
    virtual unsigned long id( void )                    {   return MID_VRZ1;  }
    virtual const char*   name_str( void )              {   return "MID_VRZ1";}
    virtual unsigned long reg_base_addr_pa( void )      {   return MT6575_VRZ1_BASE;    }
    virtual unsigned long reg_range( void )             {   return MT6575_VRZ1_REG_RANGE;    }

protected:/*[VRZ_I]*/
    virtual int _ConfigPre( void );
    virtual int _ConfigPost( void );
    virtual int _EnablePre( void );
    virtual int _EnablePost( void );
    virtual int _DisablePre( void );
    virtual int _DisablePost( void );
};



/*/////////////////////////////////////////////////////////////////////////////
    ROTDMA_I
  /////////////////////////////////////////////////////////////////////////////*/
#define MT6575ROTDMA_MAX_RINGBUFFER_CNT MDP_MAX_RINGBUFFER_CNT

class ROTDMA_I:public MDPELEMENT_I
{
public:/*[member veriable]*/
    MdpYuvAddr      dst_img_yuv_addr[MT6575ROTDMA_MAX_RINGBUFFER_CNT]; //Support at most 16 Address to fit descript mode
    MdpRect         src_img_roi;    //image roi before rotate, (x,y) of roi should be 0, offset is supported by startaddress.(w,h) of roi is the real image size,not stride
    MdpSize         dst_img_size;   //Size before rotate (image stride)
    MdpColorFormat  dst_color_format;

    unsigned long uOutBufferCnt:5;
    unsigned long bContinuous:1;// 1:Continuous mode does not allow time sharing.
    unsigned long bFlip:1;
    unsigned long bRotate:2;//0:0, 1:90, 2:180, 3:270
    unsigned long bDithering:1;
    unsigned long uAlpha:8;// valid if bSpecifyAlpha is enabled
    unsigned long bCamIn:1;// 1: real time path, ex : camera input
    unsigned long bEnHWTriggerRDMA0:1;// 0:Disable , 1:Enable
    unsigned long uOutBufferCntZSD:5;

    MdpYuvAddr      dst_img_yuv_addr_last_frame_[MT6575ROTDMA_MAX_RINGBUFFER_CNT]; //last preview frame for zsd
    MdpSize         dst_img_size_last_frame_;//last preview frame for zsd
    MdpYuvAddr      dst_img_yuv_adapt_addr_last_frame_[MT6575ROTDMA_MAX_RINGBUFFER_CNT]; //last preview frame for zsd


private:
        /*Internal Use Only*/

        unsigned long   adapt_m4u_flag_bit_last_frame_;
        unsigned long   alloc_mva_flag_bit_last_frame_;
        unsigned long   dst_img_buffer_size_total_last_frame_;

        MdpYuvAddr      dst_img_yuv_adapt_addr[MT6575RDMA_MAX_RINGBUFFER_CNT];
        unsigned long   adapt_m4u_flag_bit_;
        unsigned long   alloc_mva_flag_bit_;
        unsigned long   dst_img_buffer_size_total_;
    
private:
        MdpMemBuffer    sysram_;        //Allocated sysram for this element usage
        MdpMemBuffer    sysram_sub_;    //Allocated sysram for this element usage

private:/*Descriptor Mode*/
        unsigned long   desc_sw_write_index_;
        //MdpYuvAddr      desc_dst_img_yuv_addr_[MT6575RDMA_MAX_RINGBUFFER_CNT];  //yuv address for descriptor

public:/*[member veriable]*/
        /*ZSD path use*/
        /*When this ROT is used for ZSD in descriptor mode, need extra descriptors field*/
        int             b_is_zero_shutter_encode_port_; /*1:zero shutter enable*/

        MdpRect         desc_src_img_roi_;    //image roi before rotate
        MdpSize         desc_dst_img_size_;    //Size before rotate (image stride)
        unsigned long   desc_is_generic_yuv_;
        unsigned long   desc_byte_per_pixel_;
        unsigned long   desc_yv_,desc_yh_,desc_uvv_,desc_uvh_;
        unsigned long   desc_y_stride_align_;
        unsigned long   desc_uv_stride_align_;
        unsigned long   desc_y_stride_;
        unsigned long   desc_uv_stride_;
        unsigned long   desc_y_frame_start_in_byte_;
        unsigned long   desc_uv_frame_start_in_byte_;
        
        

    
public:
    ROTDMA_I():
        uOutBufferCnt(1),
        bContinuous(0),
        bFlip(0),
        bRotate(0),
        bDithering(0),
        uAlpha(0xFF),
        bCamIn(0),
        bEnHWTriggerRDMA0(0),
        adapt_m4u_flag_bit_(0),
        alloc_mva_flag_bit_(0),
        dst_img_buffer_size_total_(0),
        desc_sw_write_index_(0),
        b_is_zero_shutter_encode_port_(0),
        desc_is_generic_yuv_(0),
        desc_byte_per_pixel_(0),
        desc_yv_(0),desc_yh_(0),desc_uvv_(0),desc_uvh_(0),
        desc_y_stride_align_(0),desc_uv_stride_align_(0),
        desc_y_stride_(0),
        desc_uv_stride_(0),
        desc_y_frame_start_in_byte_(0),
        desc_uv_frame_start_in_byte_(0)
        {};
    

public: /*[MDPELEMENT_I]*/
    virtual int is_bypass( void )                       {   return 0;       }
    virtual int CheckBusy( unsigned long* desc_read_pointer /*unsigned long* param*/ );

private:/*[MDPELEMENT_I]*/
    virtual int _Config( void );
    virtual int _Enable( void );
    virtual int _Disable( void );

    virtual int Reset( void );
    virtual int DumpRegisterCustom( int mode );   //mode:  0: dump mdp element register   1: dump mmsys1 registerpublic:

public:
    virtual int HardReset( void );
    virtual int DumpDebugInfo( void );

public: /*[ROTDMA_I]*/
    unsigned long   desc_sw_write_index( void ) {   return desc_sw_write_index_;    }
    int             DescQueueGetFreeList( unsigned long *p_StartIndex, unsigned long *p_Count );/*Equel to De Queue Buffer in 73*/
    int             DescQueueRefill( void );/*Equel to En Queue Buffer in 73*/
    int             DescQueueRefillEx( void ) ;/*Equel to En Queue Buffer in 73*/
    int             DescQueueWaitEmpty( void );
    int         DescQueueManualRefill(unsigned long dst_width, unsigned long dst_height);
    int         AdpteLastPreviewFrameBuffer( void );
    int         UnAdpteLastPreviewFrameBuffer( void );
    int        QueryCurrentSwReadIndex(void);

private: /*[ROTDMA_I]*/
    inline int  DescWriteWaitBusy( unsigned long reg_base ); 
    int         _DescQueueRefillByIndex( unsigned long index , unsigned long reg_base );
    int         _DescQueueRefillByIndexEx( unsigned long index , unsigned long reg_base );

public: /*[ROTDMA_I]*/   
    typedef struct
    {
        MdpRect         desc_src_img_roi;    //image roi before rotate
        MdpSize         desc_dst_img_size;    //Size before rotate (image stride)
        unsigned long   desc_y_stride;
        unsigned long   desc_uv_stride;
        unsigned long   desc_y_frame_start_in_byte;
        unsigned long   desc_uv_frame_start_in_byte;
    
    } RotdmaZsdZoomParam;
    
    int ConfigZsdZoom_DescUpdate( RotdmaZsdZoomParam param );
    
    int  ConfigZsd_DescUpdate( RotdmaZsdZoomParam param );
    int             DescQueueFullFill(RotdmaZsdZoomParam param );

public: /*[ROTDMA_I]*/
    typedef struct 
    {
        unsigned long   rotate;             /*rotation w/o flip. 0-3*/
        MdpRect         src_img_roi;        /*src roi image before rotation*/
        MdpSize         dst_img_size;       /*stride before rotation*/
        MdpColorFormat  dst_color_format;
        int             b_is_generic_yuv;
        unsigned long   byte_per_pixel;
        /*y,u,v sampling period*/
        unsigned long   yv;
        unsigned long   yh;
        unsigned long   uvv;
        unsigned long   uvh;
        
        //Stride Align: 0:None 1:2 align 2:4 align 3:8 align 4:16 align
        unsigned long   y_stride_align;
        unsigned long   uv_stride_align;
        
    } CalcFrameStartAddress_In;

    typedef struct 
    {
        unsigned long  y_stride;
        unsigned long  uv_stride;
        unsigned long  y_frame_start_in_byte;
        unsigned long  uv_frame_start_in_byte;
        
    } CalcFrameStartAddress_Out;
    
    int     CalcFrameStartAddress( CalcFrameStartAddress_In* p_in, CalcFrameStartAddress_Out* p_out );
    int     CalculateBufferSizeAndYuvOffset( MdpSize Dst_Img_Size, MdpRect Src_Img_Roi );
    int     DoWhenOutputSizeChange( MdpSize Dst_Img_Size, MdpRect Src_Img_Roi );
    int     CalcBufferSize( void );
    

    

};

/*/////////////////////////////////////////////////////////////////////////////
    VDOROT0
  /////////////////////////////////////////////////////////////////////////////*/
class VDOROT0:public ROTDMA_I
{
public:
    unsigned long src_sel;// 0-PRZ0_MOUT, 1-IPP_MOUT

public:/*ctor/dtor*/
    VDOROT0():
        src_sel(0)
        {    }

public:/*[MDPELEMENT_I]*/
    virtual unsigned long id( void )                    {   return MID_VDO_ROT0;  }
    virtual const char*   name_str( void )              {   return "MID_VDO_ROT0";}
    virtual unsigned long reg_base_addr_pa( void )      {   return MT6575_VDO_ROT0_BASE;    }
    virtual unsigned long reg_range( void )             {   return MT6575_VDO_ROT0_REG_RANGE;    }


protected:/*[ROTDMA_I]*/
    virtual int _ConfigPre( void );
    virtual int _ConfigPost( void );
    virtual int _EnablePre( void );
    virtual int _EnablePost( void );
    virtual int _DisablePre( void );
    virtual int _DisablePost( void );

};

/*/////////////////////////////////////////////////////////////////////////////
    VDOROT1
  /////////////////////////////////////////////////////////////////////////////*/
class VDOROT1:public ROTDMA_I
{
public:
    unsigned long src_sel;// 0-PRZ1, 1-VRZ0

public:/*ctor/dtor*/
    VDOROT1():
        src_sel(0)
        {    }

public:/*[MDPELEMENT_I]*/
    virtual unsigned long id( void )                {   return MID_VDO_ROT1;  }
    virtual const char*   name_str( void )              {   return "MID_VDO_ROT1";}
    virtual unsigned long reg_base_addr_pa( void )  {   return MT6575_VDO_ROT1_BASE;    }
    virtual unsigned long reg_range( void )         {   return MT6575_VDO_ROT1_REG_RANGE;    }


protected:/*[ROTDMA_I]*/
    virtual int _ConfigPre( void );
    virtual int _ConfigPost( void );
    virtual int _EnablePre( void );
    virtual int _EnablePost( void );
    virtual int _DisablePre( void );
    virtual int _DisablePost( void );

};

/*/////////////////////////////////////////////////////////////////////////////
    RGBROT0
  /////////////////////////////////////////////////////////////////////////////*/
class RGBROT0:public ROTDMA_I
{
public:
    unsigned long src_sel;//0-PRZ0_MOUT ,1-IPP_MOUT

public:/*ctor/dtor*/
    RGBROT0():
        src_sel(0)
        {     }

public:/*[MDPELEMENT_I]*/
    virtual unsigned long id( void )                    {   return MID_RGB_ROT0;  }
    virtual const char*   name_str( void )              {   return "MID_RGB_ROT0";}
    virtual unsigned long reg_base_addr_pa( void )      {   return MT6575_RGB_ROT0_BASE;    }
    virtual unsigned long reg_range( void )             {   return MT6575_RGB_ROT0_REG_RANGE;    }


protected:/*[ROTDMA_I]*/
    virtual int _ConfigPre( void );
    virtual int _ConfigPost( void );
    virtual int _EnablePre( void );
    virtual int _EnablePost( void );
    virtual int _DisablePre( void );
    virtual int _DisablePost( void );

};

/*/////////////////////////////////////////////////////////////////////////////
    RGBROT1
  /////////////////////////////////////////////////////////////////////////////*/
class RGBROT1:public ROTDMA_I
{
public:
    /*Nothing*/

public:/*ctor/dtor*/
    RGBROT1()     {    }

public:/*[MDPELEMENT_I]*/
    virtual unsigned long id( void )                    {   return MID_RGB_ROT1;  }
    virtual const char*   name_str( void )              {   return "MID_RGB_ROT1";}
    virtual unsigned long reg_base_addr_pa( void )      {   return MT6575_RGB_ROT1_BASE;    }
    virtual unsigned long reg_range( void )             {   return MT6575_RGB_ROT1_REG_RANGE;    }


protected:/*[ROTDMA_I]*/
    virtual int _ConfigPre( void );
    virtual int _ConfigPost( void );
    virtual int _EnablePre( void );
    virtual int _EnablePost( void );
    virtual int _DisablePre( void );
    virtual int _DisablePost( void );

};

/*/////////////////////////////////////////////////////////////////////////////
    RGBROT2
  /////////////////////////////////////////////////////////////////////////////*/
class RGBROT2:public ROTDMA_I
{
public:
    /*Nothing*/

public:/*ctor/dtor*/
        RGBROT2()     {    }

public:/*[MDPELEMENT_I]*/
    virtual unsigned long id( void )                {   return MID_RGB_ROT2;  }
    virtual const char*   name_str( void )          {   return "MID_RGB_ROT2";}
    virtual unsigned long reg_base_addr_pa( void )  {   return MT6575_RGB_ROT2_BASE;   }
    virtual unsigned long reg_range( void )         {   return MT6575_RGB_ROT2_REG_RANGE;    }


protected:/*[ROTDMA_I]*/
    virtual int _ConfigPre( void );
    virtual int _ConfigPost( void );
    virtual int _EnablePre( void );
    virtual int _EnablePost( void );
    virtual int _DisablePre( void );
    virtual int _DisablePost( void );

};


/*/////////////////////////////////////////////////////////////////////////////
    BRZ
  /////////////////////////////////////////////////////////////////////////////*/
class BRZ:public MDPELEMENT_I
{
public:/*[member veriable]*/
    MdpSize         src_img_size;   /*Width/Height after shrinking*/
    unsigned long   shrink_factor;  // 0 : 1x, 1: 1/4, 2: 1/16, 3: 1/64
    unsigned long   input_format;   //integer : 411, 422, 444, 400, 410 etc...

    /*output sel*/
    unsigned int    to_vrz1;
    unsigned int    to_prz0;
    unsigned int    to_vrz0;
    unsigned int    to_cam;


private:
    MdpMemBuffer    sysram_;    //Allocated sysram for this element usage


public:/*ctor/dtor*/
    BRZ():
        shrink_factor(0),
        input_format(0),
        to_vrz1(0),
        to_prz0(0),
        to_vrz0(0),
        to_cam(0)
        {    }

public: /*[MDPELEMENT_I]*/
    virtual int CheckBusy( unsigned long* param );


public:/*[MDPELEMENT_I]*/
    virtual unsigned long id( void )                    {   return MID_BRZ;  }
    virtual const char*   name_str( void )              {   return "MID_BRZ";}
    virtual unsigned long reg_base_addr_pa( void )      {   return MT6575_BRZ_BASE;  }
    virtual unsigned long reg_range( void )             {   return MT6575_BRZ_REG_RANGE;  }
    virtual int           is_bypass( void )             {   return 0;       }

protected:/*[MDPELEMENT_I]*/
    virtual int _ConfigPre( void )  {   return 0; } //Instance Config (Pre)
    virtual int _Config( void );                    //Core Config
    virtual int _ConfigPost( void ) {   return 0; } //Instance Config (Post)
    
    virtual int _EnablePre( void )      {   return 0; }
    virtual int _Enable( void );
    virtual int _EnablePost( void )     {   return 0; }
    
    virtual int _DisablePre( void )     {   return 0; }
    virtual int _Disable( void );
    virtual int _DisablePost( void )    {   return 0; }

    virtual int Reset( void );
    virtual int DumpRegisterCustom( int mode );   //mode:  0: dump mdp element register   1: dump mmsys1 register

public:
    virtual int HardReset( void );
    virtual int DumpDebugInfo( void );
    

};


/*/////////////////////////////////////////////////////////////////////////////
    JPEGDMA
  /////////////////////////////////////////////////////////////////////////////*/
class JPEGDMA:public MDPELEMENT_I
{
public:/*[member veriable]*/
    MdpSize         src_img_size;   /*Width/Height after shrinking*/
    unsigned long   output_format;   //integer : 411, 422, 444, 400, 410 etc...

    /*camera option*/
    unsigned long bContinuous:1;
    unsigned long bCamIn:1;// 1: camera input

    /*source sel*/
    unsigned long src_sel:1;// 0-IPP_MOUT, 1-MOUT


private:
    MdpMemBuffer    sysram_;    //Allocated sysram for this element usage


public:/*ctor/dtor*/
    JPEGDMA():
        output_format(0),
        bContinuous(0),
        bCamIn(0),
        src_sel(0)
        {   }

public: /*[MDPELEMENT_I]*/
    virtual int is_bypass( void )                       {   return 0;       }
    virtual int CheckBusy( unsigned long* param );


public:/*[MDPELEMENT_I]*/
    virtual unsigned long id( void )                    {   return MID_JPEG_DMA;  }
    virtual const char*   name_str( void )              {   return "MID_JPEG_DMA";}
    virtual unsigned long reg_base_addr_pa( void )      {   return MT6575_JPEG_DMA_BASE;  }
    virtual unsigned long reg_range( void )             {   return MT6575_JPEG_DMA_REG_RANGE;  }

protected:/*[MDPELEMENT_I]*/
    virtual int _ConfigPre( void )  {   return 0; } //Instance Config (Pre)
    virtual int _Config( void );                    //Core Config
    virtual int _ConfigPost( void ) {   return 0; } //Instance Config (Post)
    
    virtual int _EnablePre( void )      {   return 0; }
    virtual int _Enable( void );
    virtual int _EnablePost( void )     {   return 0; }
    
    virtual int _DisablePre( void )     {   return 0; }
    virtual int _Disable( void );
    virtual int _DisablePost( void )    {   return 0; }

    virtual int Reset( void );
    virtual int DumpRegisterCustom( int mode );   //mode:  0: dump mdp element register   1: dump mmsys1 register

public:
    virtual int HardReset( void );
    virtual int DumpDebugInfo( void );

};


/*/////////////////////////////////////////////////////////////////////////////
    ROTDMAEX_I
  /////////////////////////////////////////////////////////////////////////////*/
class ROTDMAEX_I:public MDPELEMENT_I
{
public:/*[member veriable]*/
    MdpYuvAddr      dst_img_yuv_addr[MT6575ROTDMA_MAX_RINGBUFFER_CNT]; //Support at most 16 Address to fit descript mode
    MdpSize         src_img_size;
    MdpRect         src_img_roi;    //image roi before rotate ,(x,y) of roi should be 0, offset is supported by startaddress.(w,h) of roi is the real image size,not stride
    MdpSize         dst_img_size;   //Size before rotate (image stride)
    MdpRect         dst_img_roi;
    MdpColorFormat  dst_color_format;

    unsigned long uOutBufferCnt:5;
    unsigned long bContinuous:1;// 1:Continuous mode does not allow time sharing.
    unsigned long bFlip:1;
    unsigned long bRotate:2;//0:0, 1:90, 2:180, 3:270
    unsigned long bDithering:1;
    unsigned long uAlpha:8;// valid if bSpecifyAlpha is enabled
    unsigned long bCamIn:1;// 1: real time path, ex : camera input
    unsigned long bEnHWTriggerRDMA0:1;// 0:Disable , 1:Enable

private:
        /*Internal Use Only*/
        MdpYuvAddr      dst_img_yuv_adapt_addr[MT6575RDMA_MAX_RINGBUFFER_CNT];
        unsigned long   adapt_m4u_flag_bit_;
        unsigned long   alloc_mva_flag_bit_;
        unsigned long   dst_img_buffer_size_total_;
    
private:
        MdpMemBuffer    sysram_;    //Allocated sysram for this element usage

private:/*Descriptor Mode*/
        unsigned long   desc_sw_write_index_;
        //MdpYuvAddr      desc_dst_img_yuv_addr_[MT6575RDMA_MAX_RINGBUFFER_CNT];  //yuv address for descriptor

public:/*[member veriable]*/
        /*ZSD path use*/
        /*When this ROT is used for ZSD in descriptor mode, need extra descriptors field*/
        int             b_is_zero_shutter_encode_port_; /*1:zero shutter enable*/

        MdpSize         desc_src_img_size_;
        MdpRect         desc_src_img_roi_;    //image roi before rotate
        MdpSize         desc_dst_img_size_;    //Size before rotate (image stride)
        MdpRect         desc_dst_img_roi_;
        unsigned long   desc_is_generic_yuv_;
        unsigned long   desc_byte_per_pixel_;
        unsigned long   desc_yv_,desc_yh_,desc_uvv_,desc_uvh_;
        unsigned long   desc_y_stride_align_;
        unsigned long   desc_uv_stride_align_;
        unsigned long   desc_y_stride_;
        unsigned long   desc_uv_stride_;
        unsigned long   desc_y_frame_start_in_byte_;
        unsigned long   desc_uv_frame_start_in_byte_;
        
        

    
public:
    ROTDMAEX_I():
        uOutBufferCnt(1),
        bContinuous(0),
        bFlip(0),
        bRotate(0),
        bDithering(0),
        uAlpha(0xFF),
        bCamIn(0),
        bEnHWTriggerRDMA0(0),
        adapt_m4u_flag_bit_(0),
        alloc_mva_flag_bit_(0),
        dst_img_buffer_size_total_(0),
        desc_sw_write_index_(0),
        b_is_zero_shutter_encode_port_(0),
        desc_is_generic_yuv_(0),
        desc_byte_per_pixel_(0),
        desc_yv_(0),desc_yh_(0),desc_uvv_(0),desc_uvh_(0),
        desc_y_stride_align_(0),desc_uv_stride_align_(0),
        desc_y_stride_(0),
        desc_uv_stride_(0),
        desc_y_frame_start_in_byte_(0),
        desc_uv_frame_start_in_byte_(0)
        {};
    

public: /*[MDPELEMENT_I]*/
    virtual int is_bypass( void )                       {   return 0;       }
    virtual int CheckBusy( unsigned long* desc_read_pointer /*unsigned long* param*/ );

private:/*[MDPELEMENT_I]*/
    virtual int _Config( void );
    virtual int _Enable( void );
    virtual int _Disable( void );

    virtual int Reset( void );
    virtual int DumpRegisterCustom( int mode );   //mode:  0: dump mdp element register   1: dump mmsys1 register

public:
    virtual int HardReset( void );
    virtual int DumpDebugInfo( void );

public: /*[ROTDMA_I]*/
    unsigned long   desc_sw_write_index( void ) {   return desc_sw_write_index_;    }
    int             DescQueueGetFreeList( unsigned long *p_StartIndex, unsigned long *p_Count );/*Equel to De Queue Buffer in 73*/
    int             DescQueueRefill( void );/*Equel to En Queue Buffer in 73*/
    int             DescQueueWaitEmpty( void );

private: /*[ROTDMA_I]*/
    inline int  DescWriteWaitBusy( unsigned long reg_base ); 
    int         _DescQueueRefillByIndex( unsigned long index , unsigned long reg_base );

    

public: /*[ROTDMA_I]*/
    typedef struct 
    {
        unsigned long   rotate;             /*rotation w/o flip. 0-3*/
        MdpRect         dst_img_roi;        /*src roi image before rotation, (x,y) of roi should be 0, offset is supported by startaddress.(w,h) of roi is the real image size,not stride*/
        MdpSize         dst_img_size;       /*stride before rotation*/
        MdpColorFormat  dst_color_format;
        int             b_is_generic_yuv;
        unsigned long   byte_per_pixel;
        /*y,u,v sampling period*/
        unsigned long   yv;
        unsigned long   yh;
        unsigned long   uvv;
        unsigned long   uvh;

        //Stride Align: 0:None 1:2 align 2:4 align 3:8 align 4:16 align
        unsigned long   y_stride_align;
        unsigned long   uv_stride_align;
        
    } CalcFrameStartAddress_In;

    typedef struct 
    {
        unsigned long  y_stride;
        unsigned long  uv_stride;
        unsigned long  y_frame_start_in_byte;
        unsigned long  uv_frame_start_in_byte;
        
    } CalcFrameStartAddress_Out;
    
    int     CalcFrameStartAddress( CalcFrameStartAddress_In* p_in, CalcFrameStartAddress_Out* p_out );
    int     CalculateBufferSizeAndYuvOffset( MdpSize Dst_Img_Size, MdpRect Dst_Img_Roi );
    

    

};

/*/////////////////////////////////////////////////////////////////////////////
    RGBROT1EX
  /////////////////////////////////////////////////////////////////////////////*/
class RGBROT1EX:public ROTDMAEX_I
{
public:
    /*Nothing*/

public:/*ctor/dtor*/
        RGBROT1EX()     {    }

public:/*[MDPELEMENT_I]*/
    virtual unsigned long id( void )                {   return MID_RGB_ROT1;  }
    virtual const char*   name_str( void )          {   return "MID_RGB_ROT1EX";}
    virtual unsigned long reg_base_addr_pa( void )  {   return MT6575_RGB_ROT1_BASE;   }
    virtual unsigned long reg_range( void )         {   return MT6575_RGB_ROT1_REG_RANGE;    }


protected:/*[ROTDMA_I]*/
    virtual int _ConfigPre( void );
    virtual int _ConfigPost( void );
    virtual int _EnablePre( void );
    virtual int _EnablePost( void );
    virtual int _DisablePre( void );
    virtual int _DisablePost( void );

};


/*/////////////////////////////////////////////////////////////////////////////
    RGBROT2EX
  /////////////////////////////////////////////////////////////////////////////*/
class RGBROT2EX:public ROTDMAEX_I
{
public:
    /*Nothing*/

public:/*ctor/dtor*/
        RGBROT2EX()     {    }

public:/*[MDPELEMENT_I]*/
    virtual unsigned long id( void )                {   return MID_RGB_ROT2;  }
    virtual const char*   name_str( void )          {   return "MID_RGB_ROT2EX";}
    virtual unsigned long reg_base_addr_pa( void )  {   return MT6575_RGB_ROT2_BASE;   }
    virtual unsigned long reg_range( void )         {   return MT6575_RGB_ROT2_REG_RANGE;    }


protected:/*[ROTDMA_I]*/
    virtual int _ConfigPre( void );
    virtual int _ConfigPost( void );
    virtual int _EnablePre( void );
    virtual int _EnablePost( void );
    virtual int _DisablePre( void );
    virtual int _DisablePost( void );

};



#endif /*__MT6575_MDP_ELEMENT_H__*/
