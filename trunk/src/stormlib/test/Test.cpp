/*****************************************************************************/
/* StormLibTest.cpp                       Copyright (c) Ladislav Zezula 2003 */
/*---------------------------------------------------------------------------*/
/* Test module for StormLib                                                  */
/*---------------------------------------------------------------------------*/
/*   Date    Ver   Who  Comment                                              */
/* --------  ----  ---  -------                                              */
/* 25.03.03  1.00  Lad  The first version of StormLibTest.cpp                */
/*****************************************************************************/

#define _CRT_SECURE_NO_DEPRECATE
#define __INCLUDE_CRYPTOGRAPHY__
#define __STORMLIB_SELF__                   // Don't use StormLib.lib
#include <stdio.h>

#ifdef _MSC_VER
#include <crtdbg.h>
#endif

#include "../src/StormLib.h"
#include "../src/StormCommon.h"

#ifdef _MSC_VER
#pragma warning(disable: 4505)              // 'XXX' : unreferenced local function has been removed
#endif

//------------------------------------------------------------------------------
// Defines

#ifdef PLATFORM_WINDOWS
#define WORK_PATH_ROOT "E:\\Multimedia\\MPQs\\"
#endif

#ifdef PLATFORM_LINUX
#define WORK_PATH_ROOT "/home/user/MPQs/"
#endif

#ifdef PLATFORM_MAC
#define WORK_PATH_ROOT "/Users/sam/Downloads/"
#endif

#ifndef LANG_CZECH
#define LANG_CZECH 0x0405
#endif

#define MPQ_SECTOR_SIZE 0x1000

#define MAKE_PATH(path) (WORK_PATH_ROOT path)

//-----------------------------------------------------------------------------
// Constants

static const char * szWorkDir = MAKE_PATH("Work");

static unsigned int AddFlags[] = 
{
//  Compression          Encryption             Fixed key           Single Unit            Sector CRC
    0                 |  0                   |  0                 | 0                    | 0,
    0                 |  MPQ_FILE_ENCRYPTED  |  0                 | 0                    | 0,
    0                 |  MPQ_FILE_ENCRYPTED  |  MPQ_FILE_FIX_KEY  | 0                    | 0,
    0                 |  0                   |  0                 | MPQ_FILE_SINGLE_UNIT | 0,
    0                 |  MPQ_FILE_ENCRYPTED  |  0                 | MPQ_FILE_SINGLE_UNIT | 0,
    0                 |  MPQ_FILE_ENCRYPTED  |  MPQ_FILE_FIX_KEY  | MPQ_FILE_SINGLE_UNIT | 0,
    MPQ_FILE_IMPLODE  |  0                   |  0                 | 0                    | 0,
    MPQ_FILE_IMPLODE  |  MPQ_FILE_ENCRYPTED  |  0                 | 0                    | 0,
    MPQ_FILE_IMPLODE  |  MPQ_FILE_ENCRYPTED  |  MPQ_FILE_FIX_KEY  | 0                    | 0,
    MPQ_FILE_IMPLODE  |  0                   |  0                 | MPQ_FILE_SINGLE_UNIT | 0,
    MPQ_FILE_IMPLODE  |  MPQ_FILE_ENCRYPTED  |  0                 | MPQ_FILE_SINGLE_UNIT | 0,
    MPQ_FILE_IMPLODE  |  MPQ_FILE_ENCRYPTED  |  MPQ_FILE_FIX_KEY  | MPQ_FILE_SINGLE_UNIT | 0,
    MPQ_FILE_IMPLODE  |  0                   |  0                 | 0                    | MPQ_FILE_SECTOR_CRC,
    MPQ_FILE_IMPLODE  |  MPQ_FILE_ENCRYPTED  |  0                 | 0                    | MPQ_FILE_SECTOR_CRC,
    MPQ_FILE_IMPLODE  |  MPQ_FILE_ENCRYPTED  |  MPQ_FILE_FIX_KEY  | 0                    | MPQ_FILE_SECTOR_CRC,
    MPQ_FILE_COMPRESS |  0                   |  0                 | 0                    | 0,
    MPQ_FILE_COMPRESS |  MPQ_FILE_ENCRYPTED  |  0                 | 0                    | 0,
    MPQ_FILE_COMPRESS |  MPQ_FILE_ENCRYPTED  |  MPQ_FILE_FIX_KEY  | 0                    | 0,
    MPQ_FILE_COMPRESS |  0                   |  0                 | MPQ_FILE_SINGLE_UNIT | 0,
    MPQ_FILE_COMPRESS |  MPQ_FILE_ENCRYPTED  |  0                 | MPQ_FILE_SINGLE_UNIT | 0,
    MPQ_FILE_COMPRESS |  MPQ_FILE_ENCRYPTED  |  MPQ_FILE_FIX_KEY  | MPQ_FILE_SINGLE_UNIT | 0,
    MPQ_FILE_COMPRESS |  0                   |  0                 | 0                    | MPQ_FILE_SECTOR_CRC,
    MPQ_FILE_COMPRESS |  MPQ_FILE_ENCRYPTED  |  0                 | 0                    | MPQ_FILE_SECTOR_CRC,
    MPQ_FILE_COMPRESS |  MPQ_FILE_ENCRYPTED  |  MPQ_FILE_FIX_KEY  | 0                    | MPQ_FILE_SECTOR_CRC,
    0xFFFFFFFF
};

//-----------------------------------------------------------------------------
// Local testing functions

static void clreol()
{
#ifdef PLATFORM_WINDOWS
    CONSOLE_SCREEN_BUFFER_INFO ScreenInfo;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    LPTSTR szConsoleLine;
    int nConsoleChars;
    int i = 0;

    GetConsoleScreenBufferInfo(hConsole, &ScreenInfo);
    nConsoleChars = (ScreenInfo.srWindow.Right - ScreenInfo.srWindow.Left);
    if(nConsoleChars > 0)
    {
        szConsoleLine = new TCHAR[nConsoleChars + 3];
        if(szConsoleLine != NULL)
        {
            szConsoleLine[i++] = '\r';
            for(; i < nConsoleChars; i++)
                szConsoleLine[i] = ' ';
            szConsoleLine[i++] = '\r';
            szConsoleLine[i] = 0;

            printf(szConsoleLine);
            delete []  szConsoleLine;
        }
    }
#endif // PLATFORM_WINDOWS
}

static const char * GetPlainName(const char * szFileName)
{
    const char * szTemp;

    if((szTemp = strrchr(szFileName, '\\')) != NULL)
        szFileName = szTemp + 1;
    return szFileName;
}

int GetFirstDiffer(void * ptr1, void * ptr2, int nSize)
{
    char * buff1 = (char *)ptr1;
    char * buff2 = (char *)ptr2;
    int nDiffer;

    for(nDiffer = 0; nDiffer < nSize; nDiffer++)
    {
        if(*buff1++ != *buff2++)
            return nDiffer;
    }
    return -1;
}

static void WINAPI CompactCB(void * /* lpParam */, DWORD dwWork, ULONGLONG BytesDone, ULONGLONG TotalBytes)
{
    clreol();

    printf("%u of %u ", (DWORD)BytesDone, (DWORD)TotalBytes);
    switch(dwWork)
    {
        case CCB_CHECKING_FILES:
            printf("Checking files in archive ...\r");
            break;

        case CCB_CHECKING_HASH_TABLE:
            printf("Checking hash table ...\r");
            break;

        case CCB_COPYING_NON_MPQ_DATA:
            printf("Copying non-MPQ data ...\r");
            break;

        case CCB_COMPACTING_FILES:
            printf("Compacting archive ...\r");
            break;

        case CCB_CLOSING_ARCHIVE:
            printf("Closing archive ...\r");
            break;
    }
}

