# TEX Specification

This is a specification for Yin's second-generation texture format,
otherwise known as 'TEX'.

```c
typedef enum TEXCompression
{
    COMMON_TEX_COMPRESSION_NONE     = 0,
    COMMON_TEX_COMPRESSION_DEFLATE  = 1,
};

typedef enum TEXFormat
{
    COMMON_TEX_FORMAT_CLUSTER   = 0,
    COMMON_TEX_FORMAT_PALETTE   = 1,
    COMMON_TEX_FORMAT_ASTC      = 2,
    
    COMMON_MAX_TEX_FORMATS
};

typedef struct TEXHeader
{
    uint32_t    magic;          // 'TEX\0'
    uint8_t     version;
    uint8_t     compression;    // compression format
    
    if ( compression != COMMON_TEX_COMPRESSION_NONE )
    {
        uint32_t fileSize;
        uint32_t compressedSize;
    }
    
    uint8_t     format;         // TEXFormat
    uint16_t    width;
    uint16_t    height;
    uint8_t     numMips;
} TEXHeader;

typedef struct TEXClusterHeader
{
    uint8_t     channels;
    uint16_t    numBlocks;  // if this returns 0, it means there's just plain data
};
```
