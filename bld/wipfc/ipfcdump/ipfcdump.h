#include <stdint.h>
#include <stdio.h>

#define STRING_MAX_LEN      256
#define WSTRING_MAX_LEN     256

#pragma pack(push, 1)
typedef struct {
    uint8_t     id[3];              // "HSP"
    uint8_t     flags;              // 0x01 if INF style file, 0x10 if HLP style file
    uint16_t    size;               // total size of header, in bytes
    uint8_t     version_hi;
    uint8_t     version_lo;
    uint16_t    tocCount;           // number of toc items
    uint32_t    tocOffset;          // file offset to start of TocEntry elements
    uint32_t    tocSize;            // size of all TocEntry data
    uint32_t    tocOffsetOffset;    // file offset to array of file offsets to TocEntry
    uint16_t    panelCount;         // number of panels with resource numbers
    uint32_t    panelOffset;        // file offset to panel number table
                                    // 2 consecutive arrays: panel number, TocEntry index
    uint16_t    nameCount;          // number of named panels
    uint32_t    nameOffset;         // file offset to panel name table
                                    // 2 consecutive arrays: panel name index, TocEntry index
    uint16_t    indexCount;         // number of index entries
    uint32_t    indexOffset;        // file offset to index table
    uint32_t    indexSize;          // size of index table
    uint16_t    icmdCount;          // number of icmd index items
    uint32_t    icmdOffset;         // file offset to icmd index items
    uint32_t    icmdSize;           // size of icmd index table
    uint32_t    searchOffset;       // file offset to full text search table; if high bit set, size of search record size is 16-bit
    uint32_t    searchSize;         // size of full text search table
    uint16_t    cellCount;          // number of words
    uint32_t    cellOffsetOffset;   // file offset to array of file offsets to cells
    uint32_t    dictSize;           // number of bytes occupied by the dictionary
    uint16_t    dictCount;          // number of entries in the dictionary
    uint32_t    dictOffset;         // file offset array of DictString
                                    // string table is built from this
    uint32_t    imageOffset;        // file offset of image data
    uint8_t     maxCVTIndex;        // highest index inside panel's local dictionary,
                                    // always seems to be 245
    uint32_t    nlsOffset;          // file offset to NLS table
    uint32_t    nlsSize;            // size of NLS table
    uint32_t    extOffset;          // file offset to extended data block
    uint8_t     reserved[12];       // reserved for future use
    uint8_t     title[48];          // title of database
} IpfHeader;

// Extended header info
typedef struct {
    uint16_t    fontCount;           // number of font entries
    uint32_t    fontOffset;          // file offset in file
    uint16_t    dbCount;             // number of external files
    uint32_t    dbOffset;            // file offset of external files table
    uint32_t    dbSize;              // size of db table
    uint16_t    gNameCount;          // number of global name entries
    uint32_t    gNameOffset;         // file offset of global names
    uint32_t    stringsOffset;       // file offset of strings
    uint16_t    stringsSize;         // size of string data
    uint32_t    childPagesOffset;    // file offset of child pages table
    uint32_t    childPagesSize;      // size of child pages
    uint32_t    gIndexCount;         // number of global index items
    uint32_t    ctrlOffset;          // file offset to button control data
    uint32_t    ctrlSize;            // size of button control data
    uint32_t    reserved[4];         // reserved for future use
} IpfExtHeader;

#define IsFTS16Data(x)      (((x).searchOffset & (1L <<31)) != 0)
#define FTSDataOffset(x)    ((x).searchOffset & 0x7fffffffL)

typedef struct {
    uint8_t     size;
    //variable length data follows:
    //int8_t    text[size - 1];
} DictString;

// Dictionary:
/*
DictString * Dictionary[IpfHeader.dictCount];
*/

// Cell:
typedef struct {
    uint8_t     zero;               //=0
    uint32_t    dictOffset;         //file offset to uint16_t array
    uint8_t     dictCount;          // <=254
    uint16_t    textCount;
    //variable length data follows:
    //uint8_t   text[textCount];    //encoded text (indexes into dict)
    //uint16_t  dict[dictCount];    //index to global dictionary
} Cell;
//text of cell (part of a panel or article) is Dictionary[dict[text[i]]]