static void GenerateRandomDataBlock(LPBYTE pbBuffer, DWORD cbBuffer)
{
    LPBYTE pbBufferEnd = pbBuffer + cbBuffer;
    LPBYTE pbPtr = pbBuffer;
    DWORD cbBytesToPut = 0;
    BYTE ByteToPut = 0;
    bool bRandomData = false;

    while(pbPtr < pbBufferEnd)
    {
        // If there are no bytes to put, we will generate new byte and length
        if(cbBytesToPut == 0)
        {
            bRandomData = false;
            switch(rand() % 10)
            {
                case 0:     // A short sequence of zeros
                    cbBytesToPut = rand() % 0x08;
                    ByteToPut = 0;
                    break;

                case 1:     // A long sequence of zeros
                    cbBytesToPut = rand() % 0x80;
                    ByteToPut = 0;
                    break;

                case 2:     // A short sequence of non-zeros
                    cbBytesToPut = rand() % 0x08;
                    ByteToPut = (BYTE)(rand() % 0x100);
                    break;

                case 3:     // A long sequence of non-zeros
                    cbBytesToPut = rand() % 0x80;
                    ByteToPut = (BYTE)(rand() % 0x100);
                    break;

                case 4:     // A short random data
                    cbBytesToPut = rand() % 0x08;
                    bRandomData = true;
                    break;

                case 5:     // A long random data
                    cbBytesToPut = rand() % 0x80;
                    bRandomData = true;
                    break;

                default:    // A single random byte
                    cbBytesToPut = 1;
                    ByteToPut = (BYTE)(rand() % 0x100);
                    break;
            }
        }

        // Generate random byte, if needed
        if(bRandomData)
            ByteToPut = (BYTE)(rand() % 0x100);

        // Put next byte to the output buffer
        *pbPtr++ = ByteToPut;
        cbBytesToPut--;
    }
}

static bool CompareArchivedFiles(const char * szFileName, HANDLE hFile1, HANDLE hFile2, DWORD dwBlockSize)
{
    LPBYTE pbBuffer1 = NULL;
    LPBYTE pbBuffer2 = NULL;
    DWORD dwRead1;                      // Number of bytes read (Storm.dll)
    DWORD dwRead2;                      // Number of bytes read (StormLib)
    bool bResult1 = false;              // Result from Storm.dll
    bool bResult2 = false;              // Result from StormLib
    bool bResult = true;
    int nDiff;

    szFileName = szFileName;

    // Allocate buffers
    pbBuffer1 = new BYTE[dwBlockSize];
    pbBuffer2 = new BYTE[dwBlockSize];

    for(;;)
    {
        // Read the file's content by both methods and compare the result
        memset(pbBuffer1, 0, dwBlockSize);
        memset(pbBuffer2, 0, dwBlockSize);
        bResult1 = SFileReadFile(hFile1, pbBuffer1, dwBlockSize, &dwRead1, NULL);
        bResult2 = SFileReadFile(hFile2, pbBuffer2, dwBlockSize, &dwRead2, NULL);
        if(bResult1 != bResult2)
        {
            printf("Different results from SFileReadFile, Mpq1 %u, Mpq2 %u\n", bResult1, bResult2);
            bResult = false;
            break;
        }

        // Test the number of bytes read
        if(dwRead1 != dwRead2)
        {
            printf("Different bytes read from SFileReadFile, Mpq1 %u, Mpq2 %u\n", dwRead1, dwRead2);
            bResult = false;
            break;
        }

        // No more bytes ==> OK
        if(dwRead1 == 0)
            break;
        
        // Test the content
        if((nDiff = GetFirstDiffer(pbBuffer1, pbBuffer2, dwRead1)) != -1)
        {
            bResult = false;
            break;
        }
    }

    delete [] pbBuffer2;
    delete [] pbBuffer1;
    return bResult;
}

// Random read version
static bool CompareArchivedFilesRR(const char * /* szFileName */, HANDLE hFile1, HANDLE hFile2, DWORD dwBlockSize)
{
    const char * szPositions[3] = {"FILE_BEGIN  ", "FILE_CURRENT", "FILE_END    "};
    LPBYTE pbBuffer1 = NULL;
    LPBYTE pbBuffer2 = NULL;
    DWORD dwFileSize1;                  // File size (Storm.dll)
    DWORD dwFileSize2;                  // File size (StormLib)
    DWORD dwRead1;                      // Number of bytes read (Storm.dll)
    DWORD dwRead2;                      // Number of bytes read (StormLib)
    bool bResult1 = false;              // Result from Storm.dll
    bool bResult2 = false;              // Result from StormLib
    int nError = ERROR_SUCCESS;

    // Test the file size
    dwFileSize1 = SFileGetFileSize(hFile1, NULL);
    dwFileSize2 = SFileGetFileSize(hFile2, NULL);
    if(dwFileSize1 != dwFileSize2)
    {
        printf("Different size from SFileGetFileSize, Storm.dll: %u, StormLib: %u !!!!\n", dwFileSize1, dwFileSize2);
        return false;
    }

    if(dwFileSize1 != 0)
    {
        for(int i = 0; i < 10000; i++)
        {
            DWORD dwRandom     = rand() * rand();
            DWORD dwMoveMethod = dwRandom % 3;
            DWORD dwPosition   = dwRandom % dwFileSize1;
            DWORD dwToRead     = dwRandom % dwBlockSize; 

            // Also test negative seek
            if(rand() & 1)
            {
                int nPosition = (int)dwPosition;
                dwPosition = (DWORD)(-nPosition);
            }

            // Allocate buffers
            pbBuffer1 = new BYTE[dwToRead];
            pbBuffer2 = new BYTE[dwToRead];

            // Set the file pointer
            printf("RndRead (%u): pos %8i from %s, size %u ...\r", i, dwPosition, szPositions[dwMoveMethod], dwToRead);
            dwRead1 = SFileSetFilePointer(hFile1, dwPosition, NULL, dwMoveMethod);
            dwRead2 = SFileSetFilePointer(hFile2, dwPosition, NULL, dwMoveMethod);
            if(dwRead1 != dwRead2)
            {
                printf("Difference returned by SFileSetFilePointer, Storm.dll: %u, StormLib: %u !!!!\n", dwRead1, dwRead2);
                nError = ERROR_CAN_NOT_COMPLETE;
                break;
            }

            // Read the file's content by both methods and compare the result
            bResult1 = SFileReadFile(hFile1, pbBuffer1, dwToRead, &dwRead1, NULL);
            bResult2 = SFileReadFile(hFile2, pbBuffer2, dwToRead, &dwRead2, NULL);
            if(bResult1 != bResult2)
            {
                printf("Different results from SFileReadFile, Storm.dll: %u, StormLib: %u !!!!\n", bResult1, bResult2);
                nError = ERROR_CAN_NOT_COMPLETE;
                break;
            }

            // Test the number of bytes read
            if(dwRead1 != dwRead2)
            {
                printf("Different bytes read from SFileReadFile, Storm.dll: %u, StormLib: %u !!!!\n", dwRead1, dwRead2);
                nError = ERROR_CAN_NOT_COMPLETE;
                break;
            }
            
            // Test the content
            if(dwRead1 != 0 && memcmp(pbBuffer1, pbBuffer2, dwRead1))
            {
                printf("Different data content from SFileReadFile !!\n");
                nError = ERROR_CAN_NOT_COMPLETE;
                break;
            }

            delete [] pbBuffer2;
            delete [] pbBuffer1;
        }
    }
    clreol();
    return (nError == ERROR_SUCCESS) ? true : false;
}

//-----------------------------------------------------------------------------
// Partial file reading

