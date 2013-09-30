/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////
// BMQFile.h  $Revision$
////////////////////////////////////////////////////////////////////////////////

//! \file  BMQFile.h
//! \brief

#ifndef _BMQFILE_
#define _BMQFILE_

//#include "AcdkTypes.h"
//#include <stdio.h>
#define  BMQ_IMAGE_XSIZE         4032
#define  BMQ_IMAGE_YSIZE         3024
#define  BMQ_BITS_PER_PIXEL      16
#define  BMQ_BYTES_PER_PIXEL     2

#define  BMQ_SIZE_BYTE(w,h) ((w * h * BMQ_BYTES_PER_PIXEL * 3) / 4)

///////////////////////////////////////////////////////////////////////////////
//
//! \brief Structure for Raw file header: size = 54 bytes
//
///////////////////////////////////////////////////////////////////////////////
struct BMQFileHeader
{
    MUINT16  stSignature;            //!<: 'BM' = 0x4d42
    MUINT32  ulFileSize;             //!<: Xsize * Ysize * sizeof(WORD) + header length
    MUINT16  reserved1;              //!<: (not used)
    MUINT16  reserved2;              //!<: (not used)
    MUINT32  headerSize;             //!<: header size or offset to data = 54
    MUINT32  ulInfoHeaderSize;       //!<: size of infoHeader = 40
    MUINT32  ulXLength;              //!<: x size = Horizontal pixels
    MUINT32  ulYLength;              //!<: y size = Vertical pixel
    MUINT16  uPlanes;                //!<: number of planes =1
    MUINT16  uBitPerPixel;           //!<: pixel bit depth = bits per pixel = 16

    MUINT32   a_1;                    //!<: Compression method = 0 (not used)
    MUINT32   a_2;                    //!<: Rawdata size for image without header
    MUINT32   a_3;                    //!<: Horizontal resolution =0 (not used)
    MUINT32   a_4;                    //!<: Vertical resolution =0 (not used)
    MUINT32   a_5;                    //!<: Color = 0 (not used)
    MUINT32   a_6;                    //!<: Effective bit depth for BMQ format(1024 or 4096)

}__attribute__((packed));

typedef struct BMQFileHeader BMQFileHeader; //!<: BMQFileHeader structure


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//! BMQFile Class - Base class of the BMQ object
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

class BMQFile
{

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// Constructor/Destructor
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
  public:

    ////////////////////////////////////////////////////////////////////
    //
    //  BMQFile()
    //! \brief Construct a file object with full file path and open mode.
    //
    ////////////////////////////////////////////////////////////////////
    BMQFile(MUINT8 *pName);


    ////////////////////////////////////////////////////////////////////
    //
    //  ~BMQFile()
    //! \brief RAW object destructor.
    //
    ////////////////////////////////////////////////////////////////////
   virtual ~BMQFile();


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// Public
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//  public:
    /////////////////////////////////////////////////////////////////////////
    // Public Data/Properties
    /////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////
    // Public Methods
    /////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    //
    //! \brief Application Note:
    //!
    //! When building a Raw file, RawFile::buildFile() is the only function
    //! that needs to be called. If any RawFiler information needs to be set
    //! or changed, call the corresponding member function before calling
    //! RawFile::buildFile().
    //
    ///////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////
    //
    //  buildFile():
    //! \brief Build a RAW data file.
    //!
    //! NOTE: All proper header entries must be set before calling this function.
    //! Otherwise, the default setup will be used.
    //!
    //! \param  pDataBuf - pointer to raw data
    //! \param  ulXSize - X length of image
    //! \param  ulYSize - Y length of Image
    //!
    //! \return TRUE if successful, FALSE if fails.
    //!
    ///////////////////////////////////////////////////////////////////////////
    BOOL buildFile ( MUINT8* pDataBuf,
                      MUINT32 ulXSize,
                      MUINT32 ulYSize   );


    ///////////////////////////////////////////////////////////////////////////
    //
    //  writeHeader():
    //! \brief Build and write raw file header.
    //!
    //! \return TRUE if successful, FALSE if fails.
    //
    ///////////////////////////////////////////////////////////////////////////
    BOOL writeHeader();


