/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2014, ITU/ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ITU/ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file     TComPic.cpp
    \brief    picture class
*/

#include "TComPic.h"
#include "SEI.h"

//! \ingroup TLibCommon
//! \{

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TComPic::TComPic()
: m_uiTLayer                              (0)
, m_bUsedByCurr                           (false)
, m_bIsLongTerm                           (false)
, m_apcPicSym                             (NULL)
, m_pcPicYuvPred                          (NULL)
, m_pcPicYuvResi                          (NULL)
, m_bReconstructed                        (false)
, m_bNeededForOutput                      (false)
, m_uiCurrSliceIdx                        (0)
, m_bCheckLTMSB                           (false)
{
  for(UInt i=0; i<NUM_PIC_YUV; i++)
  {
    m_apcPicYuv[i]      = NULL;
  }
}

TComPic::~TComPic()
{
}

Void TComPic::create( Int iWidth, Int iHeight, ChromaFormat chromaFormatIDC, UInt uiMaxWidth, UInt uiMaxHeight, UInt uiMaxDepth, Window &conformanceWindow, Window &defaultDisplayWindow,
                      Int *numReorderPics, Bool bIsVirtual)
{
  m_apcPicSym     = new TComPicSym;  m_apcPicSym   ->create( chromaFormatIDC, iWidth, iHeight, uiMaxWidth, uiMaxHeight, uiMaxDepth );
  if (!bIsVirtual)
  {
    m_apcPicYuv[PIC_YUV_ORG]  = new TComPicYuv;  m_apcPicYuv[PIC_YUV_ORG]->create( iWidth, iHeight, chromaFormatIDC, uiMaxWidth, uiMaxHeight, uiMaxDepth );
    m_apcPicYuv[PIC_YUV_TRUE_ORG]  = new TComPicYuv;  m_apcPicYuv[PIC_YUV_TRUE_ORG]->create( iWidth, iHeight, chromaFormatIDC, uiMaxWidth, uiMaxHeight, uiMaxDepth );
  }
  m_apcPicYuv[PIC_YUV_REC]  = new TComPicYuv;  m_apcPicYuv[PIC_YUV_REC]->create( iWidth, iHeight, chromaFormatIDC, uiMaxWidth, uiMaxHeight, uiMaxDepth );

  // there are no SEI messages associated with this picture initially
  if (m_SEIs.size() > 0)
  {
    deleteSEIs (m_SEIs);
  }
  m_bUsedByCurr = false;

  /* store conformance window parameters with picture */
  m_conformanceWindow = conformanceWindow;

  /* store display window parameters with picture */
  m_defaultDisplayWindow = defaultDisplayWindow;

  /* store number of reorder pics with picture */
  memcpy(m_numReorderPics, numReorderPics, MAX_TLAYER*sizeof(Int));

#if SCM__Q0248_INTER_ME_HASH_SEARCH
  m_hashMap.clearAll();
#endif

  return;
}

Void TComPic::destroy()
{
  if (m_apcPicSym)
  {
    m_apcPicSym->destroy();
    delete m_apcPicSym;
    m_apcPicSym = NULL;
  }

  for(UInt i=0; i<NUM_PIC_YUV; i++)
  {
    if (m_apcPicYuv[i])
    {
      m_apcPicYuv[i]->destroy();
      delete m_apcPicYuv[i];
      m_apcPicYuv[i]  = NULL;
    }
  }

#if SCM__Q0248_INTER_ME_HASH_SEARCH
  m_hashMap.clearAll();
#endif

  deleteSEIs(m_SEIs);
}

Void TComPic::compressMotion()
{
  TComPicSym* pPicSym = getPicSym();
  for ( UInt uiCUAddr = 0; uiCUAddr < pPicSym->getFrameHeightInCU()*pPicSym->getFrameWidthInCU(); uiCUAddr++ )
  {
    TComDataCU* pcCU = pPicSym->getCU(uiCUAddr);
    pcCU->compressMV();
  }
}

Bool  TComPic::getSAOMergeAvailability(Int currAddr, Int mergeAddr)
{
  Bool mergeCtbInSliceSeg = (mergeAddr >= getPicSym()->getCUOrderMap(getCU(currAddr)->getSlice()->getSliceCurStartCUAddr()/getNumPartInCU()));
  Bool mergeCtbInTile     = (getPicSym()->getTileIdxMap(mergeAddr) == getPicSym()->getTileIdxMap(currAddr));
  return (mergeCtbInSliceSeg && mergeCtbInTile);
}

#if SCM__Q0248_INTER_ME_HASH_SEARCH
Void TComPic::addPictureToHashMapForInter()
{
  Int picWidth = getSlice( 0 )->getSPS()->getPicWidthInLumaSamples();
  Int picHeight = getSlice( 0 )->getSPS()->getPicHeightInLumaSamples();

  m_hashMap.create();
  m_hashMap.addToHashMapByRow( getPicYuvOrg(), picWidth, picHeight, 8, 8 );
  m_hashMap.addToHashMapByRow( getPicYuvOrg(), picWidth, picHeight, 16, 16 );
  m_hashMap.addToHashMapByRow( getPicYuvOrg(), picWidth, picHeight, 32, 32 );
  m_hashMap.addToHashMapByRow( getPicYuvOrg(), picWidth, picHeight, 64, 64 );
}
#endif 

//! \}