static int TestPartFileRead(const char * szFileName)
{
    ULONGLONG ByteOffset;
    ULONGLONG FileSize = 0;
    TFileStream * pStream;
    BYTE BigBuffer[0x7000];
    BYTE Buffer[0x100];
    int nError = ERROR_SUCCESS;

    // Open the partial file
    pStream = FileStream_OpenFile(szFileName, false);
    if(pStream == NULL)
        nError = GetLastError();

    // Get the size of the stream
    if(nError == ERROR_SUCCESS)
    {
        if(!FileStream_GetSize(pStream, FileSize))
            nError = GetLastError();
    }

    // Read the last 0x7000 bytes
    if(nError == ERROR_SUCCESS)
    {
        ByteOffset = FileSize - sizeof(BigBuffer);
        if(!FileStream_Read(pStream, &ByteOffset, BigBuffer, sizeof(BigBuffer)))
            nError = GetLastError();
    }

    // Read the last 0x100 bytes
    if(nError == ERROR_SUCCESS)
    {
        ByteOffset = FileSize - sizeof(Buffer);
        if(!FileStream_Read(pStream, &ByteOffset, Buffer, sizeof(Buffer)))
            nError = GetLastError();
    }

    // Read 0x100 bytes from position (FileSize - 0xFF)
    if(nError == ERROR_SUCCESS)
    {
        ByteOffset = FileSize - sizeof(Buffer) + 1;
        if(!FileStream_Read(pStream, &ByteOffset, Buffer, sizeof(Buffer)))
            nError = GetLastError();
    }

    FileStream_Close(pStream);
    return nError;
}

//-----------------------------------------------------------------------------
// Compare LZMA decompression

#ifdef PLATFORM_WINDOWS
typedef void * (*ALLOC_MEMORY)(size_t);
typedef void (*FREE_MEMORY)(void *);
typedef int (GIVE_DATA)(void *);

extern "C" int starcraft_decompress_lzma(char * pbInBuffer, int cbInBuffer, char * pbOutBuffer, int cbOutBuffer, int * pcbOutBuffer, ALLOC_MEMORY pfnAllocMemory, FREE_MEMORY pfnFreeMemory);
extern "C" int starcraft_compress_lzma(char * pbInBuffer, int cbInBuffer, int dummy1, char * pbOutBuffer, int cbOutBuffer, int dummy2, int * pcbOutBuffer, ALLOC_MEMORY pfnAllocMemory, FREE_MEMORY pfnFreeMemory, GIVE_DATA pfnGiveData);
void Compress_LZMA(char * pbOutBuffer, int * pcbOutBuffer, char * pbInBuffer, int cbInBuffer, int *, int);
int Decompress_LZMA(char * pbOutBuffer, int * pcbOutBuffer, char * pbInBuffer, int cbInBuffer);

extern "C" void * operator_new(size_t sz)
{
    return malloc(sz);
}

void * Memory_Allocate(size_t byte_size)
{
    return malloc(byte_size);
}

void Memory_Free(void * address)
{
    if(address != NULL)
        free(address);
}

int GiveData(void *)
{
    return 0;
}

static int StarcraftCompress_LZMA(char * pbOutBuffer, int * pcbOutBuffer, char * pbInBuffer, int cbInBuffer)
{
    return starcraft_compress_lzma(pbInBuffer,
                                   cbInBuffer,
                                   0,
                                   pbOutBuffer,
                                  *pcbOutBuffer,
                                   0,
                                   pcbOutBuffer,
                                   Memory_Allocate,
                                   Memory_Free,
                                   GiveData);
}

static int StarcraftDecompress_LZMA(char * pbOutBuffer, int * pcbOutBuffer, char * pbInBuffer, int cbInBuffer)
{
    return starcraft_decompress_lzma(pbInBuffer,
                                     cbInBuffer,
                                     pbOutBuffer,
                                     *pcbOutBuffer,
                                     pcbOutBuffer,
                                     Memory_Allocate,
                                     Memory_Free);
}

static int CompareLzmaCompressions(int nSectorSize)
{
    LPBYTE pbCompressed1 = NULL;            // Compressed by our code
    LPBYTE pbCompressed2 = NULL;            // Compressed by Blizzard's code
    LPBYTE pbDecompressed1 = NULL;          // Decompressed by our code
    LPBYTE pbDecompressed2 = NULL;          // Decompressed by Blizzard's code
    LPBYTE pbOriginalData = NULL;
    int nError = ERROR_SUCCESS;

    // Allocate buffers
    // Must allocate twice blocks due to probable bug in Storm.dll.
    // Storm.dll corrupts stack when uncompresses data with PKWARE DCL
    // and no compression occurs.
    pbDecompressed1 = new BYTE [nSectorSize];
    pbDecompressed2 = new BYTE [nSectorSize];
    pbCompressed1 = new BYTE [nSectorSize];
    pbCompressed2 = new BYTE [nSectorSize];
    pbOriginalData = new BYTE[nSectorSize];
    if(!pbDecompressed1 || !pbDecompressed2 || !pbCompressed1 || !pbCompressed2 || !pbOriginalData)
        nError = ERROR_NOT_ENOUGH_MEMORY;

    if(nError == ERROR_SUCCESS)
    {
        for(int i = 0; i < 100000; i++)
        {
            int   nDcmpLength1;
            int   nDcmpLength2;
            int   nCmpLength1;
            int   nCmpLength2;
            int   nDiff;

            clreol();
            printf("Testing compression of sector %u\r", i + 1);

            // Generate random data sector
            GenerateRandomDataBlock(pbOriginalData, nSectorSize);

            // Compress the sector by both methods
            nCmpLength1 = nCmpLength2 = nSectorSize;
//          Compress_LZMA((char *)pbCompressed1, &nCmpLength1, (char *)pbOriginalData, nSectorSize, 0, 0);
            StarcraftCompress_LZMA((char *)pbCompressed1, &nCmpLength2, (char *)pbOriginalData, nSectorSize);

__TryToDecompress:

            // Only test decompression when the compression actually succeeded
            if(nCmpLength1 < nSectorSize)
            {
                // Decompress both data
                nDcmpLength2 = nDcmpLength1 = nSectorSize;
//              Decompress_LZMA((char *)pbDecompressed1, &nDcmpLength1, (char *)pbCompressed1, nCmpLength1);
                StarcraftDecompress_LZMA((char *)pbDecompressed2, &nDcmpLength2, (char *)pbCompressed1, nCmpLength1);

                // Compare the length of the output data
                if(nDcmpLength1 != nDcmpLength2)
                {
                    printf("Difference in compressed blocks lengths (%u vs %u)\n", nDcmpLength1, nDcmpLength2);
                    goto __TryToDecompress;             
                }

                // Compare the output
                if((nDiff = GetFirstDiffer(pbDecompressed1, pbDecompressed2, nDcmpLength1)) != -1)
                {
                    printf("Difference in decompressed blocks (offset 0x%08X)\n", nDiff);
                    goto __TryToDecompress;
                }

                // Check for data overflow
                if(pbDecompressed1[nSectorSize] != 0xFD || pbDecompressed1[nSectorSize] != 0xFD)
                {
                    printf("Damage after decompressed sector !!!\n");
                    goto __TryToDecompress;
                }

                // Compare the decompressed data against original data
                if((nDiff = GetFirstDiffer(pbDecompressed1, pbOriginalData, nDcmpLength1)) != -1)
                {
                    printf("Difference between original data and decompressed data (offset 0x%08X)\n", nDiff);
                    goto __TryToDecompress;
                }
            }
        }
    }

    // Cleanup
    if(pbOriginalData != NULL)
        delete [] pbOriginalData;
    if(pbCompressed2 != NULL)
        delete [] pbCompressed2;
    if(pbCompressed1 != NULL)
        delete [] pbCompressed1;
    if(pbDecompressed2 != NULL)
        delete [] pbDecompressed2;
    if(pbDecompressed1 != NULL)
        delete [] pbDecompressed1;
    clreol();
    return nError;
}
#endif // PLATFORM_WINDOWS

