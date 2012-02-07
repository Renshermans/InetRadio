/*
 *  Copyright STREAMIT BV, 2010.
 *
 *  Project             : SIR
 *  Module              : Util
 *  File name  $Workfile: Util.c  $
 *       Last Save $Date: 2006/05/11 9:53:22  $
 *             $Revision: 0.1  $
 *  Creation Date       : 2006/05/11 9:53:22
 *
 *  Description         : Utility functions for the SIR project
 *
 */

#define LOG_MODULE  LOG_UTIL_MODULE

/*--------------------------------------------------------------------------*/
/*  Include files                                                           */
/*--------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>

#include <sys/heap.h>

//#pragma text:appcode

#include "system.h"
#include "log.h"

#include "util.h"

/*--------------------------------------------------------------------------*/
/*  Constant definitions                                                    */
/*--------------------------------------------------------------------------*/
#ifdef DEBUG
//#define UTIL_DEBUG
#endif /* #ifdef DEBUG */

/*--------------------------------------------------------------------------*/
/*  Type declarations                                                       */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*  Local variables                                                         */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*  Global variables                                                        */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*  Local functions                                                         */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*  Global functions                                                        */
/*--------------------------------------------------------------------------*/

/*!
 * \brief Allocate memory.
 *
 * \param   unSize [in] Amount of memory to allocate.
 *
 * \return  pointer to the allocated memory.
 *          NULL if there is no memory left or you requested 0 size.
 */
void *MyMalloc(unsigned int unSize)
{
    void *pResult = NULL;

    if ((unSize != 0) &&
        ((pResult = NutHeapAlloc(unSize)) == NULL))
    {
        LogMsg_P(LOG_ERR, PSTR("No memory [%u]"), unSize);
    }
    return (pResult);
}

/*!
 * \brief Create a copy of a string.
 *
 * Allocates sufficient memory from heap for a copy of the string
 * and does the copy.
 *
 * \param   str [in] Pointer to the string to copy.
 *
 * \return  A pointer to the new string.
 *          NULL if allocating memory failed.
 */
char *strdup(CONST char *str)
{
    char *copy = NULL;

    if (str != NULL)
    {
        size_t siz = strlen(str) + 1;

        if ((copy = MyMalloc(siz)) != NULL)
        {
            memcpy(copy, str, siz);
        }
    }
    return (copy);
}

/*!
 * \brief Allocate new memory if needed.
 *
 * Checks if a memory block is large enough to hold additional data.
 * If it is not, the buffer is reallocated so it can hold the additional data.
 *
 * \param   ppcBuf [in,out] Address of a pointer to a memory block.
 * \param   punBufSize [in,out] The currently allocated size, [out] the new blocksize
 * \param   unBufInUse [in] Currently in use.
 * \param   unSizeNeeded [in] Size of the data to add.
 *
 * \return  0 when the buffer is large enough to add the data.
 *          -1 if no new memory could be allocated.
 */
int BufferMakeRoom(char **ppcBuf, unsigned int *punBufSize, unsigned int unBufInUse, unsigned int unSizeNeeded)
{
#ifdef UTIL_DEBUG
    LogMsg_P(LOG_DEBUG, PSTR("Have %u,Need %u"), (*punBufSize - unBufInUse), unSizeNeeded);
#endif /* #ifdef UTIL_DEBUG */

    if (unSizeNeeded > (*punBufSize - unBufInUse))
    {
        unsigned int unBlockSize = 256;
        char *pNewBuf = NULL;

        if (unBlockSize < unSizeNeeded)
        {
            unBlockSize = unSizeNeeded;
        }

        pNewBuf = MyMalloc(*punBufSize + unBlockSize);
        if (pNewBuf == NULL)
        {
            return (-1);
        }
        else
        {
            *punBufSize += unBlockSize;

#ifdef UTIL_DEBUG
            LogMsg_P(LOG_DEBUG, PSTR("MemBlock is %u now"), *punBufSize);
#endif /* #ifdef UTIL_DEBUG */

            if (*ppcBuf != NULL)
            {
                memcpy(pNewBuf, *ppcBuf, unBufInUse);
            }
            MyFree(*ppcBuf);
            *ppcBuf = pNewBuf;
        }
    }
    return (0);
}

/*!
 * \brief Add a string to a memory block.
 *
 * \param   ppcBuf [in] Address of a pointer to a memory block.
 * \param   punBufSize [in,out] The currently allocated size, [out] the new blocksize
 * \param   unBufInUse [in,out] Currently in use, [out] in use after adding the string
 * \param   pszString [in] String to add.
 *
 * \return  0 when the string was successfully added.
 *          -1 on errors.
 */
int BufferAddString(char **ppcBuf, unsigned int *punBufSize, unsigned int *punBufInUse, CONST char *pszString)
{
    unsigned int unStringLen = 0;

    if (pszString == NULL)
    {
        return (-1);
    }

    /*
     * Add the line to the response buffer
     */
    unStringLen = strlen(pszString);
    if (unStringLen > 0)
    {
        unStringLen += 1;   /* Correct for \0 */
        if (BufferMakeRoom(ppcBuf, punBufSize, *punBufInUse, unStringLen) < 0)
        {
            return (-1);
        }
        else
        {
            /* Only count one \0 (so, in the InUse counter, only count the \0 the very first time) */
            if (*punBufInUse != 0)
            {
                *punBufInUse -= 1;
            }
            memcpy(&(*ppcBuf)[*punBufInUse], pszString, unStringLen);
            *punBufInUse += unStringLen;
        }
    }
    return (0);
}

/*!
 * \brief Find a descriptor for a piece of text.
 *
 * A LookUp Table (LUT) is searched for matching text.
 * The row in which the match was found is returned.
 * If no match was found, the descriptor of the last entry
 * is returned. As a result, the table should always contain
 * at least one entry. And that last one should be the
 * default/empty or error value, depending on your needs.
 *
 * \note    The compare used is not case sensitive.
 * \note    If byLen is 0, only the first part of pcText
 *          needs to match.
 *          E.g. "foo" in the LUT will match "foo",
 *          E.g. "foo" in the LUT will match "foobar"
 *          By passing the length of pcText (not including the
 *          \0 character), will force an exact match.
 *          E.g. "foo" in the LUT will match "foo",
 *          E.g. "foo" in the LUT will not match "foobar",
 *
 * \param   tLookupTable [in] The lookup table.
 * \param   pcText [in] The text to find the value for
 *          this does not need to be 0 terminated.
 * \param   byLen [in] See notes above.
 *
 * \return  The descriptor of the row that matched.
 *          Or the descriptor of the last row if no match.
 */
void *LutSearch(CONST tLut tLookupTable[],
                CONST char *pcText,
                unsigned char byLen)
{
    unsigned char byRow = 0;

    for (byRow = 0; byRow < (unsigned char)(-1); byRow++)
    {
        unsigned char byTagLen = 0;

        if (tLookupTable[byRow].pszTag == NULL)
        {
            break;
        }

        byTagLen = strlen_P(tLookupTable[byRow].pszTag);

        if ((byLen != 0) && (byTagLen != byLen))
        {
            continue;   /* not the same size; keep looking */
        }

        /* case-insensitive compare */
        if (strncasecmp_P(pcText, tLookupTable[byRow].pszTag, byTagLen) == 0)
        {
            break;
        }
    }

#ifdef UTIL_DEBUG
    LogMsg_P(LOG_DEBUG, PSTR("Match %d"), byRow);
#endif /* #ifdef UTIL_DEBUG */

    return (tLookupTable[byRow].pDesc);
}