// CellOffsets
//uint32_t CellOffset[];

#define END_PARAGRAPH   0xFA
#define CENTER          0xFB
#define TOGGLE_SPACING  0xFC
#define LINE_BREAK      0xFD
#define SPACE           0xFE
#define ESCAPE          0xFF

// EscSeq:
typedef struct {
    uint8_t esc;                    //=0xFF
    uint8_t size;                   //2 + optional arguments
    uint8_t type;
    //variable length data follows:
/*
Type    Meaning
0x01:   unknown

0x02:   set left margin of current line
        Generated by: lm, li, dt, dd, dthd, ddhd
0x11:   set left margin and start new line
        Generated by: dd
0x12:   set left margin fit
        Generated by: dd if new margin >= dt margin
        Arguments:
            uint8_t margin; in spaces, 0 == no margin

0x03:   set right margin
        Generated by: rm
        Arguments:
            uint8_t margin; in spaces, 1 == no margin

0x04:   change style
        Generated by: hp#, ehp#
        Arguments:
            uint8_t style;
                0 returns to plain text
                1 italic            (same as :hp#.)
                2 bold              (same as :hp#.)
                3 bold italic       (same as :hp#.)
                4 underlined        (same as :hp#-1.)
                5 italic underscored(same as :hp#-1.)
                6 bold underscored  (same as :hp#-1.)
                7 bold italic underscored

0x05:   beginning of cross reference
        Generated by: link
        Arguments:
            uint16_t tocIndex;
            uint8_t flag1;
            uint8_t flag2;
        The optional flag bytes describe the size, position and
        characteristics of the window created when the cross-reference
        is followed.
        Flag1 bit 7: 'split' window
              bit 6: autolink
              bit 3: window style specified
              bit 2: viewport ** 1
              bit 1: target size supplied
              bit 0: target position supplied
        Flag2 bit 0: ?
              bit 1: dependent
              bit 2: group supplied

0x06:   unknown

0x07:   link to footnote (:fn. tag)
        Generated by: link
        Arguments:
            uint16_t tocIndex;  // toc entry number of text

0x08:   end link hypertext
        Generated by: elink

0x09:   unknown

0x0A:   unknown

0x0B:   begin monospaced example (char graphics)
        Generated by: cgraphics, xmp

0x0C:   end monospace example (char graphics)
        Generated by: ecgraphics, exmp

0x0D:   special text colors
        Generated by: hp4, hp8, hp9, ehp4, ehp8, ehp9
        Arguments:
            uint8_t color
                0: default color
                1,2,3: same as :hp4,8,9.

0x0E:   Bitmap image (not hypergraphic)
        Generated by: artwork
        Arguments:
            uint8_t flag;
                4: runin
                3: fit (scale) to window
                2: align center
                1: align right
                0: align left (default)
            uint32_t bitmap_offset;
        Note: first bitmap always has offset 0

0x0F:   Image map
        Arguments:
            uint8_t subcode;
        Subcodes:
            DEFINE = 0; (define as hypergraphic)
                Generated by: artwork
                uint8_t align; (see above)
                uint32_t offset; (of bitmap)
            PT_HDREF = 1;
            PT_FNREF = 2;
                Define partial bitmap as hypergraphic link to footnote panel
                Generated by: link
                uint16_t index; (toc or resource to link to)
                uint16_t xorg;
                uint16_t yorg;
                uint16_t width;
                uint16_t height;
            PT_SPREF = 3;
            HDREF = 4;
            FNREF = 5;
                Define full bitmap as hypergraphic link to footnote panel
                Generated by: link
                uint16_t index; (toc or resource to link to)
            SPREF = 6;
            LAUNCH = 7;
                Define full bitmap as hypergraphic link to app
                Generated by: link
                uint8_t  reserved; (=0)
                int8_t           launchString;
            PT_LAUNCH = 8;
                Define partial bitmap as hypergraphic link to app
                Generated by: link
                uint8_t  reserved; (=0)
                uint16_t xorg;
                uint16_t yorg;
                uint16_t width;
                uint16_t height;
                int8_t           launchString;
            INFORM = 9;
                Define full bitmap as hypergraphic sending message to app
                Generated by: link
                uint16_t resNumber;
            PT_INFORM = 10;
                Define partial bitmap as hypergraphic sending message to app
                Generated by: link
                uint16_t resNumber;
                uint16_t xorg;
                uint16_t yorg;
                uint16_t width;
                uint16_t height;
            // ?? 11 ??
            EXTERN_PT_HDREF = 12;
                Define partial bitmap as hypergraphic
                Generated by: link
                uint16_t index; (toc or resource to link to)
                uint16_t xorg;
                uint16_t yorg;
                uint16_t width;
                uint16_t height;
            EXTERN_PT_SPREF = 13;
            EXTERN_HDREF = 14;
                Define full bitmap as hypergraphic link to panel
                Generated by: link
                uint16_t index; (toc or resource to link to)
            EXTERN_SPREF = 15;
            GLOBAL_HDREF = 16;
                Define full bitmap as hypergraphic linking to panel in external database
                Generated by: link
                uint16_t dbIndex;
                uint8_t  idSize;
                int8_t           id[idSize]
            GLOBAL_PT_HDREF = 17;
                Define partial bitmap as hypergraphic linking to panel in external database
                Generated by: link
                uint16_t dbIndex;
                uint8_t  idSize;
                uint16_t xorg;
                uint16_t yorg;
                uint16_t width;
                uint16_t height;
                int8_t           id[idSize]

0x10:   Start link to launch app
        Generated by: link, reftype=launch
        Arguments:
            uint8_t reserved;          //=0
            int8_t launchString[size - 3];

0x13:   Set foreground color
        Generated by: color
0x14:   Set background color
        Generated by: color
        Arguments:
            uint8_t color;
                0 - default
                1 - blue
                2 - red
                3 - pink
                4 - green
                5 - cyan
                6 - yellow
                7 - neutral
                8 - dark gray
                9 - dark blue
               10 - dark red
               11 - dark pink
               12 - dark green
               13 - dark cyan
               14 - black
               15 - pale gray

0x15:   tutorial
        Generated by: h1 - h6
        Arguments:
            int8_t name[size - 2]

0x16:   start link sending information to app
        Generated by: link reftype=inform
        Arguments:
            uint16_t resNumber;

0x17:   hide text
        Generated by: hide
        Arguments:
            uint8_t key[];
                key required to show text

0x18:   end of hidden text
        Generated by: ehide

0x19:   change font
        Generated by: font
        Arguments:
            uint8_t fontIndex;

0x1A:   begin :lines. sequence.
        Generated by: lines, fig
        Arguments:
            uint8_t alignment;
                1 = left
                2 = right
                4 = center

0x1B:   end :lines. sequence.
        Generated by: elines, efig

0x1C:   Set left margin to current position. Margin is reset at end of paragraph.
        Generated by: nt

0x1D:   Start external link by resource id
        Generated by: link
        Arguments:
            uint16_t resourceId;

0x1F:   Start external link in external database
        Generated by: link reftype=hd database=...
        Arguments:
            uint8_t dbIndex;
            uint8_t size;
            int8_t          id[size];

0x20:   :ddf.
        Arguments:
            uint16_t res;
                value of res attribute

*/
} EscSeq;