//-----------------------------------------------------------------------------
// Compression method test

static int TestSectorCompress(int nSectorSize)
{
    LPBYTE pbDecompressed = NULL;
    LPBYTE pbCompressed = NULL;
    LPBYTE pbOriginal = NULL;
    int nError = ERROR_SUCCESS;

    // Allocate buffers
    pbDecompressed = new BYTE[nSectorSize];
    pbCompressed = new BYTE[nSectorSize];
    pbOriginal = new BYTE[nSectorSize];
    if(!pbDecompressed || !pbCompressed || !pbOriginal)
        nError = ERROR_NOT_ENOUGH_MEMORY;

    if(nError == ERROR_SUCCESS)
    {
        for(int i = 0; i < 100000; i++)
        {
            int nOriginalLength = nSectorSize % (rand() + 1);
            int nCompressedLength;
            int nDecompressedLength;
            int nCmp = MPQ_COMPRESSION_SPARSE | MPQ_COMPRESSION_ZLIB | MPQ_COMPRESSION_BZIP2 | MPQ_COMPRESSION_PKWARE;
            int nDiff;

            clreol();
            printf("Testing compression of sector %u\r", i + 1);

            // Generate random data sector
            GenerateRandomDataBlock(pbOriginal, nOriginalLength);
            if(nOriginalLength == 0x123)
                nOriginalLength = 0;

__TryAgain:

            // Compress the sector
            nCompressedLength = nOriginalLength;
            SCompCompress((char *)pbCompressed, &nCompressedLength, (char *)pbOriginal, nOriginalLength, nCmp, 0, -1);
//          SCompImplode((char *)pbCompressed, &nCompressedLength, (char *)pbOriginal, nOriginalLength);

            // When the method was unable to compress data,
            // the compressed data must be identical to original data
            if(nCompressedLength == nOriginalLength)
            {
                if((nDiff = GetFirstDiffer(pbCompressed, pbOriginal, nOriginalLength)) != -1)
                {
                    printf("Compression error: Fail when unable to compress the data (Offset 0x%08X).\n", nDiff);
                    goto __TryAgain;
                }
            }

            // Uncompress the sector
            nDecompressedLength = nOriginalLength;
            SCompDecompress((char *)pbDecompressed, &nDecompressedLength, (char *)pbCompressed, nCompressedLength);
//          SCompExplode((char *)pbDecompressed, &nDecompressedLength, (char *)pbCompressed, nCompressedLength);

            // Check the decompressed length against original length
            if(nDecompressedLength != nOriginalLength)
            {
                printf("Length of uncompressed data does not agree with original data length !!!\n");
                goto __TryAgain;
            }
            
            // Check decompressed block against original block
            if((nDiff = GetFirstDiffer(pbDecompressed, pbOriginal, nOriginalLength)) != -1)
            {
                printf("Decompressed sector does not agree with the original data !!! (Offset 0x%08X)\n", nDiff);
                goto __TryAgain;
            }
        }
    }

    // Cleanup
    delete [] pbOriginal;
    delete [] pbCompressed;
    delete [] pbDecompressed;
    clreol();
    return nError;
}

static int TestArchiveOpenAndClose(const char * szMpqName)
{
    const char * szFileName1 = "war3map.wpm";
    TMPQArchive * ha;
    HANDLE hFile1 = NULL;
    HANDLE hFile2 = NULL;
    HANDLE hMpq = NULL;
    int nError = ERROR_SUCCESS;

    if(nError == ERROR_SUCCESS)
    {
        printf("Opening archive %s ...\n", szMpqName);
        if(!SFileOpenArchive(szMpqName, 0, MPQ_OPEN_FORCE_MPQ_V1, /* MPQ_OPEN_ENCRYPTED,*/ &hMpq))
            nError = GetLastError();
        ha = (TMPQArchive *)hMpq;
    }

    // Open listfile from the MPQ
    if(nError == ERROR_SUCCESS)
    {
        // Verify the archive
        SFileVerifyRawData(hMpq, SFILE_VERIFY_MPQ_HEADER, NULL);
        SFileVerifyRawData(hMpq, SFILE_VERIFY_HET_TABLE, NULL);
        SFileVerifyRawData(hMpq, SFILE_VERIFY_BET_TABLE, NULL);
//      SFileVerifyRawData(hMpq, SFILE_VERIFY_HASH_TABLE, NULL);
//      SFileVerifyRawData(hMpq, SFILE_VERIFY_BLOCK_TABLE, NULL);
        SFileVerifyRawData(hMpq, SFILE_VERIFY_HIBLOCK_TABLE, NULL);
        SFileVerifyRawData(hMpq, SFILE_VERIFY_FILE, LISTFILE_NAME);
        SFileVerifyRawData(hMpq, SFILE_VERIFY_FILE, ATTRIBUTES_NAME);

        // Try toopen a file
        if(!SFileOpenFileEx(hMpq, szFileName1, SFILE_OPEN_FROM_MPQ, &hFile1))
        {
            nError = GetLastError();
            printf("%s - file integrity error\n", szFileName1);
        }
    }

    // Dummy read from the file
    if(nError == ERROR_SUCCESS)
	{
        DWORD dwBytesRead = 0;
		BYTE Buffer[0x1000];

        SFileReadFile(hFile1, Buffer, sizeof(Buffer), &dwBytesRead);
	}

    // Verify the MPQ listfile
    if(nError == ERROR_SUCCESS)
    {
        SFileVerifyFile(hMpq, szFileName1, 0xFFFFFFFF); 
        if(!CompareArchivedFilesRR(szFileName1, hFile1, hFile2, 0x100000))
            nError = ERROR_CAN_NOT_COMPLETE;
    }

    if(hFile1 != NULL)
        SFileCloseFile(hFile1);
    if(hMpq != NULL)
        SFileCloseArchive(hMpq);
    return nError;
}

static int TestFindFiles(const char * szMpqName)
{
    TMPQFile * hf;
    HANDLE hFile;
    HANDLE hMpq = NULL;
    BYTE Buffer[100];
    int nError = ERROR_SUCCESS;
    int nFiles = 0;
    int nFound = 0;

    // Open the archive
    if(nError == ERROR_SUCCESS)
    {
        printf("Opening \"%s\" for finding files ...\n", szMpqName);
        if(!SFileOpenArchive(szMpqName, 0, 0, &hMpq))
            nError = GetLastError();
    }

    // Compact the archive
    if(nError == ERROR_SUCCESS)
    {
        SFILE_FIND_DATA sf;
        HANDLE hFind;
        DWORD dwExtraDataSize;
        bool bFound = true;

        hFind = SFileFindFirstFile(hMpq, "*", &sf, "c:\\Tools32\\ListFiles\\ListFile.txt");
        while(hFind != NULL && bFound != false)
        {
            if(SFileOpenFileEx(hMpq, sf.cFileName, 0, &hFile))
            {
                hf = (TMPQFile *)hFile;
                SFileReadFile(hFile, Buffer, sizeof(Buffer));
                nFiles++;

                if(sf.dwFileFlags & MPQ_FILE_SECTOR_CRC)
                {
                    dwExtraDataSize = hf->SectorOffsets[hf->dwSectorCount - 1] - hf->SectorOffsets[hf->dwSectorCount - 2];
                    if(dwExtraDataSize != 0)
                        nFound++;
                }

                SFileCloseFile(hFile);
            }

            bFound = SFileFindNextFile(hFind, &sf);
        }
    }

    if(hMpq != NULL)
        SFileCloseArchive(hMpq);
    if(nError == ERROR_SUCCESS)
        printf("Search complete\n");
    return nError;
}