    ///////////////////////////////////////////////////////////////////////////
    //
    //  writeData():
    //! \brief Write raw file data.
    //!
    //! NOTE: This function can only be called after the file handler points
    //! to data area.
    //!
    //! \param  pData - data to write
    //! \param  ulSize - the size of data to write
    //!
    //! \return number of bytes written
    //
    ///////////////////////////////////////////////////////////////////////////
    MINT32 writeData(MUINT8* pData, MUINT32 ulSize);


    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    //
    //! \brief Application NOTE: the sequence of fetching a Raw file is: <br>
    //! 1). decodeFile()__return the size of audio data size <br>
    //! 2). allocate memory from heap based upon the size from decodeFile() <br>
    //! 3). readData()
    //
    ///////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////
    //
    //  decodeFile():
    //! \brief Decode raw file and return data size.
    //!
    //! \param  ulDataSize -  size of wave data
    //!
    //! \return TRUE if successful, FALSE if fails.
    //
    ///////////////////////////////////////////////////////////////////////////
    BOOL decodeFile (MUINT32&  ulDataSize);


    ///////////////////////////////////////////////////////////////////////////
    //
    //  readHeader():
    //! \brief Read and validate raw file header.
    //!
    //! \return TRUE if successful, FALSE if fails.
    //!
    ///////////////////////////////////////////////////////////////////////////
    BOOL readHeader();

    ///////////////////////////////////////////////////////////////////////////
    //
    //  readData():
    //! \brief Read raw file data.
    //!
    //! NOTE: this function can only be called after the file handler points
    //! to data area.
    //!
    //! \param  pData - data to read
    //! \param  ulSize - size of data to read
    //!
    //! \return number of bytes read
    //
    ///////////////////////////////////////////////////////////////////////////
    MINT32 readData(MUINT8* pData, MUINT32 ulSize);


    ///////////////////////////////////////////////////////////////////////////
    //
    //  setXYLength():
    //! \brief Set X,Y dimension for raw image.
    //!
    //! \param  ulXLength - X length
    //! \param  ulYLength - Y length
    //
    ///////////////////////////////////////////////////////////////////////////
    MVOID setXYLength(MUINT32 ulXLength,MUINT32 ulYLength);


    ///////////////////////////////////////////////////////////////////////////
    //
    //  getXYLength():
    //! \brief Get X,Y dimension for raw image.
    //!
    //! \param  ulXLength - X length
    //! \param  ulYLength - Y length
    //
    ///////////////////////////////////////////////////////////////////////////
    MVOID getXYLength(MUINT32& ulXLength,MUINT32& ulYLength);


    ///////////////////////////////////////////////////////////////////////////
    //
    //  getDataLength():
    //! \brief Get raw data length.
    //!
    //! \return data length
    //
    ///////////////////////////////////////////////////////////////////////////
    MUINT32 getDataLength();


    ///////////////////////////////////////////////////////////////////////////
    //
    //  setBitsPerPixel():
    //! \brief Set bits per pixel for raw image.
    //!
    //! \param  uBits - the number of bits
    //
    ///////////////////////////////////////////////////////////////////////////
    MVOID setBitsPerPixel(MUINT16 uBits);


    ///////////////////////////////////////////////////////////////////////////
    //
    //  getBitsPerPixel():
    //! \brief Get bits per pixel for raw image.
    //
    ///////////////////////////////////////////////////////////////////////////
    MUINT16 getBitsPerPixel();


    ///////////////////////////////////////////////////////////////////////////
    //
    //  setBitsDepth():
    //! \brief Set effective bits depth, 10bit=1024, 12 bits= 4096.
    //!
    //! \param  ulDepth - the bit depth
    //
    ///////////////////////////////////////////////////////////////////////////
    MVOID setBitsDepth(MUINT32 ulDepth);


    ///////////////////////////////////////////////////////////////////////////
    //
    //  getBitsDepth():
    //! \brief Get bits per pixel for raw image.
    //
    ///////////////////////////////////////////////////////////////////////////
    MUINT32 getBitsDepth();

    BOOL bOpenFile(); 

    BOOL bCloseFile(); 


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// Protected
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
protected:


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// Private
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
private:

    /////////////////////////////////////////////////////////////////////////
    // Private Data/Properties
    /////////////////////////////////////////////////////////////////////////
    BMQFileHeader  m_stHeader;
    MUINT8 *m_puFileName; 
    FILE *m_pFile; 

};

#endif // _BMQ_FILE_H_