// TocEntry: located at offset pointed to by tocOffset_offset[i]
typedef struct {
    uint8_t size;                   // size of the entry
    uint8_t nestLevel  :4;          // nesting level
    uint8_t unknown    :1;
    uint8_t extended   :1;          // extended entry format
    uint8_t hidden     :1;          // don't show this toc entry
    uint8_t hasChildren:1;          // following nodes are a higher level
    uint8_t cellCount;              // number of Cells occupied by the text for this toc entry
    //variable length data follows:
    //if !extended
    //  uint16_t    cellIndex[cellCount];
    //  int8_t      title[length - 2 - 2 * cellCount];
    //else
    // ExtTocEntry
} TocEntry;

typedef struct {
    uint16_t    setPos  :1;
    uint16_t    setSize :1;
    uint16_t    setView :1;
    uint16_t    setStyle:1;
    uint16_t    noSearch:1;
    uint16_t    noPrint :1;
    uint16_t    setCtrl :1;
    uint16_t    setTutor:1;
    uint16_t    clear   :1;
    uint16_t    unknown1:1;
    uint16_t    setGroup:1;
    uint16_t    isParent:1;
    uint16_t    unknown2:4;
} ExtTocEntry;

typedef enum {
    ABSOLUTE_CHAR = 0,
    RELATIVE_PERCENT,
    ABSOLUTE_PIXEL,
    ABSOLUTE_POINTS,
    DYNAMIC
} Position;