static int TestMpqCompacting(const char * szMpqName)
{
    HANDLE hMpq = NULL;
    int nError = ERROR_SUCCESS;

    // Open the archive
    if(nError == ERROR_SUCCESS)
    {
        printf("Opening \"%s\" for compacting ...\n", szMpqName);
        if(!SFileOpenArchive(szMpqName, 0, 0, &hMpq))
            nError = GetLastError();
    }
/*
    if(nError == ERROR_SUCCESS)
    {
        char * szFileName = "Campaign\\Human\\Human01.pud";

        printf("Deleting file %s ...\r", szFileName);
        if(!SFileRemoveFile(hMpq, szFileName))
            nError = GetLastError();
    }
*/
    // Compact the archive
    if(nError == ERROR_SUCCESS)
    {
        printf("Compacting archive ...\r");
        SFileSetCompactCallback(hMpq, CompactCB, NULL);
        if(!SFileCompactArchive(hMpq, NULL))
            nError = GetLastError();
    }

    if(hMpq != NULL)
        SFileCloseArchive(hMpq);
    if(nError == ERROR_SUCCESS)
        printf("Compacting complete (No errors)\n");
    return nError;
}


static int TestCreateArchiveV2(const char * szMpqName)
{
    TFileStream * pStream;
    const char * szFileName1 = MAKE_PATH("FileTest.exe");
    const char * szFileName2 = MAKE_PATH("ZeroSize.txt");
    HANDLE hMpq = NULL;                 // Handle of created archive 
    DWORD dwVerifyResult;
    LCID LocaleIDs[] = {0x000, 0x405, 0x406, 0x407, 0xFFFF};
    char szMpqFileName[MAX_PATH];
    int nError = ERROR_SUCCESS;

    // Create the new file
    printf("Creating %s ...\n", szMpqName);
    pStream = FileStream_CreateFile(szMpqName);
    if(pStream == NULL)
        nError = GetLastError();

    // Write some data
    if(nError == ERROR_SUCCESS)
    {
        ULONGLONG FileSize = 0x100000;
      
        FileStream_SetSize(pStream, FileSize);
        FileStream_Close(pStream);
    }

    // Well, now create the MPQ archive
    if(nError == ERROR_SUCCESS)
    {
        if(!SFileCreateArchive(szMpqName,
                               MPQ_CREATE_ARCHIVE_V4 | MPQ_CREATE_ATTRIBUTES,
                               0x40,
                              &hMpq))
        {
            nError = GetLastError();
        }
    }

    // Add the same file multiple times
    if(nError == ERROR_SUCCESS)
    {
        SFileCompactArchive(hMpq);
//      if(!SFileAddFileEx(hMpq,
//                         "e:\\Multimedia\\MPQs\\Achievement.dbc",
//                         "enGB\\DBFilesClient\\Achievement.dbc", 
//                         MPQ_FILE_COMPRESS,
//                         MPQ_COMPRESSION_ZLIB))
//      {
//          printf("Failed to add the file \"%s\".\n", szMpqFileName);
//      }

        // Add FileTest.exe
        for(int i = 0; AddFlags[i] != 0xFFFFFFFF; i++)
        {
            sprintf(szMpqFileName, "FileTest_%02u.exe", i);
            printf("Adding %s as %s ...\n", szFileName1, szMpqFileName);
            if(!SFileAddFileEx(hMpq, szFileName1, szMpqFileName, AddFlags[i], MPQ_COMPRESSION_ZLIB))
                printf("Failed to add the file \"%s\".\n", szMpqFileName);

            dwVerifyResult = SFileVerifyFile(hMpq, szMpqFileName, MPQ_ATTRIBUTE_CRC32 | MPQ_ATTRIBUTE_MD5);
            if(dwVerifyResult & (VERIFY_OPEN_ERROR | VERIFY_READ_ERROR | VERIFY_SECTOR_CHECKSUM_ERROR | VERIFY_FILE_CHECKSUM_ERROR | VERIFY_FILE_MD5_ERROR))
                printf("CRC error on \"%s\"\n", szMpqFileName);
        }

        // Add ZeroSize.txt (1)
        sprintf(szMpqFileName, "ZeroSize_1.txt");
        for(int i = 0; LocaleIDs[i] != 0xFFFF; i++)
        {
            printf("Adding %s as %s (locale %04x) ...\n", szFileName2, szMpqFileName, LocaleIDs[i]);
            SFileSetLocale(LocaleIDs[i]);
            if(!SFileAddFileEx(hMpq, szFileName2, szMpqFileName, MPQ_FILE_COMPRESS | MPQ_FILE_ENCRYPTED, MPQ_COMPRESSION_ZLIB))
                printf("Cannot add the file\n");
        }

        // Add ZeroSize.txt (1)
        sprintf(szMpqFileName, "ZeroSize_2.txt");
        for(int i = 0; LocaleIDs[i] != 0xFFFF; i++)
        {
            printf("Adding %s as %s (locale %04x) ...\n", szFileName2, szMpqFileName, LocaleIDs[i]);
            SFileSetLocale(LocaleIDs[i]);
            if(!SFileAddFileEx(hMpq, szFileName2, szMpqFileName, MPQ_FILE_COMPRESS | MPQ_FILE_ENCRYPTED, MPQ_COMPRESSION_ZLIB))
                printf("Cannot add the file\n");
        }
    }

    // Test rename function
    if(nError == ERROR_SUCCESS)
    {
        printf("Testing rename files ...\n");
        SFileSetLocale(LANG_NEUTRAL);
        if(!SFileRenameFile(hMpq, "FileTest_08.exe", "FileTest_08a.exe"))
        {
            nError = GetLastError();
            printf("Failed to rename the file\n");
        }

        if(!SFileRenameFile(hMpq, "FileTest_08a.exe", "FileTest_08.exe"))
        {
            nError = GetLastError();
            printf("Failed to rename the file\n");
        }

        if(!SFileRenameFile(hMpq, "FileTest_10.exe", "FileTest_10a.exe"))
        {
            nError = GetLastError();
            printf("Failed to rename the file\n");
        }

        if(!SFileRenameFile(hMpq, "FileTest_10a.exe", "FileTest_10.exe"))
        {
            nError = GetLastError();
            printf("Failed to rename the file\n");
        }
        
        if(nError == ERROR_SUCCESS)
            printf("Rename test succeeded.\n\n");
        else
            printf("Rename test failed.\n\n");
    }

    // Test changing hash table size
    if(nError == ERROR_SUCCESS)
        SFileSetHashTableSize(hMpq, 0x100);

    if(hMpq != NULL)
        SFileCloseArchive(hMpq);
    printf("\n");
    return nError;
}

static int TestAddFileToMpq(
    const char * szMpqName,
    const char * szFileName)
{
    HANDLE hMpq;
    int nError = ERROR_SUCCESS;

    if(!SFileOpenArchive(szMpqName, 0, 0, &hMpq))
        return GetLastError();

    if(!SFileAddFileEx(hMpq, szFileName,
                             GetPlainFileName(szFileName),
                             MPQ_FILE_COMPRESS,
                             MPQ_COMPRESSION_ZLIB))
    {
        nError = GetLastError();
    }

    SFileCloseArchive(hMpq);
    return nError;
}

