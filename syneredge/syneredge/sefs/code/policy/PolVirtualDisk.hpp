
#ifndef POL_VIRTUAL_DISK_HPP
#define POL_VIRTUAL_DISK_HPP

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>

#include <DBinterface.hpp>
#include <DBexception.hpp>
#include <PolUtil.hpp>
#include <SEDB.h>

// ------------------------------------------------------------------------

// define the pertinent information for a single physical disk
typedef struct VDBlockFileStruct
{
    char host[sizeof(SEDB_block_file__host)];           // name of physical disk
    char directory[sizeof(SEDB_block_file__directory)]; // name of physical disk
    int64 bytes;                         // remaining number of kilobytes (from database)
    int64 taking;                        // amount taking from bytes
} VDBlockFile;

typedef VDBlockFile* VDBlockFilePtr;

typedef vector<VDBlockFilePtr> VDBlockFileList;

// ------------------------------------------------------------------------

/** minimum size of a disk block */
#define MIN_BLOCK_SIZE (256)

/** Minimum number of blocks for a virtual disk. */
#define MIN_NUMBER_OF_BLOCKS (16)

// ------------------------------------------------------------------------

class PolVirtualDisk
{
  private:
    /** database handle */
    DBinterface *db;

    /** contains latest error message */
    string errorMessage;

    /** True if a Select has been performed and the result is
     * still valid.  Any other query executed on the database will
     * invalidate the result and set this to false.
     */
    bool selected;

    bool initialize(void);
    bool validateNewPolVirtualDisk(const char *virtDiskName);
    bool getPolicy(const char* policyName, string *policy);
    bool validateBytes(const char *bytes, const char *blockSize,
                       int64 *b, int32 *bs);
    bool insertPolVirtualDisk(const char *virtDiskName,
                           const char *policyName,
                           const int32 bs,
                           const VDBlockFileList *blockFiles);
    bool updateBlockFiles(int64 requiredKbytes, VDBlockFileList *disks);
    VDBlockFileList *getDiskList(const int64 requiredKbytes, const char *policy);

  public:
    PolVirtualDisk(void);
    ~PolVirtualDisk(void);
    bool Insert(const char *virtDiskName,
                const char *virtualDiskPolicy,
                const char *bytes,
                const char *blockSize);
    bool Delete(const char *name);
    bool Select(void);
    const char *get(int row, int col);
    const char *Error(void);
};

#endif