typedef enum {
    DYNAMIC_LEFT    = 1,
    DYNAMIC_RIGHT   = 2,
    DYNAMIC_TOP     = 4,
    DYNAMIC_BOTTOM  = 8,
    DYNAMIC_CENTER  = 16
} DynamicPosition;

typedef struct {
    uint8_t     yPosType:4;
    uint8_t     xPosType:4;
    uint16_t    xpos;
    uint16_t    ypos;
} PanelOrigin;

typedef struct {
    uint8_t     widthType :4;
    uint8_t     heightType:4;
    uint16_t    width;
    uint16_t    height;
} PanelSize;

typedef struct {
    uint16_t    word;
} PanelStyle;

typedef struct {
    uint16_t    word;
} PanelControls;

typedef struct {
    uint16_t    id;                 //a panel number
} Group;

// TOCOffset
// uint32_t TOCOffset[IpfHeader.tocCount]

// IndexItem:
typedef struct {
    uint8_t     size;               // size of item text
    uint8_t     primary  :1;        // bit 0 always set?
    uint8_t     secondary:1;        // bit 1 set: indent (:i2.)
    uint8_t     unknown  :4;
    uint8_t     global   :1;        // bit 6 set: global entry
    uint8_t     sortKey  :1;        // bit 7 set: sort key
    uint8_t     synonymCount;       // number synonym entries following
    uint16_t    tocPanelIndex;      // toc entry number of panel
    //variable length data follows:
    //if sortKey bit set
    //  int8_t size
    //  int8_t text[size]
    //int8_t word[size];                 // index word [not zero-terminated]
    //uint32_t synonyms[synonymCount]; // 32 bit file offset to start of synonyms referencing this word
} IndexItem;

//PanelNumber:
// uint16_t PanelNumber[IpfHeader.panelCount    //in ascending order
// uint16_t TOCIndex[IpfHeader.panelCount];

//PanelName:
// uint16_t dictIndex[IpfHeader.panelCount];    //in ascending order
// uint16_t TOCIndex[IpfHeader.panelCount];

// Synonym Table
typedef struct {
    uint16_t    size;
    //variable length data follows:
    //DictString words[];
} Synonym;

// Database (external files) table
// DictString filenames[];

// Font Entry
typedef struct {
    int8_t      faceName[33];       //null terminated
    uint16_t    height;             //reversed from docs
    uint16_t    width;
    uint16_t    codePage;
} FontEntry;

// NLS Data
enum NlsRecType {
    CONTROL,
    TEXT,
    GRAPHIC
};

typedef struct {
    uint16_t  size;
    uint8_t   type;
    uint8_t   format;
} NlsHeader;

typedef struct {
    //NlsHeader.size = 10/12
    //NlsHeader.type = NLSRecType.CONTROL
    //NlsHeader.format = 0
    uint16_t    value;              //=256
    uint16_t    code;               //country code
    uint16_t    page;               //code page
    uint16_t    reserved;
} NlsCountryDef;

typedef struct {                    //Single-byte character set
    //NlsHeader.size = 36
    //NlsHeader.type = NLSRecType.WORD || NLSRecType.GRAPHIC
    //NlsHeader.format = 0
    uint8_t     bits[32];           //high-order bit first
} SbcsNlsGrammarDef;

//typedef struct {                      //Double-byte character set
    //NlsHeader.size = 4 + (# ranges * 4)
    //NlsHeader.type = NLSRecType.TEXT || NLSRecType.GRAPHIC
    //NlsHeader.format = 1
    //variable length data follows
    //int8_t    lsb;
    //int8_t    msb;
//    } DbcsNlsGrammarDef;

// BitMap accessory strctures
typedef struct {
    uint32_t    size;               //should be 12
    uint16_t    width;
    uint16_t    height;
    uint16_t    planes;             //always 1?
    uint16_t    bitsPerPixel;
    //Windows adds 6 dwords
} BitmapInfoHeader;                 //==BitMapCoreHeader