static int TestCreateArchiveFromMemory(const char * szMpqName)
{
#define FILE_SIZE 65535

    HANDLE hFile;
    HANDLE hMPQ;
    char* data = new char [FILE_SIZE];          // random memory data
    char szFileName[100];
    int i;
 
    // Create an mpq file for testing
    if(SFileCreateArchive(szMpqName, MPQ_CREATE_ARCHIVE_V2|MPQ_CREATE_ATTRIBUTES, 0x100000, &hMPQ))
    {
        for(i = 0; i < 1000; i++)
        {
            sprintf(szFileName, "File%03u.bin", i);
            printf("Adding file %s\r", szFileName);

            if(SFileCreateFile(hMPQ, szFileName, NULL, FILE_SIZE, NULL, MPQ_FILE_COMPRESS, &hFile))
            {
                SFileWriteFile(hFile, data, FILE_SIZE, MPQ_COMPRESSION_ZLIB);
                SFileFinishFile(hFile);
            }
        }
    }
    SFileCloseArchive(hMPQ);
    delete [] data;
    return ERROR_SUCCESS;
}

static int TestFileReadAndWrite(
    const char * szMpqName,
    const char * szFileName)
{
    LPBYTE pvFile = NULL;
    HANDLE hFile = NULL;
    HANDLE hMpq = NULL;
    DWORD dwBytesRead;
    DWORD dwFileSize = 0;
    int nError = ERROR_SUCCESS;

    if(!SFileOpenArchive(szMpqName, 0, 0, &hMpq))
    {
        nError = GetLastError();
        printf("Failed to open the archive %s (%u).\n", szMpqName, nError);
    }

    if(nError == ERROR_SUCCESS)
    {
        if(!SFileOpenFileEx(hMpq, szFileName, 0, &hFile))
        {
            nError = GetLastError();
            printf("Failed to open the file %s (%u).\n", szFileName, nError);
        }
    }

    if(nError == ERROR_SUCCESS)
    {
        if(!SFileGetFileInfo(hFile, SFILE_INFO_FILE_SIZE, &dwFileSize, sizeof(DWORD)))
        {
            nError = GetLastError();
            printf("Failed to get the file size (%u).\n", nError);
        }
    }

    if(nError == ERROR_SUCCESS)
    {
        pvFile = new BYTE[dwFileSize];
        if(pvFile == NULL)
        {
            nError = ERROR_NOT_ENOUGH_MEMORY;
            printf("Failed to allocate buffer for the file (%u).\n", nError);
        }
    }

    if(nError == ERROR_SUCCESS)
    {
        if(!SFileReadFile(hFile, pvFile, dwFileSize, &dwBytesRead))
        {
            nError = GetLastError();
            printf("Failed to read file (%u).\n", nError);
        }
    }

    if(hFile != NULL)
    {
        SFileCloseFile(hFile);
        hFile = NULL;
    }

    if(nError == ERROR_SUCCESS)
    {
        if(!SFileCreateFile(hMpq, szFileName, NULL, dwFileSize, 0, MPQ_FILE_REPLACEEXISTING, &hFile))
        {
            nError = GetLastError();
            printf("Failed to create %s in the archive (%u).\n", szFileName, nError);
        }
    }

    if(nError == ERROR_SUCCESS)
    {
        if(!SFileWriteFile(hFile, pvFile, dwFileSize, 0))
        {
            nError = GetLastError();
            printf("Failed to write the data to the MPQ (%u).\n", nError);
        }
    }

    if(hFile != NULL)
    {
        if(!SFileFinishFile(hFile))
        {
            nError = GetLastError();
            printf("Failed to finalize file creation (%u).\n", nError);
        }
    }

    if(pvFile != NULL)
        delete [] pvFile;
    if(hMpq != NULL)
        SFileCloseArchive(hMpq);
    return nError;
}

static int TestSignatureVerify(const char * szMpqName)
{
    HANDLE hMpq;
    
    if(SFileOpenArchive(szMpqName, 0, 0, &hMpq))
    {
        printf("Verifying digital signature in %s:\n", szMpqName);
        switch(SFileVerifyArchive(hMpq))
        {
            case ERROR_NO_SIGNATURE:
                printf("No digital signature present.\n");
                break;
        
            case ERROR_VERIFY_FAILED:
                printf("Failed to verify signature.\n");
                break;
            
            case ERROR_WEAK_SIGNATURE_OK:
                printf("Weak signature is OK.\n");
                break;

            case ERROR_WEAK_SIGNATURE_ERROR:
                printf("Weak signature mismatch.\n");
                break;

            case ERROR_STRONG_SIGNATURE_OK:
                printf("Strong signature is OK.\n");
                break;

            case ERROR_STRONG_SIGNATURE_ERROR:
                printf("Strong signature mismatch.\n");
                break;
        }
        
        SFileCloseArchive(hMpq);
        printf("\n");
    }

    return 0;
}


static int TestCreateArchiveCopy(const char * szMpqName, const char * szMpqCopyName, const char * szListFile)
{
    TFileStream * pStream;
    char   szLocalFile[MAX_PATH] = "";
    HANDLE hMpq1 = NULL;                // Handle of existing archive
    HANDLE hMpq2 = NULL;                // Handle of created archive 
    DWORD dwHashTableSize = 0;
    int nError = ERROR_SUCCESS;

    // If no listfile or an empty one, use NULL
    if(szListFile == NULL || *szListFile == 0)
        szListFile = NULL;

    // Create the new file
    pStream = FileStream_CreateFile(szMpqCopyName);
    if(pStream == NULL)
        nError = GetLastError();

    // Write some data
    if(nError == ERROR_SUCCESS)
    {
        ULONGLONG FileSize = 0x100000;
        
        FileStream_SetSize(pStream, FileSize);
        FileStream_Close(pStream);
    }

    // Open the existing MPQ archive
    if(nError == ERROR_SUCCESS)
    {
        printf("Opening %s ...\n", szMpqName);
        if(!SFileOpenArchive(szMpqName, 0, 0, &hMpq1))
            nError = GetLastError();
    }

    // Well, now create the MPQ archive
    if(nError == ERROR_SUCCESS)
    {
        printf("Creating %s ...\n", szMpqCopyName);
        SFileGetFileInfo(hMpq1, SFILE_INFO_HASH_TABLE_SIZE, &dwHashTableSize, 4);
        if(!SFileCreateArchive(szMpqCopyName, 0, dwHashTableSize, &hMpq2))
            nError = GetLastError();
    }

    // Copy all files from one archive to another
    if(nError == ERROR_SUCCESS)
    {
        SFILE_FIND_DATA sf;
        HANDLE hFind = SFileFindFirstFile(hMpq1, "*", &sf, szListFile);
        bool bResult = true;

        printf("Copying files ...\n");

        if(hFind != NULL)
        {
            while(bResult)
            {
                if(strcmp(sf.cFileName, LISTFILE_NAME) && strcmp(sf.cFileName, ATTRIBUTES_NAME))
                {
                    SFileSetLocale(sf.lcLocale);

                    // Create the local file name
                    sprintf(szLocalFile, "%s/%s", szWorkDir, sf.szPlainName);
                    if(SFileExtractFile(hMpq1, sf.cFileName, szLocalFile))
                    {
                        printf("Extracting %s ... OK\n", sf.cFileName);
                        if(!SFileAddFile(hMpq2, szLocalFile, sf.cFileName, sf.dwFileFlags))
                        {
                            nError = GetLastError();
                            printf("Adding %s ... Failed\n\n", sf.cFileName);
                            remove(szLocalFile);
                            break;
                        }
                        else
                        {
                            printf("Adding %s ... OK\n", sf.cFileName);
                        }
                    }
                    else
                    {
                        printf("Extracting %s ... Failed\n", sf.cFileName);
                    }

                    // Delete the added file
                    remove(szLocalFile);
                }

                // Find the next file
                bResult = SFileFindNextFile(hFind, &sf);
            }

            // Close the search handle
            SFileFindClose(hFind);
            printf("\n");
        }
    }

    // Close both archives
    if(hMpq2 != NULL)
        SFileCloseArchive(hMpq2);
    if(hMpq1 != NULL)
        SFileCloseArchive(hMpq1);
    return nError;
}

