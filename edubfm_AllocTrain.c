/******************************************************************************/
/*                                                                            */
/*    ODYSSEUS/EduCOSMOS Educational-Purpose Object Storage System            */
/*                                                                            */
/*    Developed by Professor Kyu-Young Whang et al.                           */
/*                                                                            */
/*    Database and Multimedia Laboratory                                      */
/*                                                                            */
/*    Computer Science Department and                                         */
/*    Advanced Information Technology Research Center (AITrc)                 */
/*    Korea Advanced Institute of Science and Technology (KAIST)              */
/*                                                                            */
/*    e-mail: kywhang@cs.kaist.ac.kr                                          */
/*    phone: +82-42-350-7722                                                  */
/*    fax: +82-42-350-8380                                                    */
/*                                                                            */
/*    Copyright (c) 1995-2013 by Kyu-Young Whang                              */
/*                                                                            */
/*    All rights reserved. No part of this software may be reproduced,        */
/*    stored in a retrieval system, or transmitted, in any form or by any     */
/*    means, electronic, mechanical, photocopying, recording, or otherwise,   */
/*    without prior written permission of the copyright owner.                */
/*                                                                            */
/******************************************************************************/
/*
 * Module: edubfm_AllocTrain.c
 *
 * Description : 
 *  Allocate a new buffer from the buffer pool.
 *
 * Exports:
 *  Four edubfm_AllocTrain(Four)
 */


#include <errno.h>
#include "EduBfM_common.h"
#include "EduBfM_Internal.h"


extern CfgParams_T sm_cfgParams;

/*@================================
 * edubfm_AllocTrain()
 *================================*/
/*
 * Function: Four edubfm_AllocTrain(Four)
 *
 * Description : 
 * (Following description is for original ODYSSEUS/COSMOS BfM.
 *  For ODYSSEUS/EduCOSMOS EduBfM, refer to the EduBfM project manual.)
 *
 *  Allocate a new buffer from the buffer pool.
 *  The used buffer pool is specified by the parameter 'type'.
 *  This routine uses the second chance buffer replacement algorithm
 *  to select a victim.  That is, if the reference bit of current checking
 *  entry (indicated by BI_NEXTVICTIM(type), macro for
 *  bufInfo[type].nextVictim) is set, then simply clear
 *  the bit for the second chance and proceed to the next entry, otherwise
 *  the current buffer indicated by BI_NEXTVICTIM(type) is selected to be
 *  returned.
 *  Before return the buffer, if the dirty bit of the victim is set, it 
 *  must be force out to the disk.
 *
 * Returns;
 *  1) An index of a new buffer from the buffer pool
 *  2) Error codes: Negative value means error code.
 *     eNOUNFIXEDBUF_BFM - There is no unfixed buffer.
 *     some errors caused by fuction calls
 */
Four edubfm_AllocTrain(
    Four 	type)			/* IN type of buffer (PAGE or TRAIN) */
{
    Four 	e;			/* for error */
    Four 	victim;			/* return value */
    Four 	i;
    

	/* Error check whether using not supported functionality by EduBfM */
	if(sm_cfgParams.useBulkFlush) ERR(eNOTSUPPORTED_EDUBFM);

    // char *bufpoolptr = BI_BUFFERPOOL(type);
    UTwo candidate_idx = BI_NEXTVICTIM(type);
    char* candidate =  BI_BUFFER(type, candidate_idx);
    One candidate_bits =  BI_BITS(type, candidate_idx);
    // One    	bits;		/* bit 1 : DIRTY(0b01), bit 2 : VALID(0b10), bit 3 : REFER(0b11), bit 4 : NEW(0b100) */
    for (i = 0; i < BI_NBUFS(type); i ++){ // TODO BI_NBUFS 타입 Two 
        candidate_idx = BI_NEXTVICTIM(type);
        BI_NEXTVICTIM(type) += 1;
        BI_NEXTVICTIM(type) %= BI_NBUFS(type);
        
        if (BI_FIXED(type, candidate_idx))
            continue;

        if(BI_BITS(type, candidate_idx) & VALID){ // new?
            victim = candidate_idx;
        }
        else if(BI_BITS(type, candidate_idx) & REFER){
            BI_BITS(type, candidate_idx) &= ~REFER;
        }
    }

    // flush and do hash reordering
    PageID * pid = &BI_KEY(type, victim);
    if ( !IS_NILBFMHASHKEY( *((BfMHashKey*)pid) ) ) {
        e = bfm_FlushTrain(pid, type); // TODO to implement edubfm_FlushTrain
        if ( e < 0 ) ERR( e );
        e = bfm_Delete(pid, type); // TODO to implement edubfm_Delete
        if ( e < 0 ) ERR( e );
    }






    
    return( victim );
    
}  /* edubfm_AllocTrain */