typedef struct {
    uint8_t     type[2];            // 'bM' for bitmap, 'mF' for metafile
    uint32_t    size;               //including header, before compression
    int16_t     xHotspot;
    int16_t     yHotspot;
    uint32_t    bitsOffset;         //offset to bitmap data
    BitmapInfoHeader info;
} BitmapFileHeader;                 //same as Windows

typedef struct {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
} RGB;

// BitMap
typedef struct {
    BitmapFileHeader hdr;
    //variable length data follows:
    //uint8_t rgb[(1 << hdr.info.bitsPerPixel) * 3]
    //uint32_t size;                //starting with next field, used to SEEK_CUR to next bitmap
    //uint16_t blockSize;
    //BitMapBlock[];
} BitMap;

// BitMapBlock
typedef struct {
    uint16_t    size;               //starting with next field
    uint8_t     type;               //0 == uncompressed, 2 == LZW compressed
    //variable length data follows:
    //data
} BitMapBlock;

// StringTable strings from :docprof objectname, dll, objectinfo attributes
// DictString StringTable[];

// Controls
typedef struct {
    uint16_t    controlCount;       //number of ControlDef records
    uint16_t    groupCount;         //number of GroupDef records
    uint16_t    groupIndex;         //for cover page
    uint16_t    reserved;
} Controls;

typedef struct {
    uint16_t    type;               //type of control
    uint16_t    resid;              //resource id (panel) it belongs to
    //variable length data follows, contains button text
    //DictString    text;
} ControlDef;

typedef struct {
    uint16_t    count;              //number of indexes into ControlDef records
    //variable length data follows
    //uint16_t  index[count];
} GroupDef;

// FTSData: full text search data
typedef struct {
    uint8_t     size;
    uint8_t     compression;
    //variable length data follows
    //bitstring: 1 bit per panel
} FTS8Data;

typedef struct {
    uint16_t  size;
    uint8_t   compression;
    //variable length data follows
    //bitstring: 1 bit per panel
} FTS16Data;

enum CompressionCode {
    NONE,                   //word is in no panel, no bitstring
    ALL,                    //word is in every panel, no bitsring
    RLE,                    //Run-length encoded bitstring
    PRESENT,                //list of panel numbers (uint16_t) word is in
    NOT_PRESENT,            //list of panel numbers (uint16_t) word is not in
    TRUNC,                  //no empty bytes after last set bit
    DBL_TRUNC               //first panel number, then bitstring with no empty bytes
};                          //  after last set bit
//first byte has lowest panel numbers, first bit is highest panel number in that byte

typedef struct {
    uint8_t   size;
    uint16_t  parent;     //index into TOC
    //variable length data follows
    //uint16_t  children[(size - 3) / 2];
} ChildPages;
#pragma pack(pop)

extern void         readHeader( FILE *, FILE * );
extern void         readGNames( FILE *, FILE * );
extern void         readExtFiles( FILE *, FILE * );
extern void         readNLS( FILE *, FILE * );
extern void         readFonts( FILE *, FILE * );
extern void         readControls( FILE *, FILE * );
extern void         readStrings( FILE *, FILE * );
extern void         readTOC( FILE *, FILE * );
extern char         *getPosString( uint8_t );
extern char         *getDPosString( uint8_t );
extern void         readDictionary( FILE *, FILE * );
extern void         readPanels( FILE *, FILE * );
extern void         readCells( FILE *, FILE * );
extern void         readIndex( FILE *, FILE * );
extern void         readIcmdIndex( FILE *, FILE * );
extern void         readFTS( FILE *, FILE *);
extern void         readBitMaps( FILE *, FILE * );
extern void         readChildPages( FILE *, FILE * );
extern size_t       readDictString( FILE *, wchar_t * );
extern const char   *bstring( uint8_t );
extern int          isBigFTS( IpfHeader *hdr );
extern uint32_t     dataOffsetFTS( IpfHeader *hdr );

//Global variables
extern IpfHeader    Hdr;
extern IpfExtHeader eHdr;
extern wchar_t      **Vocabulary;