static int TestCompareTwoArchives(
    const char * szMpqName1,
    const char * szMpqName2,
    const char * szListFile,
    DWORD dwBlockSize)
{
    TMPQArchive * ha1 = NULL;
    TMPQArchive * ha2 = NULL;
    LPBYTE pbBuffer1 = NULL;
    LPBYTE pbBuffer2 = NULL;
    HANDLE hMpq1 = NULL;                // Handle of the first archive
    HANDLE hMpq2 = NULL;                // Handle of the second archive
    HANDLE hFile1 = NULL;
    HANDLE hFile2 = NULL;
    int nError = ERROR_SUCCESS;

    // If no listfile or an empty one, use NULL
    if(szListFile == NULL || *szListFile == 0)
        szListFile = NULL;

    // Allocate both buffers
    pbBuffer1 = new BYTE[dwBlockSize];
    pbBuffer2 = new BYTE[dwBlockSize];
    if(pbBuffer1 == NULL || pbBuffer2 == NULL)
        nError = ERROR_NOT_ENOUGH_MEMORY;

    printf("=============== Comparing MPQ archives ===============\n");

    // Open the first MPQ archive
    if(nError == ERROR_SUCCESS && szMpqName1 != NULL)
    {
        printf("Opening %s ...\n", szMpqName1);
        if(!SFileOpenArchive(szMpqName1, 0, 0, &hMpq1))
            nError = GetLastError();
        ha1 = (TMPQArchive *)hMpq1;
    }

    // Open the second MPQ archive
    if(nError == ERROR_SUCCESS && szMpqName2 != NULL)
    {
        printf("Opening %s ...\n", szMpqName2);
        if(!SFileOpenArchive(szMpqName2, 0, 0, &hMpq2))
            nError = GetLastError();
        ha2 = (TMPQArchive *)hMpq2;
    }

    // Compare the header
    if(nError == ERROR_SUCCESS && (ha1 != NULL && ha2 != NULL))
    {
        if(ha1->pHeader->dwHeaderSize != ha2->pHeader->dwHeaderSize)
            printf(" - Header size is different\n");
        if(ha1->pHeader->wFormatVersion != ha2->pHeader->wFormatVersion)
            printf(" - Format version is different\n");
        if(ha1->pHeader->wSectorSize != ha2->pHeader->wSectorSize)
            printf(" - Block size Format version is different\n");
        if(ha1->pHeader->dwHashTableSize != ha2->pHeader->dwHashTableSize)
            printf(" - Hash table size is different\n");
        if(ha1->pHeader->dwBlockTableSize != ha2->pHeader->dwBlockTableSize)
            printf(" - Block table size is different\n");
    }

    // Find all files in the first archive and compare them
    if(nError == ERROR_SUCCESS)
    {
        SFILE_FIND_DATA sf;
        HANDLE hFind = SFileFindFirstFile(hMpq1, "*", &sf, szListFile);
        DWORD dwSearchScope1 = SFILE_OPEN_FROM_MPQ;
        DWORD dwSearchScope2 = SFILE_OPEN_FROM_MPQ;
        bool bResult = true;

        if(hMpq1 == NULL)
            dwSearchScope1 = SFILE_OPEN_LOCAL_FILE;
        if(hMpq2 == NULL)
            dwSearchScope2 = SFILE_OPEN_LOCAL_FILE;

        while(hFind != NULL && bResult == true)
        {
//          ShowProcessedFile(sf.cFileName);
            SFileSetLocale(sf.lcLocale);

            // Open the first file
            if(!SFileOpenFileEx(hMpq1, sf.cFileName, dwSearchScope1, &hFile1))
            {
                printf("Failed to open the file %s in the first archive\n", sf.cFileName);
                continue;
            }

            if(!SFileOpenFileEx(hMpq2, sf.cFileName, dwSearchScope2, &hFile2))
            {
                printf("Failed to open the file %s in the second archive\n", sf.cFileName);
                continue;
            }

            if(dwSearchScope1 == SFILE_OPEN_FROM_MPQ && dwSearchScope2 == SFILE_OPEN_FROM_MPQ)
            {
                TMPQFile * hf1 = (TMPQFile *)hFile1;
                TMPQFile * hf2 = (TMPQFile *)hFile2;

                // Compare the file sizes
                if(hf1->pFileEntry->dwFileSize != hf2->pFileEntry->dwFileSize)
                    printf(" - %s different size (%u x %u)\n", sf.cFileName, hf1->pFileEntry->dwFileSize, hf2->pFileEntry->dwFileSize);

                if(hf1->pFileEntry->dwFlags != hf2->pFileEntry->dwFlags)
                    printf(" - %s different flags (%08X x %08X)\n", sf.cFileName, hf1->pFileEntry->dwFlags, hf2->pFileEntry->dwFlags);
            }

            if(!CompareArchivedFiles(sf.cFileName, hFile1, hFile2, 0x1001))
                printf(" - %s different content\n", sf.cFileName);

            if(!CompareArchivedFilesRR(sf.cFileName, hFile1, hFile2, 0x100000))
                printf(" - %s different content\n", sf.cFileName);

            // Close both files
            SFileCloseFile(hFile2);
            SFileCloseFile(hFile1);
            hFile2 = hFile1 = NULL;

            // Find the next file
            bResult = SFileFindNextFile(hFind, &sf);
        }

        // Close all handles
        if(hFile2 != NULL)
            SFileCloseFile(hFile2);
        if(hFile1 != NULL)
            SFileCloseFile(hFile1);
        if(hFind != NULL)
            SFileFindClose(hFind);
    }

    // Close both archives
    clreol();
    printf("================ MPQ compare complete ================\n");
    if(hMpq2 != NULL)
        SFileCloseArchive(hMpq2);
    if(hMpq1 != NULL)
        SFileCloseArchive(hMpq1);
    if(pbBuffer2 != NULL)
        delete [] pbBuffer2;
    if(pbBuffer1 != NULL)
        delete [] pbBuffer1;
    return nError;
}

static int TestOpenPatchedArchive(const char * szMpqName, ...)
{
    TFileStream * pStream;
    HANDLE hFile = NULL;
    HANDLE hMpq = NULL;
    va_list argList;
//  const char * szFileName = "DBFilesClient\\Achievement.dbc";
    const char * szFileName = "creature/murloc/deathwingbabymurloc.m2";
    const char * szExtension;
    const char * szLocale;
    char szPatchPrefix[MPQ_PATCH_PREFIX_LEN];
    char szLocFileName[MAX_PATH];
    LPBYTE pbFullFile = NULL;
    DWORD dwFileSize;
    int nError = ERROR_SUCCESS;

    // Determine patch prefix for patches
    strcpy(szPatchPrefix, "Base");
    szExtension = strrchr(szMpqName, '.');
    if(szExtension != NULL)
    {
        for(szLocale = szExtension; szLocale > szMpqName; szLocale--)
        {
            if(*szLocale == '-')
            {
                if((szExtension - szLocale) == 5)
                {
                    strncpy(szPatchPrefix, szLocale + 1, 4);
                    szPatchPrefix[4] = 0;
                }
                break;
            }
        }
    }

    // Open the primary MPQ
    printf("Opening %s ...\n", szMpqName);
    if(!SFileOpenArchive(szMpqName, 0, MPQ_OPEN_READ_ONLY, &hMpq))
    {
        nError = GetLastError();
        printf("Failed to open the archive %s ...\n", szMpqName);
    }

    // Add all patches
    if(nError == ERROR_SUCCESS)
    {
        va_start(argList, szMpqName);
        while((szMpqName = va_arg(argList, const char *)) != NULL)
        {
            printf("Adding patch %s ...\n", szMpqName);
            if(!SFileOpenPatchArchive(hMpq, szMpqName, szPatchPrefix, 0))
            {
                nError = GetLastError();
                printf("Failed to add patch %s ...\n", szMpqName);
            }
        }
        va_end(argList);
    }
/*
    // Now search all files
    if(nError == ERROR_SUCCESS)
    {
        SFILE_FIND_DATA sf;
        HANDLE hFind;
        BOOL bResult = TRUE;

        hFind = SFileFindFirstFile(hMpq, "*", &sf, NULL);
        while(hFind && bResult)
        {
            printf("%s\n", sf.cFileName);
            bResult = SFileFindNextFile(hFind, &sf);
        }
    }
*/
    // Now try to open patched version of "Achievement.dbc"
    if(nError == ERROR_SUCCESS)
    {
        printf("Opening patched file \"%s\" ...\n", szFileName);
        if(!SFileOpenFileEx(hMpq, szFileName, SFILE_OPEN_PATCHED_FILE, &hFile))
        {
            nError = GetLastError();
            printf("Failed to open patched file \"%s\"\n", szFileName);
        }
    }

    // Verify of the patched version is correct
    if(nError == ERROR_SUCCESS)
    {
        // Get the size of the full patched file
        dwFileSize = SFileGetFileSize(hFile, NULL);
        if(dwFileSize != 0)
        {
            // Allocate space for the full file
            pbFullFile = new BYTE[dwFileSize];
            if(pbFullFile != NULL)
            {
                if(!SFileReadFile(hFile, pbFullFile, dwFileSize))
                {           
                    nError = GetLastError();
                    printf("Failed to read full patched file data \"%s\"\n", szFileName);
                }
                
                if(nError == ERROR_SUCCESS)
                {
                    strcpy(szLocFileName, MAKE_PATH("Work//"));
                    strcat(szLocFileName, GetPlainName(szFileName));

                    pStream = FileStream_CreateFile(szLocFileName);
                    if(pStream != NULL)
                    {
                        FileStream_Write(pStream, NULL, pbFullFile, dwFileSize);
                        FileStream_Close(pStream);
                    }
                }
                
                delete [] pbFullFile;
            }
        }
    }

    // Close handles
    if(hFile != NULL)
        SFileCloseFile(hFile);
    if(hMpq != NULL)
        SFileCloseArchive(hMpq);
    return nError;
}

//-----------------------------------------------------------------------------
// Main
// 

int main(void)
{
    int nError = ERROR_SUCCESS;

#if defined(_MSC_VER) && defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif  // defined(_MSC_VER) && defined(_DEBUG)

    // Mix the random number generator
//  srand(GetTickCount());

    // Test structure sizes
//  if(nError == ERROR_SUCCESS)
//      nError = TestStructureSizes();

    // Test reading partial file
//  if(nError == ERROR_SUCCESS)
//      nError = TestPartFileRead(MAKE_PATH("2009 - PartialMPQs/patch.MPQ.part"));

    // Test LZMA compression method against the code ripped from Starcraft II
//  if(nError == ERROR_SUCCESS)
//      nError = CompareLzmaCompressions(MPQ_SECTOR_SIZE);

    // Test compression methods
//  if(nError == ERROR_SUCCESS)
//      nError = TestSectorCompress(MPQ_SECTOR_SIZE);
                                                                                            
    // Test the archive open and close
    if(nError == ERROR_SUCCESS)
//      nError = TestArchiveOpenAndClose(MAKE_PATH("2011 - WoW-Cataclysm2/a_tvse_x_1_2_f.w3x"));
        nError = TestArchiveOpenAndClose(MAKE_PATH("2002 - Warcraft III/ProtectedMap_HashTable_FakeValid.w3x"));
//      nError = TestArchiveOpenAndClose(MAKE_PATH("2010 - Starcraft II/Installer Tome 1 enGB.MPQE"));
//      nError = TestArchiveOpenAndClose(MAKE_PATH("1997 - Diablo I/single_0.sv"));
//      nError = TestArchiveOpenAndClose(MAKE_PATH("2004 - World of Warcraft/SoundCache-enUS.MPQ"));
//      nError = TestArchiveOpenAndClose(MAKE_PATH("2009 - PartialMPQs/interface.MPQ.part"));
                                                                             
//  if(nError == ERROR_SUCCESS)
//      nError = TestFindFiles(MAKE_PATH("2002 - Warcraft III/HumanEd.mpq"));

    // Create a big MPQ archive
//  if(nError == ERROR_SUCCESS)
//      nError = TestCreateArchiveV2(MAKE_PATH("Test.mpq"));

//  if(nError == ERROR_SUCCESS)
//      nError = TestAddFileToMpq(MAKE_PATH("Test.mpq"), MAKE_PATH("Arj32.exe"));

//  if(nError == ERROR_SUCCESS)
//      nError = TestCreateArchiveFromMemory(MAKE_PATH("Test-leak.mpq"));

//  if(nError == ERROR_SUCCESS)
//      nError = TestFileReadAndWrite(MAKE_PATH("2002 - Warcraft III/(10)DustwallowKeys.w3m"), "war3map.j");

    // Verify the archive signature
//  if(nError == ERROR_SUCCESS)
//      nError = TestSignatureVerify(MAKE_PATH("1998 - Starcraft/BW-1152.exe"));
//      nError = TestSignatureVerify(MAKE_PATH("2002 - Warcraft III/(10)DustwallowKeys.w3m"));
//      nError = TestSignatureVerify(MAKE_PATH("2002 - Warcraft III/War3TFT_121b_English.exe"));
//      nError = TestSignatureVerify(MAKE_PATH("2004 - World of Warcraft/WoW-2.3.3.7799-to-2.4.0.8089-enUS-patch.exe"));
//      nError = TestSignatureVerify(MAKE_PATH("2004 - World of Warcraft/standalone.MPQ"));

    // Compact the archive        
//  if(nError == ERROR_SUCCESS)
//      nError = TestMpqCompacting(MAKE_PATH("2002 - Warcraft III/(10)DustwallowKeys.w3m"));
    
    // Create copy of the archive, appending some bytes before the MPQ header
//  if(nError == ERROR_SUCCESS)
//      nError = TestCreateArchiveCopy(MAKE_PATH("PartialMPQs/interface.MPQ.part"), MAKE_PATH("PartialMPQs/interface-copy.MPQ.part"), NULL);

//  if(nError == ERROR_SUCCESS)
//  {
//      nError = TestCompareTwoArchives(MAKE_PATH("Warcraft III/War3x.mpq"),
//                                      NULL,
//                                      NULL,
//                                      0x1001);
//  }

    if(nError == ERROR_SUCCESS)
    {
        nError = TestOpenPatchedArchive(MAKE_PATH("2011 - WoW-Cataclysm2/locale-enUS.MPQ"),
                                        MAKE_PATH("2011 - WoW-Cataclysm2/wow-update-12694.MPQ"),
                                        MAKE_PATH("2011 - WoW-Cataclysm2/wow-update-12759.MPQ"),
                                        MAKE_PATH("2011 - WoW-Cataclysm2/wow-update-12803.MPQ"),
                                        MAKE_PATH("2011 - WoW-Cataclysm2/wow-update-12857.MPQ"),
                                        MAKE_PATH("2011 - WoW-Cataclysm2/wow-update-12942.MPQ"),
                                        MAKE_PATH("2011 - WoW-Cataclysm2/wow-update-12984.MPQ"),
                                        MAKE_PATH("2011 - WoW-Cataclysm2/wow-update-13066.MPQ"),
                                        MAKE_PATH("2011 - WoW-Cataclysm2/wow-update-13117.MPQ"),
                                        NULL);
    }


    // Remove the working directory
    clreol();
    if(nError != ERROR_SUCCESS)
        printf("One or more errors occurred when testing StormLib\n");

    printf("Work complete.\n");
    return nError;
}
