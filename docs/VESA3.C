///////////////////////////////////////////////////////////////////
//                  GFXLIB Graphics Library                      //
//            Full supports 8/15/16/24/32 bits color             //
//       Support both page flipping and double buffering         //
//   Using linear frame buffer and VESA 3.0 core functions       //
//             Use double buffer for rendering                   //
//            Environment: Open Watcom C/C++ 1.9                 //
//             OS: DOS32A 32bits protected mode                  //
// Compile: wcl386 -wx -zq -ecc -op -ol -ot -bcl=dos32a gfxlib.c //
//             Target OS: DOS32A / PMODEW                        //
//                Author: Nguyen Ngoc Van                        //
//                Create: 25/05/2001                             //
//               Website: http://codedemo.net                    //
//                 Email: pherosiden@gmail.com                   //
//            References: http://crossfire-designs.de            //
///////////////////////////////////////////////////////////////////
// NOTE: You can use this code freely and any purpose without    //
// any warnings. Please leave the copyright.                     //
///////////////////////////////////////////////////////////////////

#include <dos.h>
#include <math.h>
#include <time.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

// VBE Capabilities flags
#define VBE_CAPS_DAC8           1           // bit 0
#define VBE_CAPS_NOTVGA         2           // bit 1
#define VBE_CAPS_BLANKFN9       4           // bit 2
#define VBE_CAPS_HWSTEREO       8           // bit 3
#define VBE_CAPS_STEREOEVC      16          // bit 4

// VBE Mode attributes flags
#define VBE_MASK_MODEHW         1           // bit 0
#define VBE_MASK_HASLFB         128         // bit 7
#define VBE_MASK_HW3BUF         1024        // bit 10

// VBE Memory model flags
#define VBE_MM_PACKED           4
#define VBE_MM_DCOLOR           6
#define VBE_MM_YUV              7

// VBE protected mode constant
#define VBE_DATA_SIZE           0x600       // VBE data size
#define VBE_STACK_SIZE          0x2000      // VBE stack size
#define VBE_CODE_SIZE           0x8000      // VBE code size

// VBE CRTC timing value
#define CRTC_DOUBLE_SCANLINE    1           // bit 0
#define CRTC_INTERLACED         2           // bit 1
#define CRTC_HSYNC_NEGATIVE     4           // bit 2
#define CRTC_VSYNC_NEGATIVE     8           // bit 3

// Projection constant
#define ECHE                    0.75        // must be round for each monitor

// Define M_PI
#define M_PI                    3.14159265  // redefine PI constant
#define M_2PI                   6.28318530  // redefine 2PI constant

// Time defination
#define MIN_TRIES               10
#define MAX_RUN                 5
#define PIT_RATE                0x1234DD
#define WAIT_TIME               50000

// Rountine functions
#define GetTicks()              *((unsigned long*)0x046C)
#define Swap(a, b)              {int t = a; a = b; b = t;}

// Standard timing default parameters
#define MARGIN_PERCENT          1.8         // % of active image width/height
#define CELL_GRAN               8           // character cell granularity
#define H_SYNC_PERCENT          8.0         // % of line period for nominal hsync
#define MIN_VSYNC_BP            550         // min. time of vsync + back porch (microsec)
#define V_SYNC                  4           // # lines for vsync
#define MIN_V_PORCH             3           // min. # lines for vertical front porch
#define MIN_V_BP                6           // min. # lines for vertical back porch
#define CLOCK_STEP              250         // pixel clock step size (kHz)

// Generalized blanking limitation formula constants
#define M                       600         // blanking formula gradient
#define C                       40          // blanking formula offset
#define K                       128         // blanking formula scaling factor
#define J                       20          // blanking formula scaling factor weighting

// C' and M' are part of the Blanking Duty Cycle computation
#define M_PRIME                 (M * K / 256)
#define C_PRIME                 (((C - J) * K / 256) + J)

// Fixed number
#define FIXED(x)                ((unsigned int)((x) * 1024))

// XFN font style
#define XFN_FIXED               0x01        // fixed font (all character have same size)
#define XFN_MULTI               0x02        // multiple font
#define XFN_ANIMATE             0x04        // animation font
#define XFN_ANIPOS              0x08        // random position font
#define XFN_SCALEABLE           0x10        // scaleable font
#define XFN_VECTOR              0x20        // vector font (like CHR, BGI font)
#define XFN_LOCKED              0x01000000  // locked font
#define GFX_BUFF_SIZE           131072      // maximum GFXLIB buffer
#define GFX_MAX_FONT            5           // maximum GFXLIB font loaded at same time

// Bitmap mouse and button
#define MOUSE_WIDTH             24          // mouse width
#define MOUSE_HEIGHT            24          // mouse height
#define MOUSE_SIZE              576         // mouse size (mouse width * mouse height)

#define BUTTON_WIDTH            48          // button width
#define BUTTON_HEIGHT           24          // button height
#define BUTTON_SIZE             1152        // button size (button width * button height)
#define BUTTON_BITMAPS          3

#define LEFT_BUTTON             0           // mouse left button pressed
#define MIDDLE_BUTTON           1           // mouse middle button pressed
#define RIGHT_BUTTON            2           // mouse right button pressed

#define STATE_NORM              0           // mouse state nornal
#define STATE_ACTIVE            1           // mouse state active
#define STATE_PRESSED           2           // mouse state pressed
#define STATE_WAITING           3           // mouse state waiting

#define NUM_BUTTONS             2           // 'Exit' and 'Click' button
#define NUM_MOUSE_BITMAPS       9           // total mouse pointer bitmaps

// Cohen-Sutherland clip constanst
#define CLIP_LEFT               1           // left code
#define CLIP_RIGHT              2           // right code
#define CLIP_BOTTOM             4           // bottom code
#define CLIP_TOP                8           // top code

// Fill poly constant
#define MAX_POLY_CORNERS        200         // max polygon corners
#define MAX_STACK_SIZE          2000        // max stack size use to scan

// CPUID defines constanst
#define CPUID_EAX               0           // index of EAX value in array
#define CPUID_EBX               1           // index of EBX value in array
#define CPUID_ECX               2           // index of ECX value in array
#define CPUID_EDX               3           // index of EDX value in array

#define CPUID_STD_BASE          0x00000000  // standard leaves base
#define CPUID_HV_BASE           0x40000000  // hypervisor leaves base
#define CPUID_EXT_BASE          0x80000000  // extended leaves base
#define CPUID_EXT_BASE1         0x80000001  // AMD entended leaves base
#define CPUID_BRAND_1ST         0x80000002  // first brand string leaf
#define CPUID_CENTAUR_BASE      0xC0000000  // centaur leaves base
#define CPUID_BRAND_COUNT       3           // number of brand leaves

// Processor manufacturer
#define PROCESSOR_INTEL         0           // INTEL processor
#define PROCESSOR_AMD           1           // AMD processor
#define PROCESSOR_UNKNOWN       2           // unknown processor

// Simulation real mode registers interfaces
#pragma pack(push, 1)
typedef struct
{
    unsigned int    edi;
    unsigned int    esi;
    unsigned int    ebp;
    unsigned int    rsvd;
    unsigned int    ebx;
    unsigned int    edx;
    unsigned int    ecx;
    unsigned int    eax;
    unsigned short  flags;
    unsigned short  es;
    unsigned short  ds;
    unsigned short  fs;
    unsigned short  gs;
    unsigned short  ip;
    unsigned short  cs;
    unsigned short  sp;
    unsigned short  ss;
} RM_REGS;

// VBE driver information
typedef struct
{
    unsigned char   VBESignature[4];        // VBE Info Block signature ('VESA')
    unsigned short  VBEVersion;             // VBE version (must be 0x0200 or 0x0300)
    unsigned long   OEMStringPtr;           // OEM string
    unsigned long   Capabilities;           // capabilities of graphics controller
    unsigned long   VideoModePtr;           // video mode pointer
    unsigned short  TotalMemory;            // # 64kb memory blocks

    // VBE 2.0 extensions
    unsigned short  OemSoftwareRev;         // VBE implementation software revision
    unsigned long   OemVendorNamePtr;       // vendor name
    unsigned long   OemProductNamePtr;      // product name
    unsigned long   OemProductRevPtr;       // product revision
    unsigned char   Reserved[222];          // VBE implementation scratch area
    unsigned char   OemData[256];           // data area for OEM strings
} VBE_DRIVER_INFO;

// VBE mode information
typedef struct
{
    // for all VBE revisions
    unsigned short  ModeAttributes;         // mode attributes
    unsigned char   WinAAttributes;         // window A attributes
    unsigned char   WinBAttributes;         // window B attributes
    unsigned short  WinGranularity;         // window granularity
    unsigned short  WinSize;                // window size
    unsigned short  WinASegment;            // window A start segment
    unsigned short  WinBSegment;            // window B start segment
    unsigned long   WinFuncPtr;             // real mode pointer to window function
    unsigned short  BytesPerScanline;       // bytes per scan line

    // VBE 1.2+
    unsigned short  XResolution;            // horizontal resolution in pixels (characters)
    unsigned short  YResolution;            // vertical resolution in pixels (characters)
    unsigned char   XCharSize;              // character cell width in pixels
    unsigned char   YCharSize;              // character cell height in pixels
    unsigned char   NumberOfPlanes;         // number of memory planes
    unsigned char   BitsPerPixel;           // bits per pixel
    unsigned char   NumberOfBanks;          // number of banks
    unsigned char   MemoryModel;            // memory model type
    unsigned char   BankSize;               // bank size in KB
    unsigned char   NumberOfImagePages;     // number of images
    unsigned char   Reserved1;              // reserved for page function

    // VBE 1.2+ Direct color fields (required for direct/6 and YUV/7 memory models)
    unsigned char   RedMaskSize;            // size of direct color red mask in bits      
    unsigned char   RedFieldPosition;       // bit posision of LSB of red mask
    unsigned char   GreenMaskSize;          // size of direct color green mask in bits
    unsigned char   GreenFieldPosition;     // bit posision of LSB of green mask
    unsigned char   BlueMaskSize;           // size of direct color blue mask in bits
    unsigned char   BlueFieldPosition;      // bit posision of LSB of blue mask
    unsigned char   RsvdMaskSize;           // size of direct color reserved mask in bits
    unsigned char   RsvdFieldPosition;      // bit posision of LSB of reserved mask
    unsigned char   DirectColorModeInfo;    // direct color mode attributes

    // VBE 2.0+
    unsigned long   PhysBasePtr;            // physical address for flat memory frame buffer
    unsigned long   OffScreenMemOffset;     // reserved must be 0
    unsigned short  OffScreenMemSize;       // reserved must be 0

    // VBE 3.0+
    unsigned short  LinBytesPerScanline;    // linear modes: bytes per scanline
    unsigned char   BnkNumberOfImagePages;  // banked modes: number of images
    unsigned char   LinNumberOfImagePages;  // linear modes: number of images
    unsigned char   LinRedMaskSize;         // linear modes: size of direct color red mask
    unsigned char   LinRedFieldPosition;    // linear modes: bit position of LSB of red mask
    unsigned char   LinGreenMaskSize;       // linear modes: size of direct color green mask
    unsigned char   LinGreenFieldPosition;  // linear modes: bit position of LSB of green mask
    unsigned char   LinBlueMaskSize;        // linear modes: size of direct color blue mask
    unsigned char   LinBlueFieldPosition;   // linear modes: bit position of LSB of blue mask
    unsigned char   LinRsvdMaskSize;        // linear modes: size of direct color rsvd mask
    unsigned char   LinRsvdFieldPosition;   // linear modes: bit position of LSB of rsvd mask
    unsigned long   MaxPixelClock;          // maximum pixel clock (in Hz) for graphics mode
    unsigned char   Reserved2[189];         // padding data
} VBE_MODE_INFO;

// VESA 2.0 protected mode interface
typedef struct
{
    unsigned short  SetWindow;              // SetWindow selector
    unsigned short  SetDisplayStart;        // SetDisplayStart selector
    unsigned short  SetPalette;             // SetPallete selector
    unsigned short  IOPrivInfo;             // IO privileged info
} VBE_PM_INFO;

// VESA 3.0 protected mode info block
typedef struct
{
    unsigned char   Signature[4];           // PM Info Block signature ('PMID')
    unsigned short  EntryPoint;             // offset of PM entry point within BIOS
    unsigned short  PMInitialize;           // offset of PM initialization entry point
    unsigned short  BIOSDataSel;            // selector to BIOS data area emulation block
    unsigned short  A0000Sel;               // selector to 0xa0000
    unsigned short  B0000Sel;               // selector to 0xb0000
    unsigned short  B8000Sel;               // selector to 0xb8000
    unsigned short  CodeSegSel;             // selector to access code segment as data
    unsigned char   InProtectMode;          // true if in protected mode
    unsigned char   Checksum;               // sum of all bytes in this struct must match 0
} VBE_PM_INFO_BLOCK;

// VESA 3.0 CRTC timings structure
typedef struct
{
    unsigned short  HorizontalTotal;        // horizontal total in pixels
    unsigned short  HorizontalSyncStart;    // horizontal sync start in pixels
    unsigned short  HorizontalSyncEnd;      // horizontal sync end in pixels
    unsigned short  VerticalTotal;          // vertical total in lines
    unsigned short  VerticalSyncStart;      // vertical sync start in lines
    unsigned short  VerticalSyncEnd;        // vertical sync end in lines
    unsigned char   Flags;                  // flags (double-scan, interlaced, h/v-sync polarity)
    unsigned long   PixelClock;             // pixel clock in units of Hz
    unsigned short  RefreshRate;            // refresh rate in units of 0.01 Hz
    unsigned char   Reserved[40];           // reserved for later
} VBE_CRTC_INFO_BLOCK;

// VESA 3.0 far call memory struct (48 bits address)
typedef struct
{
    unsigned int    offset;                 // 32 bits offset
    unsigned short  segment;                // 16 bits segment
} VBE_FAR_CALL;

// VBE 3.0 call registers
typedef struct {
    unsigned short  ax;
    unsigned short  bx;
    unsigned short  cx;
    unsigned short  dx;
    unsigned short  si;
    unsigned short  di;
    unsigned short  es;
} VBE_CALL_REGS16;

// VBE 3.0 call stack
typedef struct {
    unsigned int    *esp;
    unsigned int    *ss;
} VBE_CALL_STACK;

// Palette structure
typedef struct
{
    unsigned char   r;
    unsigned char   g;
    unsigned char   b;
} RGB;

// RGB structure (use for VESA 2.0 SetPaletteRange function)
typedef struct
{
    unsigned char   r;
    unsigned char   g;
    unsigned char   b;
    unsigned char   a;
} PAL;

// RGBA structure (32bits color)
typedef struct
{
    unsigned char   b;
    unsigned char   g;
    unsigned char   r;
    unsigned char   a;
} RGBA;

typedef struct
{
    float   x;
    float   y;
} POINT;

typedef struct
{
    int elem;
    int data[MAX_STACK_SIZE];
} STACK;

// GFXLIB vector stroke info
typedef struct
{
    unsigned char code;                     // stroke code (0: unuse, 1: moveto, 2: lineto)
    unsigned char x, y;                     // stroke coordinates
} XFN_STROKE_INFO;

// GFXLIB vector stroke data
typedef struct
{
    unsigned char   width;                  // stroke width
    unsigned char   height;                 // stroke height
    unsigned short  numOfLines;             // number of strokes
} XFN_STROKE_DATA;

// GFXLIB font info table
typedef struct
{
    unsigned int    startOffset;            // offset of the font start
    unsigned char   bitsPerPixel;           // bits per pixel
    unsigned short  bytesPerLine;           // bytes per line (BMP-font)
    unsigned short  width;                  // font width
    unsigned short  height;                 // font height
    unsigned short  baseLine;               // baseLine of the character
    unsigned short  descender;              // font desender
    unsigned short  startChar;              // start of character
    unsigned short  endChar;                // end of character
    unsigned char   distance;               // distance between characters
    unsigned char   randomX;                // only <> 0 if flag anipos on
    unsigned char   randomY;                // only <> 0 if flag anipos on
    unsigned int    usedColors;             // only use for BMP8 font
    unsigned int    spacer;                 // distance for non-existing chars
    unsigned char   reserved[10];           // reserved for later use
} XFN_FONT_INFO;

// GFXLIB font header
typedef struct
{
    char            sign[4];                // font signature 'Fnt2'
    unsigned short  version;                // version number 0x0101
    char            name[32];               // name of font
    char            copyRight[32];          // font copy-right (use for BGI font)
    char            fontType[4];            // font type BMP1, BMP8, VECT, ...
    unsigned short  subFonts;               // number of sub-fonts (difference size)
    unsigned char*  dataPtr;                // address of raw font data
    unsigned int    memSize;                // memory size on load raw data
    unsigned int    flags;                  // font flags (ANIPOS, ANIMATION, MULTI, ...)
    XFN_FONT_INFO   info;                   // sub-fonts data info
} XFN_FONT_HEADER;

// the structure for store bitmap data
typedef struct
{
    unsigned int    bmWidth;                // bitmap width (in pixel)
    unsigned int    bmHeight;               // bitmap height (in pixel)
    unsigned int    bmPixels;               // byte per pixels
    unsigned int    bmRowBytes;             // bytes per sscan line
    unsigned char   bmExtra[768];           // extra data (palate for 256 color, rgb mask for 15/16 bit, rgb pos)
    unsigned char*  bmData;                 // raw image data
} BITMAP;

// the structure of image data (base image for GFXLIB)
typedef struct
{
    unsigned int    mWidth;                 // image width
    unsigned int    mHeight;                // image height
    unsigned int    mPixels;                // image bytes per pixels
    unsigned int    mRowBytes;              // image bytes per line
    unsigned int    mSize;                  // image size in bytes
    unsigned char*  mData;                  // image raw data
} IMAGE;

// mouse callback data
typedef struct {
    unsigned short  max;                    // mouse code event
    unsigned short  mbx;                    // callback param bx
    unsigned short  mcx;                    // callback param cx
    unsigned short  mdx;                    // callback param dx
} MOUSE_CALLBACK_DATA;

// the structure for animated mouse pointers
typedef struct tagMOUSEBITMAP MOUSE_BITMAP;
struct tagMOUSEBITMAP
{
    unsigned int    mbHotX;                 // mouse hotspot x
    unsigned int    mbHotY;                 // mouse hotspot y
    unsigned char*  mbData;                 // mouse bitmap data
    MOUSE_BITMAP*   mbNext;                 // point to next mouse data
};

// the structure for a bitmap mouse pointer.
typedef struct
{
    unsigned int    msState;                // status
    unsigned int    msNumBtn;               // number of buttons
    unsigned int    msPosX;                 // current pos x
    unsigned int    msPosY;                 // current pos y
    unsigned int    msWidth;                // mouse image width
    unsigned int    msHeight;               // mouse image height
    unsigned int    msPixels;               // mouse image byte per pixel
    unsigned char*  msUnder;                // mouse under bacground
    MOUSE_BITMAP*   msBitmap;               // hold mouse bitmap info
} MOUSE_IMAGE;

// the structure for a bitmap button.
typedef struct
{
    unsigned int    btPosX;                 // button x
    unsigned int    btPosY;                 // button y
    unsigned int    btState;                // button state (normal, hover, click, disable, ...)
    unsigned int    btWidth;                // button width (each state)
    unsigned int    btHeight;               // button height (each state)
    unsigned int    btPixels;               // button bytes per pixel
    unsigned char*  btData[BUTTON_BITMAPS]; // hold mouse bitmap data
} BUTTON_BITMAP;

// BMP header format
typedef struct
{
    unsigned short  bfType;                 // must be 'BM' 
    unsigned int    bfSize;                 // size of the whole bitmap file
    unsigned short  bfReserved1;            // must be 0
    unsigned short  bfReserved2;            // must be 0
    unsigned int    bfOffBits;              // offset to data, bytes
} BMP_HEADER;

// BMP info format
typedef struct
{
    unsigned int    biSize;                 // size of the structure
    int             biWidth;                // bitmap bmWidth
    int             biHeight;               // bitmap bmHeight
    unsigned short  biPlanes;               // number of colour planes
    unsigned short  biBitCount;             // bits per pixel
    unsigned int    biCompression;          // compression
    unsigned int    biSizeImage;            // size of the data in bytes
    int             biXPelsPerMeter;        // pixels per meter x
    int             biYPelsPerMeter;        // pixels per meter y
    unsigned int    biClrUsed;              // colors used
    unsigned int    biClrImportant;         // important colors
} BMP_INFO;

// DMPI memory status info block
typedef struct {
    unsigned int    LargestBlockAvail;      // total system memory
    unsigned int    MaxUnlockedPage;        // maximun unlockable page
    unsigned int    LargestLockablePage;    // largest lockable page
    unsigned int    LinAddrSpace;           // linear space address
    unsigned int    NumFreePagesAvail;      // number of free page avaiable
    unsigned int    NumPhysicalPagesFree;   // number of physical free (page)
    unsigned int    TotalPhysicalPages;     // total physical page
    unsigned int    FreeLinAddrSpace;       // total free linear address space
    unsigned int    SizeOfPageFile;         // size of page
    unsigned int    Reserved[3];
} MEM_INFO;
#pragma pack(pop)

// INTEL CPU features description
static const char *edxFeatures[][2] =
{
    {"On-chip x87", "FPU"},
    {"Virtual 8086 Mode Enhancements", "VME"},
    {"Debugging Extensions", "DE"},
    {"Page Size Extensions", "PSE"},
    {"Time Stamp Counter", "TSC"},
    {"RDMSR/WRMSR Instructions", "MSR"},
    {"Physical Address Extensions", "PAE"},
    {"Machine Check Exception", "MCE"},
    {"CMPXCHG8B Instruction", "CX8"},
    {"On-chip APIC", "APIC"},
    {"Reserved 1", "R1!!"},                         // bit 10
    {"SYSENTER/SYSEXIT Instructions", "SEP"},
    {"Memory Type Range Registers", "MTRR"},
    {"PTE Global Bit", "PGE"},
    {"Machine Check Architecture", "MCA"},
    {"Conditional Move Instructions", "CMOV"},
    {"Page Attribute Table", "PAT"},
    {"36-bit Page Size Extension", "PSE-36"},
    {"Processor Serial Number", "PSN"},
    {"CFLUSH Instruction", "CLFSH"},
    {"Reserved 2", "R2!!"},                         // bit 20
    {"Debug Store", "DS"},
    {"Thermal Monitor and Clock Control", "ACPI"},
    {"MMX Technology", "MMX"},
    {"FXSAVE/FXRSTOR Instructions", "FXSR"},
    {"SSE Extensions", "SSE"},
    {"SSE2 Extensions", "SSE2"},
    {"Self Snoop", "SS"},
    {"Hyper-threading Technology", "HTT"},
    {"Thermal Monitor", "TM"},
    {"Reserved 3", "R3!!"},                         // bit 30
    {"Pending Break Enable", "PBE"}
};

// INTEL CPU extended features description
static const char *ecxFeatures[][2] =
{
    {"Streaming SIMD Extensions 3", "SSE3"},
    {"PCLMULDQ Instruction", "PCLMULDQ"},
    {"64-bit Debug Store", "DTES64"},
    {"MONITOR/MWAIT Instructions", "MONITOR"},
    {"CPL Qualified Debug Store", "DS-CPL"},
    {"Virtual Machine Extensions", "VMX"},
    {"Safer Mode Extensions", "SMX"},
    {"Enhanced Intel SpeedStep", "EIST"},
    {"Thermal Monitor 2", "TM2"},
    {"Supplemental SSE3", "SSSE3"},
    {"L1 Context ID", "CNTX-ID"},                   // bit 10
    {"Reserved 1", "R1!!"},
    {"Fused Multiply Add", "FMA"},
    {"CMPXCHG16B Instruction", "CX16"},
    {"xTPR Update Control", "xTPR"},
    {"Perfmon/Debug Capability", "PDCM"},
    {"Reserved 2", "R2!!"},
    {"Process Context Identifiers", "PCID"},
    {"Direct Cache Access", "DCA"},
    {"Streaming SIMD Extensions 4.1", "SSE4.1"},
    {"Streaming SIMD Extensions 4.2", "SSE4.2"},    // bit 20
    {"Extended xAPIC Support", "x2APIC"},
    {"MOVBE Instruction", "MOVBE"},
    {"POPCNT Instruction", "POPCNT"},
    {"Time Stamp Counter Deadline", "TSC-DEADLINE"},
    {"AES Instruction Extensions", "AES"},
    {"XSAVE/XRSTOR States", "XSAVE"},
    {"OS-Enabled Ext State Mgmt", "OSXSAVE"},
    {"Advanced Vector Extensions", "AVX"},
    {"Reserved 3", "R3!!"},
    {"Reserved 4", "R4!!"},                         // bit 30
    {"Hypervisor Present", "HVP"}
};

// AMD CPU extended features description
static const char *amdFeatures[][2] = 
{
    {"Reserved 1", "R1!!"},
    {"Reserved 2", "R2!!"},
    {"Reserved 3", "R3!!"},
    {"Reserved 4", "R4!!"},
    {"Reserved 5", "R5!!"},
    {"Reserved 6", "R6!!"},
    {"Reserved 7", "R7!!"},
    {"Reserved 8", "R8!!"},
    {"Reserved 9", "R9!!"},
    {"Reserved 10", "R10!!"},
    {"Reserved 11", "R11!!"},                       // bit 10
    {"Reserved 12", "R12!!"},
    {"Reserved 13", "R13!!"},
    {"Reserved 14", "R14!!"},
    {"Reserved 15", "R15!!"},
    {"Reserved 16", "R16!!"},
    {"Reserved 17", "R17!!"},
    {"Reserved 18", "R18!!"},
    {"Reserved 19", "R19!!"},
    {"Support SMP", "SMP"},
    {"Reserved 20", "R20!!"},                       // bit 20
    {"Reserved 21", "R21!!"},
    {"MMX extended", "MMX+"},
    {"Reserved 22", "R22!!"},
    {"Reserved 23", "R23!!"},
    {"Reserved 24", "R24!!"},
    {"Reserved 25", "R25!!"},
    {"Reserved 26", "R26!!"},
    {"Reserved 27", "R27!!"},
    {"Reserved 28", "R28!!"},
    {"Extended 3D Now!", "3DNow+!"},                // bit 30
    {"3D Now!", "3DNow!"}
};

// Pointer functions handler
void            (*ClearScreen)(unsigned int) = NULL;
void            (*FillRect)(int, int, int, int, unsigned int) = NULL;
void            (*FillRectSub)(int, int, int, int, unsigned int) = NULL;
void            (*FillRectAdd)(int, int, int, int, unsigned int) = NULL;
void            (*FillRectPattern)(int, int, int, int, unsigned int, unsigned char*) = NULL;
void            (*FillRectPatternAdd)(int, int, int, int, unsigned int, unsigned char*) = NULL;
void            (*FillRectPatternSub)(int, int, int, int, unsigned int, unsigned char*) = NULL;
void            (*PutPixel)(int, int, unsigned int) = NULL;
void            (*PutPixelAdd)(int, int, unsigned int) = NULL;
void            (*PutPixelSub)(int, int, unsigned int) = NULL;
unsigned int    (*GetPixel)(int, int) = NULL;
void            (*HorizLine)(int, int, int, unsigned int) = NULL;
void            (*HorizLineAdd)(int, int, int, unsigned int) = NULL;
void            (*HorizLineSub)(int, int, int, unsigned int) = NULL;
void            (*VertLine)(int, int, int, unsigned int) = NULL;
void            (*VertLineAdd)(int, int, int, unsigned int) = NULL;
void            (*VertLineSub)(int, int, int, unsigned int) = NULL;
void            (*GetImage)(int, int, int, int, IMAGE*) = NULL;
void            (*PutImage)(int, int, IMAGE*) = NULL;
void            (*PutImageAdd)(int, int, IMAGE*) = NULL;
void            (*PutImageSub)(int, int, IMAGE*) = NULL;
void            (*PutSprite)(int, int, unsigned int, IMAGE*) = NULL;
void            (*PutSpriteAdd)(int, int, unsigned int, IMAGE*) = NULL;
void            (*PutSpriteSub)(int, int, unsigned int, IMAGE*) = NULL;
void            (*ScaleImage)(IMAGE*, IMAGE*, int) = NULL;
void            (*FadeOutImage)(IMAGE*, unsigned char) = NULL;

// Linear frame buffer
unsigned char   *lfbPtr = NULL;             // address of render buffer
unsigned int    lfbSize = 0;                // size of render buffer (width * height * bytesperpixel)
unsigned char   bitsPerPixel = 0;           // bits per pixel
unsigned int    bytesPerPixel = 0;          // bytes per pixel
unsigned int    bytesPerScanline = 0;       // bytes per lines (width * bytesperpixel)
unsigned int    vbeSegment = 0;             // segment of VBE block
unsigned int    crtcSegment = 0;            // segment of CRTC block

// VESA 2.0 protect mode interfaces
VBE_PM_INFO     *pmInfo = NULL;             // protect mode interface info
void            *fnSetDisplayStart = NULL;  // SetDisplayStart function address
void            *fnSetWindow = NULL;        // SetWindow function address
void            *fnSetPalette = NULL;       // SetPalette function address
unsigned int    *pmBasePtr = NULL;          // address of base code
unsigned short  pmSelector = 0;             // protect mode selector

// Page flipping
unsigned int    activePage = 0;             // current active page
unsigned int    pageOffset = 0;             // page start offset (use for visual page and active page)
unsigned int    numOfPages = 0;             // number of virtual screen page

// Palette mask
unsigned int    rmask = 0, gmask = 0, bmask = 0;
unsigned int    rshift = 0, gshift = 0, bshift = 0;
unsigned int    rpos = 0, gpos = 0, bpos = 0;

// Clip cordinate handle
int             cminx = 0, cminy = 0, cmaxx = 0, cmaxy = 0;
int             maxx = 0, maxy = 0, centerx = 0, centery = 0;
int             xres = 0, yres = 0, currx = 0, curry = 0;

// 3D projection
enum            {PERSPECTIVE, PARALLELE};
float           de = 0.0, rho = 0.0, theta = 0.0, phi = 0.0;
float           aux1 = 0.0, aux2 = 0.0, aux3 = 0.0, aux4 = 0.0;
float           aux5 = 0.0, aux6 = 0.0, aux7 = 0.0, aux8 = 0.0;
float           xobs = 0.0, yobs = 0.0, zobs = 0.0;
float           xproj = 0.0, yproj = 0.0;
int             xecran = 0, yecran = 0;
unsigned char   projection = 0;

// Timer defination
unsigned int    cpuSpeed = 0;               // CPU clock rate in MHz
unsigned int    timeRes = 100;              // timer resolution (default for clock time)
unsigned char   timeType = 0;               // timer type: 0 - clock time, 1 - cpu time

// CPU features
int             haveMMX = 0;                // check for have MMX extended
int             haveSSE = 0;                // check for have SSE extended
int             have3DNow = 0;              // check for have 3DNow extended

// Mouse callback data
MOUSE_CALLBACK_DATA mcd;

// Memory status info
MEM_INFO        meminfo;

// GFXLIB font data
XFN_FONT_HEADER font[GFX_MAX_FONT] = {0};           // GFXLIB font loadable at the same time
unsigned char   *fontPalette[GFX_MAX_FONT] = {0};   // GFXLIB font palette data (BMP8 type)
unsigned char   *gfxBuff = NULL;                    // GFXLIB buffer
unsigned int    subFont = 0;                        // GFXLIB sub-fonts
unsigned int    fontType = 0;                       // current selected font (use for multiple loaded font)
unsigned int    randSeed = 0;                       // global random seed
unsigned int    factor = 0x8088405;                 // global factor
unsigned int    stackOffset = 0;                    // kernel stack offset

// Pattern filled styles
unsigned char   ptnLine[]           = {0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00};
unsigned char   ptnLiteSlash[]      = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
unsigned char   ptnSlash[]          = {0x07, 0x0E, 0x1C, 0x38, 0x70, 0xE0, 0xC1, 0x83};
unsigned char   ptnBackSlash[]      = {0x07, 0x83, 0xC1, 0xE0, 0x70, 0x38, 0x1C, 0x0E};
unsigned char   ptnLiteBackSlash[]  = {0x5A, 0x2D, 0x96, 0x4B, 0xA5, 0xD2, 0x69, 0xB4};
unsigned char   ptnHatch[]          = {0xFF, 0x88, 0x88, 0x88, 0xFF, 0x88, 0x88, 0x88};
unsigned char   ptnHatchX[]         = {0x18, 0x24, 0x42, 0x81, 0x81, 0x42, 0x24, 0x18};
unsigned char   ptnInterLeave[]     = {0xCC, 0x33, 0xCC, 0x33, 0xCC, 0x33, 0xCC, 0x33};
unsigned char   ptnWideDot[]        = {0x80, 0x00, 0x08, 0x00, 0x80, 0x00, 0x08, 0x00};
unsigned char   ptnCloseDot[]       = {0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00};

// Round up integer
int RoundInt(double x)
{
    if (x > 0.0) return x + 0.5;
    return x - 0.5;
}

// Round up to unsigned long
unsigned long RoundLong(double x)
{
    if (x > 0.0) return (unsigned long)(x + 0.5);
    return (unsigned long)(x - 0.5);
}

// Generate random value from number
int Random(int a)
{
    return (a == 0) ? 0 : rand() % a;
}

// Generate random value in range
int RandomRange(int a, int b)
{
    return (a < b) ? (a + (rand() % (b - a + 1))) : (b + (rand() % (a - b + 1)));
}

// Wrapper memcpy to use inline asm code
// prevent old compiler not optimize this function
void CopyData(void *dst, void *src, unsigned int size)
{
    _asm {
        pusha
        mov    edi, dst
        mov    esi, src
        mov    ecx, size
        shr    ecx, 2
        rep    movsd
        mov    ecx, size
        and    ecx, 3
        rep    movsb
        popa
    }
}

// A CPUID instruction wrapper
void CPUID(unsigned int *cpuinfo, unsigned int type)
{
    _asm {
        mov    eax, type
        mov    edi, cpuinfo
        cpuid
        mov    [edi     ], eax
        mov    [edi + 4 ], ebx
        mov    [edi + 8 ], ecx
        mov    [edi + 12], edx
    }
}

// Get current FPC clock (64 bits)
unsigned long long GetRDTSC()
{
    unsigned long long val = 0;

    _asm {
        xor    eax, eax
        cpuid
        rdtsc
        lea    edi, val
        mov    [edi    ], eax
        mov    [edi + 4], edx
    }
    return val;
}

// Get current CPU ticks (64 bits)
unsigned long long GetPIT()
{
    unsigned long long val = 0;

    _asm {
        xor    eax, eax
        cli
        out    43h, al
        mov    edi, 046Ch
        mov    edx, [edi]
        in     al, 40h
        db     0xEB, 0x00, 0xEB, 0x00, 0xEB, 0x00
        mov    ah, al
        in     al, 40h
        db     0xEB, 0x00, 0xEB, 0x00, 0xEB, 0x00
        xchg   ah, al
        neg    ax
        mov    edi, eax
        sti
        mov    ebx, 10000h
        mov    eax, edx
        xor    edx, edx
        mul    ebx
        add    eax, edi
        adc    edx, 0
        lea    edi, val
        mov    [edi    ], eax
        mov    [edi + 4], edx
    }
    return val;
}

// Get current CPU clock rate in Mhz
float GetCpuClockRate()
{
    unsigned int i = 0;
    unsigned long long time0 = 0, time1 = 0;
    unsigned long long stamp0 = 0, stamp1 = 0;
    unsigned long long ticks = 0, cycles = 0;
    float total = 0, freq[MIN_TRIES] = {0};

    // try to calculate CPU clock rate
    for (i = 0; i != MIN_TRIES; i++)
    {
        // start record clock cycles
        stamp0 = GetRDTSC();

        // wait for ticks count
        time0 = GetPIT();
        while ((time1 = GetPIT()) < (time0 + WAIT_TIME));

        // stop record clock cycles
        stamp1 = GetRDTSC();

        // calculate CPU frequences
        cycles = stamp1 - stamp0;
        ticks = (time1 - time0) * timeRes / PIT_RATE;
        freq[i] = 1.0 * cycles / ticks;
    }

    // calculate agverage
    for (i = 0; i != MIN_TRIES; i++) total += freq[i];
    return total / MIN_TRIES;
}

// Get current CPU speed in MHz
unsigned int GetCpuSpeed()
{
    float speed = 0.0;
    unsigned int i = 0;
    for (i = 0; i != MAX_RUN; i++) speed += GetCpuClockRate();
    return speed / MAX_RUN;
}

// Get CPU details feartures info
void PrintCpuInfo(int verbose)
{
    unsigned int    i, eax, edx, ecx;
    unsigned char   cputype = 0;
    unsigned int    cpuinfo[4] = {0};
    char            cpuvendor[16] = {0};

    // Obtain the number of CPUID leaves and the vendor string.
    CPUID(cpuinfo, 0);

    // Get highest CPUID features
    eax = cpuinfo[CPUID_EAX];

    // Get short manufactures
    *(unsigned int*)(cpuvendor    ) = cpuinfo[CPUID_EBX];
    *(unsigned int*)(cpuvendor + 4) = cpuinfo[CPUID_EDX];
    *(unsigned int*)(cpuvendor + 8) = cpuinfo[CPUID_ECX];

    if (!strcmpi(cpuvendor, "AuthenticAMD")) cputype = PROCESSOR_AMD;
    else if (!strcmpi(cpuvendor, "GenuineIntel")) cputype = PROCESSOR_INTEL;
    else cputype = PROCESSOR_UNKNOWN;

    if (cputype != PROCESSOR_UNKNOWN) printf("CPU vendor: %s\n", cpuvendor);
    else printf("CPU vendor: unknown\n");
    printf("Processor features:\n");

    // have any highest CPUID features?
    if (eax >= 1)
    {
        if (cputype == PROCESSOR_INTEL)
        {
            // Obtain the model and features.
            CPUID(cpuinfo, 1);
            edx = cpuinfo[CPUID_EDX];
            ecx = cpuinfo[CPUID_ECX];

            // Interpret the feature bits.
            for (i = 0; i != 32; i++)
            {
                if (edx & (1L << i))
                {
                    if (verbose) printf("%s (%s)\n", edxFeatures[i][0], edxFeatures[i][1]);
                    else printf("%s ", edxFeatures[i][1]);
                }
            }

            // Interpret the extended feature bits.
            for (i = 0; i != 32; i++)
            {
                if (ecx & (1L << i))
                {
                    if (verbose) printf("%s (%s)\n", ecxFeatures[i][0], ecxFeatures[i][1]);
                    else printf("%s ", ecxFeatures[i][1]);
                }
            }
        }
    }

    if (cputype == PROCESSOR_AMD)
    {
        // Get highest extended feature
        CPUID(cpuinfo, CPUID_EXT_BASE);
        eax = cpuinfo[0];
    
        // check for have any extended features     
        if (eax >= CPUID_EXT_BASE1)
        {
            // Get extended features
            CPUID(cpuinfo, CPUID_EXT_BASE1);
            edx = cpuinfo[CPUID_EDX];

            for (i = 0; i != 32; i++)
            {
                if (edx & (1L << i))
                {
                    if (verbose) printf("%s (%s)\n", amdFeatures[i][0], amdFeatures[i][1]);
                    else printf("%s ", amdFeatures[i][1]);
                }
            }
        }
    }
}

// Dump short CPU features info
void GetCpuInfo(char *vendor, char *features)
{
    unsigned int    eax, edx;
    unsigned char   cputype = 0;
    unsigned int    cpuinfo[4] = {0};
    char            cpuvendor[16] = {0};

    // Obtain the number of CPUID leaves and the vendor string.
    CPUID(cpuinfo, 0);

    // Get highest CPUID features
    eax = cpuinfo[CPUID_EAX];

    // Get short manufactures
    *(unsigned int*)(cpuvendor    ) = cpuinfo[CPUID_EBX];
    *(unsigned int*)(cpuvendor + 4) = cpuinfo[CPUID_EDX];
    *(unsigned int*)(cpuvendor + 8) = cpuinfo[CPUID_ECX];

    if (!strcmp(cpuvendor, "AuthenticAMD")) cputype = PROCESSOR_AMD;
    else if (!strcmp(cpuvendor, "GenuineIntel")) cputype = PROCESSOR_INTEL;
    else cputype = PROCESSOR_UNKNOWN;

    if (cputype != PROCESSOR_UNKNOWN) strcpy(vendor, cpuvendor);
    else strcpy(vendor, "unknown");

    if (eax >= 1)
    {
        if (cputype == PROCESSOR_INTEL)
        {
            // Obtain the model and features.
            CPUID(cpuinfo, 1);
            edx = cpuinfo[CPUID_EDX];

            // MMX feature bits
            if (edx & (1L << 23))
            {
                strcat(features, edxFeatures[23][1]);
                strcat(features, " ");
                haveMMX = 1;
            }

            // SSE feature bits
            if (edx & (1L << 25))
            {
                strcat(features, edxFeatures[25][1]);
                strcat(features, " ");
                haveSSE = 1;
            }

            // SSE2 feature bits
            if (edx & (1L << 26))
            {
                strcat(features, edxFeatures[26][1]);
                strcat(features, " ");
                haveSSE = 1;
            }
        }
    }

    if (cputype == PROCESSOR_AMD)
    {
        // Get highest extended feature
        CPUID(cpuinfo, CPUID_EXT_BASE);
        eax = cpuinfo[0];
    
        // check for have any extended features     
        if (eax >= CPUID_EXT_BASE1)
        {       
            // Get extended features
            CPUID(cpuinfo, CPUID_EXT_BASE1);
            edx = cpuinfo[CPUID_EDX];

            // 3DNow!
            if (edx & (1L << 31))
            {
                strcat(features, amdFeatures[31][1]);
                strcat(features, " ");
                have3DNow = 1;
            }

            // 3DNow+!
            if (edx & (1L << 30))
            {
                strcat(features, amdFeatures[30][1]);
                strcat(features, " ");
                have3DNow = 1;
            }

            // MMX+!
            if (edx & (1L << 30))
            {
                strcat(features, amdFeatures[22][1]);
                strcat(features, " ");
                haveMMX = 1;
            }
        }
    }

    // remove steal character
    edx = strlen(features);
    if (edx > 0) features[edx - 1] = '\0';
    else strcpy(features, "none");
}

// Get system memory status info
void GetMemoryInfo()
{
    MEM_INFO *pmem = &meminfo;
    memset(pmem, 0, sizeof(MEM_INFO));

    // call DMPI function to get system memory status info
    _asm {
        mov    eax, 0500h
        mov    edi, pmem
        int    31h
    }
}

// Init the timer to use system time or cpu clock time
// this is a first function call before init vesa mode
void InitGfxLib(unsigned char type)
{
    timeType = type;
    if (timeType)
    {
        // Timing resolution of 1 microsecond
        timeRes = 1000000;
        cpuSpeed = GetCpuSpeed();
    }
    else
    {
        // Timing resolution of 100 milisecond
        timeRes = 100;
        cpuSpeed = 0;
    }

    // Initialize random number generation
    randSeed = time(NULL);
    srand(randSeed);

    // Initialize GFXLIB buffer
    gfxBuff = (unsigned char*)malloc(GFX_BUFF_SIZE);
    if (!gfxBuff)
    {
        printf("InitGfxLib: Cannot initialize GFXLIB buffer!\n");
        exit(1);
    }
    memset(gfxBuff, 0, GFX_BUFF_SIZE);

    // Filled memory info structure
    GetMemoryInfo();
}

// Convert CPU ticks to microsecond
unsigned long long TicksToMicroSec(unsigned long long ticks)
{
    return ticks / cpuSpeed;
}

// Get current system time (in 100ms or 1 microsecond)
unsigned long long GetCurrentTime()
{
    struct dostime_t dt;
    
    // timer is use RDTSC (timing resolution in 1 microsecond)
    if (timeType) return GetRDTSC();

    // timer is use system time (timing resolution in 100 milisecond)
    _dos_gettime(&dt);
    return (dt.hsecond + (dt.second + (dt.minute + dt.hour * 60) * 60) * 100);
}

// Get elapsed time from the begining time
unsigned long long GetElapsedTime(unsigned long long tmstart)
{
    if (timeType) return TicksToMicroSec(GetCurrentTime() - tmstart);
    return (GetCurrentTime() - tmstart);
}

// Wait for time out
void WaitFor(unsigned long long tmstart, unsigned long long ms)
{
    unsigned long long tmwait = (ms * timeRes) / 1000;
    while (GetElapsedTime(tmstart) < tmwait);
}

// Sleep CPU execution
void Sleep(unsigned long long ms)
{
    unsigned long long tmwait = (ms * timeRes) / 1000;
    unsigned long long tmstart = GetCurrentTime();
    while (GetElapsedTime(tmstart) < tmwait);
}

// DPMI alloc DOS memory block
unsigned int AllocDosSegment(unsigned int pageSize)
{
    unsigned int value = 0;

    _asm {
        mov    ebx, pageSize
        cmp    ebx, 65535
        ja     error
        add    ebx, 15
        shr    ebx, 4
        mov    eax, 0100h
        int    31h
        jc     error
        and    eax, 0000FFFFh
        shl    edx, 16
        add    eax, edx
        jmp    quit
    error:
        xor    eax, eax
    quit:
        mov    value, eax
    }     

    return value;
}

// DPMI free DOS memory block
void FreeDosSegment(unsigned int *selSegment)
{
    _asm {
        mov    edi, selSegment
        mov    edx, [edi]
        push   edi
        shr    edx, 16
        mov    eax, 0101h
        int    31h
        pop    edi
        mov    dword ptr [edi], 0
    }
}

// Simulation real mode interrupt
int SimRealModeInt(unsigned char num, RM_REGS *rmRegs)
{
    int value = 0;

    _asm {
        xor    ebx, ebx
        mov    bl, num
        mov    edi, rmRegs
        xor    ecx, ecx
        mov    eax, 0300h
        int    31h
        jc     error
        mov    eax, 1
        jmp    quit
    error:
        xor    eax, eax
    quit:
        mov    value, eax
    }

    return value;
}

// DPMI alloc selector
unsigned short AllocSelector()
{
    unsigned short value = 0;

    _asm {
        xor    eax, eax
        mov    ecx, 1
        int    31h
        jnc    quit
        xor    ax, ax
    quit:
        mov    value, ax
    }

    return value;
}

// DPMI free selector
void FreeSelector(unsigned short *sel)
{
    _asm {
        mov    edi, sel
        mov    bx, [edi]
        mov    dword ptr [edi], 0
        mov    eax, 0001h
        int    31h
    }
}

// DPMI set selector access rights
int SetSelectorRights(unsigned short sel, unsigned short accRights)
{
    int value = 0;

    _asm {
        mov    bx, sel
        mov    cx, accRights
        mov    eax, 0009h
        int    31h
        jc     error
        mov    eax, 1
        jmp    quit
    error:
        xor    eax, eax
    quit:
        mov    value, eax
    }

    return value;
}

// DPMI set selector base
int SetSelectorBase(unsigned short sel, unsigned int linearAddr)
{
    int value = 0;

    _asm {
        mov    bx, sel
        mov    ecx, linearAddr
        mov    edx, ecx
        shr    ecx, 16
        and    edx, 0FFFFh
        mov    eax, 0007h
        int    31h
        jc     error
        mov    eax, 1
        jmp    quit
    error:
        xor    eax, eax
    quit:
        mov    value, eax
    }

    return value;
}

// DPMI set selector limit
int SetSelectorLimit(unsigned short sel, unsigned int selLimit)
{
    int value = 0;

    _asm {
        mov    bx, sel
        mov    ecx, selLimit
        mov    edx, ecx
        shr    ecx, 16
        and    edx, 0FFFFh
        mov    eax, 0008h
        int    31h
        jc     error
        mov    eax, 1
        jmp    quit
    error:
        xor    eax, eax
    quit:
        mov    value, eax
    }

    return value;
}

// DPMI map real address to linear address
unsigned int MapPhysicalAddress(unsigned int physAddr, unsigned int len)
{
    unsigned int value = 0;

    _asm {
        mov    ebx, physAddr
        mov    esi, len
        mov    ecx, ebx
        mov    edi, esi
        shr    ebx, 16
        and    ecx, 0FFFFh
        shr    esi, 16
        and    edi, 0FFFFh
        mov    eax, 0800h
        int    31h
        jc     error
        shl    ebx, 16
        and    ecx, 0FFFFh
        mov    eax, ebx
        or     eax, ecx
        jmp    quit
    error:
        xor    eax, eax
    quit:
        mov    value, eax
    }

    return value;
}

// DPMI free linear address
void FreePhysicalAddress(unsigned int* linearAddr)
{
    _asm {
        mov    edi, linearAddr
        mov    bx, [edi + 2]
        mov    cx, [edi]
        mov    dword ptr [edi], 0
        mov    eax, 0801h
        int    31h
    }
}

// Convert real pointer to linear pointer
unsigned int MapRealPointer(unsigned int rmSegOfs)
{
    unsigned int value = 0;

    _asm {
        mov    eax, rmSegOfs
        mov    edx, eax
        and    eax, 0FFFF0000h
        and    edx, 0000FFFFh
        shr    eax, 12
        add    eax, edx
        mov    value, eax
    }

    return value;
}

// Get VESA driver info
int GetVesaDriverInfo(VBE_DRIVER_INFO *info)
{
    RM_REGS         regs;
    VBE_DRIVER_INFO *drvInfo;

    // Alloc 1K memory to store VESA driver and mode info
    if (vbeSegment == 0) vbeSegment = AllocDosSegment(1024);
    if (vbeSegment == 0 || vbeSegment == 0xFFFF) return 0;

    // Setup pointer memory
    drvInfo = (VBE_DRIVER_INFO*)((vbeSegment & 0x0000FFFF) << 4);
    memset(drvInfo, 0, sizeof(VBE_DRIVER_INFO));
    memcpy(drvInfo->VBESignature, "VBE2", 4); // Request for VESA 2.0+

    memset(&regs, 0, sizeof(regs));
    regs.eax = 0x4F00;
    regs.es  = vbeSegment;
    regs.edi = 0;
    SimRealModeInt(0x10, &regs);

    // Check VESA 2.0+ to support linear frame buffer
    if (regs.eax == 0x004F && !memcmp(drvInfo->VBESignature, "VESA", 4) && drvInfo->VBEVersion >= 0x0200)
    {
        // Convert real string pointer to linear address
        drvInfo->OEMStringPtr       = MapRealPointer(drvInfo->OEMStringPtr);
        drvInfo->VideoModePtr       = MapRealPointer(drvInfo->VideoModePtr);
        drvInfo->OemVendorNamePtr   = MapRealPointer(drvInfo->OemVendorNamePtr);
        drvInfo->OemProductNamePtr  = MapRealPointer(drvInfo->OemProductNamePtr);
        drvInfo->OemProductRevPtr   = MapRealPointer(drvInfo->OemProductRevPtr);
        memcpy(info, drvInfo, sizeof(VBE_DRIVER_INFO));
        return 1;
    }

    return 0;
}

// Get VBE mode info
int GetVesaModeInfo(unsigned short mode, VBE_MODE_INFO *info)
{
    RM_REGS         regs;
    VBE_MODE_INFO   *modeInfo;

    if (!(mode && vbeSegment)) return 0;
    if (mode == 0xFFFF) return 0;

    // Setup memory pointer
    modeInfo = (VBE_MODE_INFO*)(((vbeSegment & 0x0000FFFF) << 4) + 512);
    memset(modeInfo, 0, sizeof(VBE_MODE_INFO));

    memset(&regs, 0, sizeof(regs));
    regs.eax = 0x4F01;
    regs.ecx = mode;
    regs.es  = vbeSegment;
    regs.edi = 512;
    SimRealModeInt(0x10, &regs);

    // Check for error call
    if (regs.eax != 0x004F) return 0;
    if (!(modeInfo->ModeAttributes & VBE_MASK_MODEHW)) return 0; // Mode must supported by hardware
    if (!(modeInfo->ModeAttributes & VBE_MASK_HASLFB)) return 0; // Mode must supported linear frame buffer
    if (!modeInfo->PhysBasePtr) return 0; // Physical address must be not null
    memcpy(info, modeInfo, sizeof(VBE_MODE_INFO));
    return 1;
}

// Close current VBE mode
void CloseVesaMode()
{
    RM_REGS regs;

    // Free memory
    memset(&regs, 0, sizeof(regs));
    regs.eax = 0x03;
    SimRealModeInt(0x10, &regs);
    FreePhysicalAddress((unsigned int*)&lfbPtr);
    lfbPtr = NULL;

    // Free selector
    if (pmSelector)
    {
        FreePhysicalAddress((unsigned int*)&pmBasePtr);
        FreeSelector(&pmSelector);
        pmBasePtr = NULL;
        pmSelector = 0;
    }

    // Free real mode VBE memory
    if (vbeSegment)
    {
        FreeDosSegment(&vbeSegment);
        vbeSegment = 0;
    }

    // Free real mode CRTC memory
    if (crtcSegment)
    {
        FreeDosSegment(&crtcSegment);
        crtcSegment = 0;
    }

    // Free protect demo info
    if (pmInfo)
    {
        free(pmInfo);
        pmInfo = NULL;
        fnSetDisplayStart = NULL;
        fnSetWindow = NULL;
        fnSetPalette = NULL;
    }

    // Free gfxBuff
    if (gfxBuff)
    {
        free(gfxBuff);
        gfxBuff = NULL;
    }
}

// Raise error message and exit program
void FatalError(const char *fmt, ...)
{
    va_list args;
    CloseVesaMode();
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    exit(1);
}

// VESA 3.0, calculate CRTC timing using GTF formular
void CalcCrtcTimingGTF(VBE_CRTC_INFO_BLOCK *crtc, int hpixels, int vlines, int freq, int interlaced, int margins)
{
    unsigned char doubleScan = 0;
    unsigned int marginLeftRight = 0;
    unsigned int marginTopBottom = 0;
    unsigned int horizPeriodEst;
    unsigned int vertSyncPlusBP;
    unsigned int vertLinesTotal;
    unsigned int horizPixelsTotal;
    unsigned int idealDutyCycle;
    unsigned int horizBlank;
    unsigned int pixelClock;

    // Re-calculate horizontial pixels
    hpixels = hpixels / CELL_GRAN * CELL_GRAN;

    // Check for double scanline
    if (vlines < 400)
    {
        doubleScan = 1;
        vlines <<= 1;
    }

    // Calculate margins
    if (margins)
    {
        marginLeftRight = (hpixels * FIXED(MARGIN_PERCENT) / (FIXED(100) * CELL_GRAN)) * CELL_GRAN;
        marginTopBottom = vlines * FIXED(MARGIN_PERCENT) / FIXED(100);
    }

    // Estimate the horizontal period
    horizPeriodEst = (2 * FIXED(FIXED(1000000) / freq) - 2 * FIXED(MIN_VSYNC_BP)) / (2 * (vlines + 2 * marginTopBottom + MIN_V_PORCH) + interlaced);

    // Find the number of lines in vSync + back porch
    vertSyncPlusBP = FIXED(MIN_VSYNC_BP) / horizPeriodEst + 1;

    // Correct range
    if (vertSyncPlusBP < V_SYNC + MIN_V_BP) vertSyncPlusBP = V_SYNC + MIN_V_BP;

    // Find the total number of lines in the vertical period
    vertLinesTotal = ((vlines + 2 * marginTopBottom + vertSyncPlusBP + MIN_V_PORCH) << interlaced) + interlaced;

    // Find the total number of pixels
    horizPixelsTotal = hpixels + 2 * marginLeftRight;

    // Find the ideal blanking duty cycle
    idealDutyCycle = C_PRIME - (M_PRIME * horizPeriodEst / FIXED(1000));

    // Correct range
    if (idealDutyCycle < 20) idealDutyCycle = 20;

    // Find the number of pixels in blanking time
    horizBlank = horizPixelsTotal * idealDutyCycle / ((100 - idealDutyCycle) * (2 * CELL_GRAN)) * (2 * CELL_GRAN);

    // Final total number of pixels
    horizPixelsTotal += horizBlank;

    // Find the pixel clock frequency
    pixelClock = FIXED(horizPixelsTotal) * (1000 / CLOCK_STEP) / horizPeriodEst * CLOCK_STEP * 1000; // in Hz

    // Store CRTC data
    crtc->HorizontalTotal           = horizPixelsTotal;
    crtc->HorizontalSyncEnd         = horizPixelsTotal - horizBlank / 2;
    crtc->HorizontalSyncStart       = crtc->HorizontalSyncEnd - horizPixelsTotal * FIXED(H_SYNC_PERCENT) / (FIXED(100) * CELL_GRAN) * CELL_GRAN;
    crtc->VerticalTotal             = vertLinesTotal;
    crtc->VerticalSyncStart         = vlines + MIN_V_PORCH;
    crtc->VerticalSyncEnd           = vlines + MIN_V_PORCH + V_SYNC;
    crtc->PixelClock                = pixelClock;
    crtc->RefreshRate               = 100 * (pixelClock / (horizPixelsTotal * vertLinesTotal));  // in 0.01 Hz
    crtc->Flags                     = CRTC_HSYNC_NEGATIVE | CRTC_VSYNC_NEGATIVE;
    if (interlaced) crtc->Flags     |= CRTC_INTERLACED;
    if (doubleScan) crtc->Flags     |= CRTC_DOUBLE_SCANLINE;
}

// VESA 3.0, Get VESA closest clock, pixel clock in Hz
unsigned long GetClosestPixelClock(unsigned short modeNum, unsigned long pixelClock)
{
    RM_REGS regs;
    memset(&regs, 0, sizeof(RM_REGS));
    regs.eax = 0x4F0B;
    regs.ebx = 0;
    regs.ecx = pixelClock;
    regs.edx = modeNum;
    SimRealModeInt(0x10, &regs);
    return (regs.eax != 0x004F) ? 0 : regs.ecx;
}

// VESA 2.0+, get protected mode info (use in protect mode)
int GetProtectModeFunctions()
{
    RM_REGS         regs;
    unsigned short  *ptrIO;
    unsigned int    pmCodeSize;
    unsigned int    pmPhysAddr;

    memset(&regs, 0, sizeof(RM_REGS));
    regs.eax = 0x4F0A;
    regs.ebx = 0;
    SimRealModeInt(0x10, &regs);
    if (regs.eax != 0x004F) return 0;

    // Have protect mode interface defined?
    if (!pmInfo && regs.ecx > 0)
    {
        pmInfo = (VBE_PM_INFO*)malloc(regs.ecx & 0x0000FFFF);
        if (!pmInfo) return 0;

        // Copy protect mode info data
        memcpy(pmInfo, (char*)(regs.es << 4 | regs.edi), regs.ecx & 0x0000FFFF);

        // Need memory mapped IO?
        if (pmInfo->IOPrivInfo)
        {
            // Get IO memory info
            ptrIO = (unsigned short*)((char*)pmInfo + pmInfo->IOPrivInfo);

            // Skip port tables
            while (*ptrIO != 0xFFFF) ptrIO++;

            // Goto selector base
            ptrIO++;

            // Have correct selector base?
            if (*ptrIO != 0xFFFF)
            {
                // Alloc new selector
                if (pmSelector == 0) pmSelector = AllocSelector();
                if (pmSelector == 0 || pmSelector == 0xFFFF) return 0;
                if (!SetSelectorRights(pmSelector, 0x8092)) return 0;

                // Get physical address and size of selector
                pmPhysAddr = *(unsigned int*)ptrIO;
                pmCodeSize = *(ptrIO + 2);

                // Map to linear address
                if (!(pmBasePtr = (unsigned int*)MapPhysicalAddress(pmPhysAddr, pmCodeSize))) return 0;
                if (!SetSelectorBase(pmSelector, (unsigned int)pmBasePtr)) return 0;
                if (!SetSelectorLimit(pmSelector, max(pmCodeSize - 1, 0xFFFF))) return 0;
            }
        }

        // Lookup functions
        fnSetWindow         = (char*)pmInfo + pmInfo->SetWindow;
        fnSetDisplayStart   = (char*)pmInfo + pmInfo->SetDisplayStart;
        fnSetPalette        = (char*)pmInfo + pmInfo->SetPalette;
    }

    return 1;
}

// VESA 2.0+ set palette range (use in protect mode)
void SetPaletteRange(PAL *pal, int from, int to)
{
    int i;
    PAL tmp[256] = {0};
    PAL *palData = tmp;

    if (!fnSetPalette) return;
    if (from < 0) from = 0;
    if (from > 255) from = 255;
    if (to < 0) to = 0;
    if (to > 255) to = 255;
    if (from > to) Swap(from, to);

    // swap the palette into the funny order VESA uses
    for (i = from; i <= to; i++)
    {
        tmp[i].r = pal[i].b;
        tmp[i].g = pal[i].g;
        tmp[i].b = pal[i].r;
    }

    _asm {
        mov   ax, pmSelector
        mov   ds, ax
        mov   ebx, 80h
        mov   ecx, to - from + 1
        mov   edx, from
        mov   edi, palData
        call  fnSetPalette
    }
}

// VESA 3.0 hardware tripple buffering
int ScheduleDisplayStart(unsigned int xpos, unsigned int ypos)
{
    RM_REGS regs;
    unsigned int offset = xpos * bytesPerPixel + ypos * bytesPerScanline;

    memset(&regs, 0, sizeof(regs));
    regs.eax = 0x4F07;
    regs.ebx = 0x02;
    regs.ecx = offset;
    SimRealModeInt(0x10, &regs);
    return (regs.eax == 0x004F);
}

// VESA 3.0 hardware tripple buffering
int GetScheduleDisplayStartStatus()
{
    RM_REGS regs;
    memset(&regs, 0, sizeof(regs));
    regs.eax = 0x4F07;
    regs.ebx = 0x04;
    SimRealModeInt(0x10, &regs);
    return (regs.eax == 0x004F) && (regs.ecx > 0);
}

// VESA 2.0+ set display start address (use in protect mode)
// Hardware scrolling is best for page flipping
int SetDisplayStart(unsigned int xpos, unsigned int ypos)
{
    RM_REGS regs;
    unsigned int val = 0, offset = 0;

    // VESA 2.0, call direct from protect mode interface
    if (fnSetDisplayStart)
    {
        _asm {
            mov   eax, xpos
            mul   bytesPerPixel
            mov   edx, eax
            mov   eax, ypos
            mul   bytesPerScanline
            add   edx, eax
            shr   edx, 2
            mov   ax, pmSelector
            mov   es, ax
            mov   ebx, 80h
            mov   ecx, edx
            and   ecx, 0FFFFh
            shr   edx, 16
            call  fnSetDisplayStart
            mov   val, eax
        }
        return (val == 0x004F);
    }

    // VESA 3.0, call from BIOS
    offset = xpos * bytesPerPixel + ypos * bytesPerScanline;
    memset(&regs, 0, sizeof(regs));
    regs.eax = 0x4F07;
    regs.ebx = 0x80;
    regs.ecx = offset & 0xFFFF;
    regs.edx = offset >> 16;
    regs.es  = pmSelector;
    SimRealModeInt(0x10, &regs);
    return (regs.eax == 0x004F);    
}

// Set drawable page
void SetActivePage(unsigned int page)
{
    if (page > numOfPages - 1) FatalError("SetActivePage: out of visual screen page: %u\n", numOfPages);
    activePage = page;
    pageOffset = page * yres;
}

// Set visible page
void SetVisualPage(unsigned int page)
{
    if (page > numOfPages - 1) FatalError("SetVisualPage: out of visual screen page: %u\n", numOfPages);
    SetDisplayStart(0, page * yres);
}

// Display all VBE info and mode info
void DisplayVesaInfo()
{
    unsigned short  *modePtr;
    VBE_DRIVER_INFO drvInfo;
    VBE_MODE_INFO   modeInfo;

    memset(&drvInfo, 0, sizeof(VBE_DRIVER_INFO));
    GetVesaDriverInfo(&drvInfo);

    printf("VESA DRIVER INFO\n");
    printf("----------------\n");

    printf("     Signature: '%c%c%c%c'\n", drvInfo.VBESignature[0], drvInfo.VBESignature[1], drvInfo.VBESignature[2], drvInfo.VBESignature[3]);
    printf("       Version: %x.%x\n", (drvInfo.VBEVersion >> 8), (drvInfo.VBEVersion & 0xFF));
    printf("    OEM String: '%s'\n", (char*)drvInfo.OEMStringPtr);
    printf("  Capabilities: %0.8X\n", drvInfo.Capabilities);
    printf(" DAC Registers: %s\n", (drvInfo.Capabilities & VBE_CAPS_DAC8) ? "Switchable to 8-bits" : "Fixed at 6-bits");
    printf(" Use Blank 09h: %s\n", (drvInfo.Capabilities & VBE_CAPS_BLANKFN9) ? "Yes" : "No");
    printf("VGA Compatible: %s\n", (drvInfo.Capabilities & VBE_CAPS_NOTVGA) ? "No" : "Yes"); 
    printf("  Video Memory: %uMB\n", (drvInfo.TotalMemory << 6) >> 10);

    // VESA 2.0+, just print OEM info
    if (drvInfo.VBEVersion >= 0x0200)
    {
        printf("  OEM Soft Rev: %0.4X\n", drvInfo.OemSoftwareRev);
        printf("    OEM Vendor: '%s'\n", (char*)drvInfo.OemVendorNamePtr);
        printf("   OEM Product: '%s'\n", (char*)drvInfo.OemProductNamePtr);
        printf("  OEM Revision: '%s'\n", (char*)drvInfo.OemProductRevPtr);
    }

    getch();
    printf("\nVESA MODE INFO\n");

    // Print all VBE modes support
    modePtr = (unsigned short*)drvInfo.VideoModePtr;
    while (modePtr != NULL && *modePtr != 0xFFFF)
    {
        memset(&modeInfo, 0, sizeof(VBE_MODE_INFO));
        if (GetVesaModeInfo(*modePtr, &modeInfo) != 0)
        {
            printf("---------------------------------------------------------\n");
            printf("              Mode Number: %0.4X\n", *modePtr);
            printf("               Attributes: %.4X\n", modeInfo.ModeAttributes);
            printf("       Hardware Supported: %s\n", (modeInfo.ModeAttributes & VBE_MASK_MODEHW) ? "Yes" : "No");
            printf("      Linear Frame Buffer: %s\n", (modeInfo.ModeAttributes & VBE_MASK_HASLFB) ? "Yes" : "No");
            printf("Hardware Triple Buffering: %s\n", (modeInfo.ModeAttributes & VBE_MASK_HW3BUF) ? "Yes" : "No");
            printf("              XResolution: %u\n", modeInfo.XResolution);
            printf("              YResolution: %u\n", modeInfo.YResolution);
            printf("           Bits per Pixel: %u\n", modeInfo.BitsPerPixel);
            printf("    Physical Frame Buffer: %.8X\n", modeInfo.PhysBasePtr);
            printf("             Memory Model: ");

            switch (modeInfo.MemoryModel)
            {
                case VBE_MM_PACKED: printf("Packed Pixel\n"); break;
                case VBE_MM_DCOLOR: printf("Direct Color\n"); break;
                case VBE_MM_YUV: printf("YUV\n"); break;
                default: printf("%.2X\n", modeInfo.MemoryModel);
            }
            
            if ((modeInfo.MemoryModel == VBE_MM_DCOLOR) || (modeInfo.MemoryModel == VBE_MM_YUV))
            {
                if (drvInfo.VBEVersion >= 0x0300)
                {
                    printf("    Number of Image Pages: %u\n", modeInfo.LinNumberOfImagePages);
                    printf("       Bytes per Scanline: %u\n", modeInfo.LinBytesPerScanline);
                    printf("            Red Mask Size: %u\n", modeInfo.LinRedMaskSize);
                    printf("            Red Field pos: %u\n", modeInfo.LinRedFieldPosition);
                    printf("          Green Mask Size: %u\n", modeInfo.LinGreenMaskSize);
                    printf("          Green Field pos: %u\n", modeInfo.LinGreenFieldPosition);
                    printf("           Blue Mask Size: %u\n", modeInfo.LinBlueMaskSize);
                    printf("           Blue Field pos: %u\n", modeInfo.LinBlueFieldPosition);
                    printf("          Max Pixel Clock: %u\n", modeInfo.MaxPixelClock);
                }
                else
                {
                    printf("    Number of Image Pages: %u\n", modeInfo.NumberOfImagePages);
                    printf("       Bytes per Scanline: %u\n", modeInfo.BytesPerScanline);
                    printf("            Red Mask Size: %u\n", modeInfo.RedMaskSize);
                    printf("            Red Field pos: %u\n", modeInfo.RedFieldPosition);
                    printf("          Green Mask Size: %u\n", modeInfo.GreenMaskSize);
                    printf("          Green Field pos: %u\n", modeInfo.GreenFieldPosition);
                    printf("           Blue Mask Size: %u\n", modeInfo.BlueMaskSize);
                    printf("           Blue Field pos: %u\n", modeInfo.BlueFieldPosition);
                }
            }

            getch();
        }

        modePtr++;
    }
}

// Extract rgb value to r,g,b values
void ToRGB(unsigned int col, unsigned char *r, unsigned char *g, unsigned char *b)
{
    *r = (unsigned char)(((col & rmask) >> rpos) << rshift);
    *g = (unsigned char)(((col & gmask) >> gpos) << gshift);
    *b = (unsigned char)(((col & bmask) >> bpos) << bshift);
}

// Merge r,g,b values to rgb value
unsigned int FromRGB(unsigned char r, unsigned char g, unsigned char b)
{
    unsigned int tr, tg, tb;
    tr = (((unsigned int)r >> rshift) << rpos) & rmask;
    tg = (((unsigned int)g >> gshift) << gpos) & gmask;
    tb = (((unsigned int)b >> bshift) << bpos) & bmask;
    return (tr | tg | tb);
}

// Vertical and Horizon retrace
void WaitRetrace()
{
    _asm {
        mov    dx, 03DAh
    waitH:
        in     al, dx
        test   al, 08h
        jz     waitH
    waitV:
        in     al, dx
        test   al, 08h
        jnz    waitV
    }
}

// Setup view port for clipping
void SetViewPort(int x1, int y1, int x2, int y2)
{
    // check correct range
    if (x1 > x2) Swap(x1, x2);
    if (y1 > y2) Swap(y1, y2);

    // update clip point
    cminx = x1;
    cmaxx = x2;
    cminy = y1;
    cmaxy = y2;

    // update center x,y
    centerx = cminx + ((cmaxx - cminx + 1) >> 1);
    centery = cminy + ((cmaxy - cminy + 1) >> 1);
}

// Pixels functions
void PutPixel8(int x, int y, unsigned int col)
{
    if (x < cminx || y < cminy || x > cmaxx || y > cmaxy) return;

    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, x
        mov    edi, lfbPtr
        add    edi, eax
        mov    eax, col
        stosb
    }
}

void PutPixel16(int x, int y, unsigned int col)
{
    if (x < cminx || y < cminy || x > cmaxx || y > cmaxy) return;

    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, x
        add    eax, x
        mov    edi, lfbPtr
        add    edi, eax
        mov    eax, col
        stosw
    }
}

void PutPixel24(int x, int y, unsigned int col)
{
    if (x < cminx || y < cminy || x > cmaxx || y > cmaxy) return;

    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, x
        shl    ebx, 1
        add    ebx, x
        add    eax, ebx
        mov    edi, lfbPtr
        add    edi, eax
        mov    eax, col
        stosw
        shr    eax, 16
        stosb
    } 
}

void PutPixel32(int x, int y, unsigned int col)
{
    if (x < cminx || y < cminy || x > cmaxx || y > cmaxy) return;

    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, x
        shl    ebx, 2
        add    eax, ebx
        mov    edi, lfbPtr
        add    edi, eax
        mov    eax, col
        stosd
    }
}

void PutPixelBob(int x, int y)
{
    _asm {
        mov    eax, y
        mov    ebx, bytesPerScanline
        mul    ebx
        mov    ebx, x
        add    eax, ebx
        mov    esi, lfbPtr
        add    esi, eax
        mov    ebx, eax
        lodsb
        mov    edi, lfbPtr
        add    edi, ebx
        inc    al
        stosb
    }
}

// Get pixel functions
unsigned int GetPixel8(int x, int y)
{
    unsigned char col = 0;

    if (x < cminx || y < cminy || x > cmaxx || y > cmaxy) return 0;

    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, x
        mov    esi, lfbPtr
        add    esi, eax
        lodsb
        mov    col, al
    }

    return col;
}

unsigned int GetPixel16(int x, int y)
{
    unsigned short col = 0;

    if (x < cminx || y < cminy || x > cmaxx || y > cmaxy) return 0;

    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, x
        add    eax, x
        mov    esi, lfbPtr
        add    esi, eax
        lodsw
        mov    col, ax
    }

    return col;
}

unsigned int GetPixel24(int x, int y)
{
    unsigned int col = 0;

    if (x < cminx || y < cminy || x > cmaxx || y > cmaxy) return 0;

    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, x
        shl    ebx, 1
        add    ebx, x
        add    eax, ebx
        mov    esi, lfbPtr
        add    esi, eax
        lodsw
        mov    word ptr [col], ax
        lodsb
        mov    byte ptr [col + 2], al
    }

    return col;
}

unsigned int GetPixel32(int x, int y)
{
    unsigned int col = 0;

    if (x < cminx || y < cminy || x > cmaxx || y > cmaxy) return 0;

    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, x
        shl    ebx, 2
        add    eax, ebx
        mov    esi, lfbPtr
        add    esi, eax
        lodsd
        mov    col, eax
    }

    return col;
}

// Put pixel add with destination
void PutPixelAdd32(int x, int y, unsigned int col)
{
    if (x < cminx || y < cminy || x > cmaxx || y > cmaxy) return;
    
    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, x
        shl    ebx, 2
        add    eax, ebx
        mov    edi, lfbPtr
        add    edi, eax
        mov    eax, [edi]
        add    al, byte ptr [col]
        jnc    bstep
        mov    al, 255
    bstep:
        add    ah, byte ptr [col + 1]
        jnc    gstep
        mov    ah, 255
    gstep:
        mov    ebx, eax
        shr    ebx, 16
        add    bl, byte ptr [col + 2]
        jnc    rstep
        mov    bl, 255
    rstep:
        shl    ebx, 16
        and    eax, 00FFFFh
        or     eax, ebx
        stosd
    }
}

void PutPixelAdd24(int x, int y, unsigned int col)
{
    if (x < cminx || y < cminy || x > cmaxx || y > cmaxy) return;
    
    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, x
        shl    ebx, 1
        add    ebx, x
        add    eax, ebx
        mov    edi, lfbPtr
        add    edi, eax
        mov    ax, [edi]
        add    al, byte ptr [col]
        jnc    bstep
        mov    al, 255
    bstep:
        add    ah, byte ptr [col + 1]
        jnc    gstep
        mov    ah, 255
    gstep:
        mov    bl, [edi + 2]
        add    bl, byte ptr [col + 2]
        jnc    rstep
        mov    bl, 255
    rstep:
        stosw
        mov    al, bl
        stosb
    }
}

// 16 bits pixel use RGB 565
void PutPixelAdd16(int x, int y, unsigned int col)
{
    if (x < cminx || y < cminy || x > cmaxx || y > cmaxy) return;
    
    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, x
        add    eax, x
        mov    edi, lfbPtr
        add    edi, eax
        mov    ax, [edi]
        mov    bx, word ptr [col]
        mov    dx, bx
        and    bx, 1111100000011111b
        and    dx, 11111100000b
        mov    cx, ax
        and    ax, 1111100000011111b
        and    cx, 11111100000b
        add    al, bl
        cmp    al, 11111b
        jna    bstep
        mov    al, 11111b
    bstep:
        add    ah, bh
        jnc    gstep
        mov    ah, 11111000b
    gstep:
        add    cx, dx
        cmp    cx, 11111100000b
        jna    rstep
        mov    cx, 11111100000b
    rstep:
        or     ax, cx
        stosw
    }
}

// 15 bits pixel use RGB 555
void PutPixelAdd15(int x, int y, unsigned int col)
{
    if (x < cminx || y < cminy || x > cmaxx || y > cmaxy) return;
    
    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, x
        add    eax, x
        mov    edi, lfbPtr
        add    edi, eax
        mov    ax, [edi]
        mov    bx, word ptr [col]
        mov    dx, bx
        and    bx, 111110000011111b
        and    dx, 1111100000b
        mov    cx, ax
        and    ax, 111110000011111b
        and    cx, 1111100000b
        add    al, bl
        cmp    al, 11111b
        jna    bstep
        mov    al, 11111b
    bstep:
        add    ah, bh
        cmp    ah, 1111100b
        jna    gstep
        mov    ah, 1111100b
    gstep:
        add    cx, dx
        cmp    cx, 1111100000b
        jna    rstep
        mov    cx, 1111100000b
    rstep:
        or     ax, cx
        stosw
    }
}

// Put pixel sub with destination
void PutPixelSub32(int x, int y, unsigned int col)
{
    if (x < cminx || y < cminy || x > cmaxx || y > cmaxy) return;
    
    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, x
        shl    ebx, 2
        add    eax, ebx
        mov    edi, lfbPtr
        add    edi, eax
        mov    eax, [edi]
        sub    al, byte ptr [col]
        jnc    bstep
        xor    al, al
    bstep:
        sub    ah, byte ptr [col + 1]
        jnc    gstep
        xor    ah, ah
    gstep:
        mov    ebx, eax
        shr    ebx, 16
        sub    bl, byte ptr [col + 2]
        jnc    rstep
        xor    bl, bl
    rstep:
        shl    ebx, 16
        and    eax, 00FFFFh
        or     eax, ebx
        stosd
    }
}

void PutPixelSub24(int x, int y, unsigned int col)
{
    if (x < cminx || y < cminy || x > cmaxx || y > cmaxy) return;
    
    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, x
        shl    ebx, 1
        add    ebx, x
        add    eax, ebx
        mov    edi, lfbPtr
        add    edi, eax
        mov    ax, [edi]
        sub    al, byte ptr [col]
        jnc    bstep
        xor    al, al
    bstep:
        sub    ah, byte ptr [col + 1]
        jnc    gstep
        xor    ah, ah
    gstep:
        mov    bl, [edi + 2]
        sub    bl, byte ptr [col + 2]
        jnc    rstep
        xor    bl, bl
    rstep:
        stosw
        mov    al, bl
        stosb
    }
}

// 16 bits pixel use RGB 565
void PutPixelSub16(int x, int y, unsigned int col)
{
    if (x < cminx || y < cminy || x > cmaxx || y > cmaxy) return;
    
    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, x
        add    eax, x
        mov    edi, lfbPtr
        add    edi, eax
        mov    ax, [edi]
        mov    bx, word ptr [col]
        mov    dx, bx
        and    bx, 1111100000011111b
        and    dx, 11111100000b
        mov    cx, ax
        and    ax, 1111100000011111b
        and    cx, 11111100000b
        sub    al, bl
        jnc    bstep
        xor    al, al
    bstep:
        sub    ah, bh
        jnc    gstep
        xor    ah, ah
    gstep:
        sub    cx, dx
        jc     rstep
        or     ax, cx
    rstep:
        stosw
    }
}

// 15 bits pixel use RGB 555
void PutPixelSub15(int x, int y, unsigned int col)
{
    if (x < cminx || y < cminy || x > cmaxx || y > cmaxy) return;
    
    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, x
        add    eax, x
        mov    edi, lfbPtr
        add    edi, eax
        mov    ax, [edi]
        mov    bx, word ptr [col]
        mov    dx, bx
        and    bx, 111110000011111b
        and    dx, 1111100000b
        mov    cx, ax
        and    ax, 111110000011111b
        and    cx, 1111100000b
        sub    al, bl
        jnc    bstep
        xor    al, al
    bstep:
        sub    ah, bh
        jnc    gstep
        xor    ah, ah
    gstep:
        sub    cx, dx
        jc     rstep
        or     ax, cx
    rstep:
        stosw
    }
}

// Clear screen with color functions
void ClearScreen8(unsigned int col)
{
    _asm {
        mov    eax, pageOffset
        mul    bytesPerScanline
        mov    edi, lfbPtr
        add    edi, eax
        mov    ecx, lfbSize
        shr    ecx, 2
        mov    ebx, lfbSize
        and    ebx, 3
        mov    eax, col
        shl    eax, 8
        or     eax, col
        shl    eax, 8
        or     eax, col
        shl    eax, 8
        or     eax, col
        rep    stosd
        mov    ecx, ebx
        rep    stosb
    }
}

void ClearScreen16(unsigned int col)
{
    _asm {
        mov    eax, pageOffset
        mul    bytesPerScanline
        mov    edi, lfbPtr
        add    edi, eax
        mov    ecx, lfbSize
        shr    ecx, 1
        mov    ebx, lfbSize
        and    ebx, 1
        mov    eax, col
        shl    eax, 16
        or     eax, col
        rep    stosd
        mov    ecx, ebx
        rep    stosw
    }
}

void ClearScreen24(unsigned int col)
{
    _asm {
        mov    eax, pageOffset
        mul    bytesPerScanline 
        mov    edi, lfbPtr
        add    edi, eax
        mov    ecx, lfbSize
    next:
        mov    eax, col
        stosw
        shr    eax, 16
        stosb
        loop   next
    }
}

void ClearScreen32(unsigned int col)
{
    _asm {
        mov    eax, pageOffset
        mul    bytesPerScanline
        mov    edi, lfbPtr
        add    edi, eax
        mov    ecx, lfbSize
        shr    ecx, 2
        mov    eax, col
        rep    stosd
    }
}

// Fill box with color functions
void FillRect8(int x1, int y1, int x2, int y2, unsigned int col)
{
    int lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, lfbMinX
        add    edi, eax
        mov    edx, bytesPerScanline
        sub    edx, lfbWidth
        push   edx
        mov    ebx, lfbWidth
        shr    ebx, 2
        mov    edx, lfbWidth
        and    edx, 3
        mov    eax, col
        shl    eax, 8
        or     eax, col
        shl    eax, 8
        or     eax, col
        shl    eax, 8
        or     eax, col
    next:
        mov    ecx, ebx
        rep    stosd
        mov    ecx, edx
        rep    stosb
        add    edi, [esp]
        dec    lfbHeight
        jnz    next
        pop    edx
    }
}

void FillRect16(int x1, int y1, int x2, int y2, unsigned int col)
{
    int lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, lfbMinX
        add    eax, lfbMinX
        add    edi, eax
        mov    edx, bytesPerScanline
        sub    edx, lfbWidth
        sub    edx, lfbWidth
        push   edx
        mov    ebx, lfbWidth
        shr    ebx, 1
        mov    edx, lfbWidth
        and    edx, 1
        mov    eax, col
        shl    eax, 16
        or     eax, col
    next:
        mov    ecx, ebx
        rep    stosd
        mov    ecx, edx
        rep    stosw
        add    edi, [esp]
        dec    lfbHeight
        jnz    next
        pop    edx
    }
}

void FillRect24(int x1, int y1, int x2, int y2, unsigned int col)
{
    int lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, lfbMinX
        shl    ebx, 1
        add    ebx, lfbMinX
        add    eax, ebx
        add    edi, eax
        mov    ebx, bytesPerScanline
        mov    ecx, lfbWidth
        shl    ecx, 1
        add    ecx, lfbWidth
        sub    ebx, ecx
    next:
        mov    ecx, lfbWidth
    plot:
        mov    eax, col
        stosw
        shr    eax, 16
        stosb
        loop   plot
        add    edi, ebx
        dec    lfbHeight
        jnz    next
    }
}

void FillRect32(int x1, int y1, int x2, int y2, unsigned int col)
{
    int lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, lfbMinX
        shl    ebx, 2
        add    eax, ebx
        add    edi, eax
        mov    ebx, bytesPerScanline
        mov    ecx, lfbWidth
        shl    ecx, 2
        sub    ebx, ecx
        mov    eax, col
    next:
        mov    ecx, lfbWidth
        rep    stosd
        add    edi, ebx
        dec    lfbHeight
        jnz    next
    }
}

// Fill rectangle with adding current pixel color
void FillRectAdd32(int x1, int y1, int x2, int y2, unsigned int col)
{
    int lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, lfbMinX
        shl    ebx, 2
        add    eax, ebx
        add    edi, eax
        mov    edx, bytesPerScanline
        mov    ecx, lfbWidth
        shl    ecx, 2
        sub    edx, ecx
    next:
        mov    ecx, lfbWidth
    plot:
        mov    eax, [edi]
        add    al, byte ptr[col]
        jnc    bstep
        mov    al, 255
    bstep:
        add    ah, byte ptr[col + 1]
        jnc    gstep
        mov    ah, 255
    gstep:
        mov    ebx, eax
        shr    ebx, 16
        add    bl, byte ptr[col + 2]
        jnc    rstep
        mov    bl, 255
    rstep:
        shl    ebx, 16
        and    eax, 00FFFFh
        or     eax, ebx
        stosd
        loop   plot
        add    edi, edx
        dec    lfbHeight
        jnz    next
    }
}

void FillRectAdd24(int x1, int y1, int x2, int y2, unsigned int col)
{
    int lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, lfbMinX
        shl    ebx, 1
        add    eax, ebx
        add    eax, lfbMinX
        add    edi, eax
        mov    edx, bytesPerScanline
        mov    ecx, lfbWidth
        shl    ecx, 1
        sub    edx, ecx
        sub    edx, lfbWidth
    next:
        mov    ecx, lfbWidth
    plot:
        mov    ax, [edi]
        add    al, byte ptr[col]
        jnc    bstep
        mov    al, 255
    bstep:
        add    ah, byte ptr[col + 1]
        jnc    gstep
        mov    ah, 255
    gstep:
        mov    bl, [edi + 2]
        add    bl, byte ptr[col + 2]
        jnc    rstep
        mov    bl, 255
    rstep:
        stosw
        mov    al, bl
        stosb
        loop   plot
        add    edi, edx
        dec    lfbHeight
        jnz    next
    }
}

void FillRectAdd16(int x1, int y1, int x2, int y2, unsigned int col)
{
    int lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    unsigned int skip = bytesPerScanline;
    
    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, lfbMinX
        add    eax, lfbMinX
        add    edi, eax
        mov    ecx, lfbWidth
        shl    ecx, 1
        sub    skip, ecx
        mov    bx, word ptr [col]
        mov    dx, bx
        and    bx, 1111100000011111b
        and    dx, 11111100000b
    next:
        push   lfbWidth
    plot:
        mov    ax, [edi]
        mov    cx, ax
        and    ax, 1111100000011111b
        and    cx, 11111100000b
        add    al, bl
        cmp    al, 11111b
        jna    bstep
        mov    al, 11111b
    bstep:
        add    ah, bh
        jnc    gstep
        mov    ah, 11111000b
    gstep:
        add    cx, dx
        cmp    cx, 11111100000b
        jna    rstep
        mov    cx, 11111100000b
    rstep:
        or     ax, cx
        stosw
        dec    lfbWidth
        jnz    plot
        add    edi, skip
        pop    lfbWidth
        dec    lfbHeight
        jnz    next
    }
}

void FillRectAdd15(int x1, int y1, int x2, int y2, unsigned int col)
{
    int lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    unsigned int skip = bytesPerScanline;
    
    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, lfbMinX
        add    eax, lfbMinX
        add    edi, eax
        mov    ecx, lfbWidth
        shl    ecx, 1
        sub    skip, ecx
        mov    bx, word ptr [col]
        mov    dx, bx
        and    bx, 111110000011111b
        and    dx, 1111100000b
    next:
        push   lfbWidth
    plot:
        mov    ax, [edi]
        mov    cx, ax
        and    ax, 111110000011111b
        and    cx, 1111100000b
        add    al, bl
        cmp    al, 11111b
        jna    bstep
        mov    al, 11111b
    bstep:
        add    ah, bh
        cmp    ah, 1111100b
        jna    gstep
        mov    ah, 1111100b
    gstep:
        add    cx, dx
        cmp    cx, 1111100000b
        jna    rstep
        mov    cx, 1111100000b
    rstep:
        or     ax, cx
        stosw
        dec    lfbWidth
        jnz    plot
        add    edi, skip
        pop    lfbWidth
        dec    lfbHeight
        jnz    next
    }
}

// Fill rectangle with substraction current pixel color
void FillRectSub32(int x1, int y1, int x2, int y2, unsigned int col)
{
    int lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, lfbMinX
        shl    ebx, 2
        add    eax, ebx
        add    edi, eax
        mov    edx, bytesPerScanline
        mov    ecx, lfbWidth
        shl    ecx, 2
        sub    edx, ecx
    next:
        mov    ecx, lfbWidth
    plot:
        mov    eax, [edi]
        sub    al, byte ptr[col]
        jnc    bstep
        xor    al, al
    bstep:
        sub    ah, byte ptr[col + 1]
        jnc    gstep
        xor    ah, ah
    gstep:
        mov    ebx, eax
        shr    ebx, 16
        sub    bl, byte ptr[col + 2]
        jnc    rstep
        xor    bl, bl
    rstep:
        shl    ebx, 16
        and    eax, 00FFFFh
        or     eax, ebx
        stosd
        loop   plot
        add    edi, edx
        dec    lfbHeight
        jnz    next
    }
}

void FillRectSub24(int x1, int y1, int x2, int y2, unsigned int col)
{
    int lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, lfbMinX
        shl    ebx, 1
        add    eax, ebx
        add    eax, lfbMinX
        add    edi, eax
        mov    edx, bytesPerScanline
        mov    ecx, lfbWidth
        shl    ecx, 1
        sub    edx, ecx
        sub    edx, lfbWidth
    next:
        mov    ecx, lfbWidth
    plot:
        mov    ax, [edi]
        sub    al, byte ptr[col]
        jnc    bstep
        xor    al, al
    bstep:
        sub    ah, byte ptr[col + 1]
        jnc    gstep
        xor    ah, ah
    gstep:
        mov    bl, [edi + 2]
        sub    bl, byte ptr[col + 2]
        jnc    rstep
        xor    bl, bl
    rstep:
        stosw
        mov    al, bl
        stosb
        loop   plot
        add    edi, edx
        dec    lfbHeight
        jnz    next
    }
}

void FillRectSub16(int x1, int y1, int x2, int y2, unsigned int col)
{
    int lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    unsigned int skip = bytesPerScanline;
    
    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, lfbMinX
        add    eax, lfbMinX
        add    edi, eax
        mov    ecx, lfbWidth
        shl    ecx, 1
        sub    skip, ecx
        mov    bx, word ptr [col]
        mov    dx, bx
        and    bx, 1111100000011111b
        and    dx, 11111100000b
    next:
        push   lfbWidth
    plot:
        mov    ax, [edi]
        mov    cx, ax
        and    ax, 1111100000011111b
        and    cx, 11111100000b
        sub    al, bl
        jnc    bstep
        xor    al, al
    bstep:
        sub    ah, bh
        jnc    gstep
        xor    ah, ah
    gstep:
        sub    cx, dx
        jc     rstep
        or     ax, cx
    rstep:
        stosw
        dec    lfbWidth
        jnz    plot
        add    edi, skip
        pop    lfbWidth
        dec    lfbHeight
        jnz    next
    }
}

void FillRectSub15(int x1, int y1, int x2, int y2, unsigned int col)
{
    int lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    unsigned int skip = bytesPerScanline;
    
    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, lfbMinX
        add    eax, lfbMinX
        add    edi, eax
        mov    ecx, lfbWidth
        shl    ecx, 1
        sub    skip, ecx
        mov    bx, word ptr [col]
        mov    dx, bx
        and    bx, 111110000011111b
        and    dx, 1111100000b
    next:
        push   lfbWidth
    plot:
        mov    ax, [edi]
        mov    cx, ax
        and    ax, 111110000011111b
        and    cx, 1111100000b
        sub    al, bl
        jnc    bstep
        xor    al, al
    bstep:
        sub    ah, bh
        jnc    gstep
        xor    ah, ah
    gstep:
        sub    cx, dx
        jc     rstep
        or     ax, cx
    rstep:
        stosw
        dec    lfbWidth
        jnz    plot
        add    edi, skip
        pop    lfbWidth
        dec    lfbHeight
        jnz    next
    }
}

void FillRectPattern32(int x1, int y1, int x2, int y2, unsigned int col, unsigned char *pattern)
{
    uint32_t* dstPixels;
    uint32_t addDstOffs;
    uint8_t al, cl;
    int32_t y, x;
    int lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, lfbMinX
        shl    ebx, 2
        add    eax, ebx
        add    edi, eax
        mov    edx, bytesPerScanline
        mov    ecx, lfbWidth
        shl    ecx, 2
        sub    edx, ecx
        mov    esi, pattern
    plot:
        mov    ecx, lfbMinX
        and    ecx, 7
        mov    ebx, lfbHeight
        and    ebx, 7
        mov    al, [esi + ebx]
        rol    al, cl
        mov    ebx, col
        mov    ecx, lfbWidth
    next:
        test   al, 1
        jz     step
        mov    [edi], ebx
    step:
        add    edi, 4
        rol    al, 1
        loop   next
        add    edi, edx
        dec    lfbHeight
        jnz    plot
    }
}

void FillRectPattern24(int x1, int y1, int x2, int y2, unsigned int col, unsigned char *pattern)
{
    int lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, lfbMinX
        shl    ebx, 1
        add    eax, ebx
        add    eax, lfbMinX
        add    edi, eax
        mov    edx, bytesPerScanline
        mov    ecx, lfbWidth
        shl    ecx, 1
        sub    edx, ecx
        sub    edx, lfbWidth
        mov    esi, pattern
    plot:
        mov    ecx, lfbMinX
        and    ecx, 7
        mov    ebx, lfbHeight
        and    ebx, 7
        mov    al, [esi + ebx]
        rol    al, cl
        mov    ecx, lfbWidth
    next:
        test   al, 1
        jz     step
        mov    ebx, col
        mov    [edi], bx
        shr    ebx, 16
        mov    [edi + 2], bl
    step:
        add    edi, 3
        rol    al, 1
        loop   next
        add    edi, edx
        dec    lfbHeight
        jnz    plot
    }
}

void FillRectPattern16(int x1, int y1, int x2, int y2, unsigned int col, unsigned char *pattern)
{
    int lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, lfbMinX
        add    eax, lfbMinX
        add    edi, eax
        mov    edx, bytesPerScanline
        sub    edx, lfbWidth
        sub    edx, lfbWidth
        mov    esi, pattern
    plot:
        mov    ecx, lfbMinX
        and    ecx, 7
        mov    ebx, lfbHeight
        and    ebx, 7
        mov    al, [esi + ebx]
        rol    al, cl
        mov    ebx, col
        mov    ecx, lfbWidth
    next:
        test   al, 1
        jz     step
        mov    [edi], bx
    step:
        add    edi, 2
        rol    al, 1
        loop   next
        add    edi, edx
        dec    lfbHeight
        jnz    plot
    }
}

void FillRectPattern8(int x1, int y1, int x2, int y2, unsigned int col, unsigned char *pattern)
{
    int lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, lfbMinX
        add    edi, eax
        mov    edx, bytesPerScanline
        sub    edx, lfbWidth
        mov    esi, pattern
    plot:
        mov    ecx, lfbMinX
        and    ecx, 7
        mov    ebx, lfbHeight
        and    ebx, 7
        mov    al, [esi + ebx]
        rol    al, cl
        mov    ebx, col
        mov    ecx, lfbWidth
    next:
        test   al, 1
        jz     step
        mov    [edi], bl
    step:
        inc    edi
        rol    al, 1
        loop   next
        add    edi, edx
        dec    lfbHeight
        jnz    plot
    }
}

void FillRectPatternAdd32(int x1, int y1, int x2, int y2, unsigned int col, unsigned char *pattern)
{
    int lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, lfbMinX
        shl    ebx, 2
        add    eax, ebx
        add    edi, eax
        mov    edx, bytesPerScanline
        mov    ecx, lfbWidth
        shl    ecx, 2
        sub    edx, ecx
        mov    esi, pattern
    plot:
        push   edx
        mov    ecx, lfbMinX
        and    ecx, 7
        mov    ebx, lfbHeight
        and    ebx, 7
        mov    al, [esi + ebx]
        rol    al, cl
        mov    ecx, lfbWidth
    next:
        test   al, 1
        jz     step
        mov    ebx, [edi]
        add    bl, byte ptr [col]
        jnc    bstep
        mov    bl, 255
    bstep:
        add    bh, byte ptr [col + 1]
        jnc    gstep
        mov    bh, 255
    gstep:
        mov    edx, ebx
        shr    edx, 16
        add    dl, byte ptr [col + 2]
        jnc    rstep
        mov    dl, 255
    rstep:
        shl    edx, 16
        and    ebx, 00FFFFh
        or     ebx, edx
        mov    [edi], ebx
    step:
        add    edi, 4
        rol    al, 1
        loop   next
        pop    edx
        add    edi, edx
        dec    lfbHeight
        jnz    plot
    }
}

void FillRectPatternAdd24(int x1, int y1, int x2, int y2, unsigned int col, unsigned char *pattern)
{
    int lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, lfbMinX
        shl    ebx, 1
        add    eax, ebx
        add    eax, lfbMinX
        add    edi, eax
        mov    edx, bytesPerScanline
        mov    ecx, lfbWidth
        shl    ecx, 1
        sub    edx, ecx
        sub    edx, lfbWidth
        mov    esi, pattern
    plot:
        push   edx
        mov    ecx, lfbMinX
        and    ecx, 7
        mov    ebx, lfbHeight
        and    ebx, 7
        mov    al, [esi + ebx]
        rol    al, cl
        mov    ecx, lfbWidth
    next:
        test   al, 1
        jz     step
        mov    bx, [edi]
        add    bl, byte ptr [col]
        jnc    bstep
        mov    bl, 255
    bstep:
        add    bh, byte ptr [col + 1]
        jnc    gstep
        mov    bh, 255
    gstep:
        mov    dl, [edi + 2]
        add    dl, byte ptr [col + 2]
        jnc    rstep
        mov    dl, 255
    rstep:
        mov    [edi], bx
        mov    [edi + 2], dl
    step:
        add    edi, 3
        rol    al, 1
        loop   next
        pop    edx
        add    edi, edx
        dec    lfbHeight
        jnz    plot
    }
}

void FillRectPatternAdd16(int x1, int y1, int x2, int y2, unsigned int col, unsigned char *pattern)
{
    int lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    unsigned int skip = bytesPerScanline;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, lfbMinX
        add    eax, lfbMinX
        add    edi, eax
        mov    ecx, lfbWidth
        shl    ecx, 1
        sub    skip, ecx
        mov    esi, pattern
        mov    bx, word ptr [col]
        mov    dx, bx
        and    bx, 1111100000011111b
        and    dx, 11111100000b
    plot:
        mov    ecx, lfbHeight
        and    ecx, 7
        mov    al, [esi + ecx]
        mov    ecx, lfbMinX
        and    ecx, 7
        rol    al, cl
        push   lfbWidth
    next:
        test   al, 1
        jz     step
        push   eax
        mov    ax, [edi]
        mov    cx, ax
        and    ax, 1111100000011111b
        and    cx, 11111100000b
        add    al, bl
        cmp    al, 11111b
        jna    bstep
        mov    al, 11111b
    bstep:
        add    ah, bh
        jnc    gstep
        mov    ah, 11111000b
    gstep:
        add    cx, dx
        cmp    cx, 11111100000b
        jna    rstep
        mov    cx, 11111100000b
    rstep:
        or     ax, cx
        mov    [edi], ax
        pop    eax
    step:
        add    edi, 2
        rol    al, 1
        dec    lfbWidth
        jnz    next
        add    edi, skip
        pop    lfbWidth
        dec    lfbHeight
        jnz    plot
    }
}

void FillRectPatternAdd15(int x1, int y1, int x2, int y2, unsigned int col, unsigned char *pattern)
{
    int lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    unsigned int skip = bytesPerScanline;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, lfbMinX
        add    eax, lfbMinX
        add    edi, eax
        mov    ecx, lfbWidth
        shl    ecx, 1
        sub    skip, ecx
        mov    esi, pattern
        mov    bx, word ptr [col]
        mov    dx, bx
        and    bx, 111110000011111b
        and    dx, 1111100000b
    plot:
        mov    ecx, lfbHeight
        and    ecx, 7
        mov    al, [esi + ecx]
        mov    ecx, lfbMinX
        and    ecx, 7
        rol    al, cl
        push   lfbWidth
    next:
        test   al, 1
        jz     step
        push   eax
        mov    ax, [edi]
        mov    cx, ax
        and    ax, 111110000011111b
        and    cx, 1111100000b
        add    al, bl
        cmp    al, 11111b
        jna    bstep
        mov    al, 11111b
    bstep:
        add    ah, bh
        cmp    ah, 1111100b
        jna    gstep
        mov    ah, 1111100b
    gstep:
        add    cx, dx
        cmp    cx, 1111100000b
        jna    rstep
        mov    cx, 1111100000b
    rstep:
        or     ax, cx
        mov    [edi], ax
        pop    eax
    step:
        add    edi, 2
        rol    al, 1
        dec    lfbWidth
        jnz    next
        add    edi, skip
        pop    lfbWidth
        dec    lfbHeight
        jnz    plot
    }
}

void FillRectPatternSub32(int x1, int y1, int x2, int y2, unsigned int col, unsigned char *pattern)
{
    int lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, lfbMinX
        shl    ebx, 2
        add    eax, ebx
        add    edi, eax
        mov    edx, bytesPerScanline
        mov    ecx, lfbWidth
        shl    ecx, 2
        sub    edx, ecx
        mov    esi, pattern
    plot:
        push   edx
        mov    ecx, lfbMinX
        and    ecx, 7
        mov    ebx, lfbHeight
        and    ebx, 7
        mov    al, [esi + ebx]
        rol    al, cl
        mov    ecx, lfbWidth
    next:
        test   al, 1
        jz     step
        mov    ebx, [edi]
        sub    bl, byte ptr [col]
        jnc    bstep
        xor    bl, bl
    bstep:
        sub    bh, byte ptr [col + 1]
        jnc    gstep
        xor    bh, bh
    gstep:
        mov    edx, ebx
        shr    edx, 16
        sub    dl, byte ptr [col + 2]
        jnc    rstep
        xor    dl, dl
    rstep:
        shl    edx, 16
        and    ebx, 00FFFFh
        or     ebx, edx
        mov    [edi], ebx
    step:
        add    edi, 4
        rol    al, 1
        loop   next
        pop    edx
        add    edi, edx
        dec    lfbHeight
        jnz    plot
    }
}

void FillRectPatternSub24(int x1, int y1, int x2, int y2, unsigned int col, unsigned char *pattern)
{
    int lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, lfbMinX
        shl    ebx, 1
        add    eax, ebx
        add    eax, lfbMinX
        add    edi, eax
        mov    edx, bytesPerScanline
        mov    ecx, lfbWidth
        shl    ecx, 1
        sub    edx, ecx
        sub    edx, lfbWidth
        mov    esi, pattern
    plot:
        push   edx
        mov    ecx, lfbMinX
        and    ecx, 7
        mov    ebx, lfbHeight
        and    ebx, 7
        mov    al, [esi + ebx]
        rol    al, cl
        mov    ecx, lfbWidth
    next:
        test   al, 1
        jz     step
        mov    bx, [edi]
        sub    bl, byte ptr [col]
        jnc    bstep
        xor    bl, bl
    bstep:
        sub    bh, byte ptr [col + 1]
        jnc    gstep
        xor    bh, bh
    gstep:
        mov    dl, [edi + 2]
        sub    dl, byte ptr [col + 2]
        jnc    rstep
        xor    dl, dl
    rstep:
        mov    [edi], bx
        mov    [edi + 2], dl
    step:
        add    edi, 3
        rol    al, 1
        loop   next
        pop    edx
        add    edi, edx
        dec    lfbHeight
        jnz    plot
    }
}

void FillRectPatternSub16(int x1, int y1, int x2, int y2, unsigned int col, unsigned char *pattern)
{
    int lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    unsigned int skip = bytesPerScanline;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, lfbMinX
        add    eax, lfbMinX
        add    edi, eax
        mov    ecx, lfbWidth
        shl    ecx, 1
        sub    skip, ecx
        mov    esi, pattern
        mov    bx, word ptr [col]
        mov    dx, bx
        and    bx, 1111100000011111b
        and    dx, 11111100000b
    plot:
        mov    ecx, lfbHeight
        and    ecx, 7
        mov    al, [esi + ecx]
        mov    ecx, lfbMinX
        and    ecx, 7
        rol    al, cl
        push   lfbWidth
    next:
        test   al, 1
        jz     step
        push   eax
        mov    ax, [edi]
        mov    cx, ax
        and    ax, 1111100000011111b
        and    cx, 11111100000b
        sub    al, bl
        jnc    bstep
        xor    al, al
    bstep:
        sub    ah, bh
        jnc    gstep
        xor    ah, ah
    gstep:
        sub    cx, dx
        jc     rstep
        or     ax, cx
    rstep:
        mov    [edi], ax
        pop    eax
    step:
        add    edi, 2
        rol    al, 1
        dec    lfbWidth
        jnz    next
        add    edi, skip
        pop    lfbWidth
        dec    lfbHeight
        jnz    plot
    }
}

void FillRectPatternSub15(int x1, int y1, int x2, int y2, unsigned int col, unsigned char *pattern)
{
    int lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    unsigned int skip = bytesPerScanline;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, lfbMinX
        add    eax, lfbMinX
        add    edi, eax
        mov    ecx, lfbWidth
        shl    ecx, 1
        sub    skip, ecx
        mov    esi, pattern
        mov    bx, word ptr [col]
        mov    dx, bx
        and    bx, 111110000011111b
        and    dx, 1111100000b
    plot:
        mov    ecx, lfbHeight
        and    ecx, 7
        mov    al, [esi + ecx]
        mov    ecx, lfbMinX
        and    ecx, 7
        rol    al, cl
        push   lfbWidth
    next:
        test   al, 1
        jz     step
        push   eax
        mov    ax, [edi]
        mov    cx, ax
        and    ax, 111110000011111b
        and    cx, 1111100000b
        sub    al, bl
        jnc    bstep
        xor    al, al
    bstep:
        sub    ah, bh
        jnc    gstep
        xor    ah, ah
    gstep:
        sub    cx, dx
        jc     rstep
        or     ax, cx
    rstep:
        mov    [edi], ax
        pop    eax
    step:
        add    edi, 2
        rol    al, 1
        dec    lfbWidth
        jnz    next
        add    edi, skip
        pop    lfbWidth
        dec    lfbHeight
        jnz    plot
    }
}

// Alloc memory to store image data
int OpenImage(int width, int height, IMAGE *img)
{
    // Calcule buffer size, add to width and height
    unsigned int size = height * width * bytesPerPixel;
    if (!size) return 0;

    img->mData = (unsigned char*)malloc(size);
    if (!img->mData) return 0;

    // store image width and height
    memset(img->mData, 0, size);
    img->mWidth    = width;
    img->mHeight   = height;
    img->mPixels   = bytesPerPixel;
    img->mSize     = size;
    img->mRowBytes = width * bytesPerPixel;

    return 1;
}

// Cleanup image buffer
void CloseImage(IMAGE *img)
{
    if (img && img->mData)
    {
        free(img->mData);
        img->mData     = NULL;
        img->mWidth    = 0;
        img->mHeight   = 0;
        img->mPixels   = 0;
        img->mSize     = 0;
        img->mRowBytes = 0;
    }
}

// Clear image data buffer
void ClearImage(IMAGE *img)
{
    void *data = img->mData;
    unsigned int size = img->mSize;

    _asm {
        mov    edi, data
        xor    eax, eax
        mov    ecx, size
        shr    ecx, 2
        rep    stosd
        mov    ecx, size
        and    ecx, 3
        rep    stosb
    }
}

// Copy full current page to another page (non-clipping)
void CopyPage(int from, int to)
{
    IMAGE img;
    int oldPage = activePage;
    if (!OpenImage(xres, yres, &img)) FatalError("CopyPage: cannot open image.\n");
    SetActivePage(from);
    GetImage(0, 0, xres, yres, &img);
    SetActivePage(to);
    PutImage(0, 0, &img);
    SetActivePage(oldPage);
    CloseImage(&img);
}

// Get image buffer functions
void GetImage8(int x1, int y1, int width, int height, IMAGE *img)
{
    void *imgData = img->mData;
    int x2, y2, lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;

    // Calculate new position
    x2 = (x1 + width) - 1;
    y2 = (y1 + height) - 1;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    // store real image contents
    img->mWidth    = lfbWidth;
    img->mHeight   = lfbHeight;
    img->mPixels   = bytesPerPixel;
    img->mRowBytes = lfbWidth * bytesPerPixel;
    img->mSize     = lfbHeight * img->mRowBytes;
    
    _asm {
        mov    edi, imgData
        mov    esi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, lfbMinX
        add    esi, eax
        mov    edx, bytesPerScanline
        sub    edx, lfbWidth
        mov    eax, lfbWidth
        shr    eax, 2
        mov    ebx, lfbWidth
        and    ebx, 3
    next:
        mov    ecx, eax
        rep    movsd
        mov    ecx, ebx
        rep    movsb
        add    esi, edx
        dec    lfbHeight
        jnz    next
    }
}

void GetImage16(int x1, int y1, int width, int height, IMAGE *img)
{
    void *imgData = img->mData;
    int x2, y2, lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;

    // Calculate new position
    x2 = (x1 + width) - 1;
    y2 = (y1 + height) - 1;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    // store real image contents
    img->mWidth    = lfbWidth;
    img->mHeight   = lfbHeight;
    img->mPixels   = bytesPerPixel;
    img->mRowBytes = lfbWidth * bytesPerPixel;
    img->mSize     = lfbHeight * img->mRowBytes;

    _asm {
        mov    edi, imgData
        mov    esi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, lfbMinX
        add    eax, lfbMinX
        add    esi, eax
        mov    edx, bytesPerScanline
        sub    edx, lfbWidth
        sub    edx, lfbWidth
        mov    eax, lfbWidth
        shr    eax, 1
        mov    ebx, lfbWidth
        and    ebx, 1
    next:
        mov    ecx, eax
        rep    movsd
        mov    ecx, ebx
        rep    movsw
        add    esi, edx
        dec    lfbHeight
        jnz    next
    }
}

void GetImage24(int x1, int y1, int width, int height, IMAGE *img)
{
    void *imgData = img->mData;
    int x2, y2, lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;

    // Calculate new position
    x2 = (x1 + width) - 1;
    y2 = (y1 + height) - 1;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    // store real image contents
    img->mWidth    = lfbWidth;
    img->mHeight   = lfbHeight;
    img->mPixels   = bytesPerPixel;
    img->mRowBytes = lfbWidth * bytesPerPixel;
    img->mSize     = lfbHeight * img->mRowBytes;

    _asm {
        mov    edi, imgData
        mov    esi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, lfbMinX
        shl    ebx, 1
        add    ebx, lfbMinX
        add    eax, ebx
        add    esi, eax
        mov    edx, bytesPerScanline
        mov    ebx, lfbWidth
        shl    ebx, 1
        add    ebx, lfbWidth
        sub    edx, ebx
        mov    ecx, lfbWidth
        lea    ecx, [ecx + ecx * 2]
        mov    eax, ecx
        shr    eax, 2
        and    ecx, 3
        mov    ebx, ecx
    next:
        mov    ecx, eax
        rep    movsd
        mov    ecx, ebx
        rep    movsb
        add    esi, edx
        dec    lfbHeight
        jnz    next
    }
}

void GetImage32(int x1, int y1, int width, int height, IMAGE *img)
{
    void *imgData = img->mData;
    int x2, y2, lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;

    // Calculate new position
    x2 = (x1 + width) - 1;
    y2 = (y1 + height) - 1;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    // store real image contents
    img->mWidth    = lfbWidth;
    img->mHeight   = lfbHeight;
    img->mPixels   = bytesPerPixel;
    img->mRowBytes = lfbWidth * bytesPerPixel;
    img->mSize     = lfbHeight * img->mRowBytes;

    _asm {
        mov    edi, imgData
        mov    esi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, lfbMinX
        shl    ebx, 2
        add    eax, ebx
        add    esi, eax
        mov    edx, bytesPerScanline
        mov    ebx, lfbWidth
        shl    ebx, 2
        sub    edx, ebx
    next:
        mov    ecx, lfbWidth
        rep    movsd
        add    esi, edx
        dec    lfbHeight
        jnz    next
    }
}

// Put image data to screen functions
void PutImage8(int x1, int y1, IMAGE *img)
{
    int x2, y2, lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    int imgWidth  = img->mWidth;
    int imgHeight = img->mHeight;
    void *imgData = img->mData;
    unsigned int imgRowBytes = img->mRowBytes;

    // Calculate new position
    x2 = (x1 + imgWidth) - 1;
    y2 = (y1 + imgHeight) - 1;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, lfbMinX
        add    edi, eax
        mov    esi, imgData
        mov    eax, lfbMinY
        sub    eax, y1
        mul    imgRowBytes
        mov    ebx, lfbMinX
        sub    ebx, x1
        add    eax, ebx
        add    esi, eax
        mov    ebx, bytesPerScanline
        sub    ebx, lfbWidth
        push   ebx
        mov    edx, imgWidth
        sub    edx, lfbWidth
        mov    eax, lfbWidth
        shr    eax, 2
        mov    ebx, lfbWidth
        and    ebx, 3
    next:
        mov    ecx, eax
        rep    movsd
        mov    ecx, ebx
        rep    movsb
        add    edi, [esp]
        add    esi, edx
        dec    lfbHeight
        jnz    next
        pop    ebx
    }
}

void PutImage16(int x1, int y1, IMAGE *img)
{
    int x2, y2, lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    int imgWidth  = img->mWidth;
    int imgHeight = img->mHeight;
    void *imgData = img->mData;
    unsigned int imgRowBytes = img->mRowBytes;

    // Calculate new position
    x2 = (x1 + imgWidth) - 1;
    y2 = (y1 + imgHeight) - 1;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, lfbMinX
        add    eax, lfbMinX
        add    edi, eax
        mov    esi, imgData
        mov    eax, lfbMinY
        sub    eax, y1
        mul    imgRowBytes
        mov    ebx, lfbMinX
        sub    ebx, x1
        shl    ebx, 1
        add    eax, ebx
        add    esi, eax
        mov    ebx, bytesPerScanline
        sub    ebx, lfbWidth
        sub    ebx, lfbWidth
        push   ebx
        mov    edx, imgWidth
        sub    edx, lfbWidth
        shl    edx, 1
        mov    eax, lfbWidth
        shr    eax, 1
        mov    ebx, lfbWidth
        and    ebx, 1
    next:
        mov    ecx, eax
        rep    movsd
        mov    ecx, ebx
        rep    movsw
        add    edi, [esp]
        add    esi, edx
        dec    lfbHeight
        jnz    next
        pop    ebx
    }
}

void PutImage24(int x1, int y1, IMAGE *img)
{
    int x2, y2, lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    int imgWidth  = img->mWidth;
    int imgHeight = img->mHeight;
    void *imgData = img->mData;
    unsigned int imgRowBytes = img->mRowBytes;

    // Calculate new position
    x2 = (x1 + imgWidth) - 1;
    y2 = (y1 + imgHeight) - 1;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, lfbMinX
        shl    ebx, 1
        add    ebx, lfbMinX
        add    eax, ebx
        add    edi, eax
        mov    esi, imgData
        mov    eax, lfbMinY
        sub    eax, y1
        mul    imgRowBytes
        mov    ebx, lfbMinX
        sub    ebx, x1
        mov    ecx, ebx
        shl    ebx, 1
        add    ebx, ecx
        add    eax, ebx
        add    esi, eax
        mov    ebx, bytesPerScanline
        mov    edx, lfbWidth
        shl    edx, 1
        add    edx, lfbWidth
        sub    ebx, edx
        push   ebx
        mov    edx, imgWidth
        sub    edx, lfbWidth
        mov    eax, edx
        shl    edx, 1
        add    edx, eax
        mov    ecx, lfbWidth
        lea    ecx, [ecx + ecx * 2]
        mov    eax, ecx
        shr    eax, 2
        and    ecx, 3
        mov    ebx, ecx
    next:
        mov    ecx, eax
        rep    movsd
        mov    ecx, ebx
        rep    movsb
        add    edi, [esp]
        add    esi, edx
        dec    lfbHeight
        jnz    next
        pop    ebx
    }
}

void PutImage32(int x1, int y1, IMAGE *img)
{
    int x2, y2, lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    int imgWidth  = img->mWidth;
    int imgHeight = img->mHeight;
    void *imgData = img->mData;
    unsigned int imgRowBytes = img->mRowBytes;

    // Calculate new position
    x2 = (x1 + imgWidth) - 1;
    y2 = (y1 + imgHeight) - 1;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, lfbMinX
        shl    ebx, 2
        add    eax, ebx
        add    edi, eax
        mov    esi, imgData
        mov    eax, lfbMinY
        sub    eax, y1
        mul    imgRowBytes
        mov    ebx, lfbMinX
        sub    ebx, x1
        shl    ebx, 2
        add    eax, ebx
        add    esi, eax
        mov    ebx, bytesPerScanline
        mov    edx, lfbWidth
        shl    edx, 2
        sub    ebx, edx
        mov    edx, imgWidth
        sub    edx, lfbWidth
        shl    edx, 2
    next:
        mov    ecx, lfbWidth
        rep    movsd
        add    edi, ebx
        add    esi, edx
        dec    lfbHeight
        jnz    next
    }
}

// Put 32 bits transparent image
void PutImageAlpha(int x1, int y1, IMAGE *img)
{
    int x2, y2, lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    int imgWidth  = img->mWidth;
    int imgHeight = img->mHeight;
    void *imgData = img->mData;
    unsigned int imgRowBytes = img->mRowBytes;

    // Check for 32bit support
    if (bytesPerPixel != 4) return;
    if (img->mPixels != 4) return;

    // Calculate new position
    x2 = (x1 + imgWidth) - 1;
    y2 = (y1 + imgHeight) - 1;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, lfbMinX
        shl    ebx, 2
        add    eax, ebx
        add    edi, eax
        mov    esi, imgData
        mov    eax, lfbMinY
        sub    eax, y1
        mul    imgRowBytes
        mov    ebx, lfbMinX
        sub    ebx, x1
        shl    ebx, 2
        add    eax, ebx
        add    esi, eax
        mov    ebx, bytesPerScanline
        mov    edx, lfbWidth
        shl    edx, 2
        sub    ebx, edx
        mov    edx, imgWidth
        sub    edx, lfbWidth
        shl    edx, 2
    next:
        mov    ecx, lfbWidth
        push   ebx
        push   edx
    plot:
        mov    al, [esi]
        mul    byte ptr[esi + 3]
        mov    bx, ax
        mov    al, [edi]
        mov    dl, 255
        sub    dl, [esi + 3]
        mul    dl
        add    ax, bx
        shr    ax, 8
        stosb
        mov    al, [esi + 1]
        mul    byte ptr[esi + 3]
        mov    bx, ax
        mov    al, [edi]
        mov    dl, 255
        sub    dl, [esi + 3]
        mul    dl
        add    ax, bx
        shr    ax, 8
        stosb
        mov    al, [esi + 2]
        mul    byte ptr[esi + 3]
        mov    bx, ax
        mov    al, [edi]
        mov    dl, 255
        sub    dl, [esi + 3]
        mul    dl
        add    ax, bx
        shr    ax, 8
        stosb
        inc    edi
        add    esi, 4
        loop   plot
        pop    edx
        pop    ebx
        add    edi, ebx
        add    esi, edx
        dec    lfbHeight
        jnz    next
    }
}

void PutImageAdd32(int x1, int y1, IMAGE *img)
{
    int x2, y2, lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    int imgWidth  = img->mWidth;
    int imgHeight = img->mHeight;
    void *imgData = img->mData;
    unsigned int imgRowBytes = img->mRowBytes;

    // Check compatible pixel mode
    if (img->mPixels != bytesPerPixel) return;

    // Calculate new position
    x2 = (x1 + imgWidth) - 1;
    y2 = (y1 + imgHeight) - 1;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, lfbMinX
        shl    ebx, 2
        add    eax, ebx
        add    edi, eax
        mov    esi, imgData
        mov    eax, lfbMinY
        sub    eax, y1
        mul    imgRowBytes
        mov    ebx, lfbMinX
        sub    ebx, x1
        shl    ebx, 2
        add    eax, ebx
        add    esi, eax
        mov    ebx, bytesPerScanline
        mov    edx, lfbWidth
        shl    edx, 2
        sub    ebx, edx
        mov    edx, imgWidth
        sub    edx, lfbWidth
        shl    edx, 2
    next:
        mov    ecx, lfbWidth
        push   ebx
    plot:
        lodsd
        add    al, [edi]
        jnc    bstep
        mov    al, 255
    bstep:
        add    ah, [edi + 1]
        jnc    gstep
        mov    ah, 255
    gstep:
        mov    ebx, eax
        shr    ebx, 16
        add    bl, [edi + 2]
        jnc    rstep
        mov    bl, 255
    rstep:
        shl    ebx, 16
        and    eax, 00FFFFh
        or     eax, ebx
        stosd
        loop   plot
        pop    ebx
        add    edi, ebx
        add    esi, edx
        dec    lfbHeight
        jnz    next
    }
}

void PutImageAdd24(int x1, int y1, IMAGE *img)
{
    int x2, y2, lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    int imgWidth  = img->mWidth;
    int imgHeight = img->mHeight;
    void *imgData = img->mData;
    unsigned int imgRowBytes = img->mRowBytes;

    // Check compatible pixel mode
    if (img->mPixels != bytesPerPixel) return;

    // Calculate new position
    x2 = (x1 + imgWidth) - 1;
    y2 = (y1 + imgHeight) - 1;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, lfbMinX
        shl    ebx, 1
        add    eax, ebx
        add    eax, lfbMinX
        add    edi, eax
        mov    esi, imgData
        mov    eax, lfbMinY
        sub    eax, y1
        mul    imgRowBytes
        mov    ebx, lfbMinX
        sub    ebx, x1
        mov    ecx, ebx
        shl    ebx, 1
        add    eax, ebx
        add    eax, ecx
        add    esi, eax
        mov    ebx, bytesPerScanline
        mov    edx, lfbWidth
        shl    edx, 1
        sub    ebx, edx
        sub    ebx, lfbWidth
        mov    edx, imgWidth
        sub    edx, lfbWidth
        mov    ecx, edx
        shl    edx, 1
        add    edx, ecx
    next:
        mov    ecx, lfbWidth
        push   ebx
    plot:
        mov    ax, [edi]
        add    al, [esi]
        jnc    bstep
        mov    al, 255
    bstep:
        add    ah, [esi + 1]
        jnc    gstep
        mov    ah, 255
    gstep:
        mov    bl, [edi + 2]
        add    bl, [esi + 2]
        jnc    rstep
        mov    bl, 255
    rstep:
        stosw
        mov    al, bl
        stosb
        add    esi, 3
        loop   plot
        pop    ebx
        add    edi, ebx
        add    esi, edx
        dec    lfbHeight
        jnz    next
    }
}

void PutImageAdd16(int x1, int y1, IMAGE *img)
{
    int x2, y2, lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    int imgWidth  = img->mWidth;
    int imgHeight = img->mHeight;
    void *imgData = img->mData;
    unsigned int imgRowBytes = img->mRowBytes;

    // Check compatible pixel mode
    if (img->mPixels != bytesPerPixel) return;

    // Calculate new position
    x2 = (x1 + imgWidth) - 1;
    y2 = (y1 + imgHeight) - 1;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, lfbMinX
        add    eax, lfbMinX
        add    edi, eax
        mov    esi, imgData
        mov    eax, lfbMinY
        sub    eax, y1
        mul    imgRowBytes
        mov    ebx, lfbMinX
        sub    ebx, x1
        shl    ebx, 1
        add    eax, ebx
        add    esi, eax
        mov    ebx, bytesPerScanline
        sub    ebx, lfbWidth
        sub    ebx, lfbWidth
        mov    edx, imgWidth
        sub    edx, lfbWidth
        shl    edx, 1
    next:
        push   lfbWidth
        push   ebx
        push   edx
    plot:
        lodsw
        mov    bx, [edi]
        mov    dx, bx
        and    bx, 1111100000011111b
        and    dx, 11111100000b
        mov    cx, ax
        and    ax, 1111100000011111b
        and    cx, 11111100000b
        add    al, bl
        cmp    al, 11111b
        jna    bstep
        mov    al, 11111b
    bstep:
        add    ah, bh
        jnc    gstep
        mov    ah, 11111000b
    gstep:
        add    cx, dx
        cmp    cx, 11111100000b
        jna    rstep
        mov    cx, 11111100000b
    rstep:
        or     ax, cx
        stosw
        dec    lfbWidth
        jnz    plot
        pop    edx
        pop    ebx  
        pop    lfbWidth
        add    edi, ebx
        add    esi, edx
        dec    lfbHeight
        jnz    next
    }
}

void PutImageAdd15(int x1, int y1, IMAGE *img)
{
    int x2, y2, lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    int imgWidth  = img->mWidth;
    int imgHeight = img->mHeight;
    void *imgData = img->mData;
    unsigned int imgRowBytes = img->mRowBytes;

    // Check compatible pixel mode
    if (img->mPixels != bytesPerPixel) return;

    // Calculate new position
    x2 = (x1 + imgWidth) - 1;
    y2 = (y1 + imgHeight) - 1;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, lfbMinX
        add    eax, lfbMinX
        add    edi, eax
        mov    esi, imgData
        mov    eax, lfbMinY
        sub    eax, y1
        mul    imgRowBytes
        mov    ebx, lfbMinX
        sub    ebx, x1
        shl    ebx, 1
        add    eax, ebx
        add    esi, eax
        mov    ebx, bytesPerScanline
        sub    ebx, lfbWidth
        sub    ebx, lfbWidth
        mov    edx, imgWidth
        sub    edx, lfbWidth
        shl    edx, 1
    next:
        push   lfbWidth
        push   ebx
        push   edx
    plot:
        lodsw
        mov    bx, [edi]
        mov    dx, bx
        and    bx, 111110000011111b
        and    dx, 1111100000b
        mov    cx, ax
        and    ax, 111110000011111b
        and    cx, 1111100000b
        add    al, bl
        cmp    al, 11111b
        jna    bstep
        mov    al, 11111b
    bstep:
        add    ah, bh
        cmp    ah, 1111100b
        jna    gstep
        mov    ah, 1111100b
    gstep:
        add    cx, dx
        cmp    cx, 1111100000b
        jna    rstep
        mov    cx, 1111100000b
    rstep:
        or     ax, cx
        stosw
        dec    lfbWidth
        jnz    plot
        pop    edx
        pop    ebx  
        pop    lfbWidth
        add    edi, ebx
        add    esi, edx
        dec    lfbHeight
        jnz    next
    }
}

void PutImageSub32(int x1, int y1, IMAGE *img)
{
    int x2, y2, lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    int imgWidth  = img->mWidth;
    int imgHeight = img->mHeight;
    void *imgData = img->mData;
    unsigned int imgRowBytes = img->mRowBytes;

    // Check for compatible pixel mode
    if (img->mPixels != bytesPerPixel) return;

    // Calculate new position
    x2 = (x1 + imgWidth) - 1;
    y2 = (y1 + imgHeight) - 1;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, lfbMinX
        shl    ebx, 2
        add    eax, ebx
        add    edi, eax
        mov    esi, imgData
        mov    eax, lfbMinY
        sub    eax, y1
        mul    imgRowBytes
        mov    ebx, lfbMinX
        sub    ebx, x1
        shl    ebx, 2
        add    eax, ebx
        add    esi, eax
        mov    ebx, bytesPerScanline
        mov    edx, lfbWidth
        shl    edx, 2
        sub    ebx, edx
        mov    edx, imgWidth
        sub    edx, lfbWidth
        shl    edx, 2
    next:
        mov    ecx, lfbWidth
        push   ebx
    plot:
        mov    eax, [edi]
        sub    al, [esi]
        jnc    bstep
        xor    al, al
    bstep:
        sub    ah, [esi + 1]
        jnc    gstep
        xor    ah, ah
    gstep:
        mov    ebx, eax
        shr    ebx, 16
        sub    bl, [esi + 2]
        jnc    rstep
        xor    bl, bl
    rstep:
        shl    ebx, 16
        and    eax, 00FFFFh
        or     eax, ebx
        stosd
        add    esi, 4
        loop   plot
        pop    ebx
        add    edi, ebx
        add    esi, edx
        dec    lfbHeight
        jnz    next
    }
}

void PutImageSub24(int x1, int y1, IMAGE *img)
{
    int x2, y2, lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    int imgWidth  = img->mWidth;
    int imgHeight = img->mHeight;
    void *imgData = img->mData;
    unsigned int imgRowBytes = img->mRowBytes;

    // Check for compatible pixel mode
    if (img->mPixels != bytesPerPixel) return;

    // Calculate new position
    x2 = (x1 + imgWidth) - 1;
    y2 = (y1 + imgHeight) - 1;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, lfbMinX
        shl    ebx, 1
        add    eax, ebx
        add    eax, lfbMinX
        add    edi, eax
        mov    esi, imgData
        mov    eax, lfbMinY
        sub    eax, y1
        mul    imgRowBytes
        mov    ebx, lfbMinX
        sub    ebx, x1
        mov    ecx, ebx
        shl    ebx, 1
        add    eax, ebx
        add    eax, ecx
        add    esi, eax
        mov    ebx, bytesPerScanline
        mov    edx, lfbWidth
        shl    edx, 1
        sub    ebx, edx
        sub    ebx, lfbWidth
        mov    edx, imgWidth
        sub    edx, lfbWidth
        mov    ecx, edx
        shl    edx, 1
        add    edx, ecx
    next:
        mov    ecx, lfbWidth
        push   ebx
    plot:
        mov    ax, [edi]
        sub    al, [esi]
        jnc    bstep
        xor    al, al
    bstep:
        sub    ah, [esi + 1]
        jnc    gstep
        xor    ah, ah
    gstep:
        mov    bl, [edi + 2]
        sub    bl, [esi + 2]
        jnc    rstep
        xor    bl, bl
    rstep:
        stosw
        mov    al, bl
        stosb
        add    esi, 3
        loop   plot
        pop    ebx
        add    edi, ebx
        add    esi, edx
        dec    lfbHeight
        jnz    next
    }
}

void PutImageSub16(int x1, int y1, IMAGE *img)
{
    int x2, y2, lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    int imgWidth  = img->mWidth;
    int imgHeight = img->mHeight;
    void *imgData = img->mData;
    unsigned int imgRowBytes = img->mRowBytes;

    // Check compatible pixel mode
    if (img->mPixels != bytesPerPixel) return;

    // Calculate new position
    x2 = (x1 + imgWidth) - 1;
    y2 = (y1 + imgHeight) - 1;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, lfbMinX
        add    eax, lfbMinX
        add    edi, eax
        mov    esi, imgData
        mov    eax, lfbMinY
        sub    eax, y1
        mul    imgRowBytes
        mov    ebx, lfbMinX
        sub    ebx, x1
        shl    ebx, 1
        add    eax, ebx
        add    esi, eax
        mov    ebx, bytesPerScanline
        sub    ebx, lfbWidth
        sub    ebx, lfbWidth
        mov    edx, imgWidth
        sub    edx, lfbWidth
        shl    edx, 1
    next:
        push   lfbWidth
        push   ebx
        push   edx
    plot:
        mov    ax, [edi]
        mov    bx, [esi]
        mov    dx, bx
        and    bx, 1111100000011111b
        and    dx, 11111100000b
        mov    cx, ax
        and    ax, 1111100000011111b
        and    cx, 11111100000b
        sub    al, bl
        jnc    bstep
        xor    al, al
    bstep:
        sub    ah, bh
        jnc    gstep
        xor    ah, ah
    gstep:
        sub    cx, dx
        jc     rstep
        or     ax, cx
    rstep:
        stosw
        add    esi, 2
        dec    lfbWidth
        jnz    plot
        pop    edx
        pop    ebx  
        pop    lfbWidth
        add    edi, ebx
        add    esi, edx
        dec    lfbHeight
        jnz    next
    }
}

void PutImageSub15(int x1, int y1, IMAGE *img)
{
    int x2, y2, lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    int imgWidth  = img->mWidth;
    int imgHeight = img->mHeight;
    void *imgData = img->mData;
    unsigned int imgRowBytes = img->mRowBytes;

    // Check compatible pixel mode
    if (img->mPixels != bytesPerPixel) return;

    // Calculate new position
    x2 = (x1 + imgWidth) - 1;
    y2 = (y1 + imgHeight) - 1;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, lfbMinX
        add    eax, lfbMinX
        add    edi, eax
        mov    esi, imgData
        mov    eax, lfbMinY
        sub    eax, y1
        mul    imgRowBytes
        mov    ebx, lfbMinX
        sub    ebx, x1
        shl    ebx, 1
        add    eax, ebx
        add    esi, eax
        mov    ebx, bytesPerScanline
        sub    ebx, lfbWidth
        sub    ebx, lfbWidth
        mov    edx, imgWidth
        sub    edx, lfbWidth
        shl    edx, 1
    next:
        push   lfbWidth
        push   ebx
        push   edx
    plot:
        mov    ax, [edi]
        mov    bx, [esi]
        mov    dx, bx
        and    bx, 111110000011111b
        and    dx, 1111100000b
        mov    cx, ax
        and    ax, 111110000011111b
        and    cx, 1111100000b
        sub    al, bl
        jnc    bstep
        xor    al, al
    bstep:
        sub    ah, bh
        jnc    gstep
        xor    ah, ah
    gstep:
        sub    cx, dx
        jc     rstep
        or     ax, cx
    rstep:
        stosw
        add    esi, 2
        dec    lfbWidth
        jnz    plot
        pop    edx
        pop    ebx  
        pop    lfbWidth
        add    edi, ebx
        add    esi, edx
        dec    lfbHeight
        jnz    next
    }
}

// Sprite functions
void PutSprite8(int x1, int y1, unsigned int key, IMAGE *img)
{
    int x2, y2, lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    int imgWidth  = img->mWidth;
    int imgHeight = img->mHeight;
    void *imgData = img->mData;
    unsigned int imgRowBytes = img->mRowBytes;

    // Calculate new position
    x2 = (x1 + imgWidth) - 1;
    y2 = (y1 + imgHeight) - 1;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, lfbMinX
        add    edi, eax
        mov    esi, imgData
        mov    eax, lfbMinY
        sub    eax, y1
        mul    imgRowBytes
        mov    ebx, lfbMinX
        sub    ebx, x1
        add    eax, ebx
        add    esi, eax
        mov    ebx, bytesPerScanline
        sub    ebx, lfbWidth
        mov    edx, imgWidth
        sub    edx, lfbWidth
    next:
        mov    ecx, lfbWidth
    plot:
        lodsb
        cmp    al, byte ptr[key]
        je     skip
        mov    [edi], al
    skip:
        inc    edi
        loop   plot
        add    edi, ebx
        add    esi, edx
        dec    lfbHeight
        jnz    next
    }
}

void PutSprite16(int x1, int y1, unsigned int key, IMAGE *img)
{
    int x2, y2, lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    int imgWidth  = img->mWidth;
    int imgHeight = img->mHeight;
    void *imgData = img->mData;
    unsigned int imgRowBytes = img->mRowBytes;

    // Calculate new position
    x2 = (x1 + imgWidth) - 1;
    y2 = (y1 + imgHeight) - 1;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, lfbMinX
        add    eax, lfbMinX
        add    edi, eax
        mov    esi, imgData
        mov    eax, lfbMinY
        sub    eax, y1
        mul    imgRowBytes
        mov    ebx, lfbMinX
        sub    ebx, x1
        shl    ebx, 1
        add    eax, ebx
        add    esi, eax
        mov    ebx, bytesPerScanline
        sub    ebx, lfbWidth
        sub    ebx, lfbWidth
        mov    edx, imgWidth
        sub    edx, lfbWidth
        shl    edx, 1
    next:
        mov    ecx, lfbWidth
    plot:
        lodsw
        cmp    ax, word ptr[key]
        je     skip
        mov    [edi], ax
    skip:
        add    edi, 2
        loop   plot
        add    edi, ebx
        add    esi, edx
        dec    lfbHeight
        jnz    next
    }
}

void PutSprite24(int x1, int y1, unsigned int key, IMAGE *img)
{
    int x2, y2, lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    int imgWidth  = img->mWidth;
    int imgHeight = img->mHeight;
    void *imgData = img->mData;
    unsigned int imgRowBytes = img->mRowBytes;

    // Calculate new position
    x2 = (x1 + imgWidth) - 1;
    y2 = (y1 + imgHeight) - 1;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, lfbMinX
        shl    ebx, 1
        add    ebx, lfbMinX
        add    eax, ebx
        add    edi, eax
        mov    esi, imgData
        mov    eax, lfbMinY
        sub    eax, y1
        mul    imgRowBytes
        mov    ebx, lfbMinX
        sub    ebx, x1
        mov    ecx, ebx
        shl    ebx, 1
        add    ebx, ecx
        add    eax, ebx
        add    esi, eax
        mov    ebx, bytesPerScanline
        mov    edx, lfbWidth
        shl    edx, 1
        add    edx, lfbWidth
        sub    ebx, edx
        mov    edx, imgWidth
        sub    edx, lfbWidth
        mov    eax, edx
        shl    edx, 1
        add    edx, eax
    next:
        push   edx
        mov    ecx, lfbWidth
    plot:
        lodsw
        mov    edx, eax
        lodsb
        shl    eax, 16
        or     edx, eax
        and    edx, 00FFFFFFh
        cmp    edx, key
        je     skip
        mov    [edi], dx
        shr    edx, 16
        mov    [edi + 2], dl
    skip:
        add    edi, 3
        loop   plot
        pop    edx
        add    edi, ebx
        add    esi, edx
        dec    lfbHeight
        jnz    next
    }
}

void PutSprite32(int x1, int y1, unsigned int key, IMAGE *img)
{
    int x2, y2, lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    int imgWidth  = img->mWidth;
    int imgHeight = img->mHeight;
    void *imgData = img->mData;
    unsigned int imgRowBytes = img->mRowBytes;

    // Calculate new position
    x2 = (x1 + imgWidth) - 1;
    y2 = (y1 + imgHeight) - 1;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, lfbMinX
        shl    ebx, 2
        add    eax, ebx
        add    edi, eax
        mov    esi, imgData
        mov    eax, lfbMinY
        sub    eax, y1
        mul    imgRowBytes
        mov    ebx, lfbMinX
        sub    ebx, x1
        shl    ebx, 2
        add    eax, ebx
        add    esi, eax
        mov    ebx, bytesPerScanline
        mov    edx, lfbWidth
        shl    edx, 2
        sub    ebx, edx
        mov    edx, imgWidth
        sub    edx, lfbWidth
        shl    edx, 2
    next:
        mov    ecx, lfbWidth
    plot:
        lodsd
        and    eax, 00FFFFFFh
        cmp    eax, key
        je     skip
        mov    [edi], eax
    skip:
        add    edi, 4
        loop   plot
        add    edi, ebx
        add    esi, edx
        dec    lfbHeight
        jnz    next
    }
}

void PutSpriteAdd32(int x1, int y1, unsigned int key, IMAGE *img)
{
    int x2, y2, lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    int imgWidth  = img->mWidth;
    int imgHeight = img->mHeight;
    void *imgData = img->mData;
    unsigned int imgRowBytes = img->mRowBytes;

    // Calculate new position
    x2 = (x1 + imgWidth) - 1;
    y2 = (y1 + imgHeight) - 1;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, lfbMinX
        shl    ebx, 2
        add    eax, ebx
        add    edi, eax
        mov    esi, imgData
        mov    eax, lfbMinY
        sub    eax, y1
        mul    imgRowBytes
        mov    ebx, lfbMinX
        sub    ebx, x1
        shl    ebx, 2
        add    eax, ebx
        add    esi, eax
        mov    ebx, bytesPerScanline
        mov    edx, lfbWidth
        shl    edx, 2
        sub    ebx, edx
        mov    edx, imgWidth
        sub    edx, lfbWidth
        shl    edx, 2
    next:
        mov    ecx, lfbWidth
        push   ebx
    plot:
        lodsd
        and    eax, 00FFFFFFh
        cmp    eax, key
        je     skip
        add    al, [edi]
        jnc    bstep
        mov    al, 255
    bstep:
        add    ah, [edi + 1]
        jnc    gstep
        mov    ah, 255
    gstep:
        mov    ebx, eax
        shr    ebx, 16
        add    bl, [edi + 2]
        jnc    rstep
        mov    bl, 255
    rstep:
        shl    ebx, 16
        and    eax, 00FFFFh
        or     eax, ebx
        mov    [edi], eax
    skip:
        add    edi, 4
        loop   plot
        pop    ebx
        add    edi, ebx
        add    esi, edx
        dec    lfbHeight
        jnz    next
    }
}

void PutSpriteAdd24(int x1, int y1, unsigned int key, IMAGE *img)
{
    int x2, y2, lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    int imgWidth  = img->mWidth;
    int imgHeight = img->mHeight;
    void *imgData = img->mData;
    unsigned int imgRowBytes = img->mRowBytes;

    // Calculate new position
    x2 = (x1 + imgWidth) - 1;
    y2 = (y1 + imgHeight) - 1;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, lfbMinX
        shl    ebx, 1
        add    eax, ebx
        add    eax, lfbMinX
        add    edi, eax
        mov    esi, imgData
        mov    eax, lfbMinY
        sub    eax, y1
        mul    imgRowBytes
        mov    ebx, lfbMinX
        sub    ebx, x1
        mov    ecx, ebx
        shl    ebx, 1
        add    eax, ebx
        add    eax, ecx
        add    esi, eax
        mov    ebx, bytesPerScanline
        mov    edx, lfbWidth
        shl    edx, 1
        sub    ebx, edx
        sub    ebx, lfbWidth
        mov    edx, imgWidth
        sub    edx, lfbWidth
        mov    ecx, edx
        shl    edx, 1
        add    edx, ecx
    next:
        mov    ecx, lfbWidth
        push   ebx
    plot:
        lodsw
        mov    ebx, eax
        lodsb
        shl    eax, 16
        or     eax, ebx
        and    eax, 00FFFFFFh
        cmp    eax, key
        je     skip
        add    al, [edi]
        jnc    bstep
        mov    al, 255
    bstep:
        add    ah, [edi + 1]
        jnc    gstep
        mov    ah, 255
    gstep:
        mov    ebx, eax
        shr    ebx, 16
        add    bl, [edi + 2]
        jnc    rstep
        mov    bl, 255
    rstep:
        mov    [edi], ax
        mov    [edi + 2], bl
    skip:
        add    edi, 3
        loop   plot
        pop    ebx
        add    edi, ebx
        add    esi, edx
        dec    lfbHeight
        jnz    next
    }
}

void PutSpriteAdd16(int x1, int y1, unsigned int key, IMAGE *img)
{
    int x2, y2, lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    int imgWidth  = img->mWidth;
    int imgHeight = img->mHeight;
    void *imgData = img->mData;
    unsigned int imgRowBytes = img->mRowBytes;

    // Calculate new position
    x2 = (x1 + imgWidth) - 1;
    y2 = (y1 + imgHeight) - 1;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, lfbMinX
        add    eax, lfbMinX
        add    edi, eax
        mov    esi, imgData
        mov    eax, lfbMinY
        sub    eax, y1
        mul    imgRowBytes
        mov    ebx, lfbMinX
        sub    ebx, x1
        shl    ebx, 1
        add    eax, ebx
        add    esi, eax
        mov    ebx, bytesPerScanline
        sub    ebx, lfbWidth
        sub    ebx, lfbWidth
        mov    edx, imgWidth
        sub    edx, lfbWidth
        shl    edx, 1
    next:
        push   ebx
        push   edx
        push   lfbWidth
    plot:
        lodsw
        cmp    ax, word ptr[key]
        je     skip
        mov    bx, [edi]
        mov    dx, bx
        and    bx, 1111100000011111b
        and    dx, 11111100000b
        mov    cx, ax
        and    ax, 1111100000011111b
        and    cx, 11111100000b
        add    al, bl
        cmp    al, 11111b
        jna    bstep
        mov    al, 11111b
    bstep:
        add    ah, bh
        jnc    gstep
        mov    ah, 11111000b
    gstep:
        add    cx, dx
        cmp    cx, 11111100000b
        jna    rstep
        mov    cx, 11111100000b
    rstep:
        or     ax, cx
        mov    [edi], ax
    skip:
        add    edi, 2
        dec    lfbWidth
        jnz    plot
        pop    lfbWidth
        pop    edx
        pop    ebx
        add    edi, ebx
        add    esi, edx
        dec    lfbHeight
        jnz    next
    }
}

void PutSpriteAdd15(int x1, int y1, unsigned int key, IMAGE *img)
{
    int x2, y2, lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    int imgWidth  = img->mWidth;
    int imgHeight = img->mHeight;
    void *imgData = img->mData;
    unsigned int imgRowBytes = img->mRowBytes;

    // Calculate new position
    x2 = (x1 + imgWidth) - 1;
    y2 = (y1 + imgHeight) - 1;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, lfbMinX
        add    eax, lfbMinX
        add    edi, eax
        mov    esi, imgData
        mov    eax, lfbMinY
        sub    eax, y1
        mul    imgRowBytes
        mov    ebx, lfbMinX
        sub    ebx, x1
        shl    ebx, 1
        add    eax, ebx
        add    esi, eax
        mov    ebx, bytesPerScanline
        sub    ebx, lfbWidth
        sub    ebx, lfbWidth
        mov    edx, imgWidth
        sub    edx, lfbWidth
        shl    edx, 1
    next:
        push   ebx
        push   edx
        push   lfbWidth
    plot:
        lodsw
        cmp    ax, word ptr[key]
        je     skip
        mov    bx, [edi]
        mov    dx, bx
        and    bx, 111110000011111b
        and    dx, 1111100000b
        mov    cx, ax
        and    ax, 111110000011111b
        and    cx, 1111100000b
        add    al, bl
        cmp    al, 11111b
        jna    bstep
        mov    al, 11111b
    bstep:
        add    ah, bh
        cmp    ah, 1111100b
        jna    gstep
        mov    ah, 1111100b
    gstep:
        add    cx, dx
        cmp    cx, 1111100000b
        jna    rstep
        mov    cx, 1111100000b
    rstep:
        or     ax, cx
        mov    [edi], ax
    skip:
        add    edi, 2
        dec    lfbWidth
        jnz    plot
        pop    lfbWidth
        pop    edx
        pop    ebx
        add    edi, ebx
        add    esi, edx
        dec    lfbHeight
        jnz    next
    }
}

void PutSpriteSub32(int x1, int y1, unsigned int key, IMAGE *img)
{
    int x2, y2, lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    int imgWidth  = img->mWidth;
    int imgHeight = img->mHeight;
    void *imgData = img->mData;
    unsigned int imgRowBytes = img->mRowBytes;

    // Calculate new position
    x2 = (x1 + imgWidth) - 1;
    y2 = (y1 + imgHeight) - 1;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, lfbMinX
        shl    ebx, 2
        add    eax, ebx
        add    edi, eax
        mov    esi, imgData
        mov    eax, lfbMinY
        sub    eax, y1
        mul    imgRowBytes
        mov    ebx, lfbMinX
        sub    ebx, x1
        shl    ebx, 2
        add    eax, ebx
        add    esi, eax
        mov    ebx, bytesPerScanline
        mov    edx, lfbWidth
        shl    edx, 2
        sub    ebx, edx
        mov    edx, imgWidth
        sub    edx, lfbWidth
        shl    edx, 2
    next:
        mov    ecx, lfbWidth
        push   ebx
    plot:
        mov    eax, [esi]
        and    eax, 00FFFFFFh
        cmp    eax, key
        je     skip
        mov    eax, [edi]
        sub    al, [esi]
        jnc    bstep
        xor    al, al
    bstep:
        sub    ah, [esi + 1]
        jnc    gstep
        xor    ah, ah
    gstep:
        mov    ebx, eax
        shr    ebx, 16
        sub    bl, [esi + 2]
        jnc    rstep
        xor    bl, bl
    rstep:
        shl    ebx, 16
        and    eax, 00FFFFh
        or     eax, ebx
        mov    [edi], eax
    skip:
        add    edi, 4
        add    esi, 4
        loop   plot
        pop    ebx
        add    edi, ebx
        add    esi, edx
        dec    lfbHeight
        jnz    next
    }
}

void PutSpriteSub24(int x1, int y1, unsigned int key, IMAGE *img)
{
    int x2, y2, lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    int imgWidth  = img->mWidth;
    int imgHeight = img->mHeight;
    void *imgData = img->mData;
    unsigned int imgRowBytes = img->mRowBytes;

    // Calculate new position
    x2 = (x1 + imgWidth) - 1;
    y2 = (y1 + imgHeight) - 1;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, lfbMinX
        shl    ebx, 1
        add    eax, ebx
        add    eax, lfbMinX
        add    edi, eax
        mov    esi, imgData
        mov    eax, lfbMinY
        sub    eax, y1
        mul    imgRowBytes
        mov    ebx, lfbMinX
        sub    ebx, x1
        mov    ecx, ebx
        shl    ebx, 1
        add    eax, ebx
        add    eax, ecx
        add    esi, eax
        mov    ebx, bytesPerScanline
        mov    edx, lfbWidth
        shl    edx, 1
        sub    ebx, edx
        sub    ebx, lfbWidth
        mov    edx, imgWidth
        sub    edx, lfbWidth
        mov    ecx, edx
        shl    edx, 1
        add    edx, ecx
    next:
        mov    ecx, lfbWidth
        push   ebx
    plot:
        mov    ax, [esi]
        mov    bl, [esi + 2]
        shl    ebx, 16
        or     eax, ebx
        and    eax, 00FFFFFFh
        cmp    eax, key
        je     skip
        mov    ax, [edi]
        sub    al, [esi]
        jnc    bstep
        xor    al, al
    bstep:
        sub    ah, [esi + 1]
        jnc    gstep
        xor    ah, ah
    gstep:
        mov    bl, [edi + 2]
        sub    bl, [esi + 2]
        jnc    rstep
        xor    bl, bl
    rstep:
        mov    [edi], ax
        mov    [edi + 2], bl
    skip:
        add    edi, 3
        add    esi, 3
        loop   plot
        pop    ebx
        add    edi, ebx
        add    esi, edx
        dec    lfbHeight
        jnz    next
    }
}

void PutSpriteSub16(int x1, int y1, unsigned int key, IMAGE *img)
{
    int x2, y2, lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    int imgWidth  = img->mWidth;
    int imgHeight = img->mHeight;
    void *imgData = img->mData;
    unsigned int imgRowBytes = img->mRowBytes;

    // Calculate new position
    x2 = (x1 + imgWidth) - 1;
    y2 = (y1 + imgHeight) - 1;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, lfbMinX
        add    eax, lfbMinX
        add    edi, eax
        mov    esi, imgData
        mov    eax, lfbMinY
        sub    eax, y1
        mul    imgRowBytes
        mov    ebx, lfbMinX
        sub    ebx, x1
        shl    ebx, 1
        add    eax, ebx
        add    esi, eax
        mov    ebx, bytesPerScanline
        sub    ebx, lfbWidth
        sub    ebx, lfbWidth
        mov    edx, imgWidth
        sub    edx, lfbWidth
        shl    edx, 1
    next:
        push   ebx
        push   edx
        push   lfbWidth
    plot:
        mov    ax, [esi]
        cmp    ax, word ptr[key]
        je     skip
        mov    ax, [edi]
        mov    bx, [esi]
        mov    dx, bx
        and    bx, 1111100000011111b
        and    dx, 11111100000b
        mov    cx, ax
        and    ax, 1111100000011111b
        and    cx, 11111100000b
        sub    al, bl
        jnc    bstep
        xor    al, al
    bstep:
        sub    ah, bh
        jnc    gstep
        xor    ah, ah
    gstep:
        sub    cx, dx
        jc     rstep
        or     ax, cx
    rstep:
        mov    [edi], ax
    skip:
        add    edi, 2
        add    esi, 2
        dec    lfbWidth
        jnz    plot
        pop    lfbWidth
        pop    edx
        pop    ebx
        add    edi, ebx
        add    esi, edx
        dec    lfbHeight
        jnz    next
    }
}

void PutSpriteSub15(int x1, int y1, unsigned int key, IMAGE *img)
{
    int x2, y2, lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    int imgWidth  = img->mWidth;
    int imgHeight = img->mHeight;
    void *imgData = img->mData;
    unsigned int imgRowBytes = img->mRowBytes;

    // Calculate new position
    x2 = (x1 + imgWidth) - 1;
    y2 = (y1 + imgHeight) - 1;

    // Clip image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    _asm {
        mov    edi, lfbPtr
        mov    eax, lfbMinY
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, lfbMinX
        add    eax, lfbMinX
        add    edi, eax
        mov    esi, imgData
        mov    eax, lfbMinY
        sub    eax, y1
        mul    imgRowBytes
        mov    ebx, lfbMinX
        sub    ebx, x1
        shl    ebx, 1
        add    eax, ebx
        add    esi, eax
        mov    ebx, bytesPerScanline
        sub    ebx, lfbWidth
        sub    ebx, lfbWidth
        mov    edx, imgWidth
        sub    edx, lfbWidth
        shl    edx, 1
    next:
        push   ebx
        push   edx
        push   lfbWidth
    plot:
        mov    ax, [esi]
        cmp    ax, word ptr[key]
        je     skip
        mov    ax, [edi]
        mov    bx, [esi]
        mov    dx, bx
        and    bx, 111110000011111b
        and    dx, 1111100000b
        mov    cx, ax
        and    ax, 111110000011111b
        and    cx, 1111100000b
        sub    al, bl
        jnc    bstep
        xor    al, al
    bstep:
        sub    ah, bh
        jnc    gstep
        xor    ah, ah
    gstep:
        sub    cx, dx
        jc     rstep
        or     ax, cx
    rstep:
        mov    [edi], ax
    skip:
        add    edi, 2
        add    esi, 2
        dec    lfbWidth
        jnz    plot
        pop    lfbWidth
        pop    edx
        pop    ebx
        add    edi, ebx
        add    esi, edx
        dec    lfbHeight
        jnz    next
    }
}

// Draw line functions
void HorizLine8(int x, int y, int sx, unsigned int col)
{
	//check for clip-y
	if (y > cmaxx || y < cminy) return;
	if (x > cmaxx || sx <= 0) return;

	// check clip boundary
	if (x < cminx)
	{
		// re-calculate sx
		sx -= (cminx - x) + 1;
		x = cminx;
	}

	// inbound check
	if (sx > cmaxx - x) sx = (cmaxx - x) + 1;
	if (sx <= 0) return;

    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, x
        add    eax, ebx
        mov    edi, lfbPtr
        add    edi, eax
        mov    edx, sx
        shr    edx, 2
        mov    ebx, sx
        and    ebx, 3
        mov    eax, col
        shl    eax, 8
        or     eax, col
        shl    eax, 8
        or     eax, col
        shl    eax, 8
        or     eax, col
        mov    ecx, edx
        rep    stosd
        mov    ecx, ebx
        rep    stosb
    }
}

void HorizLine16(int x, int y, int sx, unsigned int col)
{
	//check for clip-y
	if (y > cmaxx || y < cminy) return;
	if (x > cmaxx || sx <= 0) return;

	// check clip boundary
	if (x < cminx)
	{
		// re-calculate sx
		sx -= (cminx - x) + 1;
		x = cminx;
	}

	// inbound check
	if (sx > cmaxx - x) sx = (cmaxx - x) + 1;
	if (sx <= 0) return;

    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, x
        add    eax, x
        mov    edi, lfbPtr
        add    edi, eax
        mov    edx, sx
        shr    edx, 1
        mov    ebx, sx
        and    ebx, 1
        mov    eax, col
        shl    eax, 16
        or     eax, col
        mov    ecx, edx
        rep    stosd
        mov    ecx, ebx
        rep    stosw
    }
}

void HorizLine24(int x, int y, int sx, unsigned int col)
{
	//check for clip-y
	if (y > cmaxx || y < cminy) return;
	if (x > cmaxx || sx <= 0) return;

	// check clip boundary
	if (x < cminx)
	{
		// re-calculate sx
		sx -= (cminx - x) + 1;
		x = cminx;
	}

	// inbound check
	if (sx > cmaxx - x) sx = (cmaxx - x) + 1;
	if (sx <= 0) return;

    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, x
        shl    ebx, 1
        add    ebx, x
        add    eax, ebx
        mov    edi, lfbPtr
        add    edi, eax
        mov    ecx, sx
        mov    eax, col
    step:
        mov    [edi], eax
        add    edi, 3
        loop   step
    }
}

void HorizLine32(int x, int y, int sx, unsigned int col)
{
	//check for clip-y
	if (y > cmaxx || y < cminy) return;
	if (x > cmaxx || sx <= 0) return;

	// check clip boundary
	if (x < cminx)
	{
		// re-calculate sx
		sx -= (cminx - x) + 1;
		x = cminx;
	}

	// inbound check
	if (sx > cmaxx - x) sx = (cmaxx - x) + 1;
	if (sx <= 0) return;

    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, x
        shl    ebx, 2
        add    eax, ebx
        mov    edi, lfbPtr
        add    edi, eax
        mov    ecx, sx
        mov    eax, col
        rep    stosd
    }
}

void HorizLineAdd32(int x, int y, int sx, unsigned int col)
{
	//check for clip-y
	if (y > cmaxx || y < cminy) return;
	if (x > cmaxx || sx <= 0) return;

	// check clip boundary
	if (x < cminx)
	{
		// re-calculate sx
		sx -= (cminx - x) + 1;
		x = cminx;
	}

	// inbound check
	if (sx > cmaxx - x) sx = (cmaxx - x) + 1;
	if (sx <= 0) return;

    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, x
        shl    ebx, 2
        add    eax, ebx
        mov    edi, lfbPtr
        add    edi, eax
        mov    ecx, sx
    next:
        mov    eax, [edi]
        add    al, byte ptr [col]
        jnc    bstep
        mov    al, 255
    bstep:
        add    ah, byte ptr [col + 1]
        jnc    gstep
        mov    ah, 255
    gstep:
        mov    ebx, eax
        shr    ebx, 16
        add    bl, byte ptr [col + 2]
        jnc    rstep
        mov    bl, 255
    rstep:
        shl    ebx, 16
        and    eax, 00FFFFh
        or     eax, ebx
        stosd
        loop   next
    }
}

void HorizLineAdd24(int x, int y, int sx, unsigned int col)
{
	//check for clip-y
	if (y > cmaxx || y < cminy) return;
	if (x > cmaxx || sx <= 0) return;

	// check clip boundary
	if (x < cminx)
	{
		// re-calculate sx
		sx -= (cminx - x) + 1;
		x = cminx;
	}

	// inbound check
	if (sx > cmaxx - x) sx = (cmaxx - x) + 1;
	if (sx <= 0) return;

    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, x
        shl    ebx, 1
        add    ebx, x
        add    eax, ebx
        mov    edi, lfbPtr
        add    edi, eax
        mov    ecx, sx
    next:
        mov    ax, [edi]
        add    al, byte ptr [col]
        jnc    bstep
        mov    al, 255
    bstep:
        add    ah, byte ptr [col + 1]
        jnc    gstep
        mov    ah, 255
    gstep:
        mov    bl, [edi + 2]
        add    bl, byte ptr [col + 2]
        jnc    rstep
        mov    bl, 255
    rstep:
        stosw
        mov    al, bl
        stosb
        loop   next
    }
}

void HorizLineAdd16(int x, int y, int sx, unsigned int col)
{
	//check for clip-y
	if (y > cmaxx || y < cminy) return;
	if (x > cmaxx || sx <= 0) return;

	// check clip boundary
	if (x < cminx)
	{
		// re-calculate sx
		sx -= (cminx - x) + 1;
		x = cminx;
	}

	// inbound check
	if (sx > cmaxx - x) sx = (cmaxx - x) + 1;
	if (sx <= 0) return;

    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, x
        add    eax, x
        mov    edi, lfbPtr
        add    edi, eax
        mov    bx, word ptr [col]
        mov    dx, bx
        and    bx, 1111100000011111b
        and    dx, 11111100000b
    next:
        mov    ax, [edi]
        mov    cx, ax
        and    ax, 1111100000011111b
        and    cx, 11111100000b
        add    al, bl
        cmp    al, 11111b
        jna    bstep
        mov    al, 11111b
    bstep:
        add    ah, bh
        jnc    gstep
        mov    ah, 11111000b
    gstep:
        add    cx, dx
        cmp    cx, 11111100000b
        jna    rstep
        mov    cx, 11111100000b
    rstep:
        or     ax, cx
        stosw
        dec    sx
        jnz    next
    }
}

void HorizLineAdd15(int x, int y, int sx, unsigned int col)
{
	//check for clip-y
	if (y > cmaxx || y < cminy) return;
	if (x > cmaxx || sx <= 0) return;

	// check clip boundary
	if (x < cminx)
	{
		// re-calculate sx
		sx -= (cminx - x) + 1;
		x = cminx;
	}

	// inbound check
	if (sx > cmaxx - x) sx = (cmaxx - x) + 1;
	if (sx <= 0) return;

    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, x
        add    eax, x
        mov    edi, lfbPtr
        add    edi, eax
        mov    bx, word ptr [col]
        mov    dx, bx
        and    bx, 111110000011111b
        and    dx, 1111100000b
    next:
        mov    ax, [edi]
        mov    cx, ax
        and    ax, 111110000011111b
        and    cx, 1111100000b
        add    al, bl
        cmp    al, 11111b
        jna    bstep
        mov    al, 11111b
    bstep:
        add    ah, bh
        cmp    ah, 1111100b
        jna    gstep
        mov    ah, 1111100b
    gstep:
        add    cx, dx
        cmp    cx, 1111100000b
        jna    rstep
        mov    cx, 1111100000b
    rstep:
        or     ax, cx
        stosw
        dec    sx
        jnz    next
    }
}

void HorizLineSub32(int x, int y, int sx, unsigned int col)
{
	//check for clip-y
	if (y > cmaxx || y < cminy) return;
	if (x > cmaxx || sx <= 0) return;

	// check clip boundary
	if (x < cminx)
	{
		// re-calculate sx
		sx -= (cminx - x) + 1;
		x = cminx;
	}

	// inbound check
	if (sx > cmaxx - x) sx = (cmaxx - x) + 1;
	if (sx <= 0) return;

    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, x
        shl    ebx, 2
        add    eax, ebx
        mov    edi, lfbPtr
        add    edi, eax
        mov    ecx, sx
    next:
        mov    eax, [edi]
        sub    al, byte ptr [col]
        jnc    bstep
        xor    al, al
    bstep:
        sub    ah, byte ptr [col + 1]
        jnc    gstep
        xor    ah, ah
    gstep:
        mov    ebx, eax
        shr    ebx, 16
        sub    bl, byte ptr [col + 2]
        jnc    rstep
        xor    bl, bl
    rstep:
        shl    ebx, 16
        and    eax, 00FFFFh
        or     eax, ebx
        stosd
        loop   next
    }
}

void HorizLineSub24(int x, int y, int sx, unsigned int col)
{
	//check for clip-y
	if (y > cmaxx || y < cminy) return;
	if (x > cmaxx || sx <= 0) return;

	// check clip boundary
	if (x < cminx)
	{
		// re-calculate sx
		sx -= (cminx - x) + 1;
		x = cminx;
	}

	// inbound check
	if (sx > cmaxx - x) sx = (cmaxx - x) + 1;
	if (sx <= 0) return;

    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, x
        shl    ebx, 1
        add    ebx, x
        add    eax, ebx
        mov    edi, lfbPtr
        add    edi, eax
        mov    ecx, sx
    next:
        mov    ax, [edi]
        sub    al, byte ptr [col]
        jnc    bstep
        xor    al, al
    bstep:
        sub    ah, byte ptr [col + 1]
        jnc    gstep
        xor    ah, ah
    gstep:
        mov    bl, [edi + 2]
        sub    bl, byte ptr [col + 2]
        jnc    rstep
        xor    bl, bl
    rstep:
        stosw
        mov    al, bl
        stosb
        loop   next
    }
}

void HorizLineSub16(int x, int y, int sx, unsigned int col)
{
	//check for clip-y
	if (y > cmaxx || y < cminy) return;
	if (x > cmaxx || sx <= 0) return;

	// check clip boundary
	if (x < cminx)
	{
		// re-calculate sx
		sx -= (cminx - x) + 1;
		x = cminx;
	}

	// inbound check
	if (sx > cmaxx - x) sx = (cmaxx - x) + 1;
	if (sx <= 0) return;

    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, x
        add    eax, x
        mov    edi, lfbPtr
        add    edi, eax
        mov    bx, word ptr [col]
        mov    dx, bx
        and    bx, 1111100000011111b
        and    dx, 11111100000b
    next:
        mov    ax, [edi]
        mov    cx, ax
        and    ax, 1111100000011111b
        and    cx, 11111100000b
        sub    al, bl
        jnc    bstep
        xor    al, al
    bstep:
        sub    ah, bh
        jnc    gstep
        xor    ah, ah
    gstep:
        sub    cx, dx
        jc     rstep
        or     ax, cx
    rstep:      
        stosw
        dec    sx
        jnz    next
    }
}

void HorizLineSub15(int x, int y, int sx, unsigned int col)
{
	//check for clip-y
	if (y > cmaxx || y < cminy) return;
	if (x > cmaxx || sx <= 0) return;

	// check clip boundary
	if (x < cminx)
	{
		// re-calculate sx
		sx -= (cminx - x) + 1;
		x = cminx;
	}

	// inbound check
	if (sx > cmaxx - x) sx = (cmaxx - x) + 1;
	if (sx <= 0) return;

    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, x
        add    eax, x
        mov    edi, lfbPtr
        add    edi, eax
        mov    bx, word ptr [col]
        mov    dx, bx
        and    bx, 111110000011111b
        and    dx, 1111100000b
    next:
        mov    ax, [edi]
        mov    cx, ax
        and    ax, 111110000011111b
        and    cx, 1111100000b
        sub    al, bl
        jnc    bstep
        xor    al, al
    bstep:
        sub    ah, bh
        jnc    gstep
        xor    ah, ah
    gstep:
        sub    cx, dx
        jc     rstep
        or     ax, cx
    rstep:
        stosw
        dec    sx
        jnz    next
    }
}

void VertLine8(int x, int y, int sy, unsigned int col)
{
	//check for clip-x
	if (x > cmaxx || x < cminx) return;
	if (y > cmaxy || sy <= 0) return;

	if (y < cminy)
	{
		//re-calculate sy
		sy -= (cminy - y) + 1;
		y = cminy;
	}

	//inbound check
	if (sy > cmaxy - y) sy = (cmaxy - y) + 1;
	if (sy <= 0) return;

    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, x
        mov    edi, lfbPtr
        add    edi, eax
        mov    ecx, sy
        mov    ebx, bytesPerScanline
        sub    ebx, 1
        mov    al, byte ptr[col]
    next:
        stosb
        add    edi, ebx
        loop   next
    }
}

void VertLine16(int x, int y, int sy, unsigned int col)
{
	//check for clip-x
	if (x > cmaxx || x < cminx) return;
	if (y > cmaxy || sy <= 0) return;

	if (y < cminy)
	{
		//re-calculate sy
		sy -= (cminy - y) + 1;
		y = cminy;
	}

	//inbound check
	if (sy > cmaxy - y) sy = (cmaxy - y) + 1;
	if (sy <= 0) return;

    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, x
        add    eax, x
        mov    edi, lfbPtr
        add    edi, eax
        mov    ecx, sy
        mov    ebx, bytesPerScanline
        sub    ebx, 2
        mov    ax, word ptr[col]
    next:
        stosw
        add    edi, ebx 
        loop   next
    }
}

void VertLine24(int x, int y, int sy, unsigned int col)
{
	//check for clip-x
	if (x > cmaxx || x < cminx) return;
	if (y > cmaxy || sy <= 0) return;

	if (y < cminy)
	{
		//re-calculate sy
		sy -= (cminy - y) + 1;
		y = cminy;
	}

	//inbound check
	if (sy > cmaxy - y) sy = (cmaxy - y) + 1;
	if (sy <= 0) return;

    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, x
        shl    ebx, 1
        add    ebx, x
        add    eax, ebx
        mov    edi, lfbPtr
        add    edi, eax
        mov    ecx, sy
        mov    ebx, bytesPerScanline
        sub    ebx, 3
    next:
        mov    eax, col
        stosw
        shr    eax, 16
        stosb
        add    edi, ebx
        loop   next
    }
}

void VertLine32(int x, int y, int sy, unsigned int col)
{
	//check for clip-x
	if (x > cmaxx || x < cminx) return;
	if (y > cmaxy || sy <= 0) return;

	if (y < cminy)
	{
		//re-calculate sy
		sy -= (cminy - y) + 1;
		y = cminy;
	}

	//inbound check
	if (sy > cmaxy - y) sy = (cmaxy - y) + 1;
	if (sy <= 0) return;

    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, x
        shl    ebx, 2
        add    eax, ebx
        mov    edi, lfbPtr
        add    edi, eax
        mov    ecx, sy
        mov    ebx, bytesPerScanline
        sub    ebx, 4
        mov    eax, col
    next:
        stosd
        add    edi, ebx 
        loop   next
    }
}

void VertLineAdd32(int x, int y, int sy, unsigned int col)
{
 	//check for clip-x
	if (x > cmaxx || x < cminx) return;
	if (y > cmaxy || sy <= 0) return;

	if (y < cminy)
	{
		//re-calculate sy
		sy -= (cminy - y) + 1;
		y = cminy;
	}

	//inbound check
	if (sy > cmaxy - y) sy = (cmaxy - y) + 1;
	if (sy <= 0) return;

    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, x
        shl    ebx, 2
        add    eax, ebx
        mov    edi, lfbPtr
        add    edi, eax
        mov    ecx, sy
        mov    ebx, bytesPerScanline
        sub    ebx, 4
    next:
        mov    eax, [edi]
        add    al, byte ptr [col]
        jnc    bstep
        mov    al, 255
    bstep:
        add    ah, byte ptr [col + 1]
        jnc    gstep
        mov    ah, 255
    gstep:
        mov    edx, eax
        shr    edx, 16
        add    dl, byte ptr [col + 2]
        jnc    rstep
        mov    dl, 255
    rstep:
        shl    edx, 16
        and    eax, 00FFFFh
        or     eax, edx
        stosd 
        add    edi, ebx 
        loop   next
    }
}

void VertLineAdd24(int x, int y, int sy, unsigned int col)
{
	//check for clip-x
	if (x > cmaxx || x < cminx) return;
	if (y > cmaxy || sy <= 0) return;

	if (y < cminy)
	{
		//re-calculate sy
		sy -= (cminy - y) + 1;
		y = cminy;
	}

	//inbound check
	if (sy > cmaxy - y) sy = (cmaxy - y) + 1;
	if (sy <= 0) return;

    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, x
        shl    ebx, 1
        add    ebx, x
        add    eax, ebx
        mov    edi, lfbPtr
        add    edi, eax
        mov    ecx, sy
        mov    ebx, bytesPerScanline
        sub    ebx, 3
    next:
        mov    ax, [edi]
        add    al, byte ptr [col]
        jnc    bstep
        mov    al, 255
    bstep:
        add    ah, byte ptr [col + 1]
        jnc    gstep
        mov    ah, 255
    gstep:
        mov    dl, [edi + 2]
        add    dl, byte ptr [col + 2]
        jnc    rstep
        mov    dl, 255
    rstep:
        stosw
        mov    al, dl
        stosb
        add    edi, ebx 
        loop   next
    }
}

void VertLineAdd16(int x, int y, int sy, unsigned int col)
{
	//check for clip-x
	if (x > cmaxx || x < cminx) return;
	if (y > cmaxy || sy <= 0) return;

	if (y < cminy)
	{
		//re-calculate sy
		sy -= (cminy - y) + 1;
		y = cminy;
	}

	//inbound check
	if (sy > cmaxy - y) sy = (cmaxy - y) + 1;
	if (sy <= 0) return;

    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, x
        add    eax, x
        mov    edi, lfbPtr
        add    edi, eax
        mov    ebx, bytesPerScanline
        sub    ebx, 2
        push   ebx
        mov    bx, word ptr [col]
        mov    dx, bx
        and    bx, 1111100000011111b
        and    dx, 11111100000b
    next:
        mov    ax, [edi]
        mov    cx, ax
        and    ax, 1111100000011111b
        and    cx, 11111100000b
        add    al, bl
        cmp    al, 11111b
        jna    bstep
        mov    al, 11111b
    bstep:
        add    ah, bh
        jnc    gstep
        mov    ah, 11111000b
    gstep:
        add    cx, dx
        cmp    cx, 11111100000b
        jna    rstep
        mov    cx, 11111100000b
    rstep:
        or     ax, cx
        stosw
        add    edi, [esp]
        dec    sy 
        jnz    next
        pop    ebx
    }
}

void VertLineAdd15(int x, int y, int sy, unsigned int col)
{
	//check for clip-x
	if (x > cmaxx || x < cminx) return;
	if (y > cmaxy || sy <= 0) return;

	if (y < cminy)
	{
		//re-calculate sy
		sy -= (cminy - y) + 1;
		y = cminy;
	}

	//inbound check
	if (sy > cmaxy - y) sy = (cmaxy - y) + 1;
	if (sy <= 0) return;

    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, x
        add    eax, x
        mov    edi, lfbPtr
        add    edi, eax
        mov    ebx, bytesPerScanline
        sub    ebx, 2
        push   ebx
        mov    bx, word ptr [col]
        mov    dx, bx
        and    bx, 111110000011111b
        and    dx, 1111100000b
    next:
        mov    ax, [edi]
        mov    cx, ax
        and    ax, 111110000011111b
        and    cx, 1111100000b
        add    al, bl
        cmp    al, 11111b
        jna    bstep
        mov    al, 11111b
    bstep:
        add    ah, bh
        cmp    ah, 1111100b
        jna    gstep
        mov    ah, 1111100b
    gstep:
        add    cx, dx
        cmp    cx, 1111100000b
        jna    rstep
        mov    cx, 1111100000b
    rstep:
        or     ax, cx
        stosw
        add    edi, [esp]
        dec    sy 
        jnz    next
        pop    ebx
    }
}

void VertLineSub32(int x, int y, int sy, unsigned int col)
{
	//check for clip-x
	if (x > cmaxx || x < cminx) return;
	if (y > cmaxy || sy <= 0) return;

	if (y < cminy)
	{
		//re-calculate sy
		sy -= (cminy - y) + 1;
		y = cminy;
	}

	//inbound check
	if (sy > cmaxy - y) sy = (cmaxy - y) + 1;
	if (sy <= 0) return;

    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, x
        shl    ebx, 2
        add    eax, ebx
        mov    edi, lfbPtr
        add    edi, eax
        mov    ecx, sy
        mov    ebx, bytesPerScanline
        sub    ebx, 4
    next:
        mov    eax, [edi]
        sub    al, byte ptr [col]
        jnc    bstep
        xor    al, al
    bstep:
        sub    ah, byte ptr [col + 1]
        jnc    gstep
        xor    ah, ah
    gstep:
        mov    edx, eax
        shr    edx, 16
        sub    dl, byte ptr [col + 2]
        jnc    rstep
        xor    dl, dl
    rstep:
        shl    edx, 16
        and    eax, 00FFFFh
        or     eax, edx
        stosd 
        add    edi, ebx 
        loop   next
    }
}

void VertLineSub24(int x, int y, int sy, unsigned int col)
{
	//check for clip-x
	if (x > cmaxx || x < cminx) return;
	if (y > cmaxy || sy <= 0) return;

	if (y < cminy)
	{
		//re-calculate sy
		sy -= (cminy - y) + 1;
		y = cminy;
	}

	//inbound check
	if (sy > cmaxy - y) sy = (cmaxy - y) + 1;
	if (sy <= 0) return;

    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, x
        shl    ebx, 1
        add    ebx, x
        add    eax, ebx
        mov    edi, lfbPtr
        add    edi, eax
        mov    ecx, sy
        mov    ebx, bytesPerScanline
        sub    ebx, 3
    next:
        mov    ax, [edi]
        sub    al, byte ptr [col]
        jnc    bstep
        xor    al, al
    bstep:
        sub    ah, byte ptr [col + 1]
        jnc    gstep
        xor    ah, ah
    gstep:
        mov    dl, [edi + 2]
        sub    dl, byte ptr [col + 2]
        jnc    rstep
        xor    dl, dl
    rstep:
        stosw
        mov    al, dl
        stosb
        add    edi, ebx 
        loop   next
    }
}

void VertLineSub16(int x, int y, int sy, unsigned int col)
{
	//check for clip-x
	if (x > cmaxx || x < cminx) return;
	if (y > cmaxy || sy <= 0) return;

	if (y < cminy)
	{
		//re-calculate sy
		sy -= (cminy - y) + 1;
		y = cminy;
	}

	//inbound check
	if (sy > cmaxy - y) sy = (cmaxy - y) + 1;
	if (sy <= 0) return;

    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, x
        add    eax, x
        mov    edi, lfbPtr
        add    edi, eax
        mov    ebx, bytesPerScanline
        sub    ebx, 2
        push   ebx
        mov    bx, word ptr [col]
        mov    dx, bx
        and    bx, 1111100000011111b
        and    dx, 11111100000b
    next:
        mov    ax, [edi]
        mov    cx, ax
        and    ax, 1111100000011111b
        and    cx, 11111100000b
        sub    al, bl
        jnc    bstep
        xor    al, al
    bstep:
        sub    ah, bh
        jnc    gstep
        xor    ah, ah
    gstep:
        sub    cx, dx
        jc     rstep
        or     ax, cx
    rstep:
        stosw
        add    edi, [esp]
        dec    sy 
        jnz    next
        pop    ebx
    }
}

void VertLineSub15(int x, int y, int sy, unsigned int col)
{
	//check for clip-x
	if (x > cmaxx || x < cminx) return;
	if (y > cmaxy || sy <= 0) return;

	if (y < cminy)
	{
		//re-calculate sy
		sy -= (cminy - y) + 1;
		y = cminy;
	}

	//inbound check
	if (sy > cmaxy - y) sy = (cmaxy - y) + 1;
	if (sy <= 0) return;
    
    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        add    eax, x
        add    eax, x
        mov    edi, lfbPtr
        add    edi, eax
        mov    ebx, bytesPerScanline
        sub    ebx, 2
        push   ebx
        mov    bx, word ptr [col]
        mov    dx, bx
        and    bx, 111110000011111b
        and    dx, 1111100000b
    next:
        mov    ax, [edi]
        mov    cx, ax
        and    ax, 111110000011111b
        and    cx, 1111100000b
        sub    al, bl
        jnc    bstep
        xor    al, al
    bstep:
        sub    ah, bh
        jnc    gstep
        xor    ah, ah
    gstep:
        sub    cx, dx
        jc     rstep
        or     ax, cx
    rstep:
        stosw
        add    edi, [esp]
        dec    sy 
        jnz    next
        pop    ebx
    }
}

// Smooth scaling with Bresenham (internal function)
// Because of several simplifications of the algorithm,
// the zoom range is restricted between 0.5 and 2. That
// is: dstwidth must be >= srcwidth/2 and <= 2*srcwidth.
// smooth is used to calculate average pixel and mid-point
void ScaleLine8(void *dst, void *src, int dw, int sw, int smooth)
{
    unsigned short val = 0;

    if (smooth)
    {
        _asm {
            pusha
            mov    ebx, dw
            mov    eax, dw
            cmp    eax, sw
            jna    start
            dec    ebx
        start:
            mov    esi, src
            mov    edi, dst
            mov    ecx, ebx
            xor    ebx, ebx
            mov    edx, dw
            shr    edx, 1
        next:
            cmp    ebx, edx
            jnae   quit
            mov    al, [esi]
            mov    val, ax
            mov    al, [esi + 1]
            add    ax, val
            shr    ax, 1
            stosb
            jmp    skip
        quit:
            mov    al, [esi]
            stosb
        skip:
            add    ebx, sw
            cmp    ebx, dw
            jnae   cycle
            sub    ebx, dw
            inc    esi
        cycle:
            loop   next
            mov    eax, dw
            cmp    eax, sw
            jna    end
            movsb
        end:
            popa
        }
    }
    else
    {
        _asm {
            pusha
            xor    edx, edx
            mov    esi, src
            mov    edi, dst
            mov    eax, sw
            mov    ebx, dw
            div    ebx
            xor    ebx, ebx
            mov    ecx, dw
        next:
            movsb
            dec    esi
            add    esi, eax
            add    ebx, edx
            cmp    ebx, dw
            jnae   quit
            sub    ebx, dw
            inc    esi
        quit:
            loop   next
            popa
        }
    }
}

void ScaleLine16(void *dst, void *src, int dw, int sw, int smooth)
{
    if (smooth)
    {
        _asm {
            pusha
            mov    ebx, dw
            mov    eax, dw
            cmp    eax, sw
            jna    start
            dec    ebx
        start:
            mov    esi, src
            mov    edi, dst
            mov    ecx, ebx
            xor    ebx, ebx
            mov    edx, dw
            shr    edx, 1
        next:
            cmp    ebx, edx
            jnae   quit
            mov    ax, [esi]
            add    ax, [esi + 2]
            shr    ax, 1
            stosw
            jmp    skip
        quit:
            mov    ax, [esi]
            stosw
        skip:
            add    ebx, sw
            cmp    ebx, dw
            jnae   cycle
            sub    ebx, dw
            add    esi, 2
        cycle:
            loop   next
            mov    eax, dw
            cmp    eax, sw
            jna    end
            movsw
        end:
            popa
        }
    }
    else
    {
        _asm {
            pusha
            xor    edx, edx
            mov    esi, src
            mov    edi, dst
            mov    eax, sw
            mov    ebx, dw
            div    ebx
            shl    eax, 1
            xor    ebx, ebx
            mov    ecx, dw
        next:
            movsw
            sub    esi, 2
            add    esi, eax
            add    ebx, edx
            cmp    ebx, dw
            jnae   quit
            sub    ebx, dw
            add    esi, 2
        quit:
            loop   next
            popa
        }
    }
}

void ScaleLine24(void *dst, void *src, int dw, int sw, int smooth)
{
    unsigned short val = 0;

    if (smooth)
    {
        _asm {
            pusha
            mov    ebx, dw
            mov    eax, dw
            cmp    eax, sw
            jna    start
            dec    ebx
        start:
            mov    esi, src
            mov    edi, dst
            mov    ecx, ebx
            xor    ebx, ebx
            mov    edx, dw
            shr    edx, 1
        next:
            cmp    ebx, edx
            jnae   quit
            mov    al, [esi]
            mov    val, ax
            mov    al, [esi + 4]
            add    ax, val
            shr    ax, 1
            stosb
            mov    al, [esi + 1]
            mov    val, ax
            mov    al, [esi + 5]
            add    ax, val
            shr    ax, 1
            stosb
            mov    al, [esi + 2]
            mov    val, ax
            mov    al, [esi + 6]
            add    ax, val
            shr    ax, 1
            stosb
            jmp    skip
        quit:
            mov    eax, [esi]
            stosw
            shr    eax, 16
            stosb
        skip:
            add    ebx, sw
            cmp    ebx, dw
            jnae   cycle
            sub    ebx, dw
            add    esi, 3
        cycle:
            loop   next
            mov    eax, dw
            cmp    eax, sw
            jna    end
            movsw
            movsb
        end:
            popa
        }
    }
    else
    {
        _asm {
            pusha
            xor    edx, edx
            mov    esi, src
            mov    edi, dst
            mov    eax, sw
            mov    ebx, dw
            div    ebx
            xor    ebx, ebx
            mov    ecx, dw
        next:
            movsw
            movsb
            sub    esi, 3
            add    esi, eax
            add    ebx, edx
            cmp    ebx, dw
            jnae   quit
            sub    ebx, dw
            add    esi, 3
        quit:
            loop   next
            popa
        }
    }
}

// Coarse scaling with Bresenham (internal function)
void ScaleLine32(void *dst, void *src, int dw, int sw, int smooth)
{
    unsigned short val = 0;

    if (smooth)
    {
        _asm {
            pusha
            mov    ebx, dw
            mov    eax, dw
            cmp    eax, sw
            jna    start
            dec    ebx
        start:
            mov    esi, src
            mov    edi, dst
            mov    ecx, ebx
            xor    ebx, ebx
            mov    edx, dw
            shr    edx, 1
        next:
            cmp    ebx, edx
            jnae   quit
            mov    al, [esi]
            mov    val, ax
            mov    al, [esi + 4]
            add    ax, val
            shr    ax, 1
            stosb
            mov    al, [esi + 1]
            mov    val, ax
            mov    al, [esi + 5]
            add    ax, val
            shr    ax, 1
            stosb
            mov    al, [esi + 2]
            mov    val, ax
            mov    al, [esi + 6]
            add    ax, val
            shr    ax, 1
            stosb
            inc    edi
            jmp    skip
        quit:
            mov    eax, [esi]
            stosd
        skip:
            add    ebx, sw
            cmp    ebx, dw
            jnae   cycle
            sub    ebx, dw
            add    esi, 4
        cycle:
            loop   next
            mov    eax, dw
            cmp    eax, sw
            jna    end
            movsd
        end:
            popa
        }
    }
    else
    {
        _asm {
            pusha
            xor    edx, edx
            mov    esi, src
            mov    edi, dst
            mov    eax, sw
            mov    ebx, dw
            div    ebx
            shl    eax, 2
            xor    ebx, ebx
            mov    ecx, dw
        next:
            movsd
            sub    esi, 4
            add    esi, eax
            add    ebx, edx
            cmp    ebx, dw
            jnae   quit
            sub    ebx, dw
            add    esi, 4
        quit:
            loop   next
            popa
        }
    }
}

// Scale image using Bresenham algorithm 
void ScaleImage8(IMAGE *dst, IMAGE *src, int smooth)
{
    // save local value
    unsigned int dstWidth  = dst->mWidth;
    unsigned int srcWidth  = src->mWidth;
    unsigned int dstHeight = dst->mHeight;
    unsigned int srcHeight = src->mHeight;
    unsigned int intp = 0, modp = 0;

    // make pointer to call in asm
    void *memcpy = CopyData;
    void *scaler = ScaleLine8;

    // save local address
    void *oldPtr = NULL;
    void *dstPtr = dst->mData;
    void *srcPtr = src->mData;

    _asm {
        mov    edi, dstPtr
        mov    esi, srcPtr
        xor    edx, edx
        mov    eax, srcHeight
        div    dstHeight
        mov    modp, edx
        mul    srcWidth
        mov    intp, eax
        xor    ebx, ebx
        mov    ecx, dstHeight
    next:
        cmp    esi, oldPtr
        jne    skip
        mov    eax, edi
        sub    eax, dstWidth
        push   dstWidth
        push   eax
        push   edi
        call   memcpy
        jmp    quit
    skip:
        mov    edx, smooth
        and    edx, 0Fh
        push   edx
        push   srcWidth
        push   dstWidth
        push   esi
        push   edi
        call   scaler
        mov    oldPtr, esi
    quit:
        add    edi, dstWidth
        add    esi, intp
        add    ebx, modp
        cmp    ebx, dstHeight
        jnae   cycle
        sub    ebx, dstHeight
        add    esi, srcWidth
    cycle:
        loop   next
    }
}

void ScaleImage16(IMAGE *dst, IMAGE *src, int smooth)
{
    // save local value
    unsigned int dstWidth  = dst->mWidth;
    unsigned int srcWidth  = src->mWidth;
    unsigned int dstHeight = dst->mHeight;
    unsigned int srcHeight = src->mHeight;

    unsigned int dsi = 0, ddi = 0;
    unsigned int intp = 0, modp = 0;

    // make pointer to call in asm
    void *memcpy = CopyData;
    void *scaler = ScaleLine16;

    // save local address
    void *oldPtr = NULL;
    void *dstPtr = dst->mData;
    void *srcPtr = src->mData;

    _asm {
        mov    edi, dstPtr
        mov    esi, srcPtr
        xor    edx, edx
        mov    eax, srcHeight
        div    dstHeight
        mov    modp, edx
        mul    srcWidth
        shl    eax, 1
        mov    intp, eax
        mov    eax, dstWidth
        shl    eax, 1
        mov    ddi, eax
        mov    eax, srcWidth
        shl    eax, 1
        mov    dsi, eax
        xor    ebx, ebx
        mov    ecx, dstHeight
    next:
        cmp    esi, oldPtr
        jne    skip
        mov    eax, edi
        sub    eax, ddi
        push   ddi
        push   eax
        push   edi
        call   memcpy
        jmp    quit
    skip:
        mov    edx, smooth
        and    edx, 0Fh
        push   edx
        push   srcWidth
        push   dstWidth
        push   esi
        push   edi
        call   scaler
        mov    oldPtr, esi
    quit:
        add    edi, ddi
        add    esi, intp
        add    ebx, modp
        cmp    ebx, dstHeight
        jnae   cycle
        sub    ebx, dstHeight
        add    esi, dsi
    cycle:
        loop   next
    }
}

void ScaleImage24(IMAGE *dst, IMAGE *src, int smooth)
{
    // save local value
    unsigned int dstWidth  = dst->mWidth;
    unsigned int srcWidth  = src->mWidth;
    unsigned int dstHeight = dst->mHeight;
    unsigned int srcHeight = src->mHeight;

    unsigned int dsi = 0, ddi = 0;
    unsigned int intp = 0, modp = 0;

    // make pointer to call in asm
    void *memcpy = CopyData;
    void *scaler = ScaleLine24;

    // save local address
    void *oldPtr = NULL;
    void *dstPtr = dst->mData;
    void *srcPtr = src->mData;

    _asm {
        mov    edi, dstPtr
        mov    esi, srcPtr
        xor    edx, edx
        mov    eax, srcHeight
        div    dstHeight
        mov    modp, edx
        mul    srcWidth
        shl    eax, 1
        add    eax, srcWidth
        mov    intp, eax
        mov    eax, dstWidth
        shl    eax, 1
        add    eax, dstWidth
        mov    ddi, eax
        mov    eax, srcWidth
        shl    eax, 1
        add    eax, srcWidth
        mov    dsi, eax
        xor    ebx, ebx
        mov    ecx, dstHeight
    next:
        cmp    esi, oldPtr
        jne    skip
        mov    eax, edi
        sub    eax, ddi
        push   ddi
        push   eax
        push   edi
        call   memcpy
        jmp    quit
    skip:
        mov    edx, smooth
        and    edx, 0Fh
        push   edx
        push   srcWidth
        push   dstWidth
        push   esi
        push   edi
        call   scaler
        mov    oldPtr, esi
    quit:
        add    edi, ddi
        add    esi, intp
        add    ebx, modp
        cmp    ebx, dstHeight
        jnae   cycle
        sub    ebx, dstHeight
        add    esi, dsi
    cycle:
        loop   next
    }
}

// Scale image using Bresenham algorithm 
void ScaleImage32(IMAGE *dst, IMAGE *src, int smooth)
{
    // save local value
    unsigned int dstWidth  = dst->mWidth;
    unsigned int srcWidth  = src->mWidth;
    unsigned int dstHeight = dst->mHeight;
    unsigned int srcHeight = src->mHeight;

    unsigned int dsi = 0, ddi = 0;
    unsigned int intp = 0, modp = 0;

    // make pointer to call in asm
    void *mcopy = CopyData;
    void *scaler = ScaleLine32;

    // save local address
    void *oldPtr = NULL;
    void *dstPtr = dst->mData;
    void *srcPtr = src->mData;

    _asm {
        mov    edi, dstPtr
        mov    esi, srcPtr
        xor    edx, edx
        mov    eax, srcHeight
        div    dstHeight
        mov    modp, edx
        mul    srcWidth
        shl    eax, 2
        mov    intp, eax
        mov    eax, dstWidth
        shl    eax, 2
        mov    ddi, eax
        mov    eax, srcWidth
        shl    eax, 2
        mov    dsi, eax
        xor    ebx, ebx
        mov    ecx, dstHeight
    next:
        cmp    esi, oldPtr
        jne    skip
        mov    eax, edi
        sub    eax, ddi
        push   ddi
        push   eax
        push   edi
        call   mcopy
        jmp    quit
    skip:
        mov    edx, smooth
        and    edx, 0Fh
        push   edx
        push   srcWidth
        push   dstWidth
        push   esi
        push   edi
        call   scaler
        mov    oldPtr, esi
    quit:
        add    edi, ddi
        add    esi, intp
        add    ebx, modp
        cmp    ebx, dstHeight
        jnae   cycle
        sub    ebx, dstHeight
        add    esi, dsi
    cycle:
        loop   next
    }
}

// Bi-linear resize image, this only work with RGB color mode
// Optimize version using integer, this is not fully optimize
void BilinearScaleImage(IMAGE *dst, IMAGE *src)
{
    unsigned int x, y;
    unsigned int *dstp = (unsigned int*)dst->mData;
    const unsigned int *srcp = (const unsigned int*)src->mData;

    // calculate ratio
    const unsigned int ws = ((src->mWidth - 1) << 16) / (dst->mWidth - 1);
    const unsigned int hs = ((src->mHeight - 1) << 16) / (dst->mHeight - 1);

    unsigned int hc0 = 0;
    for (y = 0; y != dst->mHeight; y++, hc0 += hs)
    {
        // calculate height color
        unsigned int wc0 = 0;
        const unsigned int ofsy = (hc0 >> 16) * src->mWidth;
        const unsigned int hc2  = (hc0 >> 9) & 127;
        const unsigned int hc1  = (128 - hc2);

        for (x = 0; x != dst->mWidth; x++, wc0 += ws)
        {
            // calculate width color
            const unsigned int ofsx = (wc0 >> 16);
            const unsigned int wc2  = (wc0 >> 9) & 127;
            const unsigned int wc1  = (128 - wc2);
            const unsigned int ofs1 = ofsx + ofsy;
            const unsigned int ofs2 = ofs1 + src->mWidth;

            // load four pixels
            const RGBA *px1 = (const RGBA*)(srcp + ofs1);
            const RGBA *px2 = (const RGBA*)(srcp + ofs2);
            const RGBA *px3 = (const RGBA*)(srcp + ofs1 + 1);
            const RGBA *px4 = (const RGBA*)(srcp + ofs2 + 1);

            // calculate weight of RGB
            const unsigned int outr = ((px1->r * hc1 + px2->r * hc2) * wc1 + (px3->r * hc1 + px4->r * hc2) * wc2) >> 14;
            const unsigned int outg = ((px1->g * hc1 + px2->g * hc2) * wc1 + (px3->g * hc1 + px4->g * hc2) * wc2) >> 14;
            const unsigned int outb = ((px1->b * hc1 + px2->b * hc2) * wc1 + (px3->b * hc1 + px4->b * hc2) * wc2) >> 14;

            // put pixel to destination
            *dstp++ = (outr << 16) | (outg << 8) | (outb);
        }
    }
}

// get bilinear pixel from render buffer, using FIXED-POINT
unsigned int GetBilinearPixel(const unsigned int *image, float x, float y, int width) 
{
    const unsigned int shift = 8;           // shift can have values 8 to 16
    const unsigned int fixed = 1 << shift;  // calculate fixed point

    const unsigned int dx = (x * fixed);    // convert to fixed
    const unsigned int dy = (y * fixed);    // convert to fixed
    const unsigned int px = (dx & -fixed) >> shift; // floor of x
    const unsigned int py = (dy & -fixed) >> shift; // floor of y
    
    // pointer to first pixel
    const RGBA *p0 = (const RGBA*)(image + px + py * width);

    // load the four neighboring pixels
    const RGBA *p1 = (const RGBA*)(p0);
    const RGBA *p2 = (const RGBA*)(p0 + 1);
    const RGBA *p3 = (const RGBA*)(p0 + width);
    const RGBA *p4 = (const RGBA*)(p0 + width + 1);

    // calculate the weights for each pixel
    const unsigned int fx  = dx & (fixed - 1);
    const unsigned int fy  = dy & (fixed - 1);
    const unsigned int fx1 = fixed - fx;
    const unsigned int fy1 = fixed - fy;

    const unsigned int w1 = (fx1 * fy1) >> shift;
    const unsigned int w2 = (fx  * fy1) >> shift;
    const unsigned int w3 = (fx1 * fy ) >> shift;
    const unsigned int w4 = (fx  * fy ) >> shift;

    // calculate the weighted sum of pixels (for each color channel)
    const unsigned int outr = (p1->r * w1 + p2->r * w2 + p3->r * w3 + p4->r * w4) >> shift;
    const unsigned int outg = (p1->g * w1 + p2->g * w2 + p3->g * w3 + p4->g * w4) >> shift;
    const unsigned int outb = (p1->b * w1 + p2->b * w2 + p3->b * w3 + p4->b * w4) >> shift;
    return (outr << 16) | (outg << 8) | (outb);
}

// bilinear image rotation (optimize version using FIXED POINT)
void BilinearRotateImage(IMAGE *dst, IMAGE *src, int angle)
{
    int x, y;

    // cast to image data
    unsigned int *pdst = (unsigned int*)dst->mData;
    const unsigned int *psrc = (const unsigned int*)src->mData;

    // calculate haft dimension
    const unsigned int tx = src->mWidth >> 1;
    const unsigned int ty = src->mHeight >> 1;
    
    // convert to radian
    const float theta = angle * M_PI / 180;
    const float sina = sin(-theta);
    const float cosa = cos(-theta);

    // start pixel mapmulation
    int cy = -ty;
    for (y = 0; y != src->mHeight; y++, cy++)
    {
        int cx = -tx;
        for (x = 0; x != src->mWidth; x++, cx++)
        {
            const float sx = cx * cosa - cy * sina + tx;
            const float sy = cx * sina + cy * cosa + ty;
            if (sx >= 0 && sx < src->mWidth - 1 && sy >= 0 && sy < src->mHeight - 1) *pdst++ = GetBilinearPixel(psrc, sx, sy, src->mWidth);
            else pdst++;
        }
    }
}

// fade image effects
void FadeOutImage32(IMAGE *img, unsigned char step)
{
    void *data = img->mData;
    unsigned int size = img->mSize >> 2;

    _asm {
        mov    edi, data
        mov    ecx, size
    next:
        mov    eax, [edi]
        sub    al, step
        jnc    bstep
        xor    al, al
    bstep:
        sub    ah, step
        jnc    gstep
        xor    ah, ah
    gstep:
        mov    ebx, eax
        shr    ebx, 16
        sub    bl, step
        jnc    rstep
        xor    bl, bl
    rstep:
        shl    ebx, 16
        and    eax, 00FFFFh
        or     eax, ebx
        stosd
        loop   next
    }
}

void FadeOutImage24(IMAGE *img, unsigned char step)
{
    void *data = img->mData;
    unsigned int size = img->mSize / 3;

    _asm {
        mov    edi, data
        mov    ecx, size
    next:
        mov    ax, [edi]
        sub    al, step
        jnc    bstep
        xor    al, al
    bstep:
        sub    ah, step
        jnc    gstep
        xor    ah, ah
    gstep:
        mov    bl, [edi + 2]
        sub    bl, step
        jnc    rstep
        xor    bl, bl
    rstep:
        stosw
        mov    al, bl
        stosb  
        loop   next
    }
}

void FadeOutImage16(IMAGE *img, unsigned char step)
{
    void *data = img->mData;
    unsigned int size = img->mSize >> 1;

    _asm {
        mov    edi, data
        mov    ecx, size
    next:
        mov    ax, [edi]
        mov    bx, ax
        and    ax, 1111100000011111b
        and    bx, 11111100000b
        sub    al, step
        jnc    bstep
        xor    al, al
    bstep:
        shr    ah, 3
        sub    ah, step
        jnc    gstep
        xor    ah, ah
    gstep:
        shr    bx, 5
        sub    bl, step
        jnc    rstep
        xor    bl, bl
    rstep:
        shl    ah, 3
        shl    bx, 5
        or     ax, bx
        stosw
        loop   next
    }
}

void FadeOutImage15(IMAGE *img, unsigned char step)
{
    void *data = img->mData;
    unsigned int size = img->mSize >> 1;

    _asm {
        mov    edi, data
        mov    ecx, size
    next:
        mov    ax, [edi]
        mov    bx, ax
        and    ax, 111110000011111b
        and    bx, 1111100000b
        sub    al, step
        jnc    bstep
        xor    al, al
    bstep:
        shr    ah, 2
        sub    ah, step
        jnc    gstep
        xor    ah, ah
    gstep:
        shr    bx, 5
        sub    bl, step
        jnc    rstep
        xor    bl, bl
    rstep:
        shl    ah, 2
        shl    bx, 5
        or     ax, bx
        stosw
        loop   next
    }
}

// Initialize VESA mode by resolution and bits per pixel
int SetVesaMode(int px, int py, unsigned char bits, unsigned int refreshRate)
{
    RM_REGS             regs;
    VBE_DRIVER_INFO     drvInfo;
    VBE_MODE_INFO       modeInfo;
    VBE_CRTC_INFO_BLOCK *CRTCPtr;

    unsigned short      *modePtr;
    unsigned long       pixelClock;
    unsigned int        lineWidth;

    // Check VBE driver info
    memset(&drvInfo, 0, sizeof(drvInfo));
    if (!GetVesaDriverInfo(&drvInfo)) return 0;

    // Catch mode number info in VBE mode info block
    modePtr = (unsigned short*)drvInfo.VideoModePtr;
    if (modePtr == NULL) return 0;

    // Find VESA mode match with request
    while (modePtr != NULL && *modePtr != 0xFFFF)
    {
        // Get current mode info
        memset(&modeInfo, 0, sizeof(VBE_MODE_INFO));
        if (GetVesaModeInfo(*modePtr, &modeInfo) && (px == modeInfo.XResolution) && (py == modeInfo.YResolution) && (bits == modeInfo.BitsPerPixel)) break;
        modePtr++;
    }

    // Is valid mode number?
    if (*modePtr == 0xFFFF) return 0;

    // setup init VESA function
    memset(&regs, 0, sizeof(regs));
    regs.eax = 0x4F02;
    regs.ebx = *modePtr | 0x4000; // Add linear frame buffer param D14

    // Check if request refresh rate and must be VBE 3.0
    if (refreshRate >= 50 && drvInfo.VBEVersion >= 0x0300)
    {
        // Setup memory to passing VBE
        if (crtcSegment == 0) crtcSegment = AllocDosSegment(512);
        if (crtcSegment == 0 || crtcSegment == 0xFFFF) return 0;

        // Calculate CRTC timing using GTF formular
        CRTCPtr = (VBE_CRTC_INFO_BLOCK*)((crtcSegment & 0x0000FFFF) << 4);
        memset(CRTCPtr, 0, sizeof(VBE_CRTC_INFO_BLOCK));
        CalcCrtcTimingGTF(CRTCPtr, px, py, refreshRate << 10, 0, 0);

        // Calculate actual pixel clock
        pixelClock = GetClosestPixelClock(*modePtr, CRTCPtr->PixelClock);
        if (pixelClock > 0)
        {
            // Re-calculate pixel clock and refresh rate
            CRTCPtr->PixelClock = pixelClock;
            CRTCPtr->RefreshRate = 100 * (CRTCPtr->PixelClock / (CRTCPtr->HorizontalTotal * CRTCPtr->VerticalTotal));

            // D15=don't clear screen, D14=linear/flat buffer, D11=Use user specified CRTC values for refresh rate
            regs.ebx |= 0x0800;
            regs.es  = crtcSegment;
            regs.edi = 0;
        }
    }

    // Start init VESA mode
    SimRealModeInt(0x10, &regs);
    if (regs.eax != 0x004F) return 0;

    // Calculate linear address size and map physical address to linear address
    lfbSize = modeInfo.YResolution * modeInfo.BytesPerScanline;
    if (!(lfbPtr = (unsigned char*)MapPhysicalAddress(modeInfo.PhysBasePtr, lfbSize))) return 0;

    // Only DIRECT COLOR and PACKED PIXEL mode are allowed
    if (modeInfo.MemoryModel == VBE_MM_DCOLOR || modeInfo.MemoryModel == VBE_MM_PACKED)
    {
        // Setup protect mode interface to use SetDisplayStart function
        GetProtectModeFunctions();

        // Only for VBE 3.0
        if (drvInfo.VBEVersion >= 0x0300)
        {
            // Calculate mask for RGB
            rmask = ((1UL << modeInfo.LinRedMaskSize) - 1) << modeInfo.LinRedFieldPosition;
            gmask = ((1UL << modeInfo.LinGreenMaskSize) - 1) << modeInfo.LinGreenFieldPosition;
            bmask = ((1UL << modeInfo.LinBlueMaskSize) - 1) << modeInfo.LinBlueFieldPosition;

            // Calculate RGB shifter
            rshift = 8 - modeInfo.LinRedMaskSize;
            gshift = 8 - modeInfo.LinGreenMaskSize;
            bshift = 8 - modeInfo.LinBlueMaskSize;

            // Save RGB position
            rpos = modeInfo.LinRedFieldPosition;
            gpos = modeInfo.LinGreenFieldPosition;
            bpos = modeInfo.LinBlueFieldPosition;

            // Save line size (in bytes)
            bytesPerScanline = modeInfo.LinBytesPerScanline;

            // Number of visual screen pages
            numOfPages = modeInfo.LinNumberOfImagePages;
        }
        else
        {
            // Calculate mask for RGB
            rmask = ((1UL << modeInfo.RedMaskSize) - 1) << modeInfo.RedFieldPosition;
            gmask = ((1UL << modeInfo.GreenMaskSize) - 1) << modeInfo.GreenFieldPosition;
            bmask = ((1UL << modeInfo.BlueMaskSize) - 1) << modeInfo.BlueFieldPosition;

            // Calculate RGB shifter
            rshift = 8 - modeInfo.RedMaskSize;
            gshift = 8 - modeInfo.GreenMaskSize;
            bshift = 8 - modeInfo.BlueMaskSize;

            // Save RGB position
            rpos = modeInfo.RedFieldPosition;
            gpos = modeInfo.GreenFieldPosition;
            bpos = modeInfo.BlueFieldPosition;

            // Save line size (in bytes)
            bytesPerScanline = modeInfo.BytesPerScanline;

            // Number of visual screen pages
            numOfPages = modeInfo.NumberOfImagePages;
        }

        // Save bits per pixels
        bitsPerPixel = modeInfo.BitsPerPixel;
        bytesPerPixel = (bitsPerPixel + 7) >> 3;

        // Save x, y resolution
        xres = modeInfo.XResolution;
        yres = modeInfo.YResolution;

        // Save maximun x, y and center points
        maxx = xres - 1;
        maxy = yres - 1;

        centerx = xres >> 1;
        centery = yres >> 1;

        // Check for logical width to increasing
        lineWidth = max(bytesPerScanline, xres * bytesPerPixel);
        if (lineWidth > bytesPerScanline)
        {
            memset(&regs, 0, sizeof(regs));
            regs.eax = 0x4F06;
            regs.ebx = 0;
            regs.ecx = lineWidth / bytesPerPixel;
            SimRealModeInt(0x10, &regs);
            if (regs.eax != 0x004F || lineWidth > regs.ebx) return 0;
        }

        // Mapping functions pointer
        switch (bitsPerPixel)
        {
        case 8:
            PutPixel            = PutPixel8;
            GetPixel            = GetPixel8;
            FillRect            = FillRect8;
            HorizLine           = HorizLine8;
            VertLine            = VertLine8;
            GetImage            = GetImage8;
            PutImage            = PutImage8;
            PutSprite           = PutSprite8;
            ClearScreen         = ClearScreen8;
            ScaleImage          = ScaleImage8;
            FillRectPattern     = FillRectPattern8;
            break;

        case 15:
            PutPixel            = PutPixel16;
            PutPixelAdd         = PutPixelAdd15;
            PutPixelSub         = PutPixelSub15;
            GetPixel            = GetPixel16;
            FillRect            = FillRect16;
            FillRectAdd         = FillRectAdd15;
            FillRectSub         = FillRectSub15;
            HorizLine           = HorizLine16;
            HorizLineAdd        = HorizLineAdd15;
            HorizLineSub        = HorizLineSub15;
            VertLine            = VertLine16;
            VertLineAdd         = VertLineAdd15;
            VertLineSub         = VertLineSub15;
            GetImage            = GetImage16;
            PutImage            = PutImage16;
            PutImageAdd         = PutImageAdd15;
            PutImageSub         = PutImageSub15;
            PutSprite           = PutSprite16;
            PutSpriteAdd        = PutSpriteAdd15;
            PutSpriteSub        = PutSpriteSub15;
            ClearScreen         = ClearScreen16;
            ScaleImage          = ScaleImage16;
            FadeOutImage        = FadeOutImage15;
            FillRectPattern     = FillRectPattern16;
            FillRectPatternAdd  = FillRectPatternAdd15;
            FillRectPatternSub  = FillRectPatternSub15;
            break;

        case 16:
            PutPixel            = PutPixel16;
            PutPixelAdd         = PutPixelAdd16;
            PutPixelSub         = PutPixelSub16;
            GetPixel            = GetPixel16;
            FillRect            = FillRect16;
            FillRectAdd         = FillRectAdd16;
            FillRectSub         = FillRectSub16;
            HorizLine           = HorizLine16;
            HorizLineAdd        = HorizLineAdd16;
            HorizLineSub        = HorizLineSub16;
            VertLine            = VertLine16;
            VertLineAdd         = VertLineAdd16;
            VertLineSub         = VertLineSub16;
            GetImage            = GetImage16;
            PutImage            = PutImage16;
            PutImageAdd         = PutImageAdd16;
            PutImageSub         = PutImageSub16;
            PutSprite           = PutSprite16;
            PutSpriteAdd        = PutSpriteAdd16;
            PutSpriteSub        = PutSpriteSub16;
            ClearScreen         = ClearScreen16;
            ScaleImage          = ScaleImage16;
            FadeOutImage        = FadeOutImage16;
            FillRectPattern     = FillRectPattern16;
            FillRectPatternAdd  = FillRectPatternAdd16;
            FillRectPatternSub  = FillRectPatternSub16;
            break;

        case 24:
            PutPixel            = PutPixel24;
            PutPixelAdd         = PutPixelAdd24;
            PutPixelSub         = PutPixelSub24;
            GetPixel            = GetPixel24;
            FillRect            = FillRect24;
            FillRectAdd         = FillRectAdd24;
            FillRectSub         = FillRectSub24;
            HorizLine           = HorizLine24;
            HorizLineAdd        = HorizLineAdd24;
            HorizLineSub        = HorizLineSub24;
            VertLine            = VertLine24;
            VertLineAdd         = VertLineAdd24;
            VertLineSub         = VertLineSub24;
            GetImage            = GetImage24;
            PutImage            = PutImage24;
            PutImageAdd         = PutImageAdd24;
            PutImageSub         = PutImageSub24;
            PutSprite           = PutSprite24;
            PutSpriteAdd        = PutSpriteAdd24;
            PutSpriteSub        = PutSpriteSub24;
            ClearScreen         = ClearScreen24;
            ScaleImage          = ScaleImage24;
            FadeOutImage        = FadeOutImage24;
            FillRectPattern     = FillRectPattern24;
            FillRectPatternAdd  = FillRectPatternAdd24;
            FillRectPatternSub  = FillRectPatternSub24;
            break;

        case 32:
            PutPixel            = PutPixel32;
            PutPixelAdd         = PutPixelAdd32;
            PutPixelSub         = PutPixelSub32;
            GetPixel            = GetPixel32;
            FillRect            = FillRect32;
            FillRectAdd         = FillRectAdd32;
            FillRectSub         = FillRectSub32;
            HorizLine           = HorizLine32;
            HorizLineAdd        = HorizLineAdd32;
            HorizLineSub        = HorizLineSub32;
            VertLine            = VertLine32;
            VertLineAdd         = VertLineAdd32;
            VertLineSub         = VertLineSub32;
            GetImage            = GetImage32;
            PutImage            = PutImage32;
            PutImageAdd         = PutImageAdd32;
            PutImageSub         = PutImageSub32;
            PutSprite           = PutSprite32;
            PutSpriteAdd        = PutSpriteAdd32;
            PutSpriteSub        = PutSpriteSub32;
            ClearScreen         = ClearScreen32;
            ScaleImage          = ScaleImage32;
            FadeOutImage        = FadeOutImage32;
            FillRectPattern     = FillRectPattern32;
            FillRectPatternAdd  = FillRectPatternAdd32;
            FillRectPatternSub  = FillRectPatternSub32;
            break;
        }

        // Set default view port, drawable and visible page
        SetActivePage(0);
        SetVisualPage(0);
        SetViewPort(0, 0, maxx, maxy);
        return *modePtr;
    }

    return 0;
}

// Get/Set current hardware palette entries
void SetRGB(unsigned short index, unsigned char r, unsigned char g, unsigned char b)
{
    _asm {
        mov    dx, 03C8h
        mov    ax, index
        out    dx, al
        inc    dx
        mov    al, r
        shr    al, 2
        out    dx, al
        mov    al, g
        shr    al, 2
        out    dx, al
        mov    al, b
        shr    al, 2
        out    dx, al
    }
}

void GetRGB(unsigned short index, unsigned char *r, unsigned char *g, unsigned char *b)
{
    _asm {
        mov    dx, 03C7h
        mov    ax, index
        out    dx, al
        add    dx, 2
        in     al, dx
        shl    al, 2
        mov    edi, r
        mov    [edi], al
        in     al, dx
        shl    al, 2
        mov    edi, g
        mov    [edi], al
        in     al, dx
        shl    al, 2
        mov    edi, b
        mov    [edi], al
    }
}

void ShiftPalette(void *pal)
{
    _asm {
        mov    edi, pal
        mov    ecx, 256
    step:
        mov    al, [edi]
        shl    al, 2
        mov    [edi], al
        mov    al, [edi + 1]
        shl    al, 2
        mov    [edi + 1], al
        mov    al, [edi + 2]
        shl    al, 2
        mov    [edi + 2], al
        add    edi, 3
        loop   step
    }
}

void GetPalette(void *pal)
{
    _asm {
        xor    al, al
        mov    dx, 03C7h
        out    dx, al
        add    dx, 2
        mov    edi, pal
        mov    ecx, 256
    step:
        in     al, dx
        shl    al, 2
        mov    [edi], al
        in     al, dx
        shl    al, 2
        mov    [edi + 1], al
        in     al, dx
        shl    al, 2
        mov    [edi + 2], al
        add    edi, 3
        loop   step
    }
}

// Set hardware palette entries
void SetPalette(void *pal)
{
    _asm {
        xor    al, al
        mov    dx, 03C8h
        out    dx, al
        inc    dx
        mov    esi, pal
        mov    ecx, 256
    step:
        mov    al, [esi]
        shr    al, 2
        out    dx, al
        mov    al, [esi + 1]
        shr    al, 2
        out    dx, al
        mov    al, [esi + 2]
        shr    al, 2
        out    dx, al
        add    esi, 3
        loop   step
    }
}

// Make linear palette (7 circle colors)
void MakeLinearPalette()
{
    int i, j;
    RGB pal[256] = {0};

    for (i = 0; i != 32; i++)
    {
        j = 16 + i;
        pal[j].r = 63;
        pal[j].g = 0;
        pal[j].b = 63 - (i << 1);

        j = 48 + i;
        pal[j].r = 63;
        pal[j].g = i << 1;
        pal[j].b = 0;

        j = 80 + i;
        pal[j].r = 63 - (i << 1);
        pal[j].g = 63;
        pal[j].b = 0;

        j = 112 + i;
        pal[j].r = 0;
        pal[j].g = 63;
        pal[j].b = i << 1;

        j = 144 + i;
        pal[j].r = 0;
        pal[j].g = 63 - (i << 1);
        pal[j].b = 63;

        j = 176 + i;
        pal[j].r = i << 1;
        pal[j].g = 0;
        pal[j].b = 63;
    }

    ShiftPalette(pal);
    SetPalette(pal);
}

// yet another funky palette
void MakeFunkyPalette()
{
    RGB pal[256] = {0};
    unsigned char ry, gy, by;
    int i, r, g, b, rx, gx, bx;

    r = 0;
    g = 0;
    b = 0;

    ry = 1;
    gy = 1;
    by = 1;

    rx = (rand() % 5) + 1;
    gx = (rand() % 5) + 1;
    bx = (rand() % 5) + 1;

    for (i = 0; i != 256; i++)
    {
        pal[i].r = r;
        pal[i].g = g;
        pal[i].b = b;

        if (ry) r += rx; else r -= rx;
        if (gy) g += gx; else g -= gx;
        if (by) b += bx; else b -= bx;

        if ((r + rx > 63) || (r - rx < 0))
        {
            ry = !ry;
            rx = (rand() % 5) + 1;
        }

        if ((g + gx > 63) || (g - gx < 0))
        {
            gy = !gy;
            gx = (rand() % 5) + 1;
        }

        if ((b + bx > 63) || (b - bx < 0))
        {
            by = !by;
            bx = (rand() % 5) + 1;
        }
    }

    ShiftPalette(pal);
    SetPalette(pal);
}

// Make rainbow palette
void MakeRainbowPalette()
{
    register int i;
    RGB pal[256] = {0};
    
    for (i = 0; i != 32; i++)
    {
        pal[i      ].r = (i << 1);
        pal[63 - i ].r = (i << 1);
        pal[i + 64 ].g = (i << 1);
        pal[127 - i].g = (i << 1);
        pal[i + 128].b = (i << 1);
        pal[191 - i].b = (i << 1);
        pal[i + 192].r = (i << 1);
        pal[i + 192].g = (i << 1);
        pal[i + 192].b = (i << 1);
        pal[255 - i].r = (i << 1);
        pal[255 - i].g = (i << 1);
        pal[255 - i].b = (i << 1);
    }
    
    ShiftPalette(pal);
    SetPalette(pal);
}

// Make black current palette
void SetBlackPalette()
{
    RGB pal[256] = {0};
    SetPalette(pal);
}

// Make black current palette
void SetWhitePalette()
{
    RGB pal[256] = {255};
    SetPalette(pal);
}

// Scroll current palette
void ScrollPalette(int from, int to, int step)
{
    RGB tmp = {0};
    RGB pal[256] = {0};

    GetPalette(pal);
    
    while (step--)
    {
        memcpy(&tmp, &pal[from], sizeof(tmp));
        memcpy(&pal[from], &pal[from + 1], (to - from) * sizeof(RGB));
        memcpy(&pal[to], &tmp, sizeof(tmp));
    }

    WaitRetrace();
    SetPalette(pal);
}

// Rotate current palette
void RotatePalette(int from, int to, int loop)
{
    RGB tmp = {0};
    RGB pal[256] = {0};

    GetPalette(pal);
    if (loop > 0)
    {
        while(loop--)
        {
            memcpy(&tmp, &pal[from], sizeof(tmp));
            memcpy(&pal[from], &pal[from + 1], (to - from) * sizeof(RGB));
            memcpy(&pal[to], &tmp, sizeof(tmp));
            WaitRetrace();
            SetPalette(pal);
        }
    }
    else
    {
        // remove keyboard buffer
        while(kbhit()) getch();
        while(!kbhit())
        {
            memcpy(&tmp, &pal[from], sizeof(tmp));
            memcpy(&pal[from], &pal[from + 1], (to - from) * sizeof(RGB));
            memcpy(&pal[to], &tmp, sizeof(tmp));
            WaitRetrace();
            SetPalette(pal);
        }
    }
}

// All fade functions (In/Max/Out/Min)
// Fade from black palette to current palette
void FadeIn(RGB *dest)
{
    int i, j, k;
    RGB src[256] = {0};

    GetPalette(src);
    for (i = 63; i >= 0; i--)
    {
        for (j = 0; j != 256; j++)
        {
            k = i << 2;
            if (dest[j].r > k && src[j].r < 252) src[j].r += 4;
            if (dest[j].g > k && src[j].g < 252) src[j].g += 4;
            if (dest[j].b > k && src[j].b < 252) src[j].b += 4;
        }
        WaitRetrace();
        SetPalette(src);
    }
}

// Fade from current palette to destination palette
void FadeOut(RGB *dest)
{
    int i, j, k;
    RGB src[256] = {0};

    GetPalette(src);
    for (i = 63; i >= 0; i--)
    {
        for (j = 0; j != 256; j++)
        {
            k = i << 2;
            if (dest[j].r < k && src[j].r > 4) src[j].r -= 4;
            if (dest[j].g < k && src[j].g > 4) src[j].g -= 4;
            if (dest[j].b < k && src[j].b > 4) src[j].b -= 4;
        }
        WaitRetrace();
        SetPalette(src);
    }
}

// Fade from current palette to maximun palette
void FadeMax()
{
    int i, j;
    RGB src[256] = {0};

    GetPalette(src);
    for (i = 0; i != 64; i++)
    {
        for (j = 0; j != 256; j++)
        {
            if (src[j].r < 252) src[j].r += 4; else src[j].r = 255;
            if (src[j].g < 252) src[j].g += 4; else src[j].g = 255;
            if (src[j].b < 252) src[j].b += 4; else src[j].b = 255;
        }
        WaitRetrace();
        SetPalette(src);
    }
}

// Fade from current palette to minimum palette
void FadeMin()
{
    int i, j;
    RGB src[256] = {0};

    GetPalette(src);
    for (i = 0; i != 64; i++)
    {
        for (j = 0; j != 256; j++)
        {
            if (src[j].r > 4) src[j].r -= 4; else src[j].r = 0;
            if (src[j].g > 4) src[j].g -= 4; else src[j].g = 0;
            if (src[j].b > 4) src[j].b -= 4; else src[j].b = 0;
        }
        WaitRetrace();
        SetPalette(src);
    }
}

// downto current palette
void FadeDown(RGB *pal)
{
    int i;
    for (i = 0; i != 256; i++)
    {
        if (pal[i].r > 4) pal[i].r -= 2; else pal[i].r = 0;
        if (pal[i].g > 4) pal[i].g -= 2; else pal[i].g = 0;
        if (pal[i].b > 4) pal[i].b -= 2; else pal[i].b = 0;
    }
    SetPalette(pal);
}

// calculate lookup table for drawing circle
void CalcCircle(int r, int *point)
{
    if (r <= 0) return;

    _asm {
        mov    ebx, 1
        sub    ebx, r
        mov    edi, point
        mov    esi, edi
        mov    eax, r
        shl    eax, 2
        add    esi, eax
        mov    eax, r
        xor    ecx, ecx
    start:
        or     ebx, ebx
        jns    next
        mov    edx, ecx
        shl    edx, 1
        add    edx, 3
        add    ebx, edx
        inc    ecx
        sub    esi, 4
        jmp    stop
    next:
        mov    edx, ecx
        sub    edx, eax
        shl    edx, 1
        add    edx, 3
        add    ebx, edx
        inc    ecx
        dec    eax
        sub    esi, 4
        add    edi, 4
    stop:
        mov    [edi], ecx
        mov    [esi], eax
        cmp    eax, ecx
        jg     start
    }
}

// calculate lookup table for drawing ellipse
void CalcEllipse(int rx, int ry, int *point)
{
    int r = 0, a = 0, b = 0;
    int x = 0, mx = 0, my = 0;
    int aq = 0, bq = 0, xd = 0, yd = 0;

    if (rx <= 0 || ry <= 0) return;

    _asm {
        mov    eax, rx
        mov    x, eax
        neg    eax
        mov    mx, eax
        xor    edx, edx
        mov    eax, rx
        mul    eax
        mov    aq, eax
        shl    eax, 1
        mov    xd, eax
        mov    eax, ry
        mul    eax
        mov    bq, eax
        shl    eax, 1
        mov    yd, eax
        mov    eax, rx
        mul    bq
        mov    r, eax
        shl    eax, 1
        mov    a, eax
    next:
        cmp    r, 0
        jle    skip
        inc    my
        mov    eax, b
        add    eax, xd
        mov    b, eax
        sub    r, eax
    skip:
        cmp    r, 0
        jg     quit
        dec    x
        inc    mx
        mov    eax, a
        sub    eax, yd
        mov    a, eax
        add    r, eax
    quit:
        mov    edi, point
        mov    ebx, ry
        sub    ebx, my
        shl    ebx, 2
        add    edi, ebx
        mov    eax, mx
        neg    eax
        stosd
        cmp    x, 0
        jg     next
    }
}

// Very fast fill circle and ellipse
void FillCircle(int xc, int yc, int r, unsigned int col)
{
    int i;
    int point[500] = {0};

    if (r >= 500) FatalError("FillCircle: radius must be [0-499].\n");

    yc -= r;
    CalcCircle(r, point);

    for (i = 0; i <= r - 1; i++)
    {
        HorizLine(xc - point[i], yc, point[i] << 1, col);
        yc++;
    }

    for (i = r - 1; i >= 0; i--)
    {
        HorizLine(xc - point[i], yc, point[i] << 1, col);
        yc++;
    }
}

void FillEllipse(int xc, int yc, int rx, int ry, unsigned int col)
{
    int i;
    int point[500] = {0};

    if (rx >= 500 || ry >= 500) FatalError("FillEllipse: rx, ry must be [0-499].\n");

    yc -= ry;

    if (rx != ry) CalcEllipse(rx, ry, point); else CalcCircle(rx, point);

    for (i = 0; i <= ry - 1; i++)
    {
        HorizLine(xc - point[i], yc, point[i] << 1, col);
        yc++;
    }

    for (i = ry - 1; i >= 0; i--)
    {
        HorizLine(xc - point[i], yc, point[i] << 1, col);
        yc++;
    }
}

// Fill polygon using Darel Rex Finley algorithm
// Test vectors (screen resolution: 800x600)
// pt1[] = {{300, 100}, {192, 209}, {407, 323}, {320, 380}, {214, 350}, {375, 209}};
// pt2[] = {{169, 164}, {169, 264}, {223, 300}, {296, 209}, {214, 255}, {223, 200}, {386, 192}, {341, 273}, {404, 300}, {431, 146}};
// pt3[] = {{97, 56}, {115, 236}, {205, 146}, {276, 146}, {151, 325}, {259, 433}, {510, 344}, {510, 218}, {242, 271}, {384, 110}};
// pt4[] = {{256, 150}, {148, 347}, {327, 329}, {311, 204}, {401, 204}, {418, 240}, {257, 222}, {293, 365}, {436, 383}, {455, 150}};
// pt5[] = {{287, 76}, {129, 110}, {42, 301}, {78, 353}, {146, 337}, {199, 162}, {391, 180}, {322, 353}, {321, 198}, {219, 370}, {391, 405}, {444, 232}, {496, 440}, {565, 214}};
void FillPoly(POINT *point, int num, unsigned int col)
{
    int nodex[MAX_POLY_CORNERS] = {0};
    int nodes = 0, y = 0, i = 0, j = 0, swap = 0;
    int left = 0, right = 0, top = 0, bottom = 0;

    // initialize clipping
    left = right = point[0].x;
    top = bottom = point[0].y;

    // clipping points
    for (i = 1; i != num; i++)
    {
        if (point[i].x < left) left = point[i].x;
        if (point[i].x > right) right = point[i].x;
        if (point[i].y < top) top = point[i].y;
        if (point[i].y > bottom) bottom = point[i].y;
    }

    // loop through the rows of the image
    for (y = top; y != bottom; y++)
    {
        // build a list of polygon intercepts on the current line
        nodes = 0;
        j = num - 1;

        for (i = 0; i != num; i++)
        {
            // intercept found, record it
            if ((point[i].y < y && point[j].y >= y) || (point[j].y < y && point[i].y >= y)) nodex[nodes++] = point[i].x + (y - point[i].y) / (point[j].y - point[i].y) * (point[j].x - point[i].x);
            if (nodes >= MAX_POLY_CORNERS) return;
            j = i;
        }

        // sort the nodes, via a simple "Bubble" sort
        i = 0;
        while (i < nodes - 1)
        {
            if (nodex[i] > nodex[i + 1])
            {
                swap = nodex[i];
                nodex[i] = nodex[i + 1];
                nodex[i + 1] = swap;
                if (i) i--;
            }
            else i++;
        }

        // fill the pixels between node pairs
        for (i = 0; i != nodes; i += 2)
        {
            if (nodex[i] >= right) break;
            if (nodex[i + 1] > left)
            {
                if (nodex[i] < left) nodex[i] = left;
                if (nodex[i + 1] > right) nodex[i + 1] = right;
                HorizLine(nodex[i], y, nodex[i + 1] - nodex[i], col);
            }
        }
    }
}

// another fade functions
void FadeCircle(int dir, unsigned int col)
{
    int i, x, y;

    switch (dir)
    {
    case 0:
        for (i = 0; i != 29; i++)
        {
            WaitRetrace();
            for (y = 0; y <= cmaxy / 40; y++)
            for (x = 0; x <= cmaxx / 40; x++) FillCircle(x * 40 + 20, y * 40 + 20, i, col);
        }
        break;

    case 1:
        for (i = -cmaxy / 40; i != 29; i++)
        {
            WaitRetrace();
            for (y = 0; y <= cmaxy / 40; y++)
            for (x = 0; x <= cmaxx / 40; x++)
            if (cmaxy / 40 - y + i < 29) FillCircle(x * 40 + 20, y * 40 + 20, cmaxy / 40 - y + i, col);
        }
        break;

    case 2:
        for (i = -cmaxx / 40; i != 29; i++)
        {
            WaitRetrace();
            for (y = 0; y <= cmaxy / 40; y++)
            for (x = 0; x <= cmaxx / 40; x++)
            if (cmaxx / 40 - x + i < 29) FillCircle(x * 40 + 20, y * 40 + 20, cmaxx / 40 - x + i, col);
        }
        break;

    case 3:
        for (i = -cmaxx / 40; i != 60; i++)
        {
            WaitRetrace();
            for (y = 0; y <= cmaxy / 40; y++)
            for (x = 0; x <= cmaxx / 40; x++)
            if (cmaxx / 40 - x - y + i < 29) FillCircle(x * 40 + 20, y * 40 + 20, cmaxx / 40 - x - y + i, col);
        }
        break;
    }
}

void FadeRollo(int dir, unsigned int col)
{
    int i, j;

    switch (dir)
    {
    case 0:
        for (i = 0; i != 20; i++)
        {
            WaitRetrace();
            for (j = 0; j <= cmaxy / 10; j++) HorizLine(0, j * 20 + i, cmaxx, col);
        }
        break;

    case 1:
        for (i = 0; i != 20; i++)
        {
            WaitRetrace();
            for (j = 0; j <= cmaxx / 10; j++) VertLine(j * 20 + i, 0, cmaxy, col);
        }
        break;

    case 2:
        for (i = 0; i != 20; i++)
        {
            WaitRetrace();
            for (j = 0; j <= cmaxx / 10; j++)
            {
                VertLine(j * 20 + i, 0, cmaxy, col);
                if (j * 10 < cmaxy) HorizLine(0, j * 20 + i, cmaxx, col);
            }
        }
        break;
    }
}

// Cohen-Sutherland clipping line
int GetCode(int x, int y)
{
    int code = 0;
    if (y > cmaxy) code |= CLIP_TOP;
    else if (y < cminy) code |= CLIP_BOTTOM;
    if (x > cmaxx) code |= CLIP_RIGHT;
    else if (x < cminx) code |= CLIP_LEFT;
    return code;
}

// Drawing functions (using Xiaolin Wu’s algorithm with support anti-aliased)
void DrawLine(int x0, int y0, int x1, int y1, unsigned int rgb)
{
    int dx, dy, sx, sy, err, e2;
    int x, y, accepted;
    int outcode, outcode0, outcode1;

    if (x0 <= 0 && y0 <= 0 && x1 <= 0 && y1 <= 0) return;

    accepted = 0;
    outcode0 = GetCode(x0, y0);
    outcode1 = GetCode(x1, y1);

    while (1)
    {
        if (!(outcode0 | outcode1))
        {
            accepted = 1;
            break;
        }
        else if (outcode0 & outcode1) break;
        else
        {
            outcode = outcode0 ? outcode0 : outcode1;
            if (outcode & CLIP_TOP)
            {
                x = x0 + (x1 - x0) * (cmaxy - y0) / (y1 - y0);
                y = cmaxy;
            }
            else if (outcode & CLIP_BOTTOM)
            {
                x = x0 + (x1 - x0) * (cminy - y0) / (y1 - y0);
                y = cminy;
            }
            else if (outcode & CLIP_RIGHT)
            {
                y = y0 + (y1 - y0) * (cmaxx - x0) / (x1 - x0);
                x = cmaxx;
            }
            else
            {
                y = y0 + (y1 - y0) * (cminx - x0) / (x1 - x0);
                x = cminx;
            }

            if (outcode == outcode0)
            {
                x0 = x;
                y0 = y;
                outcode0 = GetCode(x0, y0);
            }
            else
            {
                x1 = x;
                y1 = y;
                outcode1 = GetCode(x1, y1);
            }
        }
    }

    // Don't accepted line
    if (!accepted) return;

    dx = abs(x1 - x0);
    sx = (x0 < x1) ? 1 : -1;
    dy = -abs(y1 - y0);
    sy = (y0 < y1) ? 1 : -1;
    err = dx + dy;

    while (1)
    {
        PutPixel(x0, y0, rgb);
        e2 = err << 1;

        if (e2 >= dy)
        {
            if (x0 == x1) break;
            err += dy;
            x0 += sx;
        }

        if (e2 <= dx)
        {
            if (y0 == y1) break;
            err += dx;
            y0 += sy;
        }
    }
}

void DrawLineBob(int x0, int y0, int x1, int y1)
{
    int dx, dy, sx, sy, err, e2;
    int x, y, accepted;
    int outcode, outcode0, outcode1;

    if (x0 <= 0 && y0 <= 0 && x1 <= 0 && y1 <= 0) return;

    accepted = 0;
    outcode0 = GetCode(x0, y0);
    outcode1 = GetCode(x1, y1);

    while (1)
    {
        if (!(outcode0 | outcode1))
        {
            accepted = 1;
            break;
        }
        else if (outcode0 & outcode1) break;
        else
        {
            outcode = outcode0 ? outcode0 : outcode1;
            if (outcode & CLIP_TOP)
            {
                x = x0 + (x1 - x0) * (cmaxy - y0) / (y1 - y0);
                y = cmaxy;
            }
            else if (outcode & CLIP_BOTTOM)
            {
                x = x0 + (x1 - x0) * (cminy - y0) / (y1 - y0);
                y = cminy;
            }
            else if (outcode & CLIP_RIGHT)
            {
                y = y0 + (y1 - y0) * (cmaxx - x0) / (x1 - x0);
                x = cmaxx;
            }
            else
            {
                y = y0 + (y1 - y0) * (cminx - x0) / (x1 - x0);
                x = cminx;
            }

            if (outcode == outcode0)
            {
                x0 = x;
                y0 = y;
                outcode0 = GetCode(x0, y0);
            }
            else
            {
                x1 = x;
                y1 = y;
                outcode1 = GetCode(x1, y1);
            }
        }
    }

    // Don't accepted line
    if (!accepted) return;

    dx = abs(x1 - x0);
    sx = (x0 < x1) ? 1 : -1;
    dy = -abs(y1 - y0);
    sy = (y0 < y1) ? 1 : -1;
    err = dx + dy;

    while (1)
    {
        PutPixelBob(x0, y0);
        e2 = err << 1;

        if (e2 >= dy)
        {
            if (x0 == x1) break;
            err += dy;
            x0 += sx;
        }

        if (e2 <= dx)
        {
            if (y0 == y1) break;
            err += dx;
            y0 += sy;
        }
    }
}

void DrawLineAdd(int x0, int y0, int x1, int y1, unsigned int rgb)
{
    int dx, dy, sx, sy, err, e2;
    int x, y, accepted;
    int outcode, outcode0, outcode1;

    if (x0 <= 0 && y0 <= 0 && x1 <= 0 && y1 <= 0) return;

    accepted = 0;
    outcode0 = GetCode(x0, y0);
    outcode1 = GetCode(x1, y1);

    while (1)
    {
        if (!(outcode0 | outcode1))
        {
            accepted = 1;
            break;
        }
        else if (outcode0 & outcode1) break;
        else
        {
            outcode = outcode0 ? outcode0 : outcode1;
            if (outcode & CLIP_TOP)
            {
                x = x0 + (x1 - x0) * (cmaxy - y0) / (y1 - y0);
                y = cmaxy;
            }
            else if (outcode & CLIP_BOTTOM)
            {
                x = x0 + (x1 - x0) * (cminy - y0) / (y1 - y0);
                y = cminy;
            }
            else if (outcode & CLIP_RIGHT)
            {
                y = y0 + (y1 - y0) * (cmaxx - x0) / (x1 - x0);
                x = cmaxx;
            }
            else
            {
                y = y0 + (y1 - y0) * (cminx - x0) / (x1 - x0);
                x = cminx;
            }

            if (outcode == outcode0)
            {
                x0 = x;
                y0 = y;
                outcode0 = GetCode(x0, y0);
            }
            else
            {
                x1 = x;
                y1 = y;
                outcode1 = GetCode(x1, y1);
            }
        }
    }

    // Don't accepted line
    if (!accepted) return;

    dx = abs(x1 - x0);
    sx = (x0 < x1) ? 1 : -1;
    dy = -abs(y1 - y0);
    sy = (y0 < y1) ? 1 : -1;
    err = dx + dy;

    while (1)
    {
        PutPixelAdd(x0, y0, rgb);
        e2 = err << 1;

        if (e2 >= dy)
        {
            if (x0 == x1) break;
            err += dy;
            x0 += sx;
        }

        if (e2 <= dx)
        {
            if (y0 == y1) break;
            err += dx;
            y0 += sy;
        }
    }
}

void DrawLineSub(int x0, int y0, int x1, int y1, unsigned int rgb)
{
    int dx, dy, sx, sy, err, e2;
    int x, y, accepted;
    int outcode, outcode0, outcode1;

    if (x0 <= 0 && y0 <= 0 && x1 <= 0 && y1 <= 0) return;

    accepted = 0;
    outcode0 = GetCode(x0, y0);
    outcode1 = GetCode(x1, y1);

    while (1)
    {
        if (!(outcode0 | outcode1))
        {
            accepted = 1;
            break;
        }
        else if (outcode0 & outcode1) break;
        else
        {
            outcode = outcode0 ? outcode0 : outcode1;
            if (outcode & CLIP_TOP)
            {
                x = x0 + (x1 - x0) * (cmaxy - y0) / (y1 - y0);
                y = cmaxy;
            }
            else if (outcode & CLIP_BOTTOM)
            {
                x = x0 + (x1 - x0) * (cminy - y0) / (y1 - y0);
                y = cminy;
            }
            else if (outcode & CLIP_RIGHT)
            {
                y = y0 + (y1 - y0) * (cmaxx - x0) / (x1 - x0);
                x = cmaxx;
            }
            else
            {
                y = y0 + (y1 - y0) * (cminx - x0) / (x1 - x0);
                x = cminx;
            }

            if (outcode == outcode0)
            {
                x0 = x;
                y0 = y;
                outcode0 = GetCode(x0, y0);
            }
            else
            {
                x1 = x;
                y1 = y;
                outcode1 = GetCode(x1, y1);
            }
        }
    }

    // Don't accepted line
    if (!accepted) return;

    dx = abs(x1 - x0);
    sx = (x0 < x1) ? 1 : -1;
    dy = -abs(y1 - y0);
    sy = (y0 < y1) ? 1 : -1;
    err = dx + dy;

    while (1)
    {
        PutPixelSub(x0, y0, rgb);
        e2 = err << 1;

        if (e2 >= dy)
        {
            if (x0 == x1) break;
            err += dy;
            x0 += sx;
        }

        if (e2 <= dx)
        {
            if (y0 == y1) break;
            err += dx;
            y0 += sy;
        }
    }
}

void DrawCircle(int xm, int ym, int r, unsigned int rgb)
{
    int x, y, err;

    if (r <= 0) return;

    x = -r;
    y = 0;
    err = (1 - r) << 1;

    do {
        PutPixel(xm - x, ym + y, rgb);
        PutPixel(xm - y, ym - x, rgb);
        PutPixel(xm + x, ym - y, rgb);
        PutPixel(xm + y, ym + x, rgb);
        r = err;
        if (r <= y) err += ++y * 2 + 1;
        if (r > x || err > y) err += ++x * 2 + 1;
    } while (x < 0);
}

void DrawCircleAdd(int xm, int ym, int r, unsigned int rgb)
{
    int x, y, err;

    if (r <= 0) return;

    x = -r;
    y = 0;
    err = (1 - r) << 1;

    do {
        PutPixelAdd(xm - x, ym + y, rgb);
        PutPixelAdd(xm - y, ym - x, rgb);
        PutPixelAdd(xm + x, ym - y, rgb);
        PutPixelAdd(xm + y, ym + x, rgb);
        r = err;
        if (r <= y) err += ++y * 2 + 1;
        if (r > x || err > y) err += ++x * 2 + 1;
    } while (x < 0);
}

void DrawCircleSub(int xm, int ym, int r, unsigned int rgb)
{
    int x, y, err;

    if (r <= 0) return;

    x = -r;
    y = 0;
    err = (1 - r) << 1;

    do {
        PutPixelSub(xm - x, ym + y, rgb);
        PutPixelSub(xm - y, ym - x, rgb);
        PutPixelSub(xm + x, ym - y, rgb);
        PutPixelSub(xm + y, ym + x, rgb);
        r = err;
        if (r <= y) err += ++y * 2 + 1;
        if (r > x || err > y) err += ++x * 2 + 1;
    } while (x < 0);
}

void DrawEllipse(int xm, int ym, int xr, int yr, unsigned int rgb)
{
    int x, y, dy, err;
    long e2, dx;

    if (xr <= 0 || yr <= 0) return;

    x = -xr;
    y = 0;
    e2 = yr;
    dx = (1 + 2 * x) * e2 * e2;
    dy = x * x;
    err = dx + dy;

    do {
        PutPixel(xm - x, ym + y, rgb);
        PutPixel(xm + x, ym + y, rgb);
        PutPixel(xm + x, ym - y, rgb);
        PutPixel(xm - x, ym - y, rgb);

        e2 = err << 1;
        if (e2 >= dx)
        {
            x++;
            err += dx += 2 * yr * yr;
        }
        if (e2 <= dy)
        {
            y++;
            err += dy += 2 * xr * xr;
        }
    } while (x <= 0);

    while (y++ < yr)
    {
        PutPixel(xm, ym + y, rgb);
        PutPixel(xm, ym - y, rgb);
    }
}

void DrawEllipseAdd(int xm, int ym, int xr, int yr, unsigned int rgb)
{
    int x, y, dy, err;
    long e2, dx;

    if (xr <= 0 || yr <= 0) return;

    x = -xr;
    y = 0;
    e2 = yr;
    dx = (1 + 2 * x) * e2 * e2;
    dy = x * x;
    err = dx + dy;

    do {
        PutPixelAdd(xm - x, ym + y, rgb);
        PutPixelAdd(xm + x, ym + y, rgb);
        PutPixelAdd(xm + x, ym - y, rgb);
        PutPixelAdd(xm - x, ym - y, rgb);

        e2 = err << 1;
        if (e2 >= dx)
        {
            x++;
            err += dx += 2 * yr * yr;
        }
        if (e2 <= dy)
        {
            y++;
            err += dy += 2 * xr * xr;
        }
    } while (x <= 0);

    while (y++ < yr)
    {
        PutPixelAdd(xm, ym + y, rgb);
        PutPixelAdd(xm, ym - y, rgb);
    }
}

void DrawEllipseSub(int xm, int ym, int xr, int yr, unsigned int rgb)
{
    int x, y, dy, err;
    long e2, dx;

    if (xr <= 0 || yr <= 0) return;

    x = -xr;
    y = 0;
    e2 = yr;
    dx = (1 + 2 * x) * e2 * e2;
    dy = x * x;
    err = dx + dy;

    do {
        PutPixelSub(xm - x, ym + y, rgb);
        PutPixelSub(xm + x, ym + y, rgb);
        PutPixelSub(xm + x, ym - y, rgb);
        PutPixelSub(xm - x, ym - y, rgb);

        e2 = err << 1;
        if (e2 >= dx)
        {
            x++;
            err += dx += 2 * yr * yr;
        }
        if (e2 <= dy)
        {
            y++;
            err += dy += 2 * xr * xr;
        }
    } while (x <= 0);

    while (y++ < yr)
    {
        PutPixelSub(xm, ym + y, rgb);
        PutPixelSub(xm, ym - y, rgb);
    }
}

void DrawEllipseRect(int x0, int y0, int x1, int y1, unsigned int rgb)
{
    long a = abs(x1 - x0), b = abs(y1 - y0), b1 = b & 1;
    float dx = 4 * (1.0 - a) * b * b, dy = 4 * (b1 + 1) * a * a;
    float err = dx + dy + b1 * a * a, e2;

    if (x0 > x1) { x0 = x1; x1 += a; }
    if (y0 > y1) y0 = y1;

    y0 += (b + 1) / 2;
    y1 = y0 - b1;

    a = 8 * a * a;
    b1 = 8 * b * b;

    do {            
        PutPixel(x1, y0, rgb);
        PutPixel(x0, y0, rgb);
        PutPixel(x0, y1, rgb);
        PutPixel(x1, y1, rgb);
        e2 = 2 * err;
        if (e2 <= dy) { y0++; y1--; err += dy += a; }
        if (e2 >= dx || 2 * err > dy) { x0++; x1--; err += dx += b1; }
    } while (x0 <= x1);

    while (y0 - y1 <= b)
    {
        PutPixel(x0 - 1, y0, rgb);
        PutPixel(x1 + 1, y0++, rgb);
        PutPixel(x0 - 1, y1, rgb);
        PutPixel(x1 + 1, y1--, rgb);
    }
}

void DrawEllipseRectAdd(int x0, int y0, int x1, int y1, unsigned int rgb)
{
    long a = abs(x1 - x0), b = abs(y1 - y0), b1 = b & 1;
    float dx = 4 * (1.0 - a) * b * b, dy = 4 * (b1 + 1) * a * a;
    float err = dx + dy + b1 * a * a, e2;

    if (x0 > x1) { x0 = x1; x1 += a; }
    if (y0 > y1) y0 = y1;

    y0 += (b + 1) / 2;
    y1 = y0 - b1;

    a = 8 * a * a;
    b1 = 8 * b * b;

    do {            
        PutPixelAdd(x1, y0, rgb);
        PutPixelAdd(x0, y0, rgb);
        PutPixelAdd(x0, y1, rgb);
        PutPixelAdd(x1, y1, rgb);
        e2 = 2 * err;
        if (e2 <= dy) { y0++; y1--; err += dy += a; }
        if (e2 >= dx || 2 * err > dy) { x0++; x1--; err += dx += b1; }
    } while (x0 <= x1);

    while (y0 - y1 <= b)
    {
        PutPixelAdd(x0 - 1, y0, rgb);
        PutPixelAdd(x1 + 1, y0++, rgb);
        PutPixelAdd(x0 - 1, y1, rgb);
        PutPixelAdd(x1 + 1, y1--, rgb);
    }
}

void DrawEllipseRectSub(int x0, int y0, int x1, int y1, unsigned int rgb)
{
    long a = abs(x1 - x0), b = abs(y1 - y0), b1 = b & 1;
    float dx = 4 * (1.0 - a) * b * b, dy = 4 * (b1 + 1) * a * a;
    float err = dx + dy + b1 * a * a, e2;

    if (x0 > x1) { x0 = x1; x1 += a; }
    if (y0 > y1) y0 = y1;

    y0 += (b + 1) / 2;
    y1 = y0 - b1;

    a = 8 * a * a;
    b1 = 8 * b * b;

    do {            
        PutPixelSub(x1, y0, rgb);
        PutPixelSub(x0, y0, rgb);
        PutPixelSub(x0, y1, rgb);
        PutPixelSub(x1, y1, rgb);
        e2 = 2 * err;
        if (e2 <= dy) { y0++; y1--; err += dy += a; }
        if (e2 >= dx || 2 * err > dy) { x0++; x1--; err += dx += b1; }
    } while (x0 <= x1);

    while (y0 - y1 <= b)
    {
        PutPixelSub(x0 - 1, y0, rgb);
        PutPixelSub(x1 + 1, y0++, rgb);
        PutPixelSub(x0 - 1, y1, rgb);
        PutPixelSub(x1 + 1, y1--, rgb);
    }
}

void DrawRect(int x1, int y1, int x2, int y2, unsigned int col)
{
    HorizLine(x1, y1, x2 - x1 + 1, col);
    VertLine(x1, y1, y2 - y1 + 1, col);
    HorizLine(x1, y2, x2 - x1 + 1, col);
    VertLine(x2, y1, y2 - y1 + 1, col);
}

void DrawRectAdd(int x1, int y1, int x2, int y2, unsigned int col)
{
    HorizLineAdd(x1, y1, x2 - x1 + 1, col);
    VertLineAdd(x1, y1, y2 - y1 + 1, col);
    HorizLineAdd(x1, y2, x2 - x1 + 1, col);
    VertLineAdd(x2, y1, y2 - y1 + 1, col);
}

void DrawRectSub(int x1, int y1, int x2, int y2, unsigned int col)
{
    HorizLineSub(x1, y1, x2 - x1 + 1, col);
    VertLineSub(x1, y1, y2 - y1 + 1, col);
    HorizLineSub(x1, y2, x2 - x1 + 1, col);
    VertLineSub(x2, y1, y2 - y1 + 1, col);
}

void DrawRectEx(int x1, int y1, int x2, int y2, int r, unsigned int col)
{
    int width, mid, x, y;
    int point[500] = {0};

    mid = (y2 - y1) >> 1;
    if (r >= mid - 1) r = mid - 1;

    width = abs(x2 - x1);
    CalcCircle(r, point);

    HorizLine(x1 + r - point[0], y1, width - (r - point[0]) * 2 + 1, col);
    VertLine(x1, y1 + r, y2 - y1 - r * 2 + 1, col);
    HorizLine(x1 + r - point[0], y2, width - (r - point[0]) * 2 + 1, col);
    VertLine(x2, y1 + r, y2 - y1 - r * 2 + 1, col);

    for (y = 1; y <= r; y++)
    {
        for (x = r - point[y]; x <= r - point[y - 1]; x++)
        {
            PutPixel(x1 + x, y1 + y, col);
            PutPixel(x2 - x, y1 + y, col);
        }
    }

    for (y = 1; y <= r; y++)
    {
        for (x = r - point[y]; x <= r - point[y - 1]; x++)
        {
            PutPixel(x1 + x, y2 - y, col);
            PutPixel(x2 - x, y2 - y, col);
        }
    }
}

void DrawRectExAdd(int x1, int y1, int x2, int y2, int r, unsigned int col)
{
    int width, mid, x, y;
    int point[500] = {0};

    mid = (y2 - y1) >> 1;
    if (r >= mid - 1) r = mid - 1;

    width = abs(x2 - x1);
    CalcCircle(r, point);

    HorizLineAdd(x1 + r - point[0], y1, width - (r - point[0]) * 2 + 1, col);
    VertLineAdd(x1, y1 + r, y2 - y1 - r * 2 + 1, col);
    HorizLineAdd(x1 + r - point[0], y2, width - (r - point[0]) * 2 + 1, col);
    VertLineAdd(x2, y1 + r, y2 - y1 - r * 2 + 1, col);

    for (y = 1; y <= r; y++)
    {
        for (x = r - point[y]; x <= r - point[y - 1]; x++)
        {
            PutPixelAdd(x1 + x, y1 + y, col);
            PutPixelAdd(x2 - x, y1 + y, col);
        }
    }

    for (y = 1; y <= r; y++)
    {
        for (x = r - point[y]; x <= r - point[y - 1]; x++)
        {
            PutPixelAdd(x1 + x, y2 - y, col);
            PutPixelAdd(x2 - x, y2 - y, col);
        }
    }
}

void DrawRectExSub(int x1, int y1, int x2, int y2, int r, unsigned int col)
{
    int width, mid, x, y;
    int point[500] = {0};

    mid = (y2 - y1) >> 1;
    if (r >= mid - 1) r = mid - 1;

    width = abs(x2 - x1);
    CalcCircle(r, point);

    HorizLineSub(x1 + r - point[0], y1, width - (r - point[0]) * 2 + 1, col);
    VertLineSub(x1, y1 + r, y2 - y1 - r * 2 + 1, col);
    HorizLineSub(x1 + r - point[0], y2, width - (r - point[0]) * 2 + 1, col);
    VertLineSub(x2, y1 + r, y2 - y1 - r * 2 + 1, col);

    for (y = 1; y <= r; y++)
    {
        for (x = r - point[y]; x <= r - point[y - 1]; x++)
        {
            PutPixelSub(x1 + x, y1 + y, col);
            PutPixelSub(x2 - x, y1 + y, col);
        }
    }

    for (y = 1; y <= r; y++)
    {
        for (x = r - point[y]; x <= r - point[y - 1]; x++)
        {
            PutPixelSub(x1 + x, y2 - y, col);
            PutPixelSub(x2 - x, y2 - y, col);
        }
    }
}

void DrawBox(int x1, int y1, int x2, int y2, int dx, int dy, unsigned int col)
{
    int x11, y11;
    int x22, y22;

    x11 = x1 + dx;
    y11 = y1 - dy;
    x22 = x2 + dx;
    y22 = y2 - dy;

    DrawRect(x1, y1, x2, y2, col);
    DrawRect(x11, y11, x22, y22, col);
    DrawLine(x1, y1, x11, y11, col);
    DrawLine(x2, y1, x22, y11, col);
    DrawLine(x2, y2, x22, y22, col);
    DrawLine(x1, y2, x11, y22, col);
}

void DrawBoxAdd(int x1, int y1, int x2, int y2, int dx, int dy, unsigned int col)
{
    int x11, y11;
    int x22, y22;

    x11 = x1 + dx;
    y11 = y1 - dy;
    x22 = x2 + dx;
    y22 = y2 - dy;

    DrawRectAdd(x1, y1, x2, y2, col);
    DrawRectAdd(x11, y11, x22, y22, col);
    DrawLineAdd(x1, y1, x11, y11, col);
    DrawLineAdd(x2, y1, x22, y11, col);
    DrawLineAdd(x2, y2, x22, y22, col);
    DrawLineAdd(x1, y2, x11, y22, col);
}

void DrawBoxSub(int x1, int y1, int x2, int y2, int dx, int dy, unsigned int col)
{
    int x11, y11;
    int x22, y22;

    x11 = x1 + dx;
    y11 = y1 - dy;
    x22 = x2 + dx;
    y22 = y2 - dy;

    DrawRectSub(x1, y1, x2, y2, col);
    DrawRectSub(x11, y11, x22, y22, col);
    DrawLineSub(x1, y1, x11, y11, col);
    DrawLineSub(x2, y1, x22, y11, col);
    DrawLineSub(x2, y2, x22, y22, col);
    DrawLineSub(x1, y2, x11, y22, col);
}

void DrawBoxEx(int x1, int y1, int x2, int y2, int r, unsigned int col)
{
    int width, mid, x, y;
    int point[500] = {0};

    mid = (y2 - y1) >> 1;
    if (r >= mid - 1) r = mid - 1;

    width = abs(x2 - x1);
    CalcCircle(r, point);

    HorizLine(x1 + r - point[0], y1, width - (r - point[0]) * 2 + 1, col);
    VertLine(x1, y1 + r, y2 - y1 - r * 2 + 1, col);
    HorizLine(x1 + r - point[0], y2, width - (r - point[0]) * 2 + 1, col);
    VertLine(x2, y1 + r, y2 - y1 - r * 2 + 1, col);

    for (y = 1; y <= r; y++)
    {
        for (x = r - point[y]; x <= r - point[y - 1]; x++)
        {
            PutPixel(x1 + x, y1 + y, col);
            PutPixel(x2 - x, y1 + y, col);
        }
    }

    for (y = 1; y <= r; y++)
    {
        for (x = r - point[y]; x <= r - point[y - 1]; x++)
        {
            PutPixel(x1 + x, y2 - y, col);
            PutPixel(x2 - x, y2 - y, col);
        }
    }

    for (y = 1; y <= r; y++) HorizLine(x1 + r - point[y - 1] + 1, y1 + y, width - (r * 2 - point[y - 1] * 2) - 1, col);
    FillRect(x1 + 1, y1 + r + 1, x2 - 1, y2 - r - 1, col);
    for (y = r; y >= 1; y--) HorizLine(x1 + r - point[y - 1] + 1, y2 - y, width - (r * 2 - point[y - 1] * 2) - 1, col);
}

void DrawBoxExAdd(int x1, int y1, int x2, int y2, int r, unsigned int col)
{
    int width, mid, x, y;
    int point[500] = {0};

    mid = (y2 - y1) >> 1;
    if (r >= mid - 1) r = mid - 1;

    width = abs(x2 - x1);
    CalcCircle(r, point);

    HorizLineAdd(x1 + r - point[0], y1, width - (r - point[0]) * 2 + 1, col);
    VertLineAdd(x1, y1 + r, y2 - y1 - r * 2 + 1, col);
    HorizLineAdd(x1 + r - point[0], y2, width - (r - point[0]) * 2 + 1, col);
    VertLineAdd(x2, y1 + r, y2 - y1 - r * 2 + 1, col);

    for (y = 1; y <= r; y++)
    {
        for (x = r - point[y]; x <= r - point[y - 1]; x++)
        {
            PutPixelAdd(x1 + x, y1 + y, col);
            PutPixelAdd(x2 - x, y1 + y, col);
        }
    }

    for (y = 1; y <= r; y++)
    {
        for (x = r - point[y]; x <= r - point[y - 1]; x++)
        {
            PutPixelAdd(x1 + x, y2 - y, col);
            PutPixelAdd(x2 - x, y2 - y, col);
        }
    }

    for (y = 1; y <= r; y++) HorizLineAdd(x1 + r - point[y - 1] + 1, y1 + y, width - (r * 2 - point[y - 1] * 2) - 1, col);
    FillRectAdd(x1 + 1, y1 + r + 1, x2 - 1, y2 - r - 1, col);
    for (y = r; y >= 1; y--) HorizLineAdd(x1 + r - point[y - 1] + 1, y2 - y, width - (r * 2 - point[y - 1] * 2) - 1, col);
}

void DrawBoxExSub(int x1, int y1, int x2, int y2, int r, unsigned int col)
{
    int width, mid, x, y;
    int point[500] = {0};

    mid = (y2 - y1) >> 1;
    if (r >= mid - 1) r = mid - 1;

    width = abs(x2 - x1);
    CalcCircle(r, point);

    HorizLineSub(x1 + r - point[0], y1, width - (r - point[0]) * 2 + 1, col);
    VertLineSub(x1, y1 + r, y2 - y1 - r * 2 + 1, col);
    HorizLineSub(x1 + r - point[0], y2, width - (r - point[0]) * 2 + 1, col);
    VertLineSub(x2, y1 + r, y2 - y1 - r * 2 + 1, col);

    for (y = 1; y <= r; y++)
    {
        for (x = r - point[y]; x <= r - point[y - 1]; x++)
        {
            PutPixelSub(x1 + x, y1 + y, col);
            PutPixelSub(x2 - x, y1 + y, col);
        }
    }

    for (y = 1; y <= r; y++)
    {
        for (x = r - point[y]; x <= r - point[y - 1]; x++)
        {
            PutPixelSub(x1 + x, y2 - y, col);
            PutPixelSub(x2 - x, y2 - y, col);
        }
    }

    for (y = 1; y <= r; y++) HorizLineSub(x1 + r - point[y - 1] + 1, y1 + y, width - (r * 2 - point[y - 1] * 2) - 1, col);
    FillRectSub(x1 + 1, y1 + r + 1, x2 - 1, y2 - r - 1, col);
    for (y = r; y >= 1; y--) HorizLineSub(x1 + r - point[y - 1] + 1, y2 - y, width - (r * 2 - point[y - 1] * 2) - 1, col);
}

void DrawPoly(POINT *point, int num, unsigned int col)
{
    int i;

    if (num < 3) return;
    for (i = 0; i != num; i++)
    {
        DrawLine(point[i].x, point[i].y, point[(i + 1) % num].x, point[(i + 1) % num].y, col);
    }
}

void DrawPolyAdd(POINT *point, int num, unsigned int col)
{
    int i;

    if (num < 3) return;
    for (i = 0; i != num; i++)
    {
        DrawLineAdd(point[i].x, point[i].y, point[(i + 1) % num].x, point[(i + 1) % num].y, col);
    }
}

void DrawPolySub(POINT *point, int num, unsigned int col)
{
    int i;

    if (num < 3) return;
    for (i = 0; i != num; i++)
    {
        DrawLineSub(point[i].x, point[i].y, point[(i + 1) % num].x, point[(i + 1) % num].y, col);
    }
}

void MoveTo(int x, int y)
{
    currx = x;
    curry = y;
}

void LineTo(int x, int y, unsigned int col)
{
    DrawLine(currx, curry, x, y, col);
    MoveTo(x, y);
}

void LineToAdd(int x, int y, unsigned int col)
{
    DrawLineAdd(currx, curry, x, y, col);
    MoveTo(x, y);
}

void LineToSub(int x, int y, unsigned int col)
{
    DrawLineSub(currx, curry, x, y, col);
    MoveTo(x, y);
}

void PutPixelAlpha(int x, int y, unsigned int rgb, unsigned char alpha)
{
    // Only 32bit support alpha-blend mode
    if (bytesPerPixel != 4) return;

    // Check clip boundary
    if (x < cminx || x > cmaxx || y < cminy || y > cmaxy) return;

    _asm {
        mov    eax, y
        add    eax, pageOffset
        mul    bytesPerScanline
        mov    ebx, x
        shl    ebx, 2
        add    eax, ebx
        mov    edi, lfbPtr
        add    edi, eax
        mov    al, [edi]
        mul    alpha
        mov    bx, ax
        mov    al, byte ptr[rgb]
        mov    cl, 255
        sub    cl, alpha
        mul    cl
        add    ax, bx
        shr    ax, 8
        stosb
        mov    al, [edi]
        mul    alpha
        mov    bx, ax
        mov    al, byte ptr[rgb + 1]
        mov    cl, 255
        sub    cl, alpha
        mul    cl
        add    ax, bx
        shr    ax, 8
        stosb
        mov    al, [edi]
        mul    alpha
        mov    bx, ax
        mov    al, byte ptr[rgb + 2]
        mov    cl, 255
        sub    cl, alpha
        mul    cl
        add    ax, bx
        shr    ax, 8
        stosb
    }
}

void DrawLineAlpha(int x0, int y0, int x1, int y1, unsigned int rgb)
{
    int dx, sx, dy, sy, x2, e2, err, ed;
    int x, y, accepted;
    int outcode, outcode0, outcode1;

    // Only 32bit support alpha-blend mode
    if (bytesPerPixel != 4) return;
    if (x0 <= 0 && y0 <= 0 && x1 <= 0 && y1 <= 0) return;

    accepted = 0;
    outcode0 = GetCode(x0, y0);
    outcode1 = GetCode(x1, y1);

    while (1)
    {
        if (!(outcode0 | outcode1))
        {
            accepted = 1;
            break;
        }
        else if (outcode0 & outcode1) break;
        else
        {
            outcode = outcode0 ? outcode0 : outcode1;
            if (outcode & CLIP_TOP)
            {
                x = x0 + (x1 - x0) * (cmaxy - y0) / (y1 - y0);
                y = cmaxy;
            }
            else if (outcode & CLIP_BOTTOM)
            {
                x = x0 + (x1 - x0) * (cminy - y0) / (y1 - y0);
                y = cminy;
            }
            else if (outcode & CLIP_RIGHT)
            {
                y = y0 + (y1 - y0) * (cmaxx - x0) / (x1 - x0);
                x = cmaxx;
            }
            else
            {
                y = y0 + (y1 - y0) * (cminx - x0) / (x1 - x0);
                x = cminx;
            }

            if (outcode == outcode0)
            {
                x0 = x;
                y0 = y;
                outcode0 = GetCode(x0, y0);
            }
            else
            {
                x1 = x;
                y1 = y;
                outcode1 = GetCode(x1, y1);
            }
        }
    }

    // Don't accepted line
    if (!accepted) return;

    dx = abs(x1 - x0);
    sx = x0 < x1 ? 1 : -1;
    dy = abs(y1 - y0);
    sy = y0 < y1 ? 1 : -1;
    err = dx - dy;
    ed = (dx + dy == 0) ? 1 : sqrt(dx * dx + dy * dy);

    while (1)
    {
        PutPixelAlpha(x0, y0, rgb, 255 * abs(err - dx + dy) / ed);
        e2 = err;
        x2 = x0;

        if (2 * e2 >= -dx)
        {
            if (x0 == x1) break;
            if (e2 + dy < ed) PutPixelAlpha(x0, y0 + sy, rgb, 255 * (e2 + dy) / ed);

            err -= dy;
            x0 += sx;
        }

        if (2 * e2 <= dy)
        {
            if (y0 == y1) break;
            if (dx - e2 < ed) PutPixelAlpha(x2 + sx, y0, rgb, 255 * (dx - e2) / ed);

            err += dx;
            y0 += sy;
        }
    }
}

void LineToAlpha(int x, int y, unsigned int col)
{
    // Only 32bit support alpha-blend mode
    if (bytesPerPixel != 4) return;
    DrawLineAlpha(currx, curry, x, y, col);
    MoveTo(x, y);
}

void DrawCircleAlpha(int xm, int ym, int r, unsigned int rgb)
{
    int x, y, alpha, x2, e2, err;

    // Only 32bit support alpha-blend mode
    if (bytesPerPixel != 4) return;
    if (r <= 0) return;

    x = -r;
    y = 0;
    err = (1 - r) << 1;
    r = 1 - err;

    do {
        alpha = 255 * abs(err - 2 * (x + y) - 2) / r;
        PutPixelAlpha(xm - x, ym + y, rgb, alpha);
        PutPixelAlpha(xm - y, ym - x, rgb, alpha);
        PutPixelAlpha(xm + x, ym - y, rgb, alpha);
        PutPixelAlpha(xm + y, ym + x, rgb, alpha);

        e2 = err;
        x2 = x;

        if (err + y > 0)
        {
            alpha = 255 * (err - 2 * x - 1) / r;
            if (alpha < 256)
            {
                PutPixelAlpha(xm - x, ym + y + 1, rgb, alpha);
                PutPixelAlpha(xm - y - 1, ym - x, rgb, alpha);
                PutPixelAlpha(xm + x, ym - y - 1, rgb, alpha);
                PutPixelAlpha(xm + y + 1, ym + x, rgb, alpha);
            }

            err += ++x * 2 + 1;
        }

        if (e2 + x2 <= 0)
        {
            alpha = 255 * (2 * y + 3 - e2) / r;
            if (alpha < 256)
            {
                PutPixelAlpha(xm - x2 - 1, ym + y, rgb, alpha);
                PutPixelAlpha(xm - y, ym - x2 - 1, rgb, alpha);
                PutPixelAlpha(xm + x2 + 1, ym - y, rgb, alpha);
                PutPixelAlpha(xm + y, ym + x2 + 1, rgb, alpha);
            }
            err += ++y * 2 + 1;
        }
    } while (x < 0);
}

void DrawEllipseAlpha(int x0, int y0, int x1, int y1, unsigned int rgb)
{
    int f;
    long a, b, b1;
    float dx, dy, ed, alpha, err;

    // Only 32bit support alpha-blend mode
    if (bytesPerPixel != 4) return;

    a = abs(x1 - x0);
    b = abs(y1 - y0);
    b1 = b & 1;
    dx = 4 * (a - 1) * b * b;
    dy = 4 * (b1 + 1) * a * a;
    err = b1 * a * a - dx + dy;

    // Check for line
    if (a == 0 || b == 0) return;

    if (x0 > x1)
    {
        x0 = x1;
        x1 += a;
    }

    if (y0 > y1) y0 = y1;

    y0 += (b + 1) / 2;
    y1 = y0 - b1;
    a = 8 * a * a;
    b1 = 8 * b * b;

    while (1)
    {
        ed = max(dx, dy);
        alpha = min(dx, dy);

        if (y0 == y1 + 1 && err > dy && a > b1) ed = 255 * 4 / a;
        else ed = 255 / (ed + 2 * ed * alpha * alpha / (4 * ed * ed + alpha * alpha));

        alpha = ed * fabs(err + dx - dy);

        PutPixelAlpha(x0, y0, rgb, alpha);
        PutPixelAlpha(x0, y1, rgb, alpha);
        PutPixelAlpha(x1, y0, rgb, alpha);
        PutPixelAlpha(x1, y1, rgb, alpha);

        if (f = 2 * err + dy >= 0)
        {
            if (x0 >= x1) break;
            alpha = ed * (err + dx);
            if (alpha < 255)
            {
                PutPixelAlpha(x0, y0 + 1, rgb, alpha);
                PutPixelAlpha(x0, y1 - 1, rgb, alpha);
                PutPixelAlpha(x1, y0 + 1, rgb, alpha);
                PutPixelAlpha(x1, y1 - 1, rgb, alpha);
            }
        }

        if (2 * err <= dx)
        {
            alpha = ed * (dy - err);
            if (alpha < 255)
            {
                PutPixelAlpha(x0 + 1, y0, rgb, alpha);
                PutPixelAlpha(x1 - 1, y0, rgb, alpha);
                PutPixelAlpha(x0 + 1, y1, rgb, alpha);
                PutPixelAlpha(x1 - 1, y1, rgb, alpha);
            }

            y0++;
            y1--;
            err += dy += a; 
        }

        if (f)
        {
            x0++;
            x1--;
            err -= dx -= b1;
        }
    }

    if (--x0 == x1++)
    {
        while (y0 - y1 < b)
        {
            alpha = 255 * 4 * fabs(err + dx) / b1;
            PutPixelAlpha(x0, ++y0, rgb, alpha);
            PutPixelAlpha(x1, y0, rgb, alpha);
            PutPixelAlpha(x0, --y1, rgb, alpha);
            PutPixelAlpha(x1, y1, rgb, alpha);
            err += dy += a; 
        }
    }
}

void DrawQuadBezierSeg(int x0, int y0, int x1, int y1, int x2, int y2, unsigned int rgb)
{
    int sx = x2 - x1, sy = y2 - y1;
    long xx = x0 - x1, yy = y0 - y1, xy;
    float dx, dy, err, cur = xx * sy - yy * sx;

    if (sx * sx + sy * sy > xx * xx + yy * yy)
    {
        x2 = x0;
        x0 = sx + x1;
        y2 = y0;
        y0 = sy + y1;
        cur = -cur;
    }

    if (cur != 0)
    {
        xx += sx;
        xx *= sx = x0 < x2 ? 1 : -1;
        yy += sy;
        yy *= sy = y0 < y2 ? 1 : -1;
        xy = 2 * xx * yy;
        xx *= xx;
        yy *= yy;

        if (cur * sx * sy < 0)
        {
            xx = -xx;
            yy = -yy;
            xy = -xy;
            cur = -cur;
        }

        dx = 4.0 * sy * cur * (x1 - x0) + xx - xy;
        dy = 4.0 * sx * cur * (y0 - y1) + yy - xy;
        xx += xx;
        yy += yy;
        err = dx + dy + xy;

        do {
            PutPixel(x0, y0, rgb);
            if (x0 == x2 && y0 == y2) return;
            y1 = 2 * err < dx;
            if (2 * err > dy)
            {
                x0 += sx;
                dx -= xy;
                err += dy += yy;
            }
            if (y1)
            {
                y0 += sy;
                dy -= xy;
                err += dx += xx;
            }
        } while (dy < 0 && dx > 0);
    }
    DrawLine(x0, y0, x2, y2, rgb);
}

void DrawQuadBezier(int x0, int y0, int x1, int y1, int x2, int y2, unsigned int rgb)
{
    int x = x0 - x1, y = y0 - y1;
    float t = x0 - 2 * x1 + x2, r;

    if (x * (x2 - x1) > 0)
    {
        if ((y * (y2 - y1) > 0) && fabs((y0 - 2 * y1 + y2) / t * x) > abs(y))
        {
            x0 = x2; x2 = x + x1; y0 = y2; y2 = y + y1;
        }
        t = (x0 - x1) / t;
        r = (1 - t) * ((1 - t) * y0 + 2.0 * t * y1) + t * t * y2;
        t = (x0 * x2 - x1 * x1) * t / (x0 - x1);
        x = floor(t + 0.5);
        y = floor(r + 0.5);      
        r = (y1 - y0) * (t - x0) / (x1 - x0) + y0;
        DrawQuadBezierSeg(x0, y0, x, floor(r + 0.5), x, y, rgb);
        r = (y1 - y2) * (t - x2) / (x1 - x2) + y2;
        x0 = x1 = x; y0 = y; y1 = floor(r + 0.5);
    }
    if ((y0 - y1) * (y2 - y1) > 0)
    {
        t = y0 - 2 * y1 + y2;
        t = (y0 - y1) / t;       
        r = (1 - t) * ((1 - t) * x0 + 2.0 * t * x1) + t * t * x2;
        t = (y0 * y2 - y1 * y1) * t / (y0 - y1);
        x = floor(r + 0.5);
        y = floor(t + 0.5);
        r = (x1 - x0) * (t - y0) / (y1 - y0) + x0;
        DrawQuadBezierSeg(x0, y0, floor(r + 0.5), y, x, y, rgb);
        r = (x1 - x2) * (t - y2) / (y1 - y2) + x2;
        x0 = x; x1 = floor(r + 0.5); y0 = y1 = y;
    }
    DrawQuadBezierSeg(x0, y0, x1, y1, x2, y2, rgb);
}

void DrawQuadRationalBezierSeg(int x0, int y0, int x1, int y1, int x2, int y2, float w, unsigned int rgb)
{
    int sx = x2 - x1, sy = y2 - y1;
    float dx = x0 - x2, dy = y0 - y2, xx = x0 - x1, yy = y0 - y1;
    float xy = xx * sy + yy * sx, cur = xx * sy - yy * sx, err;

    if (cur != 0.0 && w > 0.0)
    {
        if (sx * sx + sy * sy > xx * xx + yy * yy)
        {
            x2 = x0;
            x0 -= dx;
            y2 = y0;
            y0 -= dy;
            cur = -cur;
        }

        xx = 2.0 * (4.0 * w * sx * xx + dx * dx);
        yy = 2.0 * (4.0 * w * sy * yy + dy * dy);
        sx = x0 < x2 ? 1 : -1;
        sy = y0 < y2 ? 1 : -1;
        xy = -2.0 * sx * sy * (2.0 * w * xy + dx * dy);

        if (cur * sx * sy < 0.0)
        {
            xx = -xx;
            yy = -yy;
            xy = -xy;
            cur = -cur;
        }

        dx = 4.0 * w * (x1 - x0) * sy * cur + xx / 2.0 + xy;
        dy = 4.0 * w * (y0 - y1) * sx * cur + yy / 2.0 + xy;

        if (w < 0.5 && (dy > xy || dx < xy))
        {
            cur = (w + 1.0) / 2.0;
            w = sqrt(w);
            xy = 1.0 / (w + 1.0);
            sx = floor((x0 + 2.0 * w * x1 + x2) * xy / 2.0 + 0.5);
            sy = floor((y0 + 2.0 * w * y1 + y2) * xy / 2.0 + 0.5);
            dx = floor((w * x1 + x0) * xy + 0.5);
            dy = floor((y1 * w + y0) * xy + 0.5);
            DrawQuadRationalBezierSeg(x0, y0, dx, dy, sx, sy, cur, rgb);
            dx = floor((w * x1 + x2) * xy + 0.5);
            dy = floor((y1 * w + y2) * xy + 0.5);
            DrawQuadRationalBezierSeg(sx, sy, dx, dy, x2, y2, cur, rgb);
            return;
        }

        err = dx + dy - xy;

        do {
            PutPixel(x0, y0, rgb);
            if (x0 == x2 && y0 == y2) return;
            x1 = 2 * err > dy;
            y1 = 2 * (err + yy) < -dy;
            if (2 * err < dx || y1) { y0 += sy; dy += xy; err += dx += xx; }
            if (2 * err > dx || x1) { x0 += sx; dx += xy; err += dy += yy; }
        } while (dy <= xy && dx >= xy);
    }
    DrawLine(x0, y0, x2, y2, rgb);
}

void DrawQuadRationalBezier(int x0, int y0, int x1, int y1, int x2, int y2, float w, unsigned int rgb)
{
    int x = x0 - 2 * x1 + x2, y = y0 - 2 * y1 + y2;
    float xx = x0 - x1, yy = y0 - y1, ww, t, q;

    if (xx * (x2 - x1) > 0)
    {
        if (yy * (y2 - y1) > 0 && fabs(xx * y) > fabs(yy * x))
        {
            x0 = x2;
            x2 = xx + x1;
            y0 = y2;
            y2 = yy + y1;
        }

        if (x0 == x2 || w == 1.0) t = (x0 - x1) / (float)x;
        else
        {
            q = sqrt(4.0 * w * w * (x0 - x1) * (x2 - x1) + (x2 - x0) * (x2 - x0));
            if (x1 < x0) q = -q;
            t = (2.0 * w * (x0 - x1) - x0 + x2 + q) / (2.0 * (1.0 - w) * (x2 - x0));
        }

        q = 1.0 / (2.0 * t * (1.0 - t) * (w - 1.0) + 1.0);
        xx = (t * t * (x0 - 2.0 * w * x1 + x2) + 2.0 * t * (w * x1 - x0) + x0) * q;
        yy = (t * t * (y0 - 2.0 * w * y1 + y2) + 2.0 * t * (w * y1 - y0) + y0) * q;
        ww = t * (w - 1.0) + 1.0;
        ww *= ww * q;
        w = ((1.0 - t) * (w - 1.0) + 1.0) * sqrt(q);
        x = floor(xx + 0.5);
        y = floor(yy + 0.5);
        yy = (xx - x0) * (y1 - y0) / (x1 - x0) + y0;
        DrawQuadRationalBezierSeg(x0, y0, x, floor(yy + 0.5), x, y, ww, rgb);
        yy = (xx - x2) * (y1 - y2) / (x1 - x2) + y2;
        y1 = floor(yy + 0.5);
        x0 = x1 = x;
        y0 = y;
    }

    if ((y0 - y1) * (y2 - y1) > 0)
    {
        if (y0 == y2 || w == 1.0) t = (y0 - y1) / (y0 - 2.0 * y1 + y2);
        else
        {
            q = sqrt(4.0 * w * w * (y0 - y1) * (y2 - y1) + (y2 - y0) * (y2-y0));
            if (y1 < y0) q = -q;
            t = (2.0 * w * (y0 - y1) - y0 + y2 + q) / (2.0 * (1.0 - w) * (y2 - y0));
        }
        q = 1.0 / (2.0 * t * (1.0 - t) * (w - 1.0) + 1.0);
        xx = (t * t * (x0 - 2.0 * w * x1 + x2) + 2.0 * t * (w * x1 - x0) + x0) * q;
        yy = (t * t * (y0 - 2.0 * w * y1 + y2) + 2.0 * t * (w * y1 - y0) + y0) * q;
        ww = t * (w - 1.0) + 1.0;
        ww *= ww * q;
        w = ((1.0 - t) * (w - 1.0) + 1.0) * sqrt(q);
        x = floor(xx + 0.5);
        y = floor(yy + 0.5);
        xx = (x1 - x0) * (yy - y0) / (y1 - y0) + x0;
        DrawQuadRationalBezierSeg(x0, y0, floor(xx + 0.5), y, x, y, ww, rgb);
        xx = (x1 - x2) * (yy - y2) / (y1 - y2) + x2;
        x1 = floor(xx + 0.5);
        x0 = x;
        y0 = y1 = y;
    }
    DrawQuadRationalBezierSeg(x0, y0, x1, y1, x2, y2, w * w, rgb);
}

void DrawRotatedEllipseRect(int x0, int y0, int x1, int y1, long zd, unsigned int rgb)
{
    int xd = x1 - x0, yd = y1 - y0;
    float w = xd * yd;

    if (zd == 0) { DrawEllipseRect(x0, y0, x1, y1, rgb); return; }
    if (w != 0.0) w = (w - zd) / (w + w);

    xd = floor(xd * w + 0.5);
    yd = floor(yd * w + 0.5);
    DrawQuadRationalBezierSeg(x0, y0 + yd, x0, y0, x0 + xd, y0, 1.0 - w, rgb);
    DrawQuadRationalBezierSeg(x0, y0 + yd, x0, y1, x1 - xd, y1, w, rgb);
    DrawQuadRationalBezierSeg(x1, y1 - yd, x1, y1, x1 - xd, y1, 1.0 - w, rgb);
    DrawQuadRationalBezierSeg(x1, y1 - yd, x1, y0, x0 + xd, y0, w, rgb);
}

void DrawRotatedEllipse(int x, int y, int a, int b, float angle, unsigned int rgb)
{
    float xd = a * a, yd = b * b;
    float s = sin(angle), zd = (xd - yd) * s;
    xd = sqrt(xd - zd * s), yd = sqrt(yd + zd * s);
    a = xd + 0.5; b = yd + 0.5; zd = zd * a * b / (xd * yd);
    DrawRotatedEllipseRect(x - a, y - b, x + a, y + b, 4 * zd * cos(angle), rgb);
}

void DrawCubicBezierSeg(int x0, int y0, float x1, float y1, float x2, float y2, int x3, int y3, unsigned int rgb)
{
    int f, fx, fy, leg = 1;
    int sx = x0 < x3 ? 1 : -1, sy = y0 < y3 ? 1 : -1;
    float xc = -fabs(x0 + x1 - x2 - x3), xa = xc - 4 * sx * (x1 - x2), xb = sx * (x0 - x1 - x2 + x3);
    float yc = -fabs(y0 + y1 - y2 - y3), ya = yc - 4 * sy * (y1 - y2), yb = sy * (y0 - y1 - y2 + y3);
    float ab, ac, bc, cb, xx, xy, yy, dx, dy, ex, *pxy, EP = 0.01;

    if (xa == 0 && ya == 0)
    {
        sx = floor((3 * x1 - x0 + 1) / 2);
        sy = floor((3 * y1 - y0 + 1) / 2);
        DrawQuadBezierSeg(x0, y0, sx, sy, x3, y3, rgb);
        return;
    }

    x1 = (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0) + 1;
    x2 = (x2 - x3) * (x2 - x3) + (y2 - y3) * (y2 - y3) + 1;
    do {
        ab = xa * yb - xb * ya;
        ac = xa * yc - xc * ya;
        bc = xb * yc - xc * yb;
        ex = ab * (ab + ac - 3 * bc) + ac * ac;
        f = ex > 0 ? 1 : sqrt(1 + 1024 / x1);
        ab *= f; ac *= f; bc *= f; ex *= f*f;
        xy = 9 * (ab + ac + bc) / 8;
        cb = 8 * (xa - ya);
        dx = 27 * (8 * ab * (yb * yb - ya * yc) + ex * (ya + 2 * yb + yc)) / 64 - ya * ya * (xy - ya);
        dy = 27 * (8 * ab * (xb * xb - xa * xc) - ex * (xa + 2 * xb + xc)) / 64 - xa * xa * (xy + xa);

        xx = 3 * (3 * ab * (3 * yb * yb - ya * ya - 2 * ya * yc) - ya * (3 * ac * (ya + yb) + ya * cb)) / 4;
        yy = 3 * (3 * ab * (3 * xb * xb - xa * xa - 2 * xa * xc) - xa * (3 * ac * (xa + xb) + xa * cb)) / 4;
        xy = xa * ya * (6 * ab + 6 * ac - 3 * bc + cb); ac = ya * ya; cb = xa * xa;
        xy = 3 * (xy + 9 * f * (cb * yb * yc - xb * xc * ac) - 18 * xb * yb * ab) / 8;

        if (ex < 0)
        {
            dx = -dx; dy = -dy; xx = -xx; yy = -yy; xy = -xy; ac = -ac; cb = -cb;
        }

        ab = 6 * ya * ac; ac = -6 * xa * ac; bc = 6 * ya * cb; cb = -6 * xa * cb;
        dx += xy; ex = dx + dy; dy += xy;

        for (pxy = &xy, fx = fy = f; x0 != x3 && y0 != y3; )
        {
            PutPixel(x0, y0, rgb);
            do {
                if (dx > *pxy || dy < *pxy) goto exit;
                y1 = 2 * ex - dy;
                if (2 * ex >= dx)
                {
                    fx--;
                    ex += dx += xx;
                    dy += xy += ac;
                    yy += bc;
                    xx += ab;
                }
                if (y1 <= 0)
                {
                    fy--;
                    ex += dy += yy;
                    dx += xy += bc;
                    xx += ac;
                    yy += cb;
                }
            } while (fx > 0 && fy > 0);

            if (2 * fx <= f) { x0 += sx; fx += f; }
            if (2 * fy <= f) { y0 += sy; fy += f; }
            if (pxy == &xy && dx < 0 && dy > 0) pxy = &EP;
        }
        exit:
        xx = x0; x0 = x3; x3 = xx; sx = -sx; xb = -xb;
        yy = y0; y0 = y3; y3 = yy; sy = -sy; yb = -yb; x1 = x2;
    } while (leg--);
    DrawLine(x0, y0, x3, y3, rgb);
}

void DrawCubicBezier(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, unsigned int rgb)
{
    int n = 0, i = 0;
    long xc = x0 + x1 - x2 - x3, xa = xc - 4 * (x1 - x2);
    long xb = x0 - x1 - x2 + x3, xd = xb + 4 * (x1 + x2);
    long yc = y0 + y1 - y2 - y3, ya = yc - 4 * (y1 - y2);
    long yb = y0 - y1 - y2 + y3, yd = yb + 4 * (y1 + y2);
    float fx0 = x0, fx1, fx2, fx3, fy0 = y0, fy1, fy2, fy3;
    float t1 = xb * xb - xa * xc, t2, t[5];

    if (xa == 0)
    {
        if (abs(xc) < 2 * abs(xb)) t[n++] = xc / (2.0 * xb);
    }
    else if (t1 > 0.0)
    {
        t2 = sqrt(t1);
        t1 = (xb - t2) / xa; if (fabs(t1) < 1.0) t[n++] = t1;
        t1 = (xb + t2) / xa; if (fabs(t1) < 1.0) t[n++] = t1;
    }

    t1 = yb * yb - ya * yc;
    if (ya == 0)
    {
        if (abs(yc) < 2 * abs(yb)) t[n++] = yc / (2.0 * yb);
    }
    else if (t1 > 0.0)
    {
        t2 = sqrt(t1);
        t1 = (yb - t2) / ya; if (fabs(t1) < 1.0) t[n++] = t1;
        t1 = (yb + t2) / ya; if (fabs(t1) < 1.0) t[n++] = t1;
    }

    for (i = 1; i != n; i++)
    {
        if ((t1 = t[i - 1]) > t[i])
        {
            t[i - 1] = t[i];
            t[i] = t1;
            i = 0;
        }
    }

    t1 = -1.0; t[n] = 1.0;
    for (i = 0; i <= n; i++)
    {
        t2 = t[i];
        fx1 = (t1 * (t1 * xb - 2 * xc) - t2 * (t1 * (t1 * xa - 2 * xb) + xc) + xd) / 8 - fx0;
        fy1 = (t1 * (t1 * yb - 2 * yc) - t2 * (t1 * (t1 * ya - 2 * yb) + yc) + yd) / 8 - fy0;
        fx2 = (t2 * (t2 * xb - 2 * xc) - t1 * (t2 * (t2 * xa - 2 * xb) + xc) + xd) / 8 - fx0;
        fy2 = (t2 * (t2 * yb - 2 * yc) - t1 * (t2 * (t2 * ya - 2 * yb) + yc) + yd) / 8 - fy0;
        fx0 -= fx3 = (t2 * (t2 * (3 * xb - t2 * xa) - 3 * xc) + xd) / 8;
        fy0 -= fy3 = (t2 * (t2 * (3 * yb - t2 * ya) - 3 * yc) + yd) / 8;
        x3 = floor(fx3 + 0.5);
        y3 = floor(fy3 + 0.5);
        if (fx0 != 0.0) { fx1 *= fx0 = (x0 - x3) / fx0; fx2 *= fx0; }
        if (fy0 != 0.0) { fy1 *= fy0 = (y0 - y3) / fy0; fy2 *= fy0; }
        if (x0 != x3 || y0 != y3) DrawCubicBezierSeg(x0, y0, x0 + fx1, y0 + fy1, x0 + fx2, y0 + fy2, x3, y3, rgb);
        x0 = x3; y0 = y3; fx0 = fx3; fy0 = fy3; t1 = t2;
    }
}

void DrawLineWidthAlpha(int x0, int y0, int x1, int y1, float wd, unsigned int rgb)
{
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1; 
    int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1; 
    int err = dx - dy, e2, x2, y2;
    float ed = dx + dy == 0 ? 1 : sqrt(dx * dx + dy * dy);

    // Only 32bit support alpha-blend mode
    if (bytesPerPixel != 4) return;

    wd = (wd + 1) / 2;
    while(1)
    {
        PutPixelAlpha(x0, y0, rgb, max(0, 255 * (abs(err - dx + dy) / ed - wd + 1)));

        e2 = err; x2 = x0;
        if (2 * e2 >= -dx)
        {
            for (e2 += dy, y2 = y0; e2 < ed * wd && (y1 != y2 || dx > dy); e2 += dx) PutPixelAlpha(x0, y2 += sy, rgb, max(0, 255 * (abs(e2) / ed - wd + 1)));
            if (x0 == x1) break;
            e2 = err;
            err -= dy;
            x0 += sx; 
        }
        if (2 * e2 <= dy)
        {
            for (e2 = dx-e2; e2 < ed * wd && (x1 != x2 || dx < dy); e2 += dy) PutPixelAlpha(x2 += sx, y0, rgb, max(0, 255 * (abs(e2) / ed - wd + 1)));
            if (y0 == y1) break;
            err += dx;
            y0 += sy; 
        }
    }
}

void DrawQuadBezierSegBlend(int x0, int y0, int x1, int y1, int x2, int y2, unsigned int rgb)
{
    int sx = x2 - x1, sy = y2 - y1;
    long xx = x0 - x1, yy = y0 - y1, xy;
    float dx, dy, err, ed, cur = xx * sy - yy * sx;

    // Only 32bit support alpha-blend mode
    if (bytesPerPixel != 4) return;

    if (sx * sx + sy * sy > xx * xx + yy * yy)
    {
        x2 = x0;
        x0 = sx + x1;
        y2 = y0;
        y0 = sy + y1;
        cur = -cur;
    }

    if (cur != 0)
    {
        xx += sx; xx *= sx = x0 < x2 ? 1 : -1;
        yy += sy; yy *= sy = y0 < y2 ? 1 : -1;
        xy = 2 * xx * yy;
        xx *= xx;
        yy *= yy;
        if (cur * sx * sy < 0)
        {
            xx = -xx;
            yy = -yy;
            xy = -xy;
            cur = -cur;
        }

        dx = 4.0 * sy * (x1 - x0) * cur + xx - xy;
        dy = 4.0 * sx * (y0 - y1) * cur + yy - xy;
        xx += xx; yy += yy; err = dx + dy + xy;

        do {
            cur = min(dx + xy, -xy - dy);
            ed = max(dx + xy, -xy - dy);
            ed += 2 * ed * cur * cur / (4 * ed * ed + cur * cur);
            PutPixelAlpha(x0, y0, rgb, 255 * fabs(err - dx - dy - xy) / ed);

            if (x0 == x2 || y0 == y2) break;

            x1 = x0; cur = dx - err; y1 = 2 * err + dy < 0;
            if (2 * err + dx > 0)
            {
                if (err - dy < ed) PutPixelAlpha(x0, y0 + sy, rgb, 255 * fabs(err - dy) / ed);
                x0 += sx; dx -= xy; err += dy += yy;
            }
            if (y1)
            {
                if (cur < ed) PutPixelAlpha(x1 + sx, y0, rgb, 255 * fabs(cur) / ed);
                y0 += sy; dy -= xy; err += dx += xx;
            }
        } while (dy < dx);
    }
    DrawLineAlpha(x0, y0, x2, y2, rgb);
}

void DrawQuadRationalBezierSegAlpha(int x0, int y0, int x1, int y1, int x2, int y2, float w, unsigned int rgb)
{
    int sx = x2 - x1, sy = y2 - y1;
    float dx = x0 - x2, dy = y0 - y2, xx = x0 - x1, yy = y0 - y1;
    float xy = xx * sy + yy * sx, cur = xx * sy - yy * sx, err, ed;
    int f;

    // Only 32bit support alpha-blend mode
    if (bytesPerPixel != 4) return;

    if (cur != 0.0 && w > 0.0)
    {
        if (sx * sx + sy * sy > xx * xx + yy * yy)
        {
            x2 = x0; x0 -= dx; y2 = y0; y0 -= dy; cur = -cur;
        }

        xx = 2.0 * (4.0 * w * sx * xx + dx * dx);
        yy = 2.0 * (4.0 * w * sy * yy + dy * dy);
        sx = x0 < x2 ? 1 : -1;
        sy = y0 < y2 ? 1 : -1;
        xy = -2.0 * sx * sy * (2.0 * w * xy + dx * dy);

        if (cur * sx * sy < 0)
        {
            xx = -xx; yy = -yy; cur = -cur; xy = -xy;
        }

        dx = 4.0 * w * (x1 - x0) * sy * cur + xx / 2.0 + xy;
        dy = 4.0 * w * (y0 - y1) * sx * cur + yy / 2.0 + xy;

        if (w < 0.5 && dy > dx)
        {
            cur = (w + 1.0) / 2.0;
            w = sqrt(w);
            xy = 1.0 / (w + 1.0);
            sx = floor((x0 + 2.0 * w * x1 + x2) * xy / 2.0 + 0.5);
            sy = floor((y0 + 2.0 * w * y1 + y2) * xy / 2.0 + 0.5);
            dx = floor((w * x1 + x0) * xy + 0.5);
            dy = floor((y1 * w + y0) * xy + 0.5);
            DrawQuadRationalBezierSegAlpha(x0, y0, dx, dy, sx, sy, cur, rgb);
            dx = floor((w * x1 + x2) * xy + 0.5);
            dy = floor((y1 * w + y2) * xy + 0.5);
            DrawQuadRationalBezierSegAlpha(sx, sy, dx, dy, x2, y2, cur, rgb);
            return;
        }

        err = dx + dy - xy;

        do {
            cur = min(dx - xy, xy - dy);
            ed = max(dx - xy, xy - dy);
            ed += 2 * ed * cur * cur / (4. * ed * ed + cur * cur);
            x1 = 255 * fabs(err - dx - dy + xy) / ed;
            if (x1 < 256) PutPixelAlpha(x0, y0, rgb, x1);
            if (f = 2 * err + dy < 0)
            {
                if (y0 == y2) return;
                if (dx - err < ed) PutPixelAlpha(x0 + sx, y0, rgb, 255 * fabs(dx - err) / ed);
            }
            if (2 * err + dx > 0)
            {
                if (x0 == x2) return;
                if (err - dy < ed) PutPixelAlpha(x0, y0 + sy, rgb, 255 * fabs(err - dy) / ed);
                x0 += sx; dx += xy; err += dy += yy;
            }
            if (f) { y0 += sy; dy += xy; err += dx += xx; }
        } while (dy < dx);
    }
    DrawLineAlpha(x0, y0, x2, y2, rgb);
}

void DrawCubicBezierSegAlpha(int x0, int y0, float x1, float y1, float x2, float y2, int x3, int y3, unsigned int rgb)
{
    int f, fx, fy, leg = 1;
    int sx = x0 < x3 ? 1 : -1, sy = y0 < y3 ? 1 : -1;
    float xc = -fabs(x0 + x1 - x2 - x3), xa = xc - 4 * sx * (x1 - x2), xb = sx * (x0 - x1 - x2 + x3);
    float yc = -fabs(y0 + y1 - y2 - y3), ya = yc - 4 * sy * (y1 - y2), yb = sy * (y0 - y1 - y2 + y3);
    float ab, ac, bc, ba, xx, xy, yy, dx, dy, ex, px, py, ed, ip, EP = 0.01;

    // Only 32bit support alpha-blend mode
    if (bytesPerPixel != 4) return;

    if (xa == 0 && ya == 0)
    {
        sx = floor((3 * x1 - x0 + 1) / 2);
        sy = floor((3 * y1 - y0 + 1) / 2);
        DrawQuadBezierSegBlend(x0, y0, sx, sy, x3, y3, rgb);
        return;
    }

    x1 = (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0) + 1;
    x2 = (x2 - x3) * (x2 - x3) + (y2 - y3) * (y2 - y3) + 1;

    do {
        ab = xa * yb - xb * ya;
        ac = xa * yc - xc * ya;
        bc = xb * yc - xc * yb;
        ip = 4 * ab * bc - ac * ac;
        ex = ab * (ab + ac - 3 * bc) + ac * ac;
        f = ex > 0 ? 1 : sqrt(1 + 1024 / x1);
        ab *= f; ac *= f; bc *= f; ex *= f * f;
        xy = 9 * (ab + ac + bc) / 8;
        ba = 8 * (xa - ya);
        dx = 27 * (8 * ab * (yb * yb - ya * yc) + ex * (ya + 2 * yb + yc)) / 64 - ya * ya * (xy - ya);
        dy = 27 * (8 * ab * (xb * xb - xa * xc) - ex * (xa + 2 * xb + xc)) / 64 - xa * xa * (xy + xa);

        xx = 3 * (3 * ab * (3 * yb * yb - ya * ya - 2 * ya * yc) - ya * (3 * ac * (ya + yb) + ya * ba)) / 4;
        yy = 3 * (3 * ab * (3 * xb * xb - xa * xa - 2 * xa * xc) - xa * (3 * ac * (xa + xb) + xa * ba)) / 4;
        xy = xa * ya * (6 * ab + 6 * ac - 3 * bc + ba); ac = ya * ya; ba = xa * xa;
        xy = 3 * (xy + 9 * f * (ba * yb * yc - xb * xc * ac) - 18 * xb * yb * ab) / 8;

        if (ex < 0)
        {
            dx = -dx; dy = -dy; xx = -xx; yy = -yy; xy = -xy; ac = -ac; ba = -ba;
        }

        ab = 6 * ya * ac; ac = -6 * xa * ac; bc = 6 * ya * ba; ba = -6 * xa * ba;
        dx += xy; ex = dx + dy; dy += xy;

        for (fx = fy = f; x0 != x3 && y0 != y3;)
        {
            y1 = min(fabs(xy - dx), fabs(dy - xy));
            ed = max(fabs(xy - dx), fabs(dy - xy));
            ed = f * (ed + 2 * ed * y1 * y1 / (4 * ed * ed + y1 * y1));
            y1 = 255 * fabs(ex - (f - fx + 1) * dx - (f - fy + 1) * dy + f * xy) / ed;
            if (y1 < 256) PutPixelAlpha(x0, y0, rgb, y1);
            px = fabs(ex - (f - fx + 1) * dx + (fy - 1) * dy);
            py = fabs(ex + (fx - 1) * dx - (f - fy + 1) * dy);
            y2 = y0;
            do {
                if ((ip >= -EP) && (dx + xx > xy || dy + yy < xy)) goto exit;
                y1 = 2 * ex + dx;
                if (2 * ex + dy > 0)
                {
                    fx--; ex += dx += xx; dy += xy += ac; yy += bc; xx += ab;
                } else if (y1 > 0) goto exit;
                if (y1 <= 0)
                {
                    fy--; ex += dy += yy; dx += xy += bc; xx += ac; yy += ba;
                }
            } while (fx > 0 && fy > 0);

            if (2 * fy <= f)
            {
                if (py < ed) PutPixelAlpha(x0 + sx, y0, rgb, 255 * py / ed);
                y0 += sy; fy += f;
            }

            if (2 * fx <= f)
            {
                if (px < ed) PutPixelAlpha(x0, y2 + sy, rgb, 255 * px / ed);
                x0 += sx; fx += f;
            }
        }

        exit:
        if (2 * ex < dy && 2 * fy <= f + 2)
        {
            if (py < ed) PutPixelAlpha(x0 + sx, y0, rgb, 255 * py / ed);
            y0 += sy;
        }

        if (2 * ex > dx && 2 * fx <= f + 2)
        {
            if (px < ed) PutPixelAlpha(x0, y2 + sy, rgb, 255 * px / ed);
            x0 += sx;
        }

        xx = x0; x0 = x3; x3 = xx; sx = -sx; xb = -xb;
        yy = y0; y0 = y3; y3 = yy; sy = -sy; yb = -yb; x1 = x2;
    } while (leg--);
    DrawLineAlpha(x0, y0, x3, y3, rgb);
}

void InitStack(STACK *stack)
{
    memset(stack->data, -1, sizeof(stack->data)); 
    stack->elem = 0;
}

void Push(STACK *stack, int item)
{
    if (stack->elem > 2000) FatalError("Push: Stack Full (n > 2000).\n");
    stack->data[stack->elem++] = item;
}

int Pop(STACK *stack)
{
    if (stack->elem == 0) FatalError("Pop: Stack Empty.\n");
    return stack->data[--stack->elem];
}

int IsEmpty(STACK *stack)
{
    return stack->elem == 0;
}

int ScanLeft(int sx, int sy, unsigned int fcol, unsigned int bcol)
{
    unsigned int col;

    do {
        sx--;
        col = GetPixel(sx, sy);
    } while (col != bcol && col != fcol && sx > 0);

    return (!sx) ? -1 : ++sx;
}

int ScanRight(int sx, int sy, unsigned int fcol, unsigned int bcol)
{
    unsigned int col;

    do {
        sx++;
        col = GetPixel(sx, sy);
    } while (col != bcol && col != fcol && sx < 2000);

    return (sx == 2000) ? -1 : --sx;
}

void RegionFill(int seedx, int seedy, unsigned int bcol, unsigned int fcol)
{
    STACK stack;
    int sx, sy;
    int xl, xll;
    int xr, xrr;

    InitStack(&stack);
    Push(&stack, seedx);
    Push(&stack, seedy);

    while (!IsEmpty(&stack))
    {
        sy = Pop(&stack);
        sx = Pop(&stack);

        xl = ScanLeft(sx, sy, fcol, bcol);
        xr = ScanRight(sx, sy, fcol, bcol);

        HorizLine(xl, sy, xr - xl + 1, fcol);
        sy++;

        xll = ScanLeft((xl + xr) >> 1, sy, fcol, bcol);
        xrr = ScanRight((xl + xr) >> 1, sy, fcol, bcol);

        if (xll == -1 || xrr == 2000) break;

        if (xll < xrr)
        {
            Push(&stack, (xll + xrr) >> 1);
            Push(&stack, sy);
        }
    }

    InitStack(&stack);
    Push(&stack, seedx);
    Push(&stack, seedy - 1);

    while (!IsEmpty(&stack))
    {
        sy = Pop(&stack);
        sx = Pop(&stack);

        xl = ScanLeft(sx, sy, fcol, bcol);
        xr = ScanRight(sx, sy, fcol, bcol);

        HorizLine(xl, sy, xr - xl + 1, fcol);
        sy--;

        xll = ScanLeft((xl + xr) >> 1, sy, fcol, bcol);
        xrr = ScanRight((xl + xr) >> 1, sy, fcol, bcol);

        if (xll < 0 || xrr >= 2000) break;

        if (xll < xrr)
        {
            Push(&stack, (xll + xrr) >> 1);
            Push(&stack, sy);
        }
    }
}

void CreatePlasma(unsigned char *dx, unsigned char *dy, unsigned char *sint, unsigned char *cost, IMAGE *img)
{
    unsigned char lx = (*dx) += 2;
    unsigned char ly = (*dy)--;

    void *data = img->mData;
    unsigned int ofs = img->mWidth;
    unsigned char sx = img->mWidth >> 1;
    unsigned char sy = img->mHeight >> 1;

    _asm {
        mov    edi, data
        xor    eax, eax
        xor    dh, dh
    next0:
        xor    bh, bh
        mov    al, dh
        mov    bl, ly
        add    ax, bx
        cmp    ax, 255
        jbe    skip0
        sub    ax, 255
    skip0:
        mov    esi, sint
        add    esi, eax
        mov    cl, [esi]
        mov    al, lx
        mov    esi, sint
        add    esi, eax
        mov    ch, [esi]
        xor    dl, dl
    next1:
        xor    bh, bh
        mov    al, dl
        mov    bl, cl
        add    ax, bx
        cmp    ax, 255
        jbe    skip1
        sub    ax, 255
    skip1:
        mov    esi, sint
        add    esi, eax
        mov    bl, [esi]
        mov    al, dh
        add    al, ch
        mov    esi, cost
        add    esi, eax
        add    bl, [esi]
        shr    bl, 1
        add    bl, 128
        mov    bh, bl
        mov    esi, ofs
        mov    [edi], bx
        mov    [edi + esi], bx
        add    edi, 2
        inc    dl
        cmp    dl, sx
        jb     next1
        add    edi, esi
        inc    dh
        cmp    dh, sy
        jb     next0
    }
}

void InitPlasma(unsigned char *sint, unsigned char *cost)
{
    int i;
    RGB pal[256] = {0};

    for (i = 0; i != 256; i++)
    {
        sint[i] = sin(2 * M_PI * i / 255) * 128 + 128;
        cost[i] = cos(2 * M_PI * i / 255) * 128 + 128;
    }

    for (i = 0; i != 64; i++)
    {
        SetRGB(i, i, 0, i << 2);
        SetRGB(127 - i, i, 0, i << 2);
        SetRGB(127 + i, i << 2, i << 1, 0);
        SetRGB(254 - i, i << 2, i << 1, 0);
    }

    GetPalette(pal);
    for (i = 127; i >= 0; i--) SetRGB(i + 128, pal[i << 1].r, pal[i << 1].g, pal[i << 1].b);
}

void PrepareTunnel(IMAGE *dimg, unsigned char *buf1, unsigned char *buf2)
{
    const int maxAng = 1024;
    const float preCalc = M_PI / (maxAng >> 1);

    int x, y, ofs;
    float ang, angDec;
    float z, zdec;
    float dst, dstInc;

    zdec   = 0.7;
    angDec = zdec;
    z      = 207;
    ang    = maxAng - 1;
    dst    = 1;
    dstInc = 0.02;

    do {
        x = RoundInt(z * sin(ang * preCalc)) + (dimg->mWidth >> 1);
        y = RoundInt(z * cos(ang * preCalc)) + (dimg->mHeight >> 1);

        ang -= angDec;
        if (ang < 0)
        {
            ang += maxAng;
            dst += dst * dstInc;
            z   -= zdec;
        }

        if (x >= 0 && x < dimg->mWidth && y >= 0 && y < dimg->mHeight)
        {
            ofs = y * dimg->mWidth + x;
            buf1[ofs] = RoundInt(dst);
            buf2[ofs] = RoundInt(dst - ang / 4);
        }
    } while (z >= 0);
}

void DrawTunnel(IMAGE *dimg, IMAGE *simg, unsigned char *buf1, unsigned char *buf2, unsigned char *mov, unsigned char step)
{
    unsigned char val;
    unsigned int size = dimg->mSize >> 2;
    unsigned int *dst = (unsigned int*)dimg->mData;
    unsigned int *src = (unsigned int*)simg->mData;

    if (bytesPerPixel != 4) return;

    *mov += step;
    while (size--)
    {
        val = *buf1++ + *mov;
        *dst++ = *(src + ((val << 8) | *buf2++));
    }
}

uint32_t RGB2INT(uint8_t r, uint8_t g, uint8_t b)
{
    return (r << 16) | (g << 8) | (b);
}

RGB INT2RGB(uint32_t color)
{
    RGB rgb;
    rgb.r = (color >> 16) & 0xFF;
    rgb.g = (color >> 8 ) & 0xFF;
    rgb.b = (color      ) & 0xFF;
    return rgb;
}

void BlurImageEx(IMAGE *dst, IMAGE *src, int blur)
{
    void *pdst = dst->mData;
    void *psrc = src->mData;
    unsigned int size = src->mSize >> 2;

    // only support 32bit color
    if (bytesPerPixel != 4) return;

    // check for small source size
    if (blur <= 0 || size <= 2 * blur) return;

    // check for max blur
    if (blur > 127) blur = 127;

    _asm {
        mov    esi, psrc
        mov    edi, pdst
        mov    ecx, size
        sub    ecx, blur
        sub    ecx, blur
        push   ecx
        mov    eax, 4
        xor    ebx, ebx
    nr1bx:
        xor    ecx, ecx
        push   esi
        push   edi
        push   eax
    lp1xb:
        xor    eax, eax
        mov    al, [esi]
        push   edi
        mov    edx, blur
        add    eax, edx
        mov    edi, 1
    lpi1xb:
        mov    bl, [esi + edi * 4]
        add    eax, ebx
        cmp    edi, ecx
        ja     nb1xb
        neg    edi
        mov    bl, [esi + edi * 4]
        neg    edi
        add    eax, ebx
        inc    edx
    nb1xb:
        inc    edi
        cmp    edi, blur
        jnae   lpi1xb
        pop    edi
        mov    ebx, edx
        xor    edx, edx
        div    ebx
        mov    edx, ebx
        mov    [edi], al
        add    esi, 4
        add    edi, 4
        inc    ecx
        cmp    ecx, blur
        jne    lp1xb
        pop    eax
        pop    edi
        pop    esi
        inc    edi
        inc    esi
        dec    eax
        jnz    nr1bx
        dec    ecx
        shl    ecx, 2
        add    esi, ecx
        add    edi, ecx
        pop    ecx
        mov    edx, blur
        add    edx, edx
        inc    edx
        mov    eax, 4
    nrxb:
        push   ecx
        push   esi
        push   edi
        push   eax
    lpxb:
        xor    eax, eax
        mov    al, [esi]
        push   ecx
        mov    ecx, blur
        add    eax, ecx
    lpixb:
        mov    bl, [esi + ecx * 4]
        neg    ecx
        add    eax, ebx
        mov    bl, [esi + ecx * 4]
        neg    ecx
        add    eax, ebx
        loop   lpixb
        pop    ecx
        mov    ebx, edx
        xor    edx, edx
        div    ebx
        mov    edx, ebx
        mov    [edi], al
        add    esi, 4
        add    edi, 4
        loop   lpxb
        pop    eax
        pop    edi
        pop    esi
        pop    ecx
        inc    edi
        inc    esi
        dec    eax
        jnz    nrxb
        dec    ecx
        shl    ecx, 2
        add    esi, ecx
        add    edi, ecx
        mov    eax, 4
    nr2xb:
        mov    ecx, blur
        push   esi
        push   edi
        push   eax
    lp2xb:
        xor    eax, eax
        mov    al, [esi]
        push   edi
        mov    edx, blur
        add    eax, edx
        mov    edi, 1
    lpi2xb:
        cmp    edi, ecx
        jae    nb2xb
        mov    bl, [esi + edi * 4]
        add    eax, ebx
        inc    edx
    nb2xb:
        neg    edi
        mov    bl, [esi + edi * 4]
        neg    edi
        add    eax, ebx
        inc    edi
        cmp    edi, blur
        jnae   lpi2xb
        pop    edi
        mov    ebx, edx
        xor    edx, edx
        div    ebx
        mov    edx, ebx
        mov    [edi], al
        add    esi, 4
        add    edi, 4
        loop   lp2xb
        pop    eax
        pop    edi
        pop    esi
        inc    edi
        inc    esi
        dec    eax
        jnz    nr2xb
    }
}

void BrightnessImage(IMAGE *dst, IMAGE *src, unsigned char bright)
{
    void *psrc = src->mData;
    void *pdst = dst->mData;
    unsigned int size = src->mSize >> 2;

    // only support 32bit color
    if (bytesPerPixel != 4) return;

    // check light range
    if (bright == 0 || bright == 255) return;

    _asm {
        mov    ecx, size
        mov    edi, pdst
        mov    esi, psrc
        xor    edx, edx
        mov    dl, bright
    next:
        mov    ebx, [esi]
        mov    al, bh
        and    ebx, 00FF00FFh
        imul   ebx, edx
        shr    ebx, 8
        mul    dl
        mov    bh, ah
        mov    eax, ebx
        stosd
        add    esi, 4
        loop   next
    }
}

void BlockOutMid(void *dst, void *src, unsigned int count, unsigned int blk)
{
    _asm {
        mov    edx, count
        mov    ebx, blk
        mov    edi, dst
        mov    esi, src
        mov    ecx, ebx
        shr    ecx, 17
        lea    esi, [esi + ecx * 4 + 4]
        mov    ecx, ebx
        shr    ecx, 16
        and    ebx, 00FFFFh
    next:
        mov    eax, [esi]
        sub    edx, ecx
        cmp    edx, 0
        jg     skip
        add    ecx, edx
        rep    stosd
        jmp    quit
    skip:
        lea    esi, [esi + ecx * 4]
        rep    stosd
        mov    ecx, ebx
        cmp    edx, 0
        jg     next
    quit:
    }
}

void BrightnessAlpha(IMAGE *img, unsigned char bright)
{
    void *data = img->mData;
    unsigned int size = img->mSize >> 2;

    // only support 32bit color
    if (bytesPerPixel != 4) return;

    // check for minimum
    if (bright == 0 || bright == 255) return;

    _asm {
        mov    ecx, size
        mov    edi, data
        xor    eax, eax
        mov    bl, bright
    next:
        mov    al, [edi + 3]
        mul    bl
        mov    [edi + 3], ah
        add    edi, 4
        loop   next
    }
}

void BlockOutMidImage(IMAGE *dst, IMAGE *src, int xb, int yb)
{
    int y, mid, cx, cy;
    unsigned int *pdst = (unsigned int*)dst->mData;
    unsigned int *psrc = (unsigned int*)src->mData;

    // only support 32bit color
    if (bytesPerPixel != 4) return;

    // check minimum blocking
    if (xb == 0) xb = 1;
    if (yb == 0) yb = 1;

    // nothing to do, make source and destination are the same
    if (xb == 1 && yb == 1) memcpy(pdst, psrc, src->mSize);
    else
    {
        // calculate deltax, deltay
        cx = (((src->mWidth >> 1) % xb) << 16) | xb;
        cy = (yb - (src->mHeight >> 1) % yb);

        // process line by line
        for (y = 0; y != src->mHeight; y++)
        {
            // blocking line by line
            if ((y + cy) % yb == 0 || y == 0)
            {
                mid = y + (cy >> 1);
                if (mid >= src->mHeight) mid = (src->mHeight + y) >> 1;
                BlockOutMid(pdst, &psrc[mid * src->mWidth], src->mWidth, cx);
            }
            // already blocking, copy it
            else memcpy(pdst, pdst - dst->mWidth, dst->mRowBytes);
            pdst += dst->mWidth;
        }
    }
}

void FadeOutCircle(float pc, int size, int type, unsigned int col)
{
    int val, dsize, max, x, y;

    if (pc > 100) pc = 100;
    if (pc < 0 ) pc = 0;

    max = size * 1.4;
    dsize = size << 1;

    switch (type)
    {
    case 0:
        for (y = 0; y < yres / dsize; y++)
        for (x = 0; x < xres / dsize; x++)
        FillCircle(x * dsize + size, y * dsize + size, max * pc / 100, col);
        break;
    case 1:
        for (y = 0; y < yres / dsize; y++)
        for (x = 0; x < xres / dsize; x++)
        {
            val = (max + (yres / dsize - y) * 2) * pc / 100;
            if (val > max) val = max;
            FillCircle(x * dsize + size, y * dsize + size, val, col);
        }
        break;
    case 2:
        for (y = 0; y < yres / dsize; y++)
        for (x = 0; x < xres / dsize; x++)
        {
            val = (max + (xres / dsize - x) * 2) * pc / 100;
            if (val > max) val = max;
            FillCircle(x * dsize + size, y * dsize + size, val, col);
        }
        break;
    case 3:
        for (y = 0; y < yres / dsize; y++)
        for (x = 0; x < xres / dsize; x++)
        {
            val = (max + (xres / size - (x + y))) * pc / 100;
            if (val > max) val = max;
            FillCircle(x * dsize + size, y * dsize + size, val, col);
        }
        break;
    }
}

void ScaleUpLine(void *dst, void *src, void *tables, int count, int yval)
{
    _asm {
        mov    ecx, count
        mov    ebx, src
        add    ebx, yval
        mov    esi, tables
        mov    edi, dst
    next:
        lodsd
        mov    eax, [ebx + eax * 4]
        stosd
        loop   next
    }
}

void ScaleUpImage(IMAGE *dst, IMAGE *src, int *tables, int xfact, int yfact)
{
    int i, y;
    unsigned int *pdst = (unsigned int*)dst->mData;

    if (bytesPerPixel != 4) FatalError("ScaleUpImage: only 32 bits supported.\n");

    // init lookup table
    for (i = 0; i != src->mWidth; i++) tables[i] = RoundInt(1.0 * i / (src->mWidth - 1) * ((src->mWidth - 1) - (xfact << 1))) + xfact;

    // scaleup line by line
    for (i = 0; i != src->mHeight; i++)
    {
        y = RoundInt(1.0 * i / (src->mHeight - 1) * ((src->mHeight - 1) - (yfact << 1))) + yfact;
        ScaleUpLine(pdst, src->mData, tables, src->mWidth, y * src->mRowBytes);
        pdst += dst->mWidth;
    }
}

void BlurImage(IMAGE *img)
{
    unsigned int tmp = 0;
    unsigned int width  = img->mWidth;
    unsigned int height = img->mHeight;
    void *data = img->mData;

    if (bytesPerPixel != 4) FatalError("BlurImage: only 32 bits supported.\n");

    _asm {
        mov    edi, data
    step:
        mov    edx, width
        sub    edx, 2
        mov    tmp, edx
        mov    edx, width
        shl    edx, 2
        mov    ebx, [edi]
        mov    esi, [edi + 4]
        and    ebx, 00FF00FFh
        and    esi, 00FF00FFh
        add    ebx, ebx
        mov    ecx, [edi + edx]
        add    esi, ebx
        and    ecx, 00FF00FFh
        add    esi, ecx
        mov    al, [edi + 5]
        mov    bl, [edi + 1]
        add    ebx, ebx
        mov    cl, [edi + edx + 1]
        add    eax, ebx
        xor    ebx, ebx
        shr    esi, 2
        add    eax, ecx
        and    esi, 00FF00FFh
        shl    eax, 6
        and    eax, 0000FF00h
        or     eax, esi
        stosd
    next:
        mov    esi, [edi - 4]
        mov    ecx, [edi + 4]
        and    esi, 00FF00FFh
        and    ecx, 00FF00FFh
        mov    ebx, [edi]
        add    esi, ecx
        and    ebx, 00FF00FFh
        mov    ecx, [edi + edx]
        add    esi, ebx
        and    ecx, 00FF00FFh
        xor    eax, eax
        mov    al, [edi - 3]
        add    esi, ecx
        mov    cl, [edi + 5]
        mov    bl, [edi + 1]
        add    eax, ecx
        mov    cl, [edi + edx + 1]
        add    eax, ebx
        shr    esi, 2
        add    eax, ecx
        and    esi, 00FF00FFh
        shl    eax, 6
        and    eax, 0000FF00h
        or     eax, esi
        stosd
        dec    tmp
        jnz    next
        mov    ebx, [edi]
        mov    esi, [edi - 4]
        and    ebx, 00FF00FFh
        and    esi, 00FF00FFh
        add    ebx, ebx
        mov    ecx, [edi + edx]
        add    esi, ebx
        and    ecx, 00FF00FFh
        add    esi, ecx
        mov    al, [edi - 3]
        mov    bl, [edi - 4]
        add    ebx, ebx
        mov    cl, [edi + edx + 1]
        add    eax, ebx
        xor    ebx, ebx
        shr    esi, 2
        add    eax, ecx
        and    esi, 00FF00FFh
        shl    eax, 6
        and    eax, 0000FF00h
        or     eax, esi
        stosd
        dec    height
        jnz    step
    }
}

void BlendImage(IMAGE *dst, IMAGE *src1, IMAGE *src2, int cover)
{
    void *psrc1 = src1->mData;
    void *psrc2 = src2->mData;
    void *pdst  = dst->mData;
    unsigned int count = src1->mSize >> 2;

    if (bytesPerPixel != 4) FatalError("BlendImage: only 32 bits supported.\n");

    _asm {
        mov    edi, pdst
        mov    esi, psrc2
        mov    ecx, cover
        mov    edx, psrc1
        neg    cl
    next:
        push   edx
        mov    ebx, [esi]
        mov    edx, [edx]
        mov    al, dh
        and    edx, 00FF00FFh
        mov    ah, bh
        and    ebx, 00FF00FFh
        sub    edx, ebx
        imul   edx, ecx
        shr    edx, 8
        add    ebx, edx
        xor    edx, edx
        mov    dl, ah
        xor    ah, ah
        mov    bh, dl
        sub    ax, dx
        mul    cx
        add    bh, ah
        pop    edx
        mov    [edi], ebx
        add    esi, 4
        add    edx, 4
        add    edi, 4
        dec    count
        jnz    next
    }
}

void RotateLine(uint8_t *dst, uint8_t *src, uint8_t *tables, int width, int siny, int cosy)
{
    int pos = (width + 1) << 3;

    _asm {
        mov    ecx, width
        dec    ecx
        mov    esi, src
        mov    edi, dst
        mov    ebx, tables
    next:
        mov    eax, ecx
        shl    eax, 3
        mov    edx, eax
        add    eax, 8
        add    edx, 12
        mov    eax, [ebx + eax]
        mov    edx, [ebx + edx]
        add    eax, cosy
        sub    edx, siny
        sar    eax, 1
        js     skip
        sar    edx, 1
        js     skip
        cmp    eax, [ebx + 4]
        jnl    skip
        cmp    edx, [ebx]
        jnl    skip
        shl    eax, 2
        add    eax, pos
        mov    eax, [ebx + eax]
        shl    edx, 2
        add    edx, eax
        mov    eax, [esi + edx]
        mov    [edi], eax
    skip:
        add    edi, 4
        dec    ecx
        jns    next
    }
}

void RotateImage(IMAGE *dst, IMAGE *src, int *tables, int axisx, int axisy, float angle, float scale)
{
    float th, sint, cost;
    float sinx, cosx, siny, cosy;

    int x, y, primex, primey, lineWidth = 0;
    unsigned int *psrc = (unsigned int*)src->mData;
    unsigned int *pdst = (unsigned int*)dst->mData;

    if (bytesPerPixel != 4) FatalError("RotateImage: only 32 bits supported.\n");

    // recalculate axisx, axisy
    axisx = dst->mWidth - axisx;
    axisy = dst->mHeight - axisy;

    // store source image width, height
    tables[0] = src->mWidth;
    tables[1] = src->mHeight;

    // calculate rotation data
    th   = (180 - angle) * M_PI / 180;
    sint = sin(th) / scale;
    cost = cos(th) / scale;

    primex = (-axisx << 1) + 1;
    sinx   = primex * sint - 1;
    cosx   = primex * cost - 1 + src->mWidth;
    sint   *= 2;
    cost   *= 2;

    // init lookup tables
    for (x = 0; x != dst->mWidth; x++)
    {
        tables[(x << 1) + 2] = sinx;
        tables[(x << 1) + 3] = cosx;
        sinx += sint;
        cosx += cost;
    }

    sint /= 2;
    cost /= 2;

    for (y = 0; y != src->mHeight; y++)
    {
        tables[y + ((src->mWidth + 1) << 1)] = lineWidth;
        lineWidth += src->mRowBytes;
    }

    primey = ((dst->mHeight - 1 - axisy) << 1) + 1;
    siny   = primey * sint;
    cosy   = primey * cost + src->mHeight;
    sint   *= 2;
    cost   *= 2;

    // process rotate line by line
    for (y = 0; y != dst->mHeight; y++)
    {
        RotateLine((uint8_t*)pdst, (uint8_t*)psrc, (uint8_t*)tables, dst->mWidth, siny, cosy);
        pdst += dst->mWidth;
        siny -= sint;
        cosy -= cost;
    }
}

void BumpImage(IMAGE *dst, IMAGE *src1, IMAGE *src2, int lx, int ly)
{
    void *src1data = src1->mData;
    void *src2data = src2->mData;
    void *dstdata  = dst->mData;

    int src1width = src1->mWidth;
    int src2width = src2->mWidth;
    int dstwidth  = dst->mWidth;
    int src1len   = src1->mRowBytes;
    
    int x = 0, y = 0, osrc2 = 0, osrc1 = 0, odst = 0;
    int nx = 0, ny = 0, vlx = 0, vly = 0, bmax = 260;
    
    _asm {
        mov    y, 120
    starty:
        mov    ebx, y
        mov    eax, src1width
        mul    ebx
        add    eax, 99
        shl    eax, 2
        mov    osrc1, eax
        mov    eax, src2width
        mul    ebx
        add    eax, 99
        shl    eax, 2
        mov    osrc2, eax
        mov    eax, dstwidth
        mul    ebx
        add    eax, 99
        shl    eax, 2
        mov    odst, eax
        mov    x, 100
    startx:
        mov    eax, x
        sub    eax, lx
        mov    vlx, eax
        mov    eax, y
        sub    eax, ly
        mov    vly, eax
        add    osrc2, 4
        add    osrc1, 4
        add    odst, 4
        mov    ecx, vlx
        mov    ebx, bmax
        mov    eax, vly
        neg    ebx
        cmp    ecx, bmax
        jnl    stop
        cmp    eax, bmax
        jnl    stop
        cmp    ecx, ebx
        jng    stop
        cmp    eax, ebx
        jng    stop
        xor    eax, eax
        xor    ebx, ebx
        mov    edi, src1data
        add    edi, osrc1
        mov    al, [edi + 1]
        or     al, al
        jz     stop
        mov    bl, [edi - 1]
        sub    eax, ebx
        mov    nx, eax
        mov    ecx, src1len
        mov    al, [edi + ecx]
        sub    edi, ecx
        mov    bl, [edi]
        sub    eax, ebx
        mov    ny, eax
        mov    eax, vlx
        sub    eax, nx
        jns    nsx
        neg    eax
    nsx:
        shr    eax, 1
        cmp    eax, 127
        jna    nax
        mov    eax, 127
    nax:
        mov    ebx, 127
        sub    ebx, eax
        jns    nsx2
        mov    ebx, 1
    nsx2:
        mov    eax, vly
        sub    eax, ny
        jns    nsy
        neg    eax
    nsy:
        shr    eax, 1
        cmp    eax, 127
        jna    nay
        mov    eax, 127
    nay:
        mov    ecx, 127
        sub    ecx, eax
        jns    nsy2
        mov    ecx, 1
    nsy2:
        add    ebx, ecx
        cmp    ebx, 128
        jna    stop
        sub    ebx, 128
        mov    edi, src2data
        add    edi, osrc2
        mov    ecx, [edi]
        mov    edi, dstdata
        add    edi, odst
        xor    eax, eax
        mov    al, cl
        mul    ebx
        shr    eax, 5
        cmp    eax, 255
        jna    nextb
        mov    eax, 255
    nextb:
        mov    [edi], al
        mov    al, ch
        mul    ebx
        shr    eax, 5
        cmp    eax, 255
        jna    nextg
        mov    eax, 255
    nextg:
        mov    [edi + 1], al
        shr    ecx, 16
        mov    al, cl
        mul    ebx
        shr    eax, 5
        cmp    eax, 255
        jna    nextr
        mov    eax, 255
    nextr:
        mov    [edi + 2], al
    stop:
        inc    x
        mov    eax, x
        cmp    eax, 510
        jna    startx
        inc    y
        mov    eax, y
        cmp    eax, 350
        jna    starty
    }
}

// Init projection params
void InitProjection()
{
    float th, ph;

    th = M_PI * theta / 180;
    ph = M_PI * phi / 180;

    aux1 = sin(th);
    aux2 = sin(ph);
    aux3 = cos(th);
    aux4 = cos(ph);

    aux5 = aux3 * aux2;
    aux6 = aux1 * aux2;
    aux7 = aux3 * aux4;
    aux8 = aux1 * aux4;
}

// projection point
void Projette(float x, float y, float z)
{
    xobs = -x * aux1 + y * aux3;
    yobs = -x * aux5 - y * aux6 + z * aux4;

    if (projection == PERSPECTIVE)
    {
        zobs = -x * aux7 - y * aux8 - z * aux2 + rho;
        xproj = de * xobs / zobs;
        yproj = de * yobs / zobs;
    }
    else if (projection == PARALLELE)
    {
        xproj = de * xobs;
        yproj = de * yobs;
    }
    else FatalError("Projette: Unknown projection type [PERSPECTIVE, PARALLELE].\n");
}

// Move current cursor in 3D mode
void DeplaceEn(float x, float y, float z)
{
    Projette(x, y, z);
    xecran = centerx + xproj * ECHE;
    yecran = centery - yproj;
    MoveTo(xecran, yecran);
}

// Draw line from current cursor in 3D mode
void TraceVers(float x, float y, float z, unsigned int col)
{
    Projette(x, y, z);
    xecran = centerx + xproj * ECHE;
    yecran = centery - yproj;
    LineTo(xecran, yecran, col);
}

void TraceVersAdd(float x, float y, float z, unsigned int col)
{
    Projette(x, y, z);
    xecran = centerx + xproj * ECHE;
    yecran = centery - yproj;
    LineToAdd(xecran, yecran, col);
}

void TraceVersSub(float x, float y, float z, unsigned int col)
{
    Projette(x, y, z);
    xecran = centerx + xproj * ECHE;
    yecran = centery - yproj;
    LineToSub(xecran, yecran, col);
}

int StrPos(char *str, const char *sub)
{
    char *ptr = strstr(str, sub);
    return ptr ? (ptr - str) : -1;
}

void InsertChar(char *str, char chr, int pos)
{
    if (pos < 0 || pos >= strlen(str)) return;
    str[pos] = chr;
}

void StrDelete(char *str, int i, int numChar)
{
    if (i < 0 || i >= strlen(str)) return;
    memmove(&str[i + 1], &str[i + numChar], strlen(str) - i - 1);
}

void SchRepl(char *str, const char *schr, char repl)
{
    int pos = StrPos(str, schr);
    while (pos >= 0)
    {
        StrDelete(str, pos, strlen(schr));
        InsertChar(str, repl, pos);
        pos = StrPos(str, schr);
    }
}

void Chr2Str(char chr, char num, char *str)
{
    str[0] = chr;
    str[1] = num;
    str[2] = 0;
}

// encode string to VNI string (format type VNI)
void MakeFont(char *str)
{
    char buff[4] = {0};
    SchRepl(str, "a8", 128);
    Chr2Str(128, '1', buff);
    SchRepl(str, buff, 129);
    Chr2Str(128, '2', buff);
    SchRepl(str, buff, 130);
    Chr2Str(128, '3', buff);
    SchRepl(str, buff, 131);
    Chr2Str(128, '4', buff);
    SchRepl(str, buff, 132);
    Chr2Str(128, '5', buff);
    SchRepl(str, buff, 133);
    SchRepl(str, "a6", 134);
    Chr2Str(134, '1', buff);
    SchRepl(str, buff, 135);
    Chr2Str(134, '2', buff);
    SchRepl(str, buff, 136);
    Chr2Str(134, '3', buff);
    SchRepl(str, buff, 137);
    Chr2Str(134, '4', buff);
    SchRepl(str, buff, 138);
    Chr2Str(134, '5', buff);
    SchRepl(str, buff, 139);
    SchRepl(str, "e6", 140);
    Chr2Str(140, '1', buff);
    SchRepl(str, buff, 141);
    Chr2Str(140, '2', buff);
    SchRepl(str, buff, 142);
    Chr2Str(140, '3', buff);
    SchRepl(str, buff, 143);
    Chr2Str(140, '4', buff);
    SchRepl(str, buff, 144);
    Chr2Str(140, '5', buff);
    SchRepl(str, buff, 145);
    SchRepl(str, "o7", 146);
    Chr2Str(146, '1', buff);
    SchRepl(str, buff, 147);
    Chr2Str(146, '2', buff);
    SchRepl(str, buff, 148);
    Chr2Str(146, '3', buff);
    SchRepl(str, buff, 149);
    Chr2Str(146, '4', buff);
    SchRepl(str, buff, 150);
    Chr2Str(146, '5', buff);
    SchRepl(str, buff, 151);
    SchRepl(str, "o6", 152);
    Chr2Str(152, '1', buff);
    SchRepl(str, buff, 153);
    Chr2Str(152, '2', buff);
    SchRepl(str, buff, 154);
    Chr2Str(152, '3', buff);
    SchRepl(str, buff, 155);
    Chr2Str(152, '4', buff);
    SchRepl(str, buff, 156);
    Chr2Str(152, '5', buff);
    SchRepl(str, buff, 157);
    SchRepl(str, "u7", 158);
    Chr2Str(158, '1', buff);
    SchRepl(str, buff, 159);
    Chr2Str(158, '2', buff);
    SchRepl(str, buff, 160);
    Chr2Str(158, '3', buff);
    SchRepl(str, buff, 161);
    Chr2Str(158, '4', buff);
    SchRepl(str, buff, 162);
    Chr2Str(158, '5', buff);
    SchRepl(str, buff, 163);
    SchRepl(str, "a1", 164);
    SchRepl(str, "a2", 165);
    SchRepl(str, "a3", 166);
    SchRepl(str, "a4", 167);
    SchRepl(str, "a5", 168);
    SchRepl(str, "e1", 169);
    SchRepl(str, "e2", 170);
    SchRepl(str, "e3", 171);
    SchRepl(str, "e4", 172);
    SchRepl(str, "e5", 173);
    SchRepl(str, "i1", 174);
    SchRepl(str, "i2", 175);
    SchRepl(str, "i3", 181);
    SchRepl(str, "i4", 182);
    SchRepl(str, "i5", 183);
    SchRepl(str, "o1", 184);
    SchRepl(str, "o2", 190);
    SchRepl(str, "o3", 198);
    SchRepl(str, "o4", 199);
    SchRepl(str, "o5", 208);
    SchRepl(str, "u1", 210);
    SchRepl(str, "u2", 211);
    SchRepl(str, "u3", 212);
    SchRepl(str, "u4", 213);
    SchRepl(str, "u5", 214);
    SchRepl(str, "y1", 215);
    SchRepl(str, "y2", 216);
    SchRepl(str, "y3", 221);
    SchRepl(str, "y4", 222);
    SchRepl(str, "y5", 248);
    SchRepl(str, "d9", 249);
    SchRepl(str, "D9", 250);
}

// Calculate file size in bytes
unsigned int GetFileSize(FILE *fpFile)
{
    unsigned int pos, len;
    pos = ftell(fpFile);
    fseek(fpFile, 0UL, SEEK_END);
    len = ftell(fpFile);
    fseek(fpFile, pos, SEEK_SET);
    return len;
}

// Generate random word array
void RandomBuffer(void *buff, int count, int range)
{
    // Check range
    if (!count || !randSeed || !range) return;

    _asm {
        mov    edi, buff
        mov    ecx, count
        mov    ebx, randSeed
    next:
        mov    eax, ebx
        mul    factor
        inc    eax
        mov    ebx, eax
        shr    eax, 16
        mul    range
        shr    eax, 16
        stosw
        loop   next
        mov    randSeed, ebx
    }
}

// Set current selected font
void SetFontType(unsigned int type)
{
    // Check current range
    if (type >= GFX_MAX_FONT) type = GFX_MAX_FONT - 1;
    fontType = type;
}

// Set current font size
void SetFontSize(unsigned int size)
{
    // Have sub-fonts
    if (font[fontType].subFonts > 0)
    {
        // Correct sub-fonts number
        if (size > font[fontType].subFonts) size = font[fontType].subFonts;
        // Copy sub-fonts header
        memcpy(&font[fontType].info, &font[fontType].dataPtr[(font[fontType].info.endChar - font[fontType].info.startChar + 1) * 4 * (font[fontType].subFonts + 1) + size * sizeof(XFN_FONT_INFO)], sizeof(XFN_FONT_INFO));
    }
    subFont = size;
}

// Get height of string with current font in pixel
int GetFontHeight(char *str)
{
    unsigned int i = 0;
    unsigned int height = 0;
    unsigned int mempos = 0;
    unsigned int size = 0;
    unsigned int len = strlen(str);

    // Check for font is loaded
    if (!font[fontType].dataPtr) return 0;
    if (!str || !len) return 0;

    // fixed font, all characters have a same height
    if (font[fontType].flags & XFN_FIXED) height = font[fontType].info.height;
    else
    {
        // vector font
        if (font[fontType].flags & XFN_VECTOR)
        {
            for (i = 0; i != len; i++)
            {
                // skip invalid character
                if (str[i] < font[fontType].info.startChar || str[i] > font[fontType].info.endChar) continue;

                // position of raw data of current character
                mempos = *(unsigned int*)&font[fontType].dataPtr[font[fontType].info.startOffset + ((str[i] - font[fontType].info.startChar) << 2)];
                size = font[fontType].dataPtr[mempos + 1];
                if (size > height) height = size;
            }
            height *= subFont;
        }
        else
        {
            // BMP1 font
            if (font[fontType].info.bitsPerPixel == 1)
            {
                for (i = 0; i != len; i++)
                {
                    // skip invalid character
                    if (str[i] < font[fontType].info.startChar || str[i] > font[fontType].info.endChar) continue;

                    // position of raw data of current character
                    mempos = *(unsigned int*)&font[fontType].dataPtr[font[fontType].info.startOffset + ((str[i] - font[fontType].info.startChar) << 2)];
                    size = font[fontType].dataPtr[mempos + 1];
                    if (size > height) height = size;
                }
            }
            else if (font[fontType].info.bitsPerPixel >= 2 && font[fontType].info.bitsPerPixel <= 32)
            {
                // BMP8 and RGB font
                for (i = 0; i != len; i++)
                {
                    // skip invalid character
                    if (str[i] < font[fontType].info.startChar || str[i] > font[fontType].info.endChar) continue;

                    // position of raw data of current character
                    mempos = *(unsigned int*)&font[fontType].dataPtr[font[fontType].info.startOffset + ((str[i] - font[fontType].info.startChar) << 2)];
                    size = *(unsigned int*)&font[fontType].dataPtr[mempos + 4] + *(unsigned int*)&font[fontType].dataPtr[mempos + 12];
                    if (size > height) height = size;
                }
            }
        }
    }

    // animation font
    if (font[fontType].flags & XFN_ANIPOS) height += font[fontType].info.randomY;
    return height;
}

// Get width of string with current font in pixel
int GetFontWidth(char *str)
{
    XFN_STROKE_DATA *data;
    unsigned int i = 0;
    unsigned int mempos = 0;
    unsigned int width = 0;
    unsigned int size = 0;
    unsigned int len = strlen(str);

    // Check for font is loaded
    if (!font[fontType].dataPtr) return 0;
    if (!str || !len) return 0;

    // fixed font, all characters have a same width
    if (font[fontType].flags & XFN_FIXED) width = (font[fontType].info.width + font[fontType].info.distance) * len;
    else
    {
        // vector font
        if (font[fontType].flags & XFN_VECTOR)
        {
            for (i = 0; i != len; i++)
            {
                // skip invalid character
                if (str[i] < font[fontType].info.startChar || str[i] > font[fontType].info.endChar) size = font[fontType].info.spacer;
                else
                {
                    // position of raw data of current character
                    mempos = *(unsigned int*)&font[fontType].dataPtr[font[fontType].info.startOffset + ((str[i] - font[fontType].info.startChar) << 2)];
                    data = (XFN_STROKE_DATA*)&font[fontType].dataPtr[mempos];
                    size = data->width * subFont;
                }
                width += size + font[fontType].info.distance;
            }
        }
        else
        {
            // BMP1 font
            if (font[fontType].info.bitsPerPixel == 1)
            {
                for (i = 0; i != len; i++)
                {
                    // skip invalid character
                    if (str[i] < font[fontType].info.startChar || str[i] > font[fontType].info.endChar)
                    {
                        width += font[fontType].info.spacer + font[fontType].info.distance;
                        continue;
                    }

                    // position of raw data of current character
                    mempos = *(unsigned int*)&font[fontType].dataPtr[font[fontType].info.startOffset + ((str[i] - font[fontType].info.startChar) << 2)];
                    width += *(unsigned char*)&font[fontType].dataPtr[mempos] + font[fontType].info.distance;
                }
            }
            else if (font[fontType].info.bitsPerPixel >= 2 && font[fontType].info.bitsPerPixel <= 32)
            {
                // BMP8 and RGB font
                for (i = 0; i != len; i++)
                {
                    // skip invalid character
                    if (str[i] < font[fontType].info.startChar || str[i] > font[fontType].info.endChar)
                    {
                        width += font[fontType].info.spacer + font[fontType].info.distance;
                        continue;
                    }

                    // position of raw data of current character
                    mempos = *(unsigned int*)&font[fontType].dataPtr[font[fontType].info.startOffset + ((str[i] - font[fontType].info.startChar) << 2)];
                    width += *(unsigned int*)&font[fontType].dataPtr[mempos] + *(unsigned int*)&font[fontType].dataPtr[mempos + 8] + font[fontType].info.distance;
                }
            }
        }
    }

    // animation font
    if (font[fontType].flags & XFN_ANIPOS) width += font[fontType].info.randomX;
    return width - font[fontType].info.distance;
}

// Load font name to memory
int LoadFont(char *fname, unsigned int type)
{
    FILE *fp;

    // Check for type range
    if (type >= GFX_MAX_FONT) return 0;

    // Read font header
     fp = fopen(fname, "rb");
    if (!fp) return 0;
    fread(&font[type], 1, sizeof(XFN_FONT_HEADER), fp);

    // Check font signature, version number and memory size
    if (memcmp(font[type].sign, "Fnt2", 4) || font[type].version != 0x0101 || !font[type].memSize)
    {
        fclose(fp);
        return 0;
    }

    // Allocate raw data buffer
    font[type].dataPtr = (unsigned char*)malloc(font[type].memSize);
    if (!font[type].dataPtr)
    {
        fclose(fp);
        return 0;
    }

    // Read raw font data
    fread(font[type].dataPtr, 1, font[type].memSize, fp);
    fclose(fp);

    // Reset font header for old font
    if (font[type].flags & XFN_MULTI) SetFontSize(0);

    // Default sub-fonts
    if (font[type].flags & XFN_VECTOR) subFont = 1;
    else subFont = 0;

    // BMP8 font palette
    if (font[type].info.usedColors > 1)
    {
        // BMP8 use up to 128 colors (128 * 4)
        fontPalette[type] = (unsigned char*)malloc(512);
        if (!fontPalette[type])
        {
            free(font[type].dataPtr);
            return 0;
        }
        memset(fontPalette[type], 0, 512);
    }
    return 1;
}

// Release current loaded font
void CloseFont(unsigned int type)
{
    // Check for type range
    if (type >= GFX_MAX_FONT) return;

    // Free font raw data buffer
    if (font[type].dataPtr)
    {
        free(font[type].dataPtr);
        font[type].dataPtr = NULL;
    }

    // Free font palette
    if (fontPalette[type])
    {
        free(fontPalette[type]);
        fontPalette[type] = NULL;
    }

    // Reset header
    memset(&font[type], 0, sizeof(XFN_FONT_HEADER));
}

// Draw a stroke of BGI font (YES we also support BGI font)
int OutStroke(int x, int y, char chr, unsigned int col, unsigned int mode)
{
    XFN_STROKE_DATA *data;
    XFN_STROKE_INFO *stroke;

    unsigned int mx, my;
    unsigned int i, mempos;

    // check for font is loaded
    if (!font[fontType].dataPtr) return 0;

    // check for non-drawable character
    if (font[fontType].info.startChar > chr || font[fontType].info.endChar < chr) return font[fontType].info.spacer;

    // memory position of character
    mempos = *(unsigned int*)&font[fontType].dataPtr[font[fontType].info.startOffset + ((chr - font[fontType].info.startChar) << 2)];
    data = (XFN_STROKE_DATA*)&font[fontType].dataPtr[mempos];
    stroke = (XFN_STROKE_INFO*)&font[fontType].dataPtr[mempos + sizeof(XFN_STROKE_DATA)];
    
    // scan for all lines
    for (i = 0; i != data->numOfLines; i++)
    {
        mx = x + stroke->x * subFont;
        my = y + stroke->y * subFont;
        
        // check for drawable
        if (stroke->code == 1) MoveTo(mx, my);
        else
        {
            // automatic antialias when in 32-bits mode
            if (bytesPerPixel == 4) LineToAlpha(mx, my, col);
            else if (mode == 2) LineToAdd(mx, my, col);
            else if (mode == 3) LineToSub(mx, my, col);
            else LineTo(mx, my, col);
        }
        stroke++;
    }

    return data->width * subFont;
}

// Draw string with current loaded font
void WriteString(int x, int y, char *str, unsigned int col, unsigned int mode)
{
    unsigned char r, g, b;
    unsigned int cx, cy;
    unsigned int i, len;
    unsigned int width, height, addx, addy;
    unsigned int data, datapos, mempos;
    
    // Check for font is loaded
    if (!font[fontType].dataPtr) return;

    len = strlen(str);

    // Check for vector font
    if (font[fontType].flags & XFN_VECTOR)
    {
        for (i = 0; i != len; i++)
        {
            x += OutStroke(x, y, str[i], col, mode) + font[fontType].info.distance;
            if (mode == 1) col++;
        }
        return;
    }

    // BMP1 font format
    if (font[fontType].info.bitsPerPixel == 1)
    {
        for (i = 0; i != len; i++)
        {
            // Invalid character, update position
            if (str[i] < font[fontType].info.startChar || str[i] > font[fontType].info.endChar)
            {
                if (!(font[fontType].flags & XFN_FIXED)) x += font[fontType].info.spacer + font[fontType].info.distance;
                else x += font[fontType].info.width + font[fontType].info.distance;
                continue;
            }

            // Memory position for each character
            mempos = *(unsigned int*)&font[fontType].dataPtr[font[fontType].info.startOffset + ((str[i] - font[fontType].info.startChar) << 2)];
            width = font[fontType].dataPtr[mempos];
            height = font[fontType].dataPtr[mempos + 1];
            mempos += 2;

            // Scans for font width and height
            for (cy = 0; cy != height; cy++)
            {
                datapos = 0;
                data = *(unsigned int*)&font[fontType].dataPtr[mempos];
                for (cx = 0; cx != width; cx++)
                {
                    if ((cx > 31) && !(cx & 31))
                    {
                        datapos += 4;
                        data = *(unsigned int*)&font[fontType].dataPtr[mempos + datapos];
                    }
                    if (data & (1 << (cx & 31)))
                    {
                        if (mode == 2) PutPixelAdd(x + cx, y + cy, col);
                        else if (mode == 3) PutPixelSub(x + cx, y + cy, col);
                        else PutPixel(x + cx, y + cy, col);
                    }
                }
                mempos += font[fontType].info.bytesPerLine;
            }
            x += width + font[fontType].info.distance;
            if (mode == 1) col++;
        }
    }
    // BMP8 font format
    else if (font[fontType].info.bitsPerPixel >= 2 && font[fontType].info.bitsPerPixel <= 7)
    {
        // Calculate font palette, use for hi-color and true-color
        if (bitsPerPixel != 8)
        {
            ToRGB(col, &r, &g, &b);
            for (i = 1; i <= font[fontType].info.usedColors; i++) *(unsigned int*)&fontPalette[fontType][i << 2] = FromRGB(r * i / (font[fontType].info.usedColors - 1), g * i / (font[fontType].info.usedColors - 1), b * i / (font[fontType].info.usedColors - 1));
        }

        // Genertate random position for animation font
        if (font[fontType].flags & XFN_ANIPOS)
        {
            RandomBuffer(gfxBuff, len + 1, font[fontType].info.randomX);
            RandomBuffer(&gfxBuff[512], len + 1, font[fontType].info.randomY);
        }

        for (i = 0; i != len; i++)
        {
            // Invalid character, update character position
            if (str[i] < font[fontType].info.startChar || str[i] > font[fontType].info.endChar)
            {
                if (!(font[fontType].flags & XFN_FIXED)) x += font[fontType].info.spacer + font[fontType].info.distance;
                else x += font[fontType].info.width + font[fontType].info.distance;
                continue;
            }

            // Lookup character position
            mempos = *(unsigned int*)&font[fontType].dataPtr[font[fontType].info.startOffset + ((str[i] - font[fontType].info.startChar) << 2)];
            addx = *(unsigned int*)&font[fontType].dataPtr[mempos];
            addy = *(unsigned int*)&font[fontType].dataPtr[mempos + 4];

            // Update position for animation font
            if (font[fontType].flags & XFN_ANIPOS)
            {
                addx += gfxBuff[i << 1];
                addy += gfxBuff[512 + (i << 1)];
            }

            // Get font width and height
            width = *(unsigned int*)&font[fontType].dataPtr[mempos + 8];
            height = *(unsigned int*)&font[fontType].dataPtr[mempos + 12];
            mempos += 16;
            x += addx;

            // Scans font raw data
            for (cy = 0; cy != height; cy++)
            {
                for (cx = 0; cx != width; cx++)
                {
                    data = font[fontType].dataPtr[mempos++];
                    if (!(data & 0x80))
                    {
                        if (bitsPerPixel == 8) PutPixel(x + cx, y + cy + addy, data);
                        else if (mode == 2) PutPixelAdd(x + cx, y + cy + addy, *(unsigned int*)&fontPalette[fontType][data << 2]);
                        else if (mode == 3) PutPixelSub(x + cx, y + cy + addy, *(unsigned int*)&fontPalette[fontType][data << 2]);
                        else PutPixel(x + cx, y + cy + addy, *(unsigned int*)&fontPalette[fontType][data << 2]);
                    }
                }
            }

            // Update next position
            if (font[fontType].flags & XFN_ANIPOS) x -= gfxBuff[i << 1];
            if (font[fontType].flags & XFN_FIXED) x += font[fontType].info.width + font[fontType].info.distance;
            else x += width + font[fontType].info.distance;
        }
    }
    // Alpha channel font
    else if (font[fontType].info.bitsPerPixel == 32)
    {
        // Genertate random position for animation font
        if (font[fontType].flags & XFN_ANIPOS)
        {
            RandomBuffer(gfxBuff, len + 1, font[fontType].info.randomX);
            RandomBuffer(&gfxBuff[512], len + 1, font[fontType].info.randomY);
        }

        for (i = 0; i != len; i++)
        {
            // Invalid character, update character position
            if (str[i] < font[fontType].info.startChar || str[i] > font[fontType].info.endChar)
            {
                if (!(font[fontType].flags & XFN_FIXED)) x += font[fontType].info.spacer + font[fontType].info.distance;
                else x += font[fontType].info.width + font[fontType].info.distance;
                continue;
            }

            // Lookup character position
            mempos = *(unsigned int*)&font[fontType].dataPtr[font[fontType].info.startOffset + ((str[i] - font[fontType].info.startChar) << 2)];
            addx = *(unsigned int*)&font[fontType].dataPtr[mempos];
            addy = *(unsigned int*)&font[fontType].dataPtr[mempos + 4];

            // Update position for animation font
            if (font[fontType].flags & XFN_ANIPOS)
            {
                addx += gfxBuff[i << 1];
                addy += gfxBuff[(i << 1) + 512];
            }

            // Get font width and height
            width = *(unsigned int*)&font[fontType].dataPtr[mempos + 8];
            height = *(unsigned int*)&font[fontType].dataPtr[mempos + 12];
            mempos += 16;
            x += addx;

            // Scans raw font data
            for (cy = 0; cy != height; cy++)
            {
                for (cx = 0; cx != width; cx++)
                {
                    data = *(unsigned int*)&font[fontType].dataPtr[mempos];
                    PutPixelAlpha(x + cx, y + addy + cy, data, 255);
                    mempos += 4;
                }
            }

            // Update next position
            if (font[fontType].flags & XFN_ANIPOS) x -= gfxBuff[i << 1];
            if (font[fontType].flags & XFN_FIXED) x += font[fontType].info.width + font[fontType].info.distance;
            else x += width + font[fontType].info.distance;
        }
    }
}

// draw muti-line string font
int DrawText(int ypos, char **str, int size)
{
    int i;

    // Check for font loaded
    if (!font[fontType].dataPtr) return 0;

    for (i = 0; i != size; i++)
    {
        if (ypos > -30) WriteString(centerx - (GetFontWidth(str[i]) >> 1), ypos, str[i], 62, 0);
        ypos += GetFontHeight(str[i]);
        if (ypos > cmaxy) break;
    }

    return ypos;
}

// draw text into image buffer
void DrawTextImage(int x, int y, char *str, unsigned int col, IMAGE *img)
{
    // save current screen address
    unsigned char *oldPtr = lfbPtr;

    // check for font loaded
    if (!font[fontType].dataPtr) return;

    // change screen address to image buffer
    lfbPtr = img->mData;
    WriteString(x, y, str, col, 2);

    // must be restored after draw
    lfbPtr = oldPtr;
}

////////////////////////////////
//      BEGIN BMP SECTION     //
////////////////////////////////
int LoadBitmap(char *fname, BITMAP *bmp)
{
    FILE *fp;
    BMP_HEADER bf;
    BMP_INFO bi;
    RGBA bmPal[256] = {0};

    unsigned int i, bmOfs, bmScanlineBytes;
    unsigned int bmSize, bmRowBytes, bmPaddingBytes;

    // Open file and check
    fp = fopen(fname, "rb");
    if (!fp) return 0;

    // Read header and check magic 'BM' signature
    fread(&bf, sizeof(BMP_HEADER), 1, fp);
    if (bf.bfType != 0x4d42)
    {
        fclose(fp);
        return 0;
    }

    // Read BMP info and check correct bits
    fread(&bi, sizeof(BMP_INFO), 1, fp);
    if (bi.biBitCount != 8 && bi.biBitCount != 16 && bi.biBitCount != 24 && bi.biBitCount != 32)
    {
        fclose(fp);
        return 0;
    }

    // Save bitmap width, height, bytes per pixels
    bmp->bmWidth = bi.biWidth;
    bmp->bmHeight = bi.biHeight;
    bmp->bmPixels = (bi.biBitCount + 7) >> 3;

    // Calculate bitmap data size
    bmSize = bf.bfSize - bf.bfOffBits;
    bmp->bmData = (unsigned char*)malloc(bmSize);
    if (!bmp->bmData)
    {
        fclose(fp);
        return 0;
    }

    // For 8 bits, need palette table
    if (bi.biBitCount == 8)
    {
        // Read and convert palette
        fread(bmPal, sizeof(bmPal), 1, fp);
        for (i = 0; i != 256; i++)
        {
            bmp->bmExtra[3 * i]     = bmPal[i].r;
            bmp->bmExtra[3 * i + 1] = bmPal[i].g;
            bmp->bmExtra[3 * i + 2] = bmPal[i].b;
        }
    }

    // For 16 bits, need RGB mask table
    else if (bi.biBitCount == 16) fread(bmp->bmExtra, 3 * sizeof(int), 1, fp);

    // Calculate bytes per scaline
    bmRowBytes = bmp->bmWidth * bmp->bmPixels;

    // BMP requires each row to have a multiple of 4 bytes
    // so sometimes padding bytes are added between rows
    bmPaddingBytes = bmRowBytes % 4;
    bmScanlineBytes = bmPaddingBytes ? bmRowBytes + (4 - bmPaddingBytes) : bmRowBytes;

    // save bytes per scan line (include padding bytes)
    bmp->bmRowBytes = bmScanlineBytes;

    // Seek to data offset, read and revert data
    fseek(fp, bf.bfOffBits, SEEK_SET);
    bmOfs = (bmp->bmHeight - 1) * bmScanlineBytes;
    for (i = 0; i != bmp->bmHeight; i++)
    {
        fread(&bmp->bmData[bmOfs], bmScanlineBytes, 1, fp);
        bmOfs -= bmScanlineBytes;
    }

    fclose(fp);
    return 1;
}

// Save screen buffer to BMP file
int SaveBitmap(char *fname, BITMAP *bmp)
{
    BMP_HEADER  bf;             // BMP file header
    BMP_INFO    bi;             // BMP file info
    RGB         aux[256] = {0}; // Current Palette entries
    RGBA        pal[256] = {0}; // BMP Palette entries

    // Open new file
    FILE  *fp = fopen(fname, "wb");

    // Calculate actual data size
    unsigned int dataSize = bmp->bmWidth * bmp->bmHeight * bmp->bmPixels;

    // Calculate line size (in bytes)
    unsigned int lineWidth = bmp->bmWidth * bmp->bmPixels;

    // Calculate padding size (round up to dword per line)
    unsigned int addPadding = (4 - lineWidth % 4) % 4;

    // addPadding data and offsets
    unsigned int hdrPadding = 0, offset = 0, i = 0;

    if (!fp) return 0;

    memset(&bf, 0, sizeof(bf));
    memset(&bi, 0, sizeof(bi));

    // Check for 8 bits color
    if (bmp->bmPixels == 1)
    {
        bi.biClrUsed = 256;         // Turn on color mode used
        hdrPadding = sizeof(pal);   // Calculate padding header size
        GetPalette(aux);            // Add palette entries

        // Calculate bitmap palette
        for (i = 0; i != 256; i++)
        {
            pal[i].r = aux[i].r;
            pal[i].g = aux[i].g;
            pal[i].b = aux[i].b;
        }
    }
    // for 15/16 bit colors add RGB mask entries (12 bytes)
    else if (bmp->bmPixels == 2)
    {
        bi.biCompression = 3;               // Turn on bit field compression for 16 bits BI_FIELDS (mode 565), for 555 use BI_RGB, bi.biCompression = 0
        hdrPadding = 3 * sizeof(int);       // Calculate padding header r,g,b mask size (12 bytes)
        *((unsigned int*)pal    ) = rmask;  // Red masked value
        *((unsigned int*)pal + 1) = gmask;  // Green masked value
        *((unsigned int*)pal + 2) = bmask;  // Blue masked value
    }

    // Fill BMP info
    bf.bfType       = 0x4d42; // BM signature
    bf.bfSize       = sizeof(bf) + sizeof(bi) + dataSize + hdrPadding + bmp->bmHeight * addPadding; // Actual data size
    bf.bfOffBits    = sizeof(bf) + sizeof(bi) + hdrPadding; // offset of data buffer
    bi.biSize       = sizeof(bi);
    bi.biWidth      = bmp->bmWidth;
    bi.biHeight     = bmp->bmHeight;
    bi.biPlanes     = 1;
    bi.biBitCount   = bitsPerPixel;
    bi.biSizeImage  = bf.bfSize - bf.bfOffBits;

    // Write header, and pading data (palette or RGB mask)
    fwrite(&bf, sizeof(bf), 1, fp);
    fwrite(&bi, sizeof(bi), 1, fp);
    fwrite(pal, hdrPadding, 1, fp);

    // Write inverted data (BMP file is stored in inverted)
    offset = dataSize - lineWidth;
    for (i = 0; i != bmp->bmHeight; i++)
    {
        fwrite(&bmp->bmData[offset], lineWidth, 1, fp); // Write raw data
        fwrite(aux, addPadding, 1, fp); // BMP file need padding to dword for each line
        offset -= lineWidth; // Skip to previous line
    }

    fclose(fp);
    return 1;
}

// close bitmap
void CloseBitmap(BITMAP *bmp)
{
    if (bmp && bmp->bmData)
    {
        free(bmp->bmData);
        bmp->bmData = NULL;
        bmp->bmWidth = 0;
        bmp->bmHeight = 0;
        bmp->bmPixels = 0;
        bmp->bmRowBytes = 0;
        memset(bmp->bmExtra, 0, 768);
    }
}

// Load and display BMP file
// (support 8/15/16/24/32 bits color)
void ShowBitmap(char *fname)
{
    BITMAP bmp;

    int rm, gm, bm;
    int bmWidth, bmHeight, bmPadding;
    int bmScanlineBytes, bmRowBytes;
    unsigned char *bmData, *bmPal;

    // Load bitmap to buffer
    if (!LoadBitmap(fname, &bmp)) FatalError("ShowBitmap: cannot load bitmap image: %s\n", fname);

    // save local values
    bmWidth         = bmp.bmWidth;
    bmHeight        = bmp.bmHeight;
    bmData          = bmp.bmData;
    bmScanlineBytes = bmp.bmRowBytes;
    bmRowBytes      = bmWidth * bmp.bmPixels;
    bmPadding       = (4 - bmRowBytes % 4) % 4;

    // load palette
    if (bmp.bmPixels == 1) bmPal = bmp.bmExtra;

    // load grb mask color
    else if (bmp.bmPixels == 2)
    {
        rm = *((unsigned int*)bmp.bmExtra    );
        gm = *((unsigned int*)bmp.bmExtra + 1);
        bm = *((unsigned int*)bmp.bmExtra + 2);
    }

    // build-in support multi-color
    switch (bitsPerPixel)
    {
    case 32:
        switch (bmp.bmPixels)
        {
        case 1:
            _asm {
                mov    eax, pageOffset
                mul    bytesPerScanline
                mov    edi, lfbPtr
                add    edi, eax
                mov    esi, bmData
                mov    ebx, bmPal
                mov    ecx, bmWidth
                mov    eax, bytesPerScanline
                mov    edx, bmWidth
                shl    edx, 2
                sub    eax, edx
                push   eax
            next:
                xor    edx, edx
                mov    dl, [esi]
                lea    edx, [edx + 2 * edx]
                mov    al, [ebx + edx + 2]
                stosb
                mov    al, [ebx + edx + 1]
                stosb
                mov    al, [ebx + edx]
                stosb
                inc    edi
                inc    esi
                loop   next
                mov    ecx, bmWidth
                add    edi, [esp]
                add    esi, bmPadding
                dec    bmHeight
                jnz    next
                pop    eax
            }
            break;

        case 2:
            if (gm == 0x03E0) // rgb555 format
            {
                _asm {
                    mov    eax, pageOffset
                    mul    bytesPerScanline
                    mov    edi, lfbPtr
                    add    edi, eax
                    mov    esi, bmData
                    mov    ecx, bmWidth
                    mov    ebx, bytesPerScanline
                    mov    edx, bmWidth
                    shl    edx, 2
                    sub    ebx, edx
                next:
                    mov    eax, [esi]
                    and    eax, bm
                    shl    eax, 3
                    stosb
                    mov    eax, [esi]
                    and    eax, gm
                    shr    eax, 5
                    shl    eax, 3
                    stosb
                    mov    eax, [esi]
                    and    eax, rm
                    shr    eax, 10
                    shl    eax, 3
                    stosb
                    inc    edi
                    add    esi, 2
                    loop   next
                    mov    ecx, bmWidth
                    add    edi, ebx
                    add    esi, bmPadding
                    dec    bmHeight
                    jnz    next
                }
            }
            else if (gm == 0x07E0) // rgb565 format
            {
                _asm {
                    mov    eax, pageOffset
                    mul    bytesPerScanline
                    mov    edi, lfbPtr
                    add    edi, eax
                    mov    esi, bmData
                    mov    ecx, bmWidth
                    mov    ebx, bytesPerScanline
                    mov    edx, bmWidth
                    shl    edx, 2
                    sub    ebx, edx
                next:
                    mov    eax, [esi]
                    and    eax, bm
                    shl    eax, 3
                    stosb
                    mov    eax, [esi]
                    and    eax, gm
                    shr    eax, 5
                    shl    eax, 2
                    stosb
                    mov    eax, [esi]
                    and    eax, rm
                    shr    eax, 11
                    shl    eax, 3
                    stosb
                    inc    edi
                    add    esi, 2
                    loop   next
                    mov    ecx, bmWidth
                    add    edi, ebx
                    add    esi, bmPadding
                    dec    bmHeight
                    jnz    next
                }
            }
            break;

        case 3:
            _asm {
                mov    eax, pageOffset
                mov    ebx, bytesPerScanline
                mul    ebx
                mov    edi, lfbPtr
                add    edi, eax
                mov    esi, bmData
                mov    ecx, bmWidth    
                mov    ebx, bytesPerScanline
                mov    edx, bmWidth
                shl    edx, 2
                sub    ebx, edx
            next:
                movsw
                movsb
                inc    edi
                loop   next
                mov    ecx, bmWidth
                add    edi, ebx
                add    esi, bmPadding
                dec    bmHeight
                jnz    next
            }
            break;

        case 4:
            _asm {
                mov    eax, pageOffset
                mul    bytesPerScanline
                mov    edi, lfbPtr
                add    edi, eax
                mov    esi, bmData
                mov    edx, bmScanlineBytes
                sub    edx, bmRowBytes
                mov    eax, bytesPerScanline
                sub    eax, bmRowBytes
            next:
                mov    ecx, bmWidth
                rep    movsd
                add    edi, eax
                add    esi, edx
                dec    bmHeight
                jnz    next
            }
            break;
        }
        break;

    case 24:
        switch (bmp.bmPixels)
        {
        case 1:
            _asm {
                mov    eax, pageOffset
                mul    bytesPerScanline
                mov    edi, lfbPtr
                add    edi, eax
                mov    esi, bmData
                mov    ebx, bmPal
                mov    ecx, bmWidth
                mov    eax, bytesPerScanline
                sub    eax, bmWidth
                sub    eax, bmWidth
                sub    eax, bmWidth
                push   eax
            next:
                xor    edx, edx
                mov    dl, [esi]
                lea    edx, [edx + 2 * edx]
                mov    al, [ebx + edx + 2]
                stosb
                mov    al, [ebx + edx + 1]
                stosb
                mov    al, [ebx + edx]
                stosb
                inc    esi
                loop   next
                mov    ecx, bmWidth
                add    edi, [esp]
                add    esi, bmPadding
                dec    bmHeight
                jnz    next
                pop    eax
            }
            break;

        case 2:
            if (gm == 0x03E0) // rgb555 format
            {
                _asm {
                    mov    eax, pageOffset
                    mul    bytesPerScanline
                    mov    edi, lfbPtr
                    add    edi, eax
                    mov    esi, bmData
                    mov    ecx, bmWidth
                    mov    ebx, bytesPerScanline
                    sub    ebx, bmWidth
                    sub    ebx, bmWidth
                    sub    ebx, bmWidth
                next:
                    mov    eax, [esi]
                    and    eax, bm
                    shl    eax, 3
                    stosb
                    mov    eax, [esi]
                    and    eax, gm
                    shr    eax, 5
                    shl    eax, 3
                    stosb
                    mov    eax, [esi]
                    and    eax, rm
                    shr    eax, 10
                    shl    eax, 3
                    stosb
                    add    esi, 2
                    loop   next
                    mov    ecx, bmWidth
                    add    edi, ebx
                    add    esi, bmPadding
                    dec    bmHeight
                    jnz    next
                }
            }
            else if (gm == 0x07E0) // rgb565 format
            {
                _asm {
                    mov    eax, pageOffset
                    mul    bytesPerScanline
                    mov    edi, lfbPtr
                    add    edi, eax
                    mov    esi, bmData
                    mov    ecx, bmWidth
                    mov    ebx, bytesPerScanline
                    sub    ebx, bmWidth
                    sub    ebx, bmWidth
                    sub    ebx, bmWidth
                next:
                    mov    eax, [esi]
                    and    eax, bm
                    shl    eax, 3
                    stosb
                    mov    eax, [esi]
                    and    eax, gm
                    shr    eax, 5
                    shl    eax, 2
                    stosb
                    mov    eax, [esi]
                    and    eax, rm
                    shr    eax, 11
                    shl    eax, 3
                    stosb
                    add    esi, 2
                    loop   next
                    mov    ecx, bmWidth
                    add    edi, ebx
                    add    esi, bmPadding
                    dec    bmHeight
                    jnz    next
                }
            }
            break;

        case 3:
            _asm {
                mov    eax, pageOffset
                mul    bytesPerScanline
                mov    edi, lfbPtr
                add    edi, eax
                mov    esi, bmData
                mov    edx, bmScanlineBytes
                sub    edx, bmRowBytes
                mov    eax, bytesPerScanline
                sub    eax, bmRowBytes
                mov    ecx, bmWidth
            next:
                movsw
                movsb
                loop   next
                add    edi, eax
                add    esi, edx
                mov    ecx, bmWidth
                dec    bmHeight
                jnz    next
            }
            break;
        }
        break;

    case 16:
        switch (bmp.bmPixels)
        {
        case 1:
            _asm {
                mov    eax, pageOffset
                mul    bytesPerScanline
                mov    edi, lfbPtr
                add    edi, eax
                mov    esi, bmData
                mov    ebx, bmPal
                mov    ecx, bmWidth    
                mov    eax, bytesPerScanline
                sub    eax, bmWidth
                sub    eax, bmWidth
                push   eax
            next:
                xor    eax, eax
                xor    edx, edx
                mov    dl, [esi]
                lea    edx, [edx + 2 * edx]
                mov    al, [ebx + edx]
                push   eax
                mov    al, [ebx + edx + 1]
                push   eax
                mov    al, [ebx + edx + 2]
                push   eax
                pop    eax
                shr    eax, 3
                and    eax, bmask
                pop    edx
                shr    edx, 2
                shl    edx, 5
                and    edx, gmask
                or     eax, edx
                pop    edx
                shr    edx, 3
                shl    edx, 11
                and    edx, rmask
                or     eax, edx
                stosw
                inc    esi
                loop   next
                mov    ecx, bmWidth
                add    edi, [esp]
                add    esi, bmPadding
                dec    bmHeight
                jnz    next
                pop    eax
            }
            break;

        case 2:
            if (gm == 0x03E0) // rgb555 format
            {
                _asm {
                    mov    eax, pageOffset
                    mul    bytesPerScanline
                    mov    edi, lfbPtr
                    add    edi, eax
                    mov    esi, bmData
                    mov    ecx, bmWidth
                    mov    eax, bytesPerScanline
                    sub    eax, bmWidth
                    sub    eax, bmWidth
                    push   eax
                next:
                    xor    edx, edx
                    xor    eax, eax
                    mov    eax, [esi]
                    and    eax, rm
                    shr    eax, 10
                    shl    eax, 3
                    shr    eax, 3
                    shl    eax, 11
                    and    eax, rmask
                    mov    ebx, [esi]
                    and    ebx, gm
                    shr    ebx, 5
                    shl    ebx, 3
                    shr    ebx, 2
                    shl    ebx, 5
                    and    ebx, gmask
                    mov    edx, [esi]
                    and    edx, bm
                    shl    edx, 3
                    shr    edx, 3
                    and    edx, bmask
                    or     eax, ebx
                    or     eax, edx
                    stosw
                    add    esi, 2
                    loop   next
                    mov    ecx, bmWidth
                    add    edi, [esp]
                    add    esi, bmPadding
                    dec    bmHeight
                    jnz    next
                    pop    eax
                }
            }
            else if (gm == 0x07E0) // rgb565 format
            {
                _asm {
                    mov    eax, pageOffset
                    mul    bytesPerScanline
                    mov    edi, lfbPtr
                    add    edi, eax
                    mov    esi, bmData
                    mov    edx, bmScanlineBytes
                    sub    edx, bmRowBytes
                    mov    eax, bytesPerScanline
                    sub    eax, bmRowBytes
                next:
                    mov    ecx, bmWidth
                    rep    movsw
                    add    edi, eax
                    add    esi, edx
                    dec    bmHeight
                    jnz    next
                }
            }
            break;
        }
        break;

    case 15:
        switch (bmp.bmPixels)
        {
        case 1:
            _asm {
                mov    eax, pageOffset
                mul    bytesPerScanline
                mov    edi, lfbPtr
                add    edi, eax
                mov    esi, bmData
                mov    ebx, bmPal
                mov    ecx, bmWidth

                mov    eax, bytesPerScanline
                sub    eax, bmWidth
                sub    eax, bmWidth
                push   eax
            next:
                xor    edx, edx
                xor    eax, eax
                mov    dl, [esi]
                lea    edx, [edx + 2 * edx]
                mov    al, [ebx + edx]
                push   eax
                mov    al, [ebx + edx + 1]
                push   eax
                mov    al, [ebx + edx + 2]
                push   eax
                pop    eax
                shr    eax, 3
                and    eax, bmask
                pop    edx
                shr    edx, 3
                shl    edx, 5
                and    edx, gmask
                or     eax, edx
                pop    edx
                shr    edx, 3
                shl    edx, 10
                and    edx, rmask
                or     eax, edx
                stosw
                inc    esi
                loop   next
                mov    ecx, bmWidth
                add    edi, [esp]
                add    esi, bmPadding
                dec    bmHeight
                jnz    next
                pop    eax
            }
            break;

        case 2:
            if (gm == 0x03E0) // rgb555 format
            {
                _asm {
                    mov    eax, pageOffset
                    mul    bytesPerScanline
                    mov    edi, lfbPtr
                    add    edi, eax
                    mov    esi, bmData
                    mov    edx, bmScanlineBytes
                    sub    edx, bmRowBytes
                    mov    eax, bytesPerScanline
                    sub    eax, bmRowBytes
                next:
                    mov    ecx, bmWidth
                    rep    movsw
                    add    edi, eax
                    add    esi, edx
                    dec    bmHeight
                    jnz    next
                }
            }
            else if (gm == 0x07E0) // rgb565 format
            {
                _asm {
                    mov    eax, pageOffset
                    mov    ebx, bytesPerScanline
                    mul    ebx
                    mov    edi, lfbPtr
                    add    edi, eax
                    mov    esi, bmData
                    mov    ecx, bmWidth    
                    mov    eax, bytesPerScanline
                    sub    eax, bmWidth
                    sub    eax, bmWidth
                    push   eax
                next:
                    mov    eax, [esi]
                    and    eax, rm
                    shr    eax, 11
                    shl    eax, 3
                    shr    eax, 3
                    shl    eax, 10
                    and    eax, rmask
                    mov    ebx, [esi]
                    and    ebx, gm
                    shr    ebx, 5
                    shl    ebx, 2
                    shr    ebx, 3
                    shl    ebx, 5
                    and    ebx, gmask
                    mov    edx, [esi]
                    and    edx, bm
                    shl    edx, 3
                    shr    edx, 3
                    and    edx, bmask
                    or     eax, ebx
                    or     eax, edx
                    stosw
                    add    esi, 2
                    loop   next
                    mov    ecx, bmWidth
                    add    edi, [esp]
                    add    esi, bmPadding
                    dec    bmHeight
                    jnz    next
                    pop    eax
                }
            }
            break;
        }
        break;

    case 8:
        SetPalette(bmPal);
        _asm {
            mov    eax, pageOffset
            mul    bytesPerScanline
            mov    edi, lfbPtr
            add    edi, eax
            mov    esi, bmData
            mov    eax, bytesPerScanline
            sub    eax, bmRowBytes
            mov    edx, bmScanlineBytes
            sub    edx, bmRowBytes
        next:
            mov    ecx, bmWidth
            rep    movsb
            add    edi, eax
            add    esi, edx
            dec    bmHeight
            jnz    next
        }
        break;
    }

    CloseBitmap(&bmp);
}

// Convert bitmap struct to image struct
void ConvertBitmap(BITMAP *bmp, IMAGE *img)
{
    unsigned int size = bmp->bmRowBytes * bmp->bmHeight;
    img->mData = (unsigned char*)malloc(size);
    if (!img->mData) FatalError("ConvertBitmap: cannot alloc memory.\n");
    img->mWidth     = bmp->bmWidth;
    img->mHeight    = bmp->bmHeight;
    img->mPixels    = bmp->bmPixels;
    img->mSize      = size;
    img->mRowBytes  = bmp->bmRowBytes;
    memcpy(img->mData, bmp->bmData, size);
}

// https://github.com/lvandeve/lodepng/commit/01ccad80e7241101db499029b71ba1c0bb456240
// PNG version string
#define PNG_VERSION_STRING  "20180611"

// The PNG color mode types (also used for raw)
typedef enum PNGColorType
{
    LCT_GREY = 0,       // greyscale: 1,2,4,8,16 bit
    LCT_RGB = 2,        // RGB: 8,16 bit
    LCT_PALETTE = 3,    // Palette: 1,2,4,8 bit
    LCT_GREY_ALPHA = 4, // greyscale with alpha: 8,16 bit
    LCT_RGBA = 6        // RGB with alpha: 8,16 bit
} PNGColorType;

// Settings for zlib decompression
typedef struct PNGDecompressSettings PNGDecompressSettings;
struct PNGDecompressSettings
{
    // check PNGDecoderSettings for more ignorable errors
    unsigned int ignoreAdler32; // if 1, continue and don't give an error message if the Adler32 checksum is corrupted

    // use custom zlib decoder instead of built in one (default: null)
    unsigned int (*customZlib)(unsigned char**, unsigned int*, const unsigned char*, unsigned int, const PNGDecompressSettings*);

    // use custom Deflate decoder instead of built in one (default: null)
    // if customZlib is used, CustomDeflate is ignored since only the built in
    // zlib function will call CustomDeflate
    unsigned int (*customInflate)(unsigned char**, unsigned int*, const unsigned char*, unsigned int, const PNGDecompressSettings*);
    const void* customContext; // optional custom settings for custom functions
};

extern const PNGDecompressSettings PNGDefaultDecompressSettings;

// Settings for zlib compression. Tweaking these settings tweaks the balance
// between speed and compression ratio.
typedef struct PNGCompressSettings PNGCompressSettings;
struct PNGCompressSettings // Deflate = compress
{
    // LZ77 related settings
    unsigned int type;          // the block type for LZ (0, 1, 2 or 3, see zlib standard). Should be 2 for proper compression.
    unsigned int useLZ77;       // whether or not to use LZ77. Should be 1 for proper compression.
    unsigned int windowSize;    // must be a power of two <= 32768. higher compresses more but is slower. Default value: 2048.
    unsigned int minMatch;      // mininum lz77 len. 3 is normally best, 6 can be better for some PNGs. Default: 0
    unsigned int niceMatch;     // stop searching if >= this len found. Set to 258 for best compression. Default: 128
    unsigned int lazyMatching;  // use lazy matching: better compression but a bit slower. Default: true

    // use custom zlib encoder instead of built in one (default: null)
    unsigned int (*customZlib)(unsigned char**, unsigned int*, const unsigned char*, unsigned int, const PNGCompressSettings*);

    // use custom Deflate encoder instead of built in one (default: null)
    // if customZlib is used, CustomDeflate is ignored since only the built in
    // zlib function will call CustomDeflate
    unsigned int (*CustomDeflate)(unsigned char**, unsigned int*, const unsigned char*, unsigned int, const PNGCompressSettings*);
    const void* customContext; // optional custom settings for custom functions
};

extern const PNGCompressSettings PNGDefaultCompressSettings;

// col modeNum of an data. Contains all information required to decode the pixel
// bits to RGBA colors. This information is the same as used in the PNG file
// format, and is used both for PNG and raw data data in LodePNG.
typedef struct PNGColorMode
{
    // header (IHDR)
    PNGColorType colorType; // colorMode type, see PNG standard or documentation further in this header file
    unsigned int bitDepth;  // bits per sample, see PNG standard or documentation further in this header file

    // palette (PLTE and tRNS)

    // Dynamically allocated with the colors of the palette, including alpha.
    // When encoding a PNG, to store your colors in the palette of the PNGColorMode, first use
    // PNGPaletteClear, then for each colorMode use PNGPaletteAdd.
    // If you encode an data without alpha with palette, don't forget to put value 255 in each A byte of the palette.
    // 
    // When decoding, by default you can ignore this palette, since LodePNG already
    // fills the palette colors in the pixels of the raw RGBA dst.
    //
    // The palette is only supported for colorMode type 3.
    unsigned char* palette; // palette in RGBARGBA... order. When allocated, must be either 0, or have size 1024
    unsigned int paletteSize; // palette size in number of colors (amount of bytes is 4 * paletteSize)

    // transparent colorMode key (tRNS)
    //
    // This colorMode uses the same bit depth as the bitDepth value in this struct, which can be 1-bit to 16-bit.
    // For greyscale PNGs, r, g and b will all 3 be set to the same.
    //
    // When decoding, by default you can ignore this information, since LodePNG sets
    // pixels with this key to transparent already in the raw RGBA dst.
    //
    // The colorMode key is only supported for colorMode types 0 and 2.
    unsigned int keyDefined;    // is a transparent colorMode key given? 0 = false, 1 = true
    unsigned int keyR;          // red/greyscale component of colorMode key
    unsigned int keyG;          // green component of colorMode key
    unsigned int keyB;          // blue component of colorMode key
} PNGColorMode;

// The information of a time chunk in PNG.
typedef struct PNGTime
{
    unsigned int year;      // 2 bytes used (0-65535)
    unsigned int month;     // 1-12
    unsigned int day;       // 1-31
    unsigned int hour;      // 0-23
    unsigned int minute;    // 0-59
    unsigned int second;    // 0-60 (to allow for leap seconds)
} PNGTime;

// Information about the PNG data, except pixels, bmWidth and bmHeight.
typedef struct PNGInfo
{
    // header (IHDR), palette (PLTE) and transparency (tRNS) chunks
    unsigned int compressionMethod; // compression method of the original file. Always 0.
    unsigned int filterMethod;      // Filter method of the original file
    unsigned int interlaceMethod;   // interlace method of the original file: 0=none, 1=Adam7
    PNGColorMode colorMode;         // colorMode type and bits, palette and transparency of the PNG file

    // suggested background colorMode chunk (bKGD)
    // This colorMode uses the same colorMode modeNum as the PNG (except alpha channel), which can be 1-bit to 16-bit.
    // 
    // For greyscale PNGs, r, g and b will all 3 be set to the same. When encoding
    // the encoder writes the red one. For palette PNGs: When decoding, the RGB value
    // will be stored, not a palette index. But when encoding, specify the index of
    // the palette in backgroundR, the other two are then ignored.
    // 
    // The decoder does not use this background colorMode to edit the colorMode of pixels.
    unsigned int backgroundDefined; // is a suggested background colorMode given?
    unsigned int backgroundR;       // red component of suggested background colorMode
    unsigned int backgroundG;       // green component of suggested background colorMode
    unsigned int backgroundB;       // blue component of suggested background colorMode

    // non-international text chunks (tEXt and zTXt)
    //
    // The char** arrays each contain num strings. The actual messages are in
    // textStrings, while textKeys are keywords that give a short description what
    // the actual text represents, e.g. Title, Author, Description, or anything else.
    //
    // A keyword is minimum 1 character and maximum 79 characters long. It's
    // discouraged to use a single line len longer than 79 characters for texts.
    //
    // Don't allocate these text buffers yourself. Use the init/cleanup functions
    // correctly and use PNGAddText and PNGClearText.
    unsigned int textNum;   // the amount of texts in these char** buffers (there may be more texts in itext)
    char** textKeys;        // the keyword of a text chunk (e.g. "Comment")
    char** textStrings;     // the actual text

    // international text chunks (iTXt)
    // Similar to the non-international text chunks, but with additional strings
    // "langtags" and "transkeys".
    unsigned int itextNum;      // the amount of international texts in this PNG
    char** itextKeys;       // the English keyword of the text chunk (e.g. "Comment")
    char** itextLangTags;   // language tag for this text's language, ISO/IEC 646 string, e.g. ISO 639 language tag
    char** itextTransKeys;  // keyword translated to the international language - UTF-8 string
    char** itextStrings;    // the actual international text - UTF-8 string

    // time chunk (tIME)
    unsigned int timeDefined;   // set to 1 to make the encoder generate a tIME chunk
    PNGTime time;

    // phys chunk (pHYs)
    unsigned int physDefined;   // if 0, there is no pHYs chunk and the values below are undefined, if 1 else there is one
    unsigned int physX;         // pixels per unit in x direction
    unsigned int physY;         // pixels per unit in y direction
    unsigned int physUnit;      // may be 0 (unknown unit) or 1 (metre)

    // unknown chunks
    // There are 3 buffers, one for each position in the PNG where unknown chunks can appear
    // each buffer contains all unknown chunks for that position consecutively
    // The 3 buffers are the unknown chunks between certain critical chunks:
    // 0: IHDR-PLTE, 1: PLTE-IDAT, 2: IDAT-IEND
    // Do not allocate or traverse this data yourself. Use the chunk traversing functions declared
    // later, such as PNGChunkNext and PNGChunkAppend, to read/write this struct.
    unsigned char* unknownChunksData[3];
    unsigned int unknownChunksSize[3];  // size in bytes of the unknown chunks, given for protection
} PNGInfo;

// Settings for the decoder. This contains settings for the PNG and the Zlib
// decoder, but not the info settings from the info structs.
typedef struct PNGDecoderSettings
{
    PNGDecompressSettings zlibSettings; // in here is the setting to ignore Adler32 checksums
    // check PNGDecompressSettings for more ignorable errors
    unsigned int ignoreCRC;                 // ignore CRC checksums
    unsigned int ignoreCritical;            // ignore unknown critical chunks
    unsigned int ignoreEnd;                 // ignore issues at end of file if possible (missing IEND chunk, too large chunk, ...)
    unsigned int colorConvert;              // whether to convert the PNG to the colorMode type you want. Default: yes
    unsigned int readTextChunks;            // if false but rememberUnknownChunks is true, they're stored in the unknown chunks
    // store all bytes from unknown chunks in the PNGInfo (off by default, useful for a png editor)
    unsigned int rememberUnknownChunks;
} PNGDecoderSettings;

// automatically use colorMode type with less bits per pixel if losslessly possible. Default: AUTO
typedef enum PNGFilterStrategy
{
    // every Filter at zero
    LFS_ZERO,

    // Use Filter that gives minumum sum, as described in the official PNG Filter heuristic.
    LFS_MINSUM,

    // Use the Filter type that gives smallest Shannon entropy for this scanline. Depending
    // on the data, this is better or worse than minsum.
    LFS_ENTROPY,

    // Brute-force-search PNG filters by compressing each Filter for each scanline.
    // Experimental, very slow, and only rarely gives better compression than MINSUM.
    LFS_BRUTE_FORCE,

    // use predefinedFilters buffer: you specify the Filter type for each scanline
    LFS_PREDEFINED
} PNGFilterStrategy;

// Gives characteristics about the colors of the data, which helps decide which colorMode model to use for encoding.
// Used internally by default if "autoConvert" is enabled. Public because it's useful for custom algorithms.
typedef struct PNGColorProfile
{
    unsigned int colored;               // not greyscale
    unsigned int key;                   // image is not opaque and color key is possible instead of full alpha
    unsigned int short keyR;            // key values, always as 16-bit, in 8-bit case the byte is duplicated, e.g. 65535 means 255
    unsigned int short keyG;
    unsigned int short keyB;
    unsigned int alpha;                 // image is not opaque and alpha channel or alpha palette required
    unsigned int numColors;             // amount of colors, up to 257. Not valid if bits == 16.
    unsigned char palette[1024];    // Remembers up to the first 256 RGBA colors, in no particular order
    unsigned int bits;                  // bits per channel (not for palette). 1,2 or 4 for greyscale only. 16 if 16-bit per channel required.
} PNGColorProfile;

// Settings for the encoder.
typedef struct PNGEncoderSettings
{
    PNGCompressSettings zlibSettings; // settings for the zlib encoder, such as window size, ...
    unsigned int autoConvert; // automatically choose dst PNG colorMode type. Default: true

    // If true, follows the official PNG heuristic: if the PNG uses a palette or lower than
    // 8 bit depth, set all filters to zero. Otherwise use the fillterStrategy. Note that to
    // completely follow the official PNG heuristic, fillterPaletteZero must be true and
    // fillterStrategy must be LFS_MINSUM
    unsigned int fillterPaletteZero;

    // Which Filter strategy to use when not using zeroes due to fillterPaletteZero.
    // Set fillterPaletteZero to 0 to ensure always using your chosen strategy. Default: LFS_MINSUM
    PNGFilterStrategy fillterStrategy;

    // used if fillterStrategy is LFS_PREDEFINED. In that case, this must point to a buffer with
    // the same len as the amount of scanLines in the data, and each value must <= 5. You
    // have to cleanup this buffer, LodePNG will never free it. Don't forget that fillterPaletteZero
    // must be set to 0 to ensure this is also used on palette or low bitDepth images.
    const unsigned char* predefinedFilters;

    // force creating a PLTE chunk if colorType is 2 or 6 (= a suggested palette).
    // If colorType is 3, PLTE is _always_ created.
    unsigned int forcePalette;

    // add LodePNG identifier and version as a text chunk, for debugging
    unsigned int addID;

    // encode text chunks as zTXt chunks instead of tEXt chunks, and use compression in iTXt chunks
    unsigned int textCompression;
} PNGEncoderSettings;

// The settings, state and information for extended encoding and decoding.
typedef struct PNGState
{
    PNGDecoderSettings decoder; // the decoding settings
    PNGEncoderSettings encoder; // the encoding settings
    PNGColorMode rawInfo;       // specifies the format in which you would like to get the raw pixel buffer
    PNGInfo pngInfo;            // info of the PNG data obtained after decoding
    unsigned int error;
} PNGState;

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
// Tools for C, and common code for PNG and Zlib.   //
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////

// Often in case of an error a value is assigned to a variable and then it breaks
// out of a loop (to go to the cleanup phase of a function). This macro does that.
// It makes the error handling code shorter and more readable.

// Example: if(!uiVectorResizeEx(&frequenciesLL, 286, 0)) ERROR_BREAK(83);
#define CERROR_BREAK(errorvar, code) { errorvar = code; break; }

// version of CERROR_BREAK that assumes the common case where the error variable is named "error"
#define ERROR_BREAK(code) CERROR_BREAK(error, code)

// Set error var to the error code, and return it.
#define CERROR_RETURN_ERROR(errorvar, code) { errorvar = code; return code; }

// Try the code, if it returns error, also return the error.
#define CERROR_TRY_RETURN(call) { int error = call; if (error) return error; }

// Set error var to the error code, and return from the void function.
#define CERROR_RETURN(errorvar, code) { errorvar = code; return; }

// About uiVector, ucVector and string:
// - All of them wrap dynamic arrays or text strings in a similar way.
// - LodePNG was originally written in C++. The vectors replace the std::vectors that were used in the C++ version.
// - The string tools are made to avoid problems with compilers that declare things like strncat as deprecated.
// - They're not used in the interface, only internally in this file as static functions.
// - As with many other structs in this file, the init and cleanup functions serve as ctor and dtor.

// dynamic vector of unsigned int ints
typedef struct uiVector
{
    unsigned int *data;     // raw vector data
    unsigned int size;      // size in number of unsigned int longs
    unsigned int allocSize; // allocated size in bytes
} uiVector;

void uiVectorCleanup(void* p)
{
    ((uiVector*)p)->size = ((uiVector*)p)->allocSize = 0;
    free(((uiVector*)p)->data);
    ((uiVector*)p)->data = NULL;
}

// returns 1 if success, 0 if failure ==> nothing done
unsigned int uiVectorReserve(uiVector *p, unsigned int allocSize)
{
    if (allocSize > p->allocSize)
    {
        unsigned int newSize = (allocSize > p->allocSize * 2) ? allocSize : (allocSize * 3 / 2);
        void *data = realloc(p->data, newSize);
        if (data)
        {
            p->allocSize = newSize;
            p->data = (unsigned int*)data;
        }
        else return 0; // error: not enough memory
    }

    return 1;
}

// returns 1 if success, 0 if failure ==> nothing done
unsigned int uiVectorResize(uiVector *p, unsigned int size)
{
    if (!uiVectorReserve(p, size * sizeof(unsigned int))) return 0;
    p->size = size;
    return 1;
}

// resize and give all new elements the value
unsigned int uiVectorResizeEx(uiVector *p, unsigned int size, unsigned int value)
{
    unsigned int oldSize = p->size, i;
    if (!uiVectorResize(p, size)) return 0;

    for (i = oldSize; i != size; i++) p->data[i] = value;

    return 1;
}

void uiVectorInit(uiVector* p)
{
    p->data = NULL;
    p->size = p->allocSize = 0;
}

// returns 1 if success, 0 if failure ==> nothing done
unsigned int uiVectorPushBack(uiVector *p, unsigned int c)
{
    if (!uiVectorResize(p, p->size + 1)) return 0;
    p->data[p->size - 1] = c;
    return 1;
}

// dynamic vector of unsigned int chars
typedef struct ucVector
{
    unsigned char* data;    // raw vector data
    unsigned int size;          // used size
    unsigned int allocSize;     // allocated size
} ucVector;

// returns 1 if success, 0 if failure ==> nothing done
unsigned int ucVectorReserve(ucVector *p, unsigned int allocSize)
{
    if (allocSize > p->allocSize)
    {
        unsigned int newSize = (allocSize > p->allocSize * 2) ? allocSize : (allocSize * 3 / 2);
        void *data = realloc(p->data, newSize);
        if (data)
        {
            p->allocSize = newSize;
            p->data = (unsigned char*)data;
        }
        else return 0;
    }

    return 1;
}

// returns 1 if success, 0 if failure ==> nothing done
unsigned int ucVectorResize(ucVector *p, unsigned int size)
{
    if (!ucVectorReserve(p, size * sizeof(unsigned char))) return 0;
    p->size = size;
    return 1;
}

void ucVectorCleanup(void *p)
{
    ((ucVector*)p)->size = ((ucVector*)p)->allocSize = 0;
    free(((ucVector*)p)->data);
    ((ucVector*)p)->data = NULL;
}

void ucVectorInit(ucVector *p)
{
    p->data = NULL;
    p->size = p->allocSize = 0;
}

// resize and give all new elements the value
unsigned int ucVectorResizeEx(ucVector *p, unsigned int size, unsigned char value)
{
    unsigned int oldSize = p->size, i;
    if (!ucVectorResize(p, size)) return 0;
    for (i = oldSize; i != size; i++) p->data[i] = value;
    return 1;
}

// you can both convert from vector to buffer&size and vica versa. If you use
// init_buffer to take over a buffer and size, it is not needed to use cleanup
void ucVectorInitBuffer(ucVector *p, unsigned char *buffer, unsigned int size)
{
    p->data = buffer;
    p->allocSize = p->size = size;
}

// returns 1 if success, 0 if failure ==> nothing done
unsigned int ucVectorPushBack(ucVector *p, unsigned char c)
{
    if (!ucVectorResize(p, p->size + 1)) return 0;
    p->data[p->size - 1] = c;
    return 1;
}

// returns 1 if success, 0 if failure ==> nothing done
unsigned int StringResize(char **out, unsigned int size)
{
    char *data = (char*)realloc(*out, size + 1);
    if (data)
    {
        data[size] = 0; // null termination char
        *out = data;
    }

    return data != NULL;
}

// init a {char*, unsigned int} pair for use as string
void StringInit(char **out)
{
    *out = NULL;
    StringResize(out, 0);
}

// free the above pair again
void StringCleanup(char **out)
{
    free(*out);
    *out = NULL;
}

void StringSet(char **out, const char *in)
{
    unsigned int inSize = strlen(in), i = 0;
    if (StringResize(out, inSize))
    {
        for (i = 0; i != inSize; i++) (*out)[i] = in[i];
    }
}

unsigned int PNGRead32bitInt(const unsigned char *buffer)
{
    return (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
}

// buffer must have at least 4 allocated bytes available
void PNGSet32bitInt(unsigned char *buffer, unsigned int value)
{
    buffer[0] = (unsigned char)((value >> 24) & 0xff);
    buffer[1] = (unsigned char)((value >> 16) & 0xff);
    buffer[2] = (unsigned char)((value >>  8) & 0xff);
    buffer[3] = (unsigned char)((value      ) & 0xff);
}

void PNGAdd32bitInt(ucVector *buffer, unsigned int value)
{
    ucVectorResize(buffer, buffer->size + 4); // todo: give error if resize failed
    PNGSet32bitInt(&buffer->data[buffer->size - 4], value);
}

/////////////////////////////////////////////////////////////////
// End of common code and tools. Begin of Zlib related code.   // 
/////////////////////////////////////////////////////////////////

// TODO: this ignores potential out of memory errors
#define AddBitToStream(/* unsigned int */ bitpointer, /* ucVector */ bitstream, /* unsigned char */ bit)\
{\
    if (((*bitpointer) & 7) == 0) ucVectorPushBack(bitstream, (unsigned char)0);\
    (bitstream->data[bitstream->size - 1]) |= (bit << ((*bitpointer) & 0x7));\
    (*bitpointer)++;\
}

void AddBitsToStream(unsigned int *bitpointer, ucVector *bitstream, unsigned int value, unsigned int nbits)
{
    unsigned int i;
    for (i = 0; i != nbits; i++) AddBitToStream(bitpointer, bitstream, (unsigned char)((value >> i) & 1));
}

void AddBitsToStreamReversed(unsigned int *bitpointer, ucVector *bitstream, unsigned int value, unsigned int nbits)
{
    unsigned int i;
    for (i = 0; i != nbits; i++) AddBitToStream(bitpointer, bitstream, (unsigned char)((value >> (nbits - 1 - i)) & 1));
}

#define ReadBit(bitpointer, bitstream) ((bitstream[bitpointer >> 3] >> (bitpointer & 0x7)) & (unsigned char)1)

unsigned char ReadBitFromStream(unsigned int *bitpointer, const unsigned char *bitstream)
{
    unsigned char result = (unsigned char)(ReadBit(*bitpointer, bitstream));
    (*bitpointer)++;
    return result;
}

unsigned int ReadBitsFromStream(unsigned int *bitpointer, const unsigned char *bitstream, unsigned int nbits)
{
    unsigned int result = 0, i;
    for (i = 0; i != nbits; i++)
    {
        result += ((unsigned int)ReadBit(*bitpointer, bitstream)) << i;
        (*bitpointer)++;
    }
    return result;
}

////////////////////////
// Deflate - Huffman  // 
////////////////////////

#define FIRST_LENGTH_CODE_INDEX 257
#define LAST_LENGTH_CODE_INDEX 285

// 256 literals, the end code, some len codes, and 2 unused codes
#define NUM_DEFLATE_CODE_SYMBOLS 288

// the distance codes have their own symbols, 30 used, 2 unused
#define NUM_DISTANCE_SYMBOLS 32

// the code len codes. 0-15: code lenghts, 16: copy previous 3-6 times, 17: 3-10 zeros, 18: 11-138 zeros
#define NUM_CODE_LENGTH_CODES 19

// the base lenghts represented by codes 257-285
const unsigned int LENGTHBASE[29] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};

// the extra bits used by codes 257-285 (added to base len)
const unsigned int LENGTHEXTRA[29] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};

// the base backwards distances (the bits of distance codes appear after len codes and use their own huffman tree)
const unsigned int DISTANCEBASE[30] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};

// the extra bits of backwards distances (added to base)
const unsigned int DISTANCEEXTRA[30] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};

// the order in which "code len alphabet code lenghts" are stored, out of this
// the huffman tree of the dynamic huffman tree lenghts is generated
const unsigned int CLCL_ORDER[NUM_CODE_LENGTH_CODES] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

// Huffman tree struct, containing multiple representations of the tree
typedef struct HuffmanTree
{
    unsigned int* tree2d;   // dimension data
    unsigned int* tree1d;   // array data
    unsigned int* lenghts;  // the lenghts of the codes of the 1d-tree
    unsigned int maxBitLen; // maximum number of bits a single code can get
    unsigned int numCodes;  // number of symbols in the alphabet = number of codes
} HuffmanTree;

void HuffmanTreeInit(HuffmanTree* tree)
{
    tree->tree2d = NULL;
    tree->tree1d = NULL;
    tree->lenghts = 0;
}

void HuffmanTreeCleanup(HuffmanTree* tree)
{
    free(tree->tree2d);
    free(tree->tree1d);
    free(tree->lenghts);
}

// the tree representation used by the decoder. return value is error
unsigned int HuffmanTreeMake2DTree(HuffmanTree* tree)
{
    unsigned int n, i;
    unsigned int nodeFilled = 0; // up to which node it is filled
    unsigned int treePos = 0; // position in the tree (1 of the numCodes columns)

    tree->tree2d = (unsigned int*)malloc(tree->numCodes * 2 * sizeof(unsigned int));
    if (!tree->tree2d) return 83;

    // convert tree1d[] to tree2d[][]. In the 2D array, a value of 32767 means
    // uninited, a value >= numCodes is an address to another bit, a value < numCodes
    // is a code. The 2 rows are the 2 possible bit values (0 or 1), there are as
    // many columns as codes - 1.
    // A good huffmann tree has num * 2 - 1 nodes, of which num - 1 are internal nodes.
    // Here, the internal nodes are stored (what their 0 and 1 option point to).
    // There is only memory for such good tree currently, if there are more nodes
    // (due to too long len codes), error 55 will happen
    for (n = 0; n < tree->numCodes * 2; n++)
    {
        tree->tree2d[n] = 32767; // 32767 here means the tree2d isn't filled there yet
    }

    for (n = 0; n < tree->numCodes; n++) // the codes
    {
        for (i = 0; i != tree->lenghts[n]; i++) // the bits for this code
        {
            unsigned char bit = (unsigned char)((tree->tree1d[n] >> (tree->lenghts[n] - i - 1)) & 1);
            if (treePos > 2147483647 || treePos + 2 > tree->numCodes) return 55; // oversubscribed, see comment in PNGErrorText
            if (tree->tree2d[2 * treePos + bit] == 32767) // not yet filled in
            {
                if (i + 1 == tree->lenghts[n]) // last bit
                {
                    tree->tree2d[2 * treePos + bit] = n; // put the current code in it
                    treePos = 0;
                }
                else
                {
                    // put address of the next step in here, first that address has to be found of course
                    // (it's just nodeFilled + 1)...
                    nodeFilled++;

                    // addresses encoded with numCodes added to it
                    tree->tree2d[2 * treePos + bit] = nodeFilled + tree->numCodes;
                    treePos = nodeFilled;
                }
            }
            else treePos = tree->tree2d[2 * treePos + bit] - tree->numCodes;
        }
    }

    for (n = 0; n < tree->numCodes * 2; n++)
    {
        if (tree->tree2d[n] == 32767) tree->tree2d[n] = 0; // remove possible remaining 32767's
    }

    return 0;
}

// second step for the ...makeFromLengths and ...makeFromFrequencies functions.
// numCodes, lenghts and maxBitLen must already be filled in correctly. return
// value is error.
unsigned int HuffmanTreeMakeFromLengths2(HuffmanTree* tree)
{
    uiVector blcount;
    uiVector nextcode;
    unsigned int bits, n, error = 0;

    uiVectorInit(&blcount);
    uiVectorInit(&nextcode);

    tree->tree1d = (unsigned int*)malloc(tree->numCodes * sizeof(unsigned int));
    if (!tree->tree1d) error = 83;

    if (!uiVectorResizeEx(&blcount, tree->maxBitLen + 1, 0) || !uiVectorResizeEx(&nextcode, tree->maxBitLen + 1, 0)) error = 83;

    if (!error)
    {
        // step 1: count number of instances of each code len
        for (bits = 0; bits < tree->numCodes; bits++) blcount.data[tree->lenghts[bits]]++;

        // step 2: generate the nextcode values
        for (bits = 1; bits <= tree->maxBitLen; bits++)
        {
            nextcode.data[bits] = (nextcode.data[bits - 1] + blcount.data[bits - 1]) << 1;
        }

        // step 3: generate all the codes
        for (n = 0; n < tree->numCodes; n++)
        {
            if (tree->lenghts[n] != 0) tree->tree1d[n] = nextcode.data[tree->lenghts[n]]++;
        }
    }

    uiVectorCleanup(&blcount);
    uiVectorCleanup(&nextcode);

    if (!error) return HuffmanTreeMake2DTree(tree);
    return error;
}

// given the code lenghts (as stored in the PNG file), generate the tree as defined
// by Deflate. maxBitLen is the maximum bits that a code in the tree can have.
// return value is error.
unsigned int HuffmanTreeMakeFromLengths(HuffmanTree* tree, const unsigned int* bitLength, unsigned int numCodes, unsigned int maxBitLen)
{
    unsigned int i;
    tree->lenghts = (unsigned int*)malloc(numCodes * sizeof(unsigned int));
    if (!tree->lenghts) return 83;

    for (i = 0; i != numCodes; i++) tree->lenghts[i] = bitLength[i];

    tree->numCodes = numCodes;
    tree->maxBitLen = maxBitLen;

    return HuffmanTreeMakeFromLengths2(tree);
}

// BPM: Boundary Package Merge, see "A Fast and Space-Economical Algorithm for len-Limited Coding",
// Jyrki Katajainen, Alistair Moffat, Andrew Turpin, 1995.

// chain node for boundary package merge
typedef struct BPMNode
{
    int weight;             // the sum of all weights in this chain
    unsigned int index;         // index of this leaf node (called "count" in the paper)
    struct BPMNode* tail;   // the next nodes in this chain (null if last)
    int used;               // in-used node
} BPMNode;

// lists of chains
typedef struct BPMLists
{
    // memory pool
    unsigned int memsize;
    BPMNode* memory;
    unsigned int numfree;
    unsigned int nextfree;
    BPMNode** freelist;

    // two heads of lookahead chains per list
    unsigned int listsize;
    BPMNode** chains0;
    BPMNode** chains1;
} BPMLists;

// creates a new chain node with the given parameters, from the memory in the lists
BPMNode* BPMNodeCreate(BPMLists* lists, int weight, unsigned int index, BPMNode* tail)
{
    unsigned int i;
    BPMNode* result;

    // memory full, so garbage collect
    if (lists->nextfree >= lists->numfree)
    {
        // mark only those that are in use
        for (i = 0; i != lists->memsize; ++i) lists->memory[i].used = 0;
        for (i = 0; i != lists->listsize; ++i)
        {
            BPMNode* node;
            for (node = lists->chains0[i]; node != 0; node = node->tail) node->used = 1;
            for (node = lists->chains1[i]; node != 0; node = node->tail) node->used = 1;
        }

        // collect those that are free
        lists->numfree = 0;
        for (i = 0; i != lists->memsize; ++i)
        {
            if (!lists->memory[i].used) lists->freelist[lists->numfree++] = &lists->memory[i];
        }
        lists->nextfree = 0;
    }

    result = lists->freelist[lists->nextfree++];
    result->weight = weight;
    result->index = index;
    result->tail = tail;

    return result;
}

// sort the leaves with stable mergesort
void BPMNodeSort(BPMNode* leaves, unsigned int num)
{
    unsigned int p, q, r, i, j, k, width, counter = 0;
    BPMNode* mem = (BPMNode*)malloc(sizeof(*leaves) * num);

    for (width = 1; width < num; width *= 2)
    {
        BPMNode* a = (counter & 1) ? mem : leaves;
        BPMNode* b = (counter & 1) ? leaves : mem;

        for (p = 0; p < num; p += 2 * width)
        {
            q = (p + width > num) ? num : (p + width);
            r = (p + 2 * width > num) ? num : (p + 2 * width);
            i = p, j = q;
            for (k = p; k < r; k++)
            {
                if (i < q && (j >= r || a[i].weight <= a[j].weight)) b[k] = a[i++];
                else b[k] = a[j++];
            }
        }
        counter++;
    }

    if (counter & 1) memcpy(leaves, mem, sizeof(*leaves) * num);
    free(mem);
}

// Boundary Package Merge step, numPresent is the amount of leaves, and c is the current chain.
void BoundaryPM(BPMLists* lists, BPMNode* leaves, unsigned int numPresent, int c, int num)
{
    unsigned int lastIndex = lists->chains1[c]->index;

    if (c == 0)
    {
        if (lastIndex >= numPresent) return;
        lists->chains0[c] = lists->chains1[c];
        lists->chains1[c] = BPMNodeCreate(lists, leaves[lastIndex].weight, lastIndex + 1, 0);
    }
    else
    {
        // sum of the weights of the head nodes of the previous lookahead chains.
        int sum = lists->chains0[c - 1]->weight + lists->chains1[c - 1]->weight;
        lists->chains0[c] = lists->chains1[c];

        if (lastIndex < numPresent && sum > leaves[lastIndex].weight)
        {
            lists->chains1[c] = BPMNodeCreate(lists, leaves[lastIndex].weight, lastIndex + 1, lists->chains1[c]->tail);
            return;
        }

        lists->chains1[c] = BPMNodeCreate(lists, sum, lastIndex, lists->chains1[c - 1]);

        // in the end we are only interested in the chain of the last list, so no
        // need to recurse if we're at the last one (this gives measurable speedup)
        if (num + 1 < (int)(2 * numPresent - 2))
        {
            BoundaryPM(lists, leaves, numPresent, c - 1, num);
            BoundaryPM(lists, leaves, numPresent, c - 1, num);
        }
    }
}

unsigned int PNGHuffmanCodeLengths(unsigned int* lenghts, const unsigned int* frequencies, unsigned int numCodes, unsigned int maxBitLen)
{
    unsigned int i, error = 0;
    unsigned int numPresent = 0; // number of symbols with non-zero frequency
    BPMNode* leaves; //the symbols, only those with > 0 frequency

    if (numCodes == 0) return 80; // error: a tree of 0 symbols is not supposed to be made
    if ((1u << maxBitLen) < (unsigned int)numCodes) return 80; // error: represent all symbols

    leaves = (BPMNode*)malloc(numCodes * sizeof(*leaves));
    if (!leaves) return 83;

    for (i = 0; i != numCodes; i++)
    {
        if (frequencies[i] > 0)
        {
            leaves[numPresent].weight = (int)frequencies[i];
            leaves[numPresent].index = i;
            numPresent++;
        }
    }

    for (i = 0; i != numCodes; i++) lenghts[i] = 0;

    // ensure at least two present symbols. There should be at least one symbol
    // according to RFC 1951 section 3.2.7. To decoders incorrectly require two. To
    // make these work as well ensure there are at least two symbols. The
    // Package-Merge code below also doesn't work correctly if there's only one
    // symbol, it'd give it the theoritical 0 bits but in practice zlib wants 1 bit
    if (numPresent == 0)
    {
        lenghts[0] = lenghts[1] = 1; // note that for RFC 1951 section 3.2.7, only lenghts[0] = 1 is needed
    }
    else if (numPresent == 1)
    {
        lenghts[leaves[0].index] = 1;
        lenghts[leaves[0].index == 0 ? 1 : 0] = 1;
    }
    else
    {
        BPMLists lists;
        BPMNode* node;

        BPMNodeSort(leaves, numPresent);

        lists.listsize = maxBitLen;
        lists.memsize = 2 * maxBitLen * (maxBitLen + 1);
        lists.nextfree = 0;
        lists.numfree = lists.memsize;
        lists.memory = (BPMNode*)malloc(lists.memsize * sizeof(*lists.memory));
        lists.freelist = (BPMNode**)malloc(lists.memsize * sizeof(BPMNode*));
        lists.chains0 = (BPMNode**)malloc(lists.listsize * sizeof(BPMNode*));
        lists.chains1 = (BPMNode**)malloc(lists.listsize * sizeof(BPMNode*));
        if (!lists.memory || !lists.freelist || !lists.chains0 || !lists.chains1) error = 83;

        if (!error)
        {
            for(i = 0; i != lists.memsize; ++i) lists.freelist[i] = &lists.memory[i];

            BPMNodeCreate(&lists, leaves[0].weight, 1, 0);
            BPMNodeCreate(&lists, leaves[1].weight, 2, 0);

            for (i = 0; i != lists.listsize; ++i)
            {
            lists.chains0[i] = &lists.memory[0];
            lists.chains1[i] = &lists.memory[1];
            }

            // each boundaryPM call adds one chain to the last list, and we need 2 * numPresent - 2 chains.
            for (i = 2; i != 2 * numPresent - 2; ++i) BoundaryPM(&lists, leaves, numPresent, (int)maxBitLen - 1, (int)i);

            for (node = lists.chains1[maxBitLen - 1]; node; node = node->tail)
            {
                for (i = 0; i != node->index; ++i) ++lenghts[leaves[i].index];
            }
        }

        free(lists.memory);
        free(lists.freelist);
        free(lists.chains0);
        free(lists.chains1);
    }

    free(leaves);
    return error;
}

// Create the Huffman tree given the symbol frequencies
unsigned int HuffmanTreeMakeFromFrequencies(HuffmanTree* tree, const unsigned int* frequencies, unsigned int minCodes, unsigned int numCodes, unsigned int maxBitLen)
{
    unsigned int error = 0;
    while (!frequencies[numCodes - 1] && numCodes > minCodes) numCodes--; // trim zeroes

    tree->maxBitLen = maxBitLen;
    tree->numCodes = numCodes; // number of symbols
    tree->lenghts = (unsigned int*)realloc(tree->lenghts, numCodes * sizeof(unsigned int));

    if (!tree->lenghts) return 83;

    // initialize all lenghts to 0
    memset(tree->lenghts, 0, numCodes * sizeof(unsigned int));

    error = PNGHuffmanCodeLengths(tree->lenghts, frequencies, numCodes, maxBitLen);
    if (!error) error = HuffmanTreeMakeFromLengths2(tree);

    return error;
}

unsigned int HuffmanTreeGetCode(const HuffmanTree* tree, unsigned int index)
{
    return tree->tree1d[index];
}

unsigned int HuffmanTreeGetLength(const HuffmanTree* tree, unsigned int index)
{
    return tree->lenghts[index];
}

// get the literal and len code tree of a deflated block with fixed tree, as per the Deflate specification
unsigned int GenerateFixedLitLenTree(HuffmanTree* tree)
{
    unsigned int i, error = 0;
    unsigned int* bitLength = (unsigned int*)malloc(NUM_DEFLATE_CODE_SYMBOLS * sizeof(unsigned int));

    if (!bitLength) return 83;

    // 288 possible codes: 0-255=literals, 256=endcode, 257-285=lengthcodes, 286-287=unused
    for (i =  0; i <= 143; i++) bitLength[i] = 8;
    for (i = 144; i <= 255; i++) bitLength[i] = 9;
    for (i = 256; i <= 279; i++) bitLength[i] = 7;
    for (i = 280; i <= 287; i++) bitLength[i] = 8;

    error = HuffmanTreeMakeFromLengths(tree, bitLength, NUM_DEFLATE_CODE_SYMBOLS, 15);

    free(bitLength);
    return error;
}

// get the distance code tree of a deflated block with fixed tree, as specified in the Deflate specification
unsigned int GenerateFixedDistanceTree(HuffmanTree* tree)
{
    unsigned int i, error = 0;
    unsigned int* bitLength = (unsigned int*)malloc(NUM_DISTANCE_SYMBOLS * sizeof(unsigned int));
    if (!bitLength) return 83;

    // there are 32 distance codes, but 30-31 are unused
    for (i = 0; i != NUM_DISTANCE_SYMBOLS; i++) bitLength[i] = 5;
    error = HuffmanTreeMakeFromLengths(tree, bitLength, NUM_DISTANCE_SYMBOLS, 15);

    free(bitLength);
    return error;
}

// returns the code, or (unsigned int)(-1) if error happened
// inBitLength is the len of the complete buffer, in bits (so its byte len times 8)
unsigned int HuffmanDecodeSymbol(const unsigned char* in, unsigned int* bp, const HuffmanTree* codeTree, unsigned int inBitLength)
{
    unsigned int treePos = 0, ct;

    for (;;)
    {
        if (*bp >= inBitLength) return (unsigned int)(-1); // error: end of src memory reached without endcode

        // decode the symbol from the tree. The "ReadBitFromStream" code is inlined in
        // the expression below because this is the biggest bottleneck while decoding
        ct = codeTree->tree2d[(treePos << 1) + ReadBit(*bp, in)];
        (*bp)++;

        if (ct < codeTree->numCodes) return ct; // the symbol is decoded, return it
        else treePos = ct - codeTree->numCodes; // symbol not yet decoded, instead move tree position

        if (treePos >= codeTree->numCodes) return (unsigned int)(-1); // error: it appeared outside the codeTree
    }
}

/////////////////////////////
// Inflator (Decompressor) // 
/////////////////////////////

// get the tree of a deflated block with fixed tree, as specified in the Deflate specification
void GetTreeInflateFixed(HuffmanTree* treeLL, HuffmanTree* treeD)
{
    // check for out of memory errors
    GenerateFixedLitLenTree(treeLL);
    GenerateFixedDistanceTree(treeD);
}

// get the tree of a deflated block with dynamic tree, the tree itself is also Huffman compressed with a known tree
unsigned int GetTreeInflateDynamic(HuffmanTree* treeLL, HuffmanTree* treeD, const unsigned char* in, unsigned int* bp, unsigned int inLength)
{
    // make sure that len values that aren't filled in will be 0, or a wrong tree will be generated
    unsigned int i, error = 0;
    unsigned int n, HLIT, HDIST, HCLEN;
    unsigned int inBitLength = inLength * 8;

    // see comments in DeflateDynamic for explanation of the context and these variables, it is analogous
    unsigned int* bitLenLL = 0; // lit,len code lenghts
    unsigned int* bitLenD = 0; // dist code lenghts

    // code len code lenghts ("clcl"), the bit lenghts of the huffman tree used to compress bitLenLL and bitLenD
    unsigned int* bitLenCL = 0;
    HuffmanTree treeCL; // the code tree for code len codes (the huffman tree for compressed huffman trees)

    if ((*bp) >> 3 >= inLength - 2) return 49; // error: the bit pointer is or will go past the memory

    // number of literal/len codes + 257. Unlike the spec, the value 257 is added to it here already
    HLIT =  ReadBitsFromStream(bp, in, 5) + 257;

    // number of distance codes. Unlike the spec, the value 1 is added to it here already
    HDIST = ReadBitsFromStream(bp, in, 5) + 1;

    // number of code len codes. Unlike the spec, the value 4 is added to it here already
    HCLEN = ReadBitsFromStream(bp, in, 4) + 4;

    HuffmanTreeInit(&treeCL);

    while (!error)
    {
        // read the code len codes out of 3 * (amount of code len codes) bits
        bitLenCL = (unsigned int*)malloc(NUM_CODE_LENGTH_CODES * sizeof(unsigned int));
        if (!bitLenCL) ERROR_BREAK(83);

        for (i = 0; i != NUM_CODE_LENGTH_CODES; i++)
        {
            if (i < HCLEN) bitLenCL[CLCL_ORDER[i]] = ReadBitsFromStream(bp, in, 3);
            else bitLenCL[CLCL_ORDER[i]] = 0; // if not, it must stay 0
        }

        error = HuffmanTreeMakeFromLengths(&treeCL, bitLenCL, NUM_CODE_LENGTH_CODES, 7);
        if (error) break;

        // now we can use this tree to read the lenghts for the tree that this function will return
        bitLenLL = (unsigned int*)malloc(NUM_DEFLATE_CODE_SYMBOLS * sizeof(unsigned int));
        bitLenD = (unsigned int*)malloc(NUM_DISTANCE_SYMBOLS * sizeof(unsigned int));

        if (!bitLenLL || !bitLenD) ERROR_BREAK(83);

        for (i = 0; i != NUM_DEFLATE_CODE_SYMBOLS; i++) bitLenLL[i] = 0;
        for (i = 0; i != NUM_DISTANCE_SYMBOLS; i++) bitLenD[i] = 0;

        // i is the current symbol we're reading in the part that contains the code lenghts of lit/len and dist codes
        i = 0;
        while (i < HLIT + HDIST)
        {
            unsigned int code = HuffmanDecodeSymbol(in, bp, &treeCL, inBitLength);
            if (code <= 15) // a len code
            {
                if (i < HLIT) bitLenLL[i] = code;
                else bitLenD[i - HLIT] = code;
                i++;
            }
            else if (code == 16) // repeat previous
            {
                unsigned int repLength = 3; // read in the 2 bits that indicate repeat len (3-6)
                unsigned int value; // set value to the previous code

                if (*bp >= inBitLength) ERROR_BREAK(50); // error, bit pointer jumps past memory
                if (i == 0) ERROR_BREAK(54); // can't repeat previous if i is 0

                repLength += ReadBitsFromStream(bp, in, 2);

                if (i < HLIT + 1) value = bitLenLL[i - 1];
                else value = bitLenD[i - HLIT - 1];

                // repeat this value in the next lenghts
                for (n = 0; n < repLength; n++)
                {
                    if (i >= HLIT + HDIST) ERROR_BREAK(13); // error: i is larger than the amount of codes
                    if (i < HLIT) bitLenLL[i] = value;
                    else bitLenD[i - HLIT] = value;
                    i++;
                }
            }
            else if (code == 17) // repeat "0" 3-10 times
            {
                unsigned int repLength = 3; // read in the bits that indicate repeat len
                if (*bp >= inBitLength) ERROR_BREAK(50); // error, bit pointer jumps past memory

                repLength += ReadBitsFromStream(bp, in, 3);

                // repeat this value in the next lenghts
                for (n = 0; n < repLength; n++)
                {
                    if (i >= HLIT + HDIST) ERROR_BREAK(14); // error: i is larger than the amount of codes

                    if (i < HLIT) bitLenLL[i] = 0;
                    else bitLenD[i - HLIT] = 0;
                    i++;
                }
            }
            else if (code == 18) // repeat "0" 11-138 times
            {
                unsigned int repLength = 11; // read in the bits that indicate repeat len
                if (*bp >= inBitLength) ERROR_BREAK(50); // error, bit pointer jumps past memory

                repLength += ReadBitsFromStream(bp, in, 7);

                // repeat this value in the next lenghts
                for (n = 0; n < repLength; n++)
                {
                    if (i >= HLIT + HDIST) ERROR_BREAK(15); // error: i is larger than the amount of codes

                    if (i < HLIT) bitLenLL[i] = 0;
                    else bitLenD[i - HLIT] = 0;
                    i++;
                }
            }
            else // if(code == (unsigned int)(-1)) // HuffmanDecodeSymbol returns (unsigned int)(-1) in case of error
            {
                if (code == (unsigned int)(-1))
                {
                    // return error code 10 or 11 depending on the situation that happened in HuffmanDecodeSymbol
                    // (10=no endcode, 11=wrong jump outside of tree)
                    error = (*bp) > inBitLength ? 10 : 11;
                }
                else error = 16; // unexisting code, this can never happen
                break;
            }
        }

        if (error) break;

        if (bitLenLL[256] == 0) ERROR_BREAK(64); // the len of the end code 256 must be larger than 0

        // now we've finally got HLIT and HDIST, so generate the code trees, and the function is done
        error = HuffmanTreeMakeFromLengths(treeLL, bitLenLL, NUM_DEFLATE_CODE_SYMBOLS, 15);
        if (error) break;

        error = HuffmanTreeMakeFromLengths(treeD, bitLenD, NUM_DISTANCE_SYMBOLS, 15);
        break; // end of error-while
    }

    free(bitLenCL);
    free(bitLenLL);
    free(bitLenD);
    HuffmanTreeCleanup(&treeCL);

    return error;
}

// Inflate a block with dynamic of fixed Huffman tree
unsigned int InflateHuffmanBlock(ucVector* out, const unsigned char* in, unsigned int* bp, unsigned int* pos, unsigned int inLength, unsigned int type)
{
    unsigned int error = 0;
    HuffmanTree treeLL; // the huffman tree for literal and len codes
    HuffmanTree treeD; // the huffman tree for distance codes
    unsigned int inBitLength = inLength * 8;

    HuffmanTreeInit(&treeLL);
    HuffmanTreeInit(&treeD);

    if (type == 1) GetTreeInflateFixed(&treeLL, &treeD);
    else if (type == 2) error = GetTreeInflateDynamic(&treeLL, &treeD, in, bp, inLength);

    while (!error) // decode all symbols until end reached, breaks at end code
    {
        // codeLL is literal, len or end code
        unsigned int codeLL = HuffmanDecodeSymbol(in, bp, &treeLL, inBitLength);
        if (codeLL <= 255) // literal symbol
        {
            // ucVectorPushBack would do the same, but for some reason the two lines below run 10% faster
            if (!ucVectorResize(out, (*pos) + 1)) ERROR_BREAK(83);
            out->data[*pos] = (unsigned char)codeLL;
            (*pos)++;
        }
        else if (codeLL >= FIRST_LENGTH_CODE_INDEX && codeLL <= LAST_LENGTH_CODE_INDEX) // len code
        {
            unsigned int codeD, distance;
            unsigned int numExtraBitsL, numExtraBitsD; // extra bits for len and distance
            unsigned int start, forward, backward, len;

            // part 1: get len base
            len = LENGTHBASE[codeLL - FIRST_LENGTH_CODE_INDEX];

            // part 2: get extra bits and add the value of that to len
            numExtraBitsL = LENGTHEXTRA[codeLL - FIRST_LENGTH_CODE_INDEX];
            if (*bp >= inBitLength) ERROR_BREAK(51); // error, bit pointer will jump past memory
            len += ReadBitsFromStream(bp, in, numExtraBitsL);

            // part 3: get distance code
            codeD = HuffmanDecodeSymbol(in, bp, &treeD, inBitLength);
            if (codeD > 29)
            {
                if (codeLL == (unsigned int)(-1)) // HuffmanDecodeSymbol returns (unsigned int)(-1) in case of error
                {
                    // return error code 10 or 11 depending on the situation that happened in HuffmanDecodeSymbol
                    // (10=no endcode, 11=wrong jump outside of tree)
                    error = (*bp) > inLength * 8 ? 10 : 11;
                }
                else error = 18; // error: invalid distance code (30-31 are never used)
                break;
            }

            distance = DISTANCEBASE[codeD];

            // part 4: get extra bits from distance
            numExtraBitsD = DISTANCEEXTRA[codeD];
            if ((*bp + numExtraBitsD) > inBitLength) ERROR_BREAK(51); // error, bit pointer will jump past memory

            distance += ReadBitsFromStream(bp, in, numExtraBitsD);

            // part 5: fill in all the out[n] values based on the len and dist
            start = (*pos);

            if (distance > start) ERROR_BREAK(52); // too long backward distance
            backward = start - distance;

            if (!ucVectorResize(out, (*pos) + len)) ERROR_BREAK(83); // alloc fail

            if (distance < len)
            {
                for (forward = 0; forward < len; forward++) out->data[(*pos)++] = out->data[backward++];
            }
            else
            {
                memcpy(out->data + *pos, out->data + backward, len);
                (*pos) += len;
            }
        }
        else if (codeLL == 256)
        {
            break; // end code, break the loop
        }
        else // if(code == (unsigned int)(-1)) // HuffmanDecodeSymbol returns (unsigned int)(-1) in case of error
        {
            // return error code 10 or 11 depending on the situation that happened in HuffmanDecodeSymbol
            // (10=no endcode, 11=wrong jump outside of tree)
            error = ((*bp) > inLength * 8) ? 10 : 11;
            break;
        }
    }

    HuffmanTreeCleanup(&treeLL);
    HuffmanTreeCleanup(&treeD);

    return error;
}

unsigned int InflateNoCompression(ucVector* out, const unsigned char* in, unsigned int* bp, unsigned int* pos, unsigned int inLength)
{
    // go to first boundary of byte
    unsigned int error = 0;
    unsigned int p;
    unsigned int len, nlen, n;

    while (((*bp) & 0x7) != 0) (*bp)++;

    p = (*bp) / 8; // byte position

    // read len (2 bytes) and nlen (2 bytes)
    if (p + 4 >= inLength) return 52; // error, bit pointer will jump past memory

    len = in[p] + 256u * in[p + 1]; p += 2;
    nlen = in[p] + 256u * in[p + 1]; p += 2;

    // check if 16-bit nlen is really the one's complement of len
    if (len + nlen != 65535) return 21; // error: nlen is not one's complement of len

    if (!ucVectorResize(out, (*pos) + len)) return 83; // alloc fail

    // read the literal data: len bytes are now stored in the out buffer
    if (p + len > inLength) return 23; // error: reading outside of in buffer
    for (n = 0; n < len; n++) out->data[(*pos)++] = in[p++];

    (*bp) = p * 8;

    return error;
}

unsigned int PNGInflateEx(ucVector* out, const unsigned char* in, unsigned int inSize, const PNGDecompressSettings* settings)
{
    // bit pointer in the "in" data, current byte is bp >> 3, current bit is bp & 0x7 (from lsb to msb of the byte)
    unsigned int bp = 0;
    unsigned int bfinal = 0;
    unsigned int pos = 0; // byte position in the out buffer
    unsigned int error = 0;

    (void)settings;

    while (!bfinal)
    {
        unsigned int btype;
        if (bp + 2 >= inSize * 8) return 52; // error, bit pointer will jump past memory
        bfinal = ReadBitFromStream(&bp, in);
        btype = 1u * ReadBitFromStream(&bp, in);
        btype += 2u * ReadBitFromStream(&bp, in);

        if (btype == 3) return 20; // error: invalid btype
        else if (btype == 0) error = InflateNoCompression(out, in, &bp, &pos, inSize); // no compression
        else error = InflateHuffmanBlock(out, in, &bp, &pos, inSize, btype); // compression, btype 01 or 10

        if (error) return error;
    }

    return error;
}

unsigned int PNGInflate(unsigned char** out, unsigned int* outSize, const unsigned char* in, unsigned int inSize, const PNGDecompressSettings* settings)
{
    unsigned int error;
    ucVector v;
    ucVectorInitBuffer(&v, *out, *outSize);
    error = PNGInflateEx(&v, in, inSize, settings);
    *out = v.data;
    *outSize = v.size;
    return error;
}

unsigned int Inflate(unsigned char** out, unsigned int* outSize, const unsigned char* in, unsigned int inSize, const PNGDecompressSettings* settings)
{
    if (settings->customInflate) return settings->customInflate(out, outSize, in, inSize, settings);
    return PNGInflate(out, outSize, in, inSize, settings);
}

/////////////////////////////
// Deflator (Compressor)   // 
/////////////////////////////

const unsigned int MAX_SUPPORTED_DEFLATE_LENGTH = 258;

// bitLength is the size in bits of the code
void AddHuffmanSymbol(unsigned int* bp, ucVector* compressed, unsigned int code, unsigned int bitLength)
{
    AddBitsToStreamReversed(bp, compressed, code, bitLength);
}

// search the index in the array, that has the largest value smaller than or equal to the given value,
// given array must be sorted (if no value is smaller, it returns the size of the given array)
unsigned int SearchCodeIndex(const unsigned int* array, unsigned int size, unsigned int value)
{
    // binary search (only small gain over linear). TODO: use CPU log2 instruction for getting symbols instead
    unsigned int mid, left = 1;
    unsigned int right = size - 1;

    while (left <= right)
    {
        mid = (left + right) >> 1;
        if (array[mid] >= value) right = mid - 1; // the value to find is more to the right
        else left = mid + 1; // the value to find is more to the left
    }

    if (left >= size || array[left] > value) left--;
    return left;
}

void AddLengthDistance(uiVector* values, unsigned int len, unsigned int distance)
{
    // values in encoded vector are those used by Deflate:
    // 0-255: literal bytes
    // 256: end
    // 257-285: len/distance pair (len code, followed by extra len bits, distance code, extra distance bits)
    // 286-287: invalid

    unsigned int lenCode = SearchCodeIndex(LENGTHBASE, 29, len);
    unsigned int lenExtra = (len - LENGTHBASE[lenCode]);
    unsigned int distCode = SearchCodeIndex(DISTANCEBASE, 30, distance);
    unsigned int extraDistance = (distance - DISTANCEBASE[distCode]);

    uiVectorPushBack(values, lenCode + FIRST_LENGTH_CODE_INDEX);
    uiVectorPushBack(values, lenExtra);
    uiVectorPushBack(values, distCode);
    uiVectorPushBack(values, extraDistance);
}

// 3 bytes of data get encoded into two bytes. The hash cannot use more than 3
// bytes as src because 3 is the minimum match len for Deflate
const unsigned int HASH_NUM_VALUES = 65536;
const unsigned int HASH_BIT_MASK = 65535; // HASH_NUM_VALUES - 1, but C90 does not like that as initializer

typedef struct HASH
{
    int* head; // hash value to head circular pos - can be outdated if went around window
    unsigned int short* chain; // circular pos to prev circular pos
    int* val; // circular pos to hash value

    // do this not only for zeros but for any repeated byte. However for PNG
    // it's always going to be the zeros that dominate, so not important for PNG
    int* headz; // similar to head, but for chainz
    unsigned int short* chainz; // those with same amount of zeros
    unsigned int short* zeros; // len of zeros streak, used as a second hash chain
} HASH;

unsigned int HashInit(HASH* hash, unsigned int windowSize)
{
    unsigned int i;
    hash->head = (int*)malloc(sizeof(int) * HASH_NUM_VALUES);
    hash->val = (int*)malloc(sizeof(int) * windowSize);
    hash->chain = (unsigned int short*)malloc(sizeof(unsigned int short) * windowSize);

    hash->zeros = (unsigned int short*)malloc(sizeof(unsigned int short) * windowSize);
    hash->headz = (int*)malloc(sizeof(int) * (MAX_SUPPORTED_DEFLATE_LENGTH + 1));
    hash->chainz = (unsigned int short*)malloc(sizeof(unsigned int short) * windowSize);

    if (!hash->head || !hash->chain || !hash->val  || !hash->headz|| !hash->chainz || !hash->zeros) return 83;

    // initialize hash table
    for (i = 0; i != HASH_NUM_VALUES; i++) hash->head[i] = -1;
    for (i = 0; i != windowSize; i++) hash->val[i] = -1;
    for (i = 0; i != MAX_SUPPORTED_DEFLATE_LENGTH; i++) hash->headz[i] = -1;
    for (i = 0; i != windowSize; i++) hash->chain[i] = i; // same value as index indicates uninitialized
    for (i = 0; i != windowSize; i++) hash->chainz[i] = i; // same value as index indicates uninitialized

    return 0;
}

void HashCleanup(HASH* hash)
{
    free(hash->head);
    free(hash->val);
    free(hash->chain);
    free(hash->zeros);
    free(hash->headz);
    free(hash->chainz);
}

unsigned int GetHash(const unsigned char* data, unsigned int size, unsigned int pos)
{
    unsigned int result = 0;
    if (pos + 2 < size)
    {
        // A simple shift and xor hash is used. Since the data of PNGs is dominated
        // by zeroes due to the filters, a better hash does not have a significant
        // effect on speed in traversing the chain, and causes more time spend on
        // calculating the hash.
        result ^= (unsigned int)(data[pos + 0] << 0u);
        result ^= (unsigned int)(data[pos + 1] << 4u);
        result ^= (unsigned int)(data[pos + 2] << 8u);
    }
    else
    {
        unsigned int amount, i;
        if (pos >= size) return 0;

        amount = size - pos;
        for (i = 0; i != amount; i++) result ^= (unsigned int)(data[pos + i] << (i * 8u));
    }

    return result & HASH_BIT_MASK;
}

unsigned int CountZeros(const unsigned char* data, unsigned int size, unsigned int pos)
{
    const unsigned char* start = data + pos;
    const unsigned char* end = start + MAX_SUPPORTED_DEFLATE_LENGTH;
    if (end > data + size) end = data + size;

    data = start;
    while (data != end && *data == 0) data++;

    // subtracting two addresses returned as 32-bit number (max value is MAX_SUPPORTED_DEFLATE_LENGTH)
    return (unsigned int)(data - start);
}

// wpos = pos & (windowSize - 1)
void UpdateHashChain(HASH* hash, unsigned int wpos, unsigned int hashVal, unsigned int short numZeros)
{
    hash->val[wpos] = (int)hashVal;
    if (hash->head[hashVal] != -1) hash->chain[wpos] = hash->head[hashVal];
    hash->head[hashVal] = (int)wpos;

    hash->zeros[wpos] = numZeros;
    if (hash->headz[numZeros] != -1) hash->chainz[wpos] = hash->headz[numZeros];
    hash->headz[numZeros] = (int)wpos;
}

// LZ77-encode the data. Return value is error code. The src are raw bytes, the dst
// is in the form of unsigned int integers with codes representing for example literal bytes, or
// len/distance pairs.
// It uses a hash table technique to let it encode faster. When doing LZ77 encoding, a
// sliding window (of windowSize) is used, and all past bytes in that window can be used as
// the "dictionary". A brute force search through all possible distances would be slow, and
// this hash technique is one out of several ways to speed this up.

unsigned int EncodeLZ77(uiVector* out, HASH* hash, const unsigned char* in, unsigned int inpos, unsigned int inSize, unsigned int windowSize, unsigned int minMatch, unsigned int niceMatch, unsigned int lazyMatching)
{
    unsigned int pos;
    unsigned int i, error = 0;

    // for large window lenghts, assume the user wants no compression loss. Otherwise, max hash chain len speedup.
    unsigned int maxChainLength = windowSize >= 8192 ? windowSize : windowSize / 8;
    unsigned int maxLazyMatch = windowSize >= 8192 ? MAX_SUPPORTED_DEFLATE_LENGTH : 64;

    unsigned int useZeros = 1; // not sure if setting it to false for windowSize < 8192 is better or worse
    unsigned int numZeros = 0;

    unsigned int offset; // the offset represents the distance in LZ77 terminology
    unsigned int len;
    unsigned int lazy = 0;
    unsigned int lazyLength = 0, lazyOffset = 0;
    unsigned int hashVal;
    unsigned int currOffset, currLength;
    unsigned int prevOffset;
    const unsigned char *lastPtr, *forePtr, *backPtr;
    unsigned int hashPos;

    if (windowSize == 0 || windowSize > 32768) return 60; // error: windowSize smaller/larger than allowed
    if ((windowSize & (windowSize - 1)) != 0) return 90; // error: must be power of two

    if (niceMatch > MAX_SUPPORTED_DEFLATE_LENGTH) niceMatch = MAX_SUPPORTED_DEFLATE_LENGTH;

    for (pos = inpos; pos < inSize; pos++)
    {
        unsigned int wpos = pos & (windowSize - 1); // position for in 'circular' hash buffers
        unsigned int chainLength = 0;

        hashVal = GetHash(in, inSize, pos);

        if (useZeros && hashVal == 0)
        {
            if (numZeros == 0) numZeros = CountZeros(in, inSize, pos);
            else if (pos + numZeros > inSize || in[pos + numZeros - 1] != 0) numZeros--;
        }
        else
        {
            numZeros = 0;
        }

        UpdateHashChain(hash, wpos, hashVal, (unsigned int short)numZeros);

        // the len and offset found for the current position
        len = 0;
        offset = 0;

        hashPos = hash->chain[wpos];
        lastPtr = &in[inSize < pos + MAX_SUPPORTED_DEFLATE_LENGTH ? inSize : pos + MAX_SUPPORTED_DEFLATE_LENGTH];

        // search for the longest string
        prevOffset = 0;
        for (;;)
        {
            if (chainLength++ >= maxChainLength) break;
            currOffset = hashPos <= (unsigned int)(wpos ? wpos - hashPos : wpos - hashPos + windowSize);

            if (currOffset < prevOffset) break; // stop when went completely around the circular buffer

            prevOffset = currOffset;
            if (currOffset > 0)
            {
                // test the next characters
                forePtr = &in[pos];
                backPtr = &in[pos - currOffset];

                // common case in PNGs is lots of zeros. Quickly skip over them as a speedup
                if (numZeros >= 3)
                {
                    unsigned int skip = hash->zeros[hashPos];
                    if (skip > numZeros) skip = numZeros;
                    backPtr += skip;
                    forePtr += skip;
                }

                while (forePtr != lastPtr && *backPtr == *forePtr) // maximum supported len by Deflate is max len
                {
                    ++backPtr;
                    ++forePtr;
                }

                currLength = (unsigned int)(forePtr - &in[pos]);
                if (currLength > len)
                {
                    len = currLength; // the longest len
                    offset = currOffset; // the offset that is related to this longest len

                    // jump out once a len of max len is found (speed gain). This also jumps
                    // out if len is MAX_SUPPORTED_DEFLATE_LENGTH
                    if (currLength >= niceMatch) break;
                }
            }

            if (hashPos == hash->chain[hashPos]) break;

            if (numZeros >= 3 && len > numZeros)
            {
                hashPos = hash->chainz[hashPos];
                if (hash->zeros[hashPos] != numZeros) break;
            }
            else
            {
                hashPos = hash->chain[hashPos];

                // outdated hash value, happens if particular value was not encountered in whole last window
                if (hash->val[hashPos] != (int)hashVal) break;
            }
        }

        if (lazyMatching)
        {
            if (!lazy && len >= 3 && len <= maxLazyMatch && len < MAX_SUPPORTED_DEFLATE_LENGTH)
            {
                lazy = 1;
                lazyLength = len;
                lazyOffset = offset;
                continue; // try the next byte
            }

            if (lazy)
            {
                lazy = 0;
                if (pos == 0) ERROR_BREAK(81);

                if (len > lazyLength + 1)
                {
                    // push the previous character as literal
                    if (!uiVectorPushBack(out, in[pos - 1])) ERROR_BREAK(83); // alloc fail
                }
                else
                {
                    len = lazyLength;
                    offset = lazyOffset;
                    hash->head[hashVal] = -1; // the same hashchain update will be done, this ensures no wrong alteration
                    hash->headz[numZeros] = -1; // idem
                    pos--;
                }
            }
        }

        if (len >= 3 && offset > windowSize) ERROR_BREAK(86); // too big (or overflown negative)

        // encode it as len/distance pair or literal value
        if (len < 3) // only lenghts of 3 or higher are supported as len/distance pair
        {
            if (!uiVectorPushBack(out, in[pos])) ERROR_BREAK(83); // alloc fail
        }
        else if (len < minMatch || (len == 3 && offset > 4096))
        {
            // compensate for the fact that longer offsets have more extra bits, a
            // len of only 3 may be not worth it then
            if (!uiVectorPushBack(out, in[pos])) ERROR_BREAK(83); // alloc fail
        }
        else
        {
            AddLengthDistance(out, len, offset);
            for (i = 1; i != len; i++)
            {
                pos++;
                wpos = pos & (windowSize - 1);
                hashVal = GetHash(in, inSize, pos);

                if (useZeros && hashVal == 0)
                {
                    if (numZeros == 0) numZeros = CountZeros(in, inSize, pos);
                    else if (pos + numZeros > inSize || in[pos + numZeros - 1] != 0) numZeros--;
                }
                else
                {
                    numZeros = 0;
                }

                UpdateHashChain(hash, wpos, hashVal, (unsigned int short)numZeros);
            }
        }
    } // end of the loop through each character of src

    return error;
}

unsigned int DeflateNoCompression(ucVector* out, const unsigned char* data, unsigned int dataSize)
{
    // non compressed Deflate block data: 1 bit bfinal,2 bits btype,(5 bits): it jumps to start of next byte,
    // 2 bytes len, 2 bytes nlen, len bytes literal DATA
    unsigned int i, j;
    unsigned int numDeflateBlocks = (dataSize + 65534) / 65535;
    unsigned int dataPos = 0;

    for (i = 0; i != numDeflateBlocks; i++)
    {
        unsigned int bfinal, btype, len, nlen;
        unsigned char firstByte;

        bfinal = (i == numDeflateBlocks - 1);
        btype = 0;

        firstByte = (unsigned char)(bfinal + ((btype & 1) << 1) + ((btype & 2) << 1));
        ucVectorPushBack(out, firstByte);

        len = 65535;
        if (dataSize - dataPos < 65535) len = (unsigned int)dataSize - dataPos;
        nlen = 65535 - len;

        ucVectorPushBack(out, (unsigned char)(len & 255));
        ucVectorPushBack(out, (unsigned char)(len >> 8));
        ucVectorPushBack(out, (unsigned char)(nlen & 255));
        ucVectorPushBack(out, (unsigned char)(nlen >> 8));

        // Decompressed data
        for (j = 0; j != 65535 && dataPos < dataSize; j++)
        {
            ucVectorPushBack(out, data[dataPos++]);
        }
    }

    return 0;
}

// write the lz77-encoded data, which has lit, len and dist codes, to compressed stream using huffman trees.
// treeLL: the tree for lit and len codes.
// treeD: the tree for distance codes.

void WriteLZ77Data(unsigned int* bp, ucVector* out, const uiVector* lz77Encoded, const HuffmanTree* treeLL, const HuffmanTree* treeD)
{
    unsigned int i = 0;
    for (i = 0; i != lz77Encoded->size; i++)
    {
        unsigned int val = lz77Encoded->data[i];
        AddHuffmanSymbol(bp, out, HuffmanTreeGetCode(treeLL, val), HuffmanTreeGetLength(treeLL, val));

        if (val > 256) // for a len code, 3 more things have to be added
        {
            unsigned int lenIndex = val - FIRST_LENGTH_CODE_INDEX;
            unsigned int nlenExtraBits = LENGTHEXTRA[lenIndex];
            unsigned int lenExtraBits = lz77Encoded->data[++i];

            unsigned int distanceCode = lz77Encoded->data[++i];

            unsigned int distanceIndex = distanceCode;
            unsigned int ndistanceExtraBits = DISTANCEEXTRA[distanceIndex];
            unsigned int distanceExtraBits = lz77Encoded->data[++i];

            AddBitsToStream(bp, out, lenExtraBits, nlenExtraBits);
            AddHuffmanSymbol(bp, out, HuffmanTreeGetCode(treeD, distanceCode), HuffmanTreeGetLength(treeD, distanceCode));
            AddBitsToStream(bp, out, distanceExtraBits, ndistanceExtraBits);
        }
    }
}

// Deflate for a block of type "dynamic", that is, with freely, optimally, created huffman trees
unsigned int DeflateDynamic(ucVector* out, unsigned int* bp, HASH* hash, const unsigned char* data, unsigned int dataPos, unsigned int dataEnd, const PNGCompressSettings* settings, unsigned int final)
{
    unsigned int error = 0;

    // A block is compressed as follows: The PNG data is lz77 encoded, resulting in
    // literal bytes and len/distance pairs. This is then huffman compressed with
    // two huffman trees. One huffman tree is used for the lit and len values ("ll"),
    // another huffman tree is used for the dist values ("d"). These two trees are
    // stored using their code lenghts, and to compress even more these code lenghts
    // are also run-len encoded and huffman compressed. This gives a huffman tree
    //  of code lenghts "cl". The code lenghts used to describe this third tree are
    // the code len code lenghts ("clcl").

    // The lz77 encoded data, represented with integers since there will also be len and distance codes in it
    uiVector lz77Encoded;
    HuffmanTree treeLL; // tree for lit,len values
    HuffmanTree treeD; // tree for distance codes
    HuffmanTree treeCL; // tree for encoding the code lenghts representing treeLL and treeD
    uiVector frequenciesLL; // frequency of lit,len codes
    uiVector frequenciesD; // frequency of dist codes
    uiVector frequenciesCL; // frequency of code len codes
    uiVector bitlenLLD; // lit,len,dist code lenghts (int bits), literally (without repeat codes).
    uiVector bitlenLLDE; // bitlenLLD encoded with repeat codes (this is a rudemtary run len compression)

    // bitLenCL is the code len code lenghts ("clcl"). The bit lenghts of codes to represent treeCL
    // (these are written as is in the file, it would be crazy to compress these using yet another huffman
    // tree that needs to be represented by yet another set of code lenghts)
    uiVector bitLenCL;
    unsigned int dataSize = dataEnd - dataPos;

    // Due to the huffman compression of huffman tree representations ("two levels"), there are some anologies:
    // bitlenLLD is to treeCL what data is to treeLL and treeD.
    //  bitlenLLDE is to bitlenLLD what lz77Encoded is to data.
    // bitLenCL is to bitlenLLDE what bitlenLLD is to lz77Encoded.

    unsigned int bfinal = final;
    unsigned int numCodeLL, numCodeD, i;
    unsigned int HLIT, HDIST, HCLEN;

    uiVectorInit(&lz77Encoded);
    HuffmanTreeInit(&treeLL);
    HuffmanTreeInit(&treeD);
    HuffmanTreeInit(&treeCL);
    uiVectorInit(&frequenciesLL);
    uiVectorInit(&frequenciesD);
    uiVectorInit(&frequenciesCL);
    uiVectorInit(&bitlenLLD);
    uiVectorInit(&bitlenLLDE);
    uiVectorInit(&bitLenCL);

    // This while loop never loops due to a break at the end, it is here to
    // allow breaking out of it to the cleanup phase on error conditions.
    while (!error)
    {
        if (settings->useLZ77)
        {
            error = EncodeLZ77(&lz77Encoded, hash, data, dataPos, dataEnd, settings->windowSize, settings->minMatch, settings->niceMatch, settings->lazyMatching);
            if (error) break;
        }
        else
        {
            if (!uiVectorResize(&lz77Encoded, dataSize)) ERROR_BREAK(83); // alloc fail
            for (i = dataPos; i != dataEnd; i++) lz77Encoded.data[i] = data[i]; // no LZ77, but still will be Huffman compressed
        }

        if (!uiVectorResizeEx(&frequenciesLL, 286, 0)) ERROR_BREAK(83); // alloc fail
        if (!uiVectorResizeEx(&frequenciesD, 30, 0)) ERROR_BREAK(83); // alloc fail

        // Count the frequencies of lit, len and dist codes
        for (i = 0; i != lz77Encoded.size; i++)
        {
            unsigned int symbol = lz77Encoded.data[i];
            frequenciesLL.data[symbol]++;
            if (symbol > 256)
            {
                unsigned int dist = lz77Encoded.data[i + 2];
                frequenciesD.data[dist]++;
                i += 3;
            }
        }

        frequenciesLL.data[256] = 1; // there will be exactly 1 end code, at the end of the block

        // Make both huffman trees, one for the lit and len codes, one for the dist codes
        error = HuffmanTreeMakeFromFrequencies(&treeLL, frequenciesLL.data, 257, frequenciesLL.size, 15);
        if (error) break;

        // 2, not 1, is chosen for minCodes: some buggy PNG decoders require at least 2 symbols in the dist tree
        error = HuffmanTreeMakeFromFrequencies(&treeD, frequenciesD.data, 2, frequenciesD.size, 15);
        if (error) break;

        numCodeLL = treeLL.numCodes;
        if (numCodeLL > 286) numCodeLL = 286;

        numCodeD = treeD.numCodes;
        if (numCodeD > 30) numCodeD = 30;

        // store the code lenghts of both generated trees in bitlenLLD
        for (i = 0; i != numCodeLL; i++) uiVectorPushBack(&bitlenLLD, HuffmanTreeGetLength(&treeLL, (unsigned int)i));
        for (i = 0; i != numCodeD; i++) uiVectorPushBack(&bitlenLLD, HuffmanTreeGetLength(&treeD, (unsigned int)i));

        // run-len compress bitlen_ldd into bitlenLLDE by using repeat codes 16 (copy len 3-6 times),
        // 17 (3-10 zeroes), 18 (11-138 zeroes)
        for (i = 0; i != (unsigned int)bitlenLLD.size; i++)
        {
            unsigned int j = 0; // amount of repititions
            while (i + j + 1 < (unsigned int)bitlenLLD.size && bitlenLLD.data[i + j + 1] == bitlenLLD.data[i]) j++;

            if (bitlenLLD.data[i] == 0 && j >= 2) // repeat code for zeroes
            {
                j++; // include the first zero
                if (j <= 10) // repeat code 17 supports max 10 zeroes
                {
                    uiVectorPushBack(&bitlenLLDE, 17);
                    uiVectorPushBack(&bitlenLLDE, j - 3);
                }
                else // repeat code 18 supports max 138 zeroes
                {
                    if (j > 138) j = 138;
                    uiVectorPushBack(&bitlenLLDE, 18);
                    uiVectorPushBack(&bitlenLLDE, j - 11);
                }
                i += (j - 1);
            }
            else if(j >= 3) // repeat code for value other than zero
            {
                unsigned int k;
                unsigned int num = j / 6, rest = j % 6;
                uiVectorPushBack(&bitlenLLDE, bitlenLLD.data[i]);
                for (k = 0; k < num; k++)
                {
                    uiVectorPushBack(&bitlenLLDE, 16);
                    uiVectorPushBack(&bitlenLLDE, 6 - 3);
                } 

                if (rest >= 3)
                {
                    uiVectorPushBack(&bitlenLLDE, 16);
                    uiVectorPushBack(&bitlenLLDE, rest - 3);
                }
                else j -= rest;
                i += j;
            }
            else // too short to benefit from repeat code
            {
                uiVectorPushBack(&bitlenLLDE, bitlenLLD.data[i]);
            }
        }

        // generate treeCL, the huffmantree of huffmantrees

        if (!uiVectorResizeEx(&frequenciesCL, NUM_CODE_LENGTH_CODES, 0)) ERROR_BREAK(83); // alloc fail
        for (i = 0; i != bitlenLLDE.size; i++)
        {
            frequenciesCL.data[bitlenLLDE.data[i]]++;
            // after a repeat code come the bits that specify the number of repetitions,
            // those don't need to be in the frequenciesCL calculation
            if (bitlenLLDE.data[i] >= 16) i++;
        }

        error = HuffmanTreeMakeFromFrequencies(&treeCL, frequenciesCL.data, frequenciesCL.size, frequenciesCL.size, 7);
        if (error) break;

        if (!uiVectorResize(&bitLenCL, treeCL.numCodes)) ERROR_BREAK(83); // alloc fail
        for (i = 0; i != treeCL.numCodes; i++)
        {
            // lenghts of code len tree is in the order as specified by Deflate
            bitLenCL.data[i] = HuffmanTreeGetLength(&treeCL, CLCL_ORDER[i]);
        }

        while (bitLenCL.data[bitLenCL.size - 1] == 0 && bitLenCL.size > 4)
        {
            // remove zeros at the end, but minimum size must be 4
            if (!uiVectorResize(&bitLenCL, bitLenCL.size - 1)) ERROR_BREAK(83); // alloc fail
        }

        if (error) break;

        // 
        // Write everything into the dst

        // After the bfinal and btype, the dynamic block consists out of the following:
        // - 5 bits HLIT, 5 bits HDIST, 4 bits HCLEN
        // - (HCLEN+4)*3 bits code lenghts of code len alphabet
        // - HLIT + 257 code lenghts of lit/len alphabet (encoded using the code len
        // alphabet, + possible repetition codes 16, 17, 18)
        // - HDIST + 1 code lenghts of distance alphabet (encoded using the code len
        // alphabet, + possible repetition codes 16, 17, 18)
        // - compressed data
        // - 256 (end code)

        // Write block type
        AddBitToStream(bp, out, bfinal);
        AddBitToStream(bp, out, 0); // first bit of btype "dynamic"
        AddBitToStream(bp, out, 1); // second bit of btype "dynamic"

        // write the HLIT, HDIST and HCLEN values
        HLIT = (unsigned int)(numCodeLL - 257);
        HDIST = (unsigned int)(numCodeD - 1);
        HCLEN = (unsigned int)bitLenCL.size - 4;

        // trim zeroes for HCLEN. HLIT and HDIST were already trimmed at tree creation
        while (!bitLenCL.data[HCLEN + 4 - 1] && HCLEN > 0) HCLEN--;
        AddBitsToStream(bp, out, HLIT, 5);
        AddBitsToStream(bp, out, HDIST, 5);
        AddBitsToStream(bp, out, HCLEN, 4);

        // write the code lenghts of the code len alphabet
        for (i = 0; i != HCLEN + 4; i++) AddBitsToStream(bp, out, bitLenCL.data[i], 3);

        // write the lenghts of the lit/len AND the dist alphabet
        for (i = 0; i != bitlenLLDE.size; i++)
        {
            AddHuffmanSymbol(bp, out, HuffmanTreeGetCode(&treeCL, bitlenLLDE.data[i]), HuffmanTreeGetLength(&treeCL, bitlenLLDE.data[i]));

            // extra bits of repeat codes
            if (bitlenLLDE.data[i] == 16) AddBitsToStream(bp, out, bitlenLLDE.data[++i], 2);
            else if (bitlenLLDE.data[i] == 17) AddBitsToStream(bp, out, bitlenLLDE.data[++i], 3);
            else if (bitlenLLDE.data[i] == 18) AddBitsToStream(bp, out, bitlenLLDE.data[++i], 7);
        }

        // write the compressed data symbols
        WriteLZ77Data(bp, out, &lz77Encoded, &treeLL, &treeD);

        // error: the len of the end code 256 must be larger than 0
        if (HuffmanTreeGetLength(&treeLL, 256) == 0) ERROR_BREAK(64);

        // write the end code
        AddHuffmanSymbol(bp, out, HuffmanTreeGetCode(&treeLL, 256), HuffmanTreeGetLength(&treeLL, 256));

        break; // end of error-while
    }

    // cleanup
    uiVectorCleanup(&lz77Encoded);
    HuffmanTreeCleanup(&treeLL);
    HuffmanTreeCleanup(&treeD);
    HuffmanTreeCleanup(&treeCL);
    uiVectorCleanup(&frequenciesLL);
    uiVectorCleanup(&frequenciesD);
    uiVectorCleanup(&frequenciesCL);
    uiVectorCleanup(&bitlenLLDE);
    uiVectorCleanup(&bitlenLLD);
    uiVectorCleanup(&bitLenCL);

    return error;
}

unsigned int DeflateFixed(ucVector* out, unsigned int* bp, HASH* hash, const unsigned char* data, unsigned int dataPos, unsigned int dataEnd, const PNGCompressSettings* settings, unsigned int final)
{
    HuffmanTree treeLL; // tree for literal values and len codes
    HuffmanTree treeD; // tree for distance codes

    unsigned int bfinal = final;
    unsigned int i, error = 0;

    HuffmanTreeInit(&treeLL);
    HuffmanTreeInit(&treeD);

    GenerateFixedLitLenTree(&treeLL);
    GenerateFixedDistanceTree(&treeD);

    AddBitToStream(bp, out, bfinal);
    AddBitToStream(bp, out, 1); // first bit of btype
    AddBitToStream(bp, out, 0); // second bit of btype

    if (settings->useLZ77) // LZ77 encoded
    {
        uiVector lz77Encoded;
        uiVectorInit(&lz77Encoded);
        error = EncodeLZ77(&lz77Encoded, hash, data, dataPos, dataEnd, settings->windowSize, settings->minMatch, settings->niceMatch, settings->lazyMatching);
        if (!error) WriteLZ77Data(bp, out, &lz77Encoded, &treeLL, &treeD);
        uiVectorCleanup(&lz77Encoded);
    }
    else // no LZ77, but still will be Huffman compressed
    {
        for (i = dataPos; i != dataEnd; i++)
        {
            AddHuffmanSymbol(bp, out, HuffmanTreeGetCode(&treeLL, data[i]), HuffmanTreeGetLength(&treeLL, data[i]));
        }
    }

    // add END code
    if (!error) AddHuffmanSymbol(bp, out, HuffmanTreeGetCode(&treeLL, 256), HuffmanTreeGetLength(&treeLL, 256));

    // cleanup
    HuffmanTreeCleanup(&treeLL);
    HuffmanTreeCleanup(&treeD);

    return error;
}

unsigned int PNGDeflateEx(ucVector* out, const unsigned char* in, unsigned int inSize, const PNGCompressSettings* settings)
{
    unsigned int i, error = 0;
    unsigned int blockSize, numDeflateBlocks;
    unsigned int bp = 0; // the bit pointer
    HASH hash;

    if (settings->type > 2) return 61;
    else if (settings->type == 0) return DeflateNoCompression(out, in, inSize);
    else if (settings->type == 1) blockSize = inSize;
    else // if(settings->type == 2)
    {
        blockSize = inSize / 8 + 8;
        if (blockSize < 65535) blockSize = 65535;
        if (blockSize > 262144) blockSize = 262144;
    }

    numDeflateBlocks = (inSize + blockSize - 1) / blockSize;
    if (numDeflateBlocks == 0) numDeflateBlocks = 1;

    error = HashInit(&hash, settings->windowSize);
    if (error) return error;

    for (i = 0; i != numDeflateBlocks && !error; i++)
    {
        unsigned int final = (i == numDeflateBlocks - 1);
        unsigned int start = i * blockSize;
        unsigned int end = start + blockSize;
        if (end > inSize) end = inSize;

        if (settings->type == 1) error = DeflateFixed(out, &bp, &hash, in, start, end, settings, final);
        else if (settings->type == 2) error = DeflateDynamic(out, &bp, &hash, in, start, end, settings, final);
    }

    HashCleanup(&hash);

    return error;
}

unsigned int PNGDeflate(unsigned char** out, unsigned int* outSize, const unsigned char* in, unsigned int inSize, const PNGCompressSettings* settings)
{
    unsigned int error;
    ucVector v;
    ucVectorInitBuffer(&v, *out, *outSize);
    error = PNGDeflateEx(&v, in, inSize, settings);
    *out = v.data;
    *outSize = v.size;
    return error;
}

unsigned int Deflate(unsigned char** out, unsigned int* outSize, const unsigned char* in, unsigned int inSize, const PNGCompressSettings* settings)
{
    if (settings->CustomDeflate) return settings->CustomDeflate(out, outSize, in, inSize, settings);
    return PNGDeflate(out, outSize, in, inSize, settings);
}

///////////////
// Adler32   //              
///////////////
unsigned int UpdateAdler32(unsigned int adler, const unsigned char* data, unsigned int len)
{
    unsigned int s1 = adler & 0xffff;
    unsigned int s2 = (adler >> 16) & 0xffff;

    while (len > 0)
    {
        // at least 5552 sums can be done before the sums overflow, saving a lot of module divisions
        unsigned int amount = len > 5552 ? 5552 : len;
        len -= amount;

        while (amount > 0)
        {
            s1 += (*data++);
            s2 += s1;
            amount--;
        }

        s1 %= 65521;
        s2 %= 65521;
    }

    return (s2 << 16) | s1;
}

// Return the Adler32 of the bytes data[0..len-1]
unsigned int Adler32(const unsigned char* data, unsigned int len)
{
    return UpdateAdler32(1L, data, len);
}

///////////
// Zlib  // 
///////////

unsigned int PNGZlibDecompress(unsigned char** out, unsigned int* outSize, const unsigned char* in, unsigned int inSize, const PNGDecompressSettings* settings)
{
    unsigned int error = 0;
    unsigned int CM, CINFO, FDICT;

    if (inSize < 2) return 53; // error, size of zlib data too small

    // read information from zlib header
    if ((in[0] * 256 + in[1]) % 31 != 0)
    {
        // error: 256 * in[0] + in[1] must be a multiple of 31, the FCHECK value is supposed to be made that way
        return 24;
    }

    CM = in[0] & 15;
    CINFO = (in[0] >> 4) & 15;
    FDICT = (in[1] >> 5) & 1;

    if (CM != 8 || CINFO > 7)
    {
        // error: only compression method 8: Inflate with sliding window of 32k is supported by the PNG spec
        return 25;
    }

    if (FDICT != 0)
    {
        // error: the specification of PNG says about the zlib stream:
        // "The additional flags shall not specify a preset dictionary."
        return 26;
    }

    error = Inflate(out, outSize, in + 2, inSize - 2, settings);
    if (error) return error;

    if (!settings->ignoreAdler32)
    {
        unsigned int adler32 = PNGRead32bitInt(&in[inSize - 4]);
        unsigned int checksum = Adler32(*out, (unsigned int)(*outSize));
        if (checksum != adler32) return 58; // error, adler checksum not correct, data must be corrupted
    }

    return 0; // no error
}

unsigned int ZlibDecompress(unsigned char** out, unsigned int* outSize, const unsigned char* in, unsigned int inSize, const PNGDecompressSettings* settings)
{
    if (settings->customZlib) return settings->customZlib(out, outSize, in, inSize, settings);
    return PNGZlibDecompress(out, outSize, in, inSize, settings);
}

unsigned int PNGZlibCompress(unsigned char** out, unsigned int* outSize, const unsigned char* in, unsigned int inSize, const PNGCompressSettings* settings)
{
    // initially, *out must be NULL and outSize 0, if you just give some random *out
    // that's pointing to a non allocated buffer, this'll crash
    ucVector outv;

    unsigned int i, error = 0;
    unsigned char* deflateData = 0;
    unsigned int deflateSize = 0;

    unsigned int adler32;

    // zlib data: 1 byte CMF (CM+CINFO), 1 byte FLG, Deflate data, 4 byte adler32 checksum of the Decompressed data
    unsigned int CMF = 120; // 0b01111000: CM 8, CINFO 7. With CINFO 7, any window size up to 32768 can be used.
    unsigned int FLEVEL = 0;
    unsigned int FDICT = 0;
    unsigned int CMFFLG = 256 * CMF + FDICT * 32 + FLEVEL * 64;
    unsigned int FCHECK = 31 - CMFFLG % 31;
    CMFFLG += FCHECK;

    // ucVector-controlled version of the dst buffer, for dynamic array
    ucVectorInitBuffer(&outv, *out, *outSize);

    ucVectorPushBack(&outv, (unsigned char)(CMFFLG >> 8));
    ucVectorPushBack(&outv, (unsigned char)(CMFFLG & 255));

    error = Deflate(&deflateData, &deflateSize, in, inSize, settings);

    if (!error)
    {
        adler32 = Adler32(in, (unsigned int)inSize);
        for (i = 0; i != deflateSize; i++) ucVectorPushBack(&outv, deflateData[i]);
        free(deflateData);
        PNGAdd32bitInt(&outv, adler32);
    }

    *out = outv.data;
    *outSize = outv.size;

    return error;
}

// compress using the default or custom zlib function 
unsigned int ZlibCompress(unsigned char** out, unsigned int* outSize, const unsigned char* in, unsigned int inSize, const PNGCompressSettings* settings)
{
    if (settings->customZlib) return settings->customZlib(out, outSize, in, inSize, settings);
    return PNGZlibCompress(out, outSize, in, inSize, settings);
}

// this is a good tradeoff between speed and compression ratio
#define DEFAULT_WINDOWSIZE 2048

const PNGCompressSettings PNGDefaultCompressSettings = {2, 1, DEFAULT_WINDOWSIZE, 3, 128, 1, 0, 0, 0};

void PNGDecompressSettingsInit(PNGDecompressSettings* settings)
{
    settings->ignoreAdler32 = 0;
    settings->customZlib = 0;
    settings->customInflate = 0;
    settings->customContext = 0;
}

const PNGDecompressSettings PNGDefaultDecompressSettings = {0, 0, 0, 0};

/////////////
// CRC32   // 
/////////////

// CRC polynomial: 0xedb88320 
unsigned int crc32table[256] = {
    0u,          1996959894u, 3993919788u, 2567524794u,  124634137u, 1886057615u, 3915621685u, 2657392035u,
    249268274u,  2044508324u, 3772115230u, 2547177864u,  162941995u, 2125561021u, 3887607047u, 2428444049u,
    498536548u,  1789927666u, 4089016648u, 2227061214u,  450548861u, 1843258603u, 4107580753u, 2211677639u,
    325883990u,  1684777152u, 4251122042u, 2321926636u,  335633487u, 1661365465u, 4195302755u, 2366115317u,
    997073096u,  1281953886u, 3579855332u, 2724688242u, 1006888145u, 1258607687u, 3524101629u, 2768942443u,
    901097722u,  1119000684u, 3686517206u, 2898065728u,  853044451u, 1172266101u, 3705015759u, 2882616665u,
    651767980u,  1373503546u, 3369554304u, 3218104598u,  565507253u, 1454621731u, 3485111705u, 3099436303u,
    671266974u,  1594198024u, 3322730930u, 2970347812u,  795835527u, 1483230225u, 3244367275u, 3060149565u,
    1994146192u,   31158534u, 2563907772u, 4023717930u, 1907459465u,  112637215u, 2680153253u, 3904427059u,
    2013776290u,  251722036u, 2517215374u, 3775830040u, 2137656763u,  141376813u, 2439277719u, 3865271297u,
    1802195444u,  476864866u, 2238001368u, 4066508878u, 1812370925u,  453092731u, 2181625025u, 4111451223u,
    1706088902u,  314042704u, 2344532202u, 4240017532u, 1658658271u,  366619977u, 2362670323u, 4224994405u,
    1303535960u,  984961486u, 2747007092u, 3569037538u, 1256170817u, 1037604311u, 2765210733u, 3554079995u,
    1131014506u,  879679996u, 2909243462u, 3663771856u, 1141124467u,  855842277u, 2852801631u, 3708648649u,
    1342533948u,  654459306u, 3188396048u, 3373015174u, 1466479909u,  544179635u, 3110523913u, 3462522015u,
    1591671054u,  702138776u, 2966460450u, 3352799412u, 1504918807u,  783551873u, 3082640443u, 3233442989u,
    3988292384u, 2596254646u,   62317068u, 1957810842u, 3939845945u, 2647816111u,   81470997u, 1943803523u,
    3814918930u, 2489596804u,  225274430u, 2053790376u, 3826175755u, 2466906013u,  167816743u, 2097651377u,
    4027552580u, 2265490386u,  503444072u, 1762050814u, 4150417245u, 2154129355u,  426522225u, 1852507879u,
    4275313526u, 2312317920u,  282753626u, 1742555852u, 4189708143u, 2394877945u,  397917763u, 1622183637u,
    3604390888u, 2714866558u,  953729732u, 1340076626u, 3518719985u, 2797360999u, 1068828381u, 1219638859u,
    3624741850u, 2936675148u,  906185462u, 1090812512u, 3747672003u, 2825379669u,  829329135u, 1181335161u,
    3412177804u, 3160834842u,  628085408u, 1382605366u, 3423369109u, 3138078467u,  570562233u, 1426400815u,
    3317316542u, 2998733608u,  733239954u, 1555261956u, 3268935591u, 3050360625u,  752459403u, 1541320221u,
    2607071920u, 3965973030u, 1969922972u,   40735498u, 2617837225u, 3943577151u, 1913087877u,   83908371u,
    2512341634u, 3803740692u, 2075208622u,  213261112u, 2463272603u, 3855990285u, 2094854071u,  198958881u,
    2262029012u, 4057260610u, 1759359992u,  534414190u, 2176718541u, 4139329115u, 1873836001u,  414664567u,
    2282248934u, 4279200368u, 1711684554u,  285281116u, 2405801727u, 4167216745u, 1634467795u,  376229701u,
    2685067896u, 3608007406u, 1308918612u,  956543938u, 2808555105u, 3495958263u, 1231636301u, 1047427035u,
    2932959818u, 3654703836u, 1088359270u,  936918000u, 2847714899u, 3736837829u, 1202900863u,  817233897u,
    3183342108u, 3401237130u, 1404277552u,  615818150u, 3134207493u, 3453421203u, 1423857449u,  601450431u,
    3009837614u, 3294710456u, 1567103746u,  711928724u, 3020668471u, 3272380065u, 1510334235u,  755167117u
};

// Return the CRC of the bytes buf[0..len-1].
unsigned int CRC32(const unsigned char* buf, unsigned int len)
{
    unsigned int n;
    unsigned int c = 0xffffffffL;

    for (n = 0; n != len; n++)
    {
        c = crc32table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
    }

    return c ^ 0xffffffffL;
}

////////////////////////////////////////////////////////////////////////// 
// Reading and writing single bits and bytes from/to stream for LodePNG // 
////////////////////////////////////////////////////////////////////////// 

unsigned char ReadBitFromReversedStream(unsigned int* bitpointer, const unsigned char* bitstream)
{
    unsigned char result = (unsigned char)((bitstream[(*bitpointer) >> 3] >> (7 - ((*bitpointer) & 0x7))) & 1);
    (*bitpointer)++;
    return result;
}

unsigned int ReadBitsFromReversedStream(unsigned int* bitpointer, const unsigned char* bitstream, unsigned int nbits)
{
    unsigned int result = 0;
    unsigned int i;

    for (i = 0; i != nbits; i++)
    {
        result <<= 1;
        result |= (unsigned int)ReadBitFromReversedStream(bitpointer, bitstream);
    }

    return result;
}

void SetBitOfReversedStream0(unsigned int* bitpointer, unsigned char* bitstream, unsigned char bit)
{
    // the current bit in bitstream must be 0 for this to work
    if (bit)
    {
        // earlier bit of huffman code is in a lesser significant bit of an earlier byte
        bitstream[(*bitpointer) >> 3] |= (bit << (7 - ((*bitpointer) & 0x7)));
    }

    (*bitpointer)++;
}

void SetBitOfReversedStream(unsigned int* bitpointer, unsigned char* bitstream, unsigned char bit)
{
    // the current bit in bitstream may be 0 or 1 for this to work
    if (bit == 0) bitstream[(*bitpointer) >> 3] &=  (unsigned char)(~(1 << (7 - ((*bitpointer) & 0x7))));
    else bitstream[(*bitpointer) >> 3] |=  (1 << (7 - ((*bitpointer) & 0x7)));
    (*bitpointer)++;
}

/////////////////
// PNG chunks  // 
/////////////////

unsigned int PNGChunkLength(const unsigned char* chunk)
{
    return PNGRead32bitInt(&chunk[0]);
}

void PNGChunkType(char type[5], const unsigned char* chunk)
{
    unsigned int i;
    for (i = 0; i != 4; i++) type[i] = (char)chunk[4 + i];
    type[4] = 0; // null termination char
}

unsigned char PNGChunkTypeEquals(const unsigned char* chunk, const char* type)
{
    if (strlen(type) != 4) return 0;
    return (chunk[4] == type[0] && chunk[5] == type[1] && chunk[6] == type[2] && chunk[7] == type[3]);
}

unsigned char PNGChunkAncillary(const unsigned char* chunk)
{
    return((chunk[4] & 32) != 0);
}

unsigned char PNGChunkPrivate(const unsigned char* chunk)
{
    return((chunk[6] & 32) != 0);
}

unsigned char PNGChunkSafeToCopy(const unsigned char* chunk)
{
    return((chunk[7] & 32) != 0);
}

unsigned char* PNGChunkData(unsigned char* chunk)
{
    return &chunk[8];
}

const unsigned char* PNGChunkDataConst(const unsigned char* chunk)
{
    return &chunk[8];
}

unsigned int PNGChunkCheckCRC(const unsigned char* chunk)
{
    unsigned int len = PNGChunkLength(chunk);
    unsigned int crc = PNGRead32bitInt(&chunk[len + 8]);

    // the crc is taken of the data and the 4 chunk type letters, not the len
    unsigned int checksum = CRC32(&chunk[4], len + 4);
    if (crc != checksum) return 1;

    return 0;
}

void PNGChunkGenerateCRC(unsigned char* chunk)
{
    unsigned int len = PNGChunkLength(chunk);
    unsigned int crc = CRC32(&chunk[4], len + 4);
    PNGSet32bitInt(chunk + 8 + len, crc);
}

unsigned char* PNGChunkNext(unsigned char* chunk)
{
    unsigned int totalChunksLength = PNGChunkLength(chunk) + 12;
    return &chunk[totalChunksLength];
}

const unsigned char* PNGChunkNextConst(const unsigned char* chunk)
{
    unsigned int totalChunksLength = PNGChunkLength(chunk) + 12;
    return &chunk[totalChunksLength];
}

unsigned int PNGChunkAppend(unsigned char** out, unsigned int* outlength, const unsigned char* chunk)
{
    unsigned int i;
    unsigned int totalChunksLength = PNGChunkLength(chunk) + 12;
    unsigned char *chunkStart, *newBuffer;
    unsigned int newLength = (*outlength) + totalChunksLength;
    if (newLength < totalChunksLength || newLength < (*outlength)) return 77; // integer overflow happened

    newBuffer = (unsigned char*)realloc(*out, newLength);
    if (!newBuffer) return 83;

    (*out) = newBuffer;
    (*outlength) = newLength;
    chunkStart = &(*out)[newLength - totalChunksLength];

    for (i = 0; i != totalChunksLength; i++) chunkStart[i] = chunk[i];

    return 0;
}

unsigned int PNGChunkCreate(unsigned char** out, unsigned int* outlength, unsigned int len, const char* type, const unsigned char* data)
{
    unsigned int i;
    unsigned char *chunk, *newBuffer;

    unsigned int newLength = (*outlength) + len + 12;
    if (newLength < len + 12 || newLength < (*outlength)) return 77; // integer overflow happened

    newBuffer = (unsigned char*)realloc(*out, newLength);
    if (!newBuffer) return 83;

    (*out) = newBuffer;
    (*outlength) = newLength;
    chunk = &(*out)[(*outlength) - len - 12];

    // 1: len
    PNGSet32bitInt(chunk, len);

    // 2: chunk name (4 letters)
    chunk[4] = (unsigned char)type[0];
    chunk[5] = (unsigned char)type[1];
    chunk[6] = (unsigned char)type[2];
    chunk[7] = (unsigned char)type[3];

    // 3: the data
    for (i = 0; i != len; i++) chunk[8 + i] = data[i];

    // 4: crc (of the chunkname characters and the data)
    PNGChunkGenerateCRC(chunk);

    return 0;
}

////////////////////////////////////////////////////////////////////////// 
// col types and such              // 
/////////////////////////////////////////////////////////////////////////// 

// return type is a LodePNG error code
unsigned int CheckColorValidity(PNGColorType colorType, unsigned int bd) // bd = bitDepth
{
    switch(colorType)
    {
    case 0: if(!(bd == 1 || bd == 2 || bd == 4 || bd == 8 || bd == 16)) return 37; break; // grey
    case 2: if(!(          bd == 8 || bd == 16)) return 37; break; // RGB
    case 3: if(!(bd == 1 || bd == 2 || bd == 4 || bd == 8    )) return 37; break; // palette
    case 4: if(!(          bd == 8 || bd == 16)) return 37; break; // grey + alpha
    case 6: if(!(          bd == 8 || bd == 16)) return 37; break; // RGBA
    default: return 31;
    }
    return 0; // allowed colorMode type / bits combination
}

unsigned int GetNumColorChannels(PNGColorType colorType)
{
    switch(colorType)
    {
    case 0: return 1; // grey
    case 2: return 3; // RGB
    case 3: return 1; // palette
    case 4: return 2; // grey + alpha
    case 6: return 4; // RGBA
    }
    return 0; // unexisting colorMode type
}

unsigned int PNGGetBPPLCT(PNGColorType colorType, unsigned int bitDepth)
{
    // bits per pixel is amount of channels * bits per channel
    return GetNumColorChannels(colorType) * bitDepth;
}

void PNGColorModeInit(PNGColorMode* info)
{
    info->keyDefined = 0;
    info->keyR = info->keyG = info->keyB = 0;
    info->colorType = LCT_RGBA;
    info->bitDepth = 8;
    info->palette = 0;
    info->paletteSize = 0;
}

void PNGPaletteClear(PNGColorMode* info)
{
    if (info->palette) free(info->palette);
    info->palette = 0;
    info->paletteSize = 0;
}

void PNGColorModeCleanup(PNGColorMode* info)
{
    PNGPaletteClear(info);
}

unsigned int PNGColorModeCopy(PNGColorMode* dest, const PNGColorMode* source)
{
    unsigned int i;
    PNGColorModeCleanup(dest);
    *dest = *source;

    if (source->palette)
    {
        dest->palette = (unsigned char*)malloc(1024);
        if (!dest->palette && source->paletteSize) return 83;
        for (i = 0; i != source->paletteSize * 4; i++) dest->palette[i] = source->palette[i];
    }

    return 0;
}

unsigned int PNGColorModeEqual(const PNGColorMode* a, const PNGColorMode* b)
{
    unsigned int i;

    if (a->colorType != b->colorType) return 0;
    if (a->bitDepth != b->bitDepth) return 0;
    if (a->keyDefined != b->keyDefined) return 0;

    if (a->keyDefined)
    {
        if (a->keyR != b->keyR) return 0;
        if (a->keyG != b->keyG) return 0;
        if (a->keyB != b->keyB) return 0;
    }

    if (a->paletteSize != b->paletteSize) return 0;

    for (i = 0; i != a->paletteSize * 4; i++)
    {
        if (a->palette[i] != b->palette[i]) return 0;
    }

    return 1;
}

unsigned int PNGPaletteAdd(PNGColorMode* info, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    unsigned char* data;

    // the same resize technique as C++ std::vectors is used, and here it's made so that for a palette with
    // the max of 256 colors, it'll have the exact alloc size
    if (!info->palette) // allocate palette if empty
    {
        // room for 256 colors with 4 bytes each
        data = (unsigned char*)realloc(info->palette, 1024);
        if (!data) return 83;
        else info->palette = data;
    }

    info->palette[4 * info->paletteSize + 0] = r;
    info->palette[4 * info->paletteSize + 1] = g;
    info->palette[4 * info->paletteSize + 2] = b;
    info->palette[4 * info->paletteSize + 3] = a;
    info->paletteSize++;

    return 0;
}

unsigned int PNGGetBPP(const PNGColorMode* info)
{
    // calculate bits per pixel out of colorType and bitDepth
    return PNGGetBPPLCT(info->colorType, info->bitDepth);
}

unsigned int PNGGetChannels(const PNGColorMode* info)
{
    return GetNumColorChannels(info->colorType);
}

unsigned int PNGIsGreyScaleType(const PNGColorMode* info)
{
    return info->colorType == LCT_GREY || info->colorType == LCT_GREY_ALPHA;
}

unsigned int PNGIsAlphaType(const PNGColorMode* info)
{
    return (info->colorType & 4) != 0; // 4 or 6
}

unsigned int PNGIsPaletteType(const PNGColorMode* info)
{
    return info->colorType == LCT_PALETTE;
}

unsigned int PNGHasPaletteAlpha(const PNGColorMode* info)
{
    unsigned int i;

    for (i = 0; i != info->paletteSize; i++)
    {
        if (info->palette[i * 4 + 3] < 255) return 1;
    }

    return 0;
}

unsigned int PNGCanHaveAlpha(const PNGColorMode* info)
{
    return info->keyDefined || PNGIsAlphaType(info) || PNGHasPaletteAlpha(info);
}

unsigned int PNGGetRawSizeLCT(unsigned int w, unsigned int h, PNGColorType colorType, unsigned int bitDepth)
{
    unsigned int bpp = PNGGetBPPLCT(colorType, bitDepth);
    unsigned int n = (unsigned int)w * (unsigned int)h;
    return ((n / 8) * bpp) + ((n & 7) * bpp + 7) / 8;
}

unsigned int PNGGetRawSize(unsigned int w, unsigned int h, const PNGColorMode* colorMode)
{
    return PNGGetRawSizeLCT(w, h, colorMode->colorType, colorMode->bitDepth);
}

// in an idat chunk, each scanline is a multiple of 8 bits, unlike the lodepng output buffer,
// and in addition has one extra byte per line: the filter byte. So this gives a larger
// result than lodepng_get_raw_size.
unsigned int PNGGetRawSizeIDAT(unsigned int w, unsigned int h, const PNGColorMode* colorMode)
{
    unsigned int bpp = PNGGetBPP(colorMode);
    // + 1 for the filter byte, and possibly plus padding bits per line
    unsigned int line = ((unsigned int)(w / 8) * bpp) + 1 + ((w & 7) * bpp + 7) / 8;
    return (unsigned int)h * line;
}

// Safely check if multiplying two integers will overflow (no undefined
// behavior, compiler removing the code, etc...) and output result.
int PNGMulOfL(unsigned int a, unsigned int b, unsigned int* result)
{
    *result = a * b; // Unsigned multiplication is well defined and safe in C90
    return (a != 0 && *result / a != b);
}

 // Safely check if adding two integers will overflow (no undefined
// behavior, compiler removing the code, etc...) and output result.
int PNGAddOfL(unsigned int a, unsigned int b, unsigned int* result)
{
    *result = a + b; // Unsigned addition is well defined and safe in C90
    return *result < a;
}

 // Safely checks whether unsigned int overflow can be caused due to amount of pixels.
// This check is overcautious rather than precise. If this check indicates no overflow,
// you can safely compute in a unsigned int (but not an unsigned):
// -(unsigned int)w * (unsigned int)h * 8
// -amount of bytes in IDAT (including filter, padding and Adam7 bytes)
// -amount of bytes in raw color model
// Returns 1 if overflow possible, 0 if not.
int PNGPixelOverflow(unsigned int w, unsigned int h, const PNGColorMode* pngcolor, const PNGColorMode* rawcolor)
{
    unsigned int bpp = max(PNGGetBPP(pngcolor), PNGGetBPP(rawcolor));
    unsigned int numpixels, total;
    unsigned int line; // bytes per line in worst case

    if (PNGMulOfL((unsigned int)w, (unsigned int)h, &numpixels)) return 1;
    if (PNGMulOfL(numpixels, 8, &total)) return 1; // bit pointer with 8-bit color, or 8 bytes per channel color
    
    // Bytes per scanline with the expression "(w / 8) * bpp) + ((w & 7) * bpp + 7) / 8"
    if (PNGMulOfL((unsigned int)(w / 8), bpp, &line)) return 1;
    if (PNGAddOfL(line, ((w & 7) * bpp + 7) / 8, &line)) return 1;
    if (PNGAddOfL(line, 5, &line)) return 1; // 5 bytes overhead per line: 1 filterbyte, 4 for Adam7 worst case
    if (PNGMulOfL(line, h, &total)) return 1; // Total bytes in worst case
    return 0; // no overflow
}

void PNGUnknownChunksInit(PNGInfo* info)
{
    unsigned int i;
    for (i = 0; i != 3; i++) info->unknownChunksData[i] = 0;
    for (i = 0; i != 3; i++) info->unknownChunksSize[i] = 0;
}

void PNGUnknownChunksCleanup(PNGInfo* info)
{
    unsigned int i;
    for (i = 0; i != 3; i++) free(info->unknownChunksData[i]);
}

unsigned int PNGUnknownChunksCopy(PNGInfo* dest, const PNGInfo* src)
{
    unsigned int i;

    PNGUnknownChunksCleanup(dest);
    for (i = 0; i != 3; i++)
    {
        unsigned int j;
        dest->unknownChunksSize[i] = src->unknownChunksSize[i];
        dest->unknownChunksData[i] = (unsigned char*)malloc(src->unknownChunksSize[i]);
        if (!dest->unknownChunksData[i] && dest->unknownChunksSize[i]) return 83;
        for (j = 0; j != src->unknownChunksSize[i]; j++)
        {
            dest->unknownChunksData[i][j] = src->unknownChunksData[i][j];
        }
    }

    return 0;
}

void PNGTextInit(PNGInfo* info)
{
    info->textNum = 0;
    info->textKeys = NULL;
    info->textStrings = NULL;
}

void PNGTextCleanup(PNGInfo* info)
{
    unsigned int i;
    for (i = 0; i != info->textNum; i++)
    {
        StringCleanup(&info->textKeys[i]);
        StringCleanup(&info->textStrings[i]);
    }

    free(info->textKeys);
    free(info->textStrings);
}

unsigned int PNGAddText(PNGInfo* info, const char* key, const char* str)
{
    char** newKeys = (char**)(realloc(info->textKeys, sizeof(char*) * (info->textNum + 1)));
    char** newStrings = (char**)(realloc(info->textStrings, sizeof(char*) * (info->textNum + 1)));

    if (!newKeys || !newStrings)
    {
        free(newKeys);
        free(newStrings);
        return 83;
    }

    info->textNum++;
    info->textKeys = newKeys;
    info->textStrings = newStrings;

    StringInit(&info->textKeys[info->textNum - 1]);
    StringSet(&info->textKeys[info->textNum - 1], key);

    StringInit(&info->textStrings[info->textNum - 1]);
    StringSet(&info->textStrings[info->textNum - 1], str);

    return 0;
}

unsigned int PNGTextCopy(PNGInfo* dest, const PNGInfo* source)
{
    unsigned int i = 0;
    dest->textKeys = 0;
    dest->textStrings = 0;
    dest->textNum = 0;

    for (i = 0; i != source->textNum; i++)
    {
        CERROR_TRY_RETURN(PNGAddText(dest, source->textKeys[i], source->textStrings[i]));
    }

    return 0;
}

void PNGClearText(PNGInfo* info)
{
    PNGTextCleanup(info);
}

void PNGITextInit(PNGInfo* info)
{
    info->itextNum = 0;
    info->itextKeys = NULL;
    info->itextLangTags = NULL;
    info->itextTransKeys = NULL;
    info->itextStrings = NULL;
}

void PNGITextCleanup(PNGInfo* info)
{
    unsigned int i;

    for (i = 0; i != info->itextNum; i++)
    {
        StringCleanup(&info->itextKeys[i]);
        StringCleanup(&info->itextLangTags[i]);
        StringCleanup(&info->itextTransKeys[i]);
        StringCleanup(&info->itextStrings[i]);
    }

    free(info->itextKeys);
    free(info->itextLangTags);
    free(info->itextTransKeys);
    free(info->itextStrings);
}

void PNGClearItext(PNGInfo* info)
{
    PNGITextCleanup(info);
}

unsigned int PNGAddItext(PNGInfo* info, const char* key, const char* langTag, const char* transKey, const char* str)
{
    char** newKeys = (char**)(realloc(info->itextKeys, sizeof(char*) * (info->itextNum + 1)));
    char** newLangTags = (char**)(realloc(info->itextLangTags, sizeof(char*) * (info->itextNum + 1)));
    char** newTransKeys = (char**)(realloc(info->itextTransKeys, sizeof(char*) * (info->itextNum + 1)));
    char** newStrings = (char**)(realloc(info->itextStrings, sizeof(char*) * (info->itextNum + 1)));

    if (!newKeys || !newLangTags || !newTransKeys || !newStrings)
    {
        free(newKeys);
        free(newLangTags);
        free(newTransKeys);
        free(newStrings);
        return 83;
    }

    info->itextNum++;
    info->itextKeys = newKeys;
    info->itextLangTags = newLangTags;
    info->itextTransKeys = newTransKeys;
    info->itextStrings = newStrings;

    StringInit(&info->itextKeys[info->itextNum - 1]);
    StringSet(&info->itextKeys[info->itextNum - 1], key);

    StringInit(&info->itextLangTags[info->itextNum - 1]);
    StringSet(&info->itextLangTags[info->itextNum - 1], langTag);

    StringInit(&info->itextTransKeys[info->itextNum - 1]);
    StringSet(&info->itextTransKeys[info->itextNum - 1], transKey);

    StringInit(&info->itextStrings[info->itextNum - 1]);
    StringSet(&info->itextStrings[info->itextNum - 1], str);

    return 0;
}

unsigned int PNGITextCopy(PNGInfo* dest, const PNGInfo* source)
{
    unsigned int i = 0;

    dest->itextKeys = 0;
    dest->itextLangTags = 0;
    dest->itextTransKeys = 0;
    dest->itextStrings = 0;
    dest->itextNum = 0;

    for (i = 0; i != source->itextNum; i++)
    {
        CERROR_TRY_RETURN(PNGAddItext(dest, source->itextKeys[i], source->itextLangTags[i], source->itextTransKeys[i], source->itextStrings[i]));
    }

    return 0;
}

void PNGInfoInit(PNGInfo* info)
{
    PNGColorModeInit(&info->colorMode);
    info->interlaceMethod = 0;
    info->compressionMethod = 0;
    info->filterMethod = 0;
    info->backgroundDefined = 0;
    info->backgroundR = info->backgroundG = info->backgroundB = 0;

    PNGTextInit(info);
    PNGITextInit(info);

    info->timeDefined = 0;
    info->physDefined = 0;

    PNGUnknownChunksInit(info);
}

void PNGInfoCleanup(PNGInfo* info)
{
    PNGColorModeCleanup(&info->colorMode);
    PNGTextCleanup(info);
    PNGITextCleanup(info);
    PNGUnknownChunksCleanup(info);
}

unsigned int PNGInfoCopy(PNGInfo* dest, const PNGInfo* source)
{
    PNGInfoCleanup(dest);
    *dest = *source;
    PNGColorModeInit(&dest->colorMode);
    CERROR_TRY_RETURN(PNGColorModeCopy(&dest->colorMode, &source->colorMode));

    CERROR_TRY_RETURN(PNGTextCopy(dest, source));
    CERROR_TRY_RETURN(PNGITextCopy(dest, source));

    PNGUnknownChunksInit(dest);
    CERROR_TRY_RETURN(PNGUnknownChunksCopy(dest, source));
    return 0;
}

void PNGInfoSwap(PNGInfo* a, PNGInfo* b)
{
    PNGInfo temp = *a;
    *a = *b;
    *b = temp;
}

// index: bitgroup index, bits: bitgroup size(1, 2 or 4), in: bitgroup value, out: octet array to add bits to
void AddColorBits(unsigned char* out, unsigned int index, unsigned int bits, unsigned int in)
{
    unsigned int m = bits == 1 ? 7 : bits == 2 ? 3 : 1; // 8 / bits - 1

    // p = the partial index in the byte, e.g. with 4 palBits it is 0 for first half or 1 for second half
    unsigned int p = index & m;
    in &= (1u << bits) - 1u; // Filter out any other bits of the src value
    in = in << (bits * (m - p));

    if (p == 0) out[index * bits / 8] = in;
    else out[index * bits / 8] |= in;
}

typedef struct ColorTree ColorTree;

// One node of a colorMode tree
// This is the data structure used to count the number of unique colors and to get a palette
// index for a colorMode. It's like an octree, but because the alpha channel is used too, each
// node has 16 instead of 8 children.

struct ColorTree
{
    ColorTree* children[16]; // up to 16 pointers to ColorTree of next level
    int index; // the payload. Only has a meaningful value if this is in the last level
};

void ColorTreeInit(ColorTree* tree)
{
    unsigned int i;
    for (i = 0; i != 16; i++) tree->children[i] = 0;
    tree->index = -1;
}

void ColorTreeCleanup(ColorTree* tree)
{
    unsigned int i;
    for (i = 0; i != 16; i++)
    {
        if (tree->children[i])
        {
            ColorTreeCleanup(tree->children[i]);
            free(tree->children[i]);
        }
    }
}

// returns -1 if colorMode not present, its index otherwise
int ColorTreeGet(ColorTree* tree, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    unsigned int bit = 0;

    for (bit = 0; bit < 8; bit++)
    {
        unsigned int i = 8 * ((r >> bit) & 1) + 4 * ((g >> bit) & 1) + 2 * ((b >> bit) & 1) + 1 * ((a >> bit) & 1);
        if (!tree->children[i]) return -1;
        else tree = tree->children[i];
    }

    return tree ? tree->index : -1;
}

int ColorTreeHas(ColorTree* tree, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    return ColorTreeGet(tree, r, g, b, a) >= 0;
}

// colorMode is not allowed to already exist.
// index should be >= 0 (it's signed to be compatible with using -1 for "doesn't exist")
void ColorTreeAdd(ColorTree* tree, unsigned char r, unsigned char g, unsigned char b, unsigned char a, unsigned int index)
{
    unsigned int bit;

    for (bit = 0; bit < 8; bit++)
    {
        unsigned int i = 8 * ((r >> bit) & 1) + 4 * ((g >> bit) & 1) + 2 * ((b >> bit) & 1) + 1 * ((a >> bit) & 1);

        if (!tree->children[i])
        {
            tree->children[i] = (ColorTree*)malloc(sizeof(ColorTree));
            ColorTreeInit(tree->children[i]);
        }

        tree = tree->children[i];
    }

    tree->index = (int)index;
}

// put a pixel, given its RGBA colorMode, into data of any colorMode type
unsigned int RGBA8ToPixel(unsigned char* out, unsigned int i, const PNGColorMode* modeNum, ColorTree* tree, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    if (modeNum->colorType == LCT_GREY)
    {
        unsigned char grey = r;
        if (modeNum->bitDepth == 8) out[i] = grey;
        else if (modeNum->bitDepth == 16) out[i * 2 + 0] = out[i * 2 + 1] = grey;
        else
        {
            // take the most significant bits of grey
            grey = (grey >> (8 - modeNum->bitDepth)) & ((1 << modeNum->bitDepth) - 1);
            AddColorBits(out, i, modeNum->bitDepth, grey);
        }
    }
    else if (modeNum->colorType == LCT_RGB)
    {
        if (modeNum->bitDepth == 8)
        {
            out[i * 3 + 0] = r;
            out[i * 3 + 1] = g;
            out[i * 3 + 2] = b;
        }
        else
        {
            out[i * 6 + 0] = out[i * 6 + 1] = r;
            out[i * 6 + 2] = out[i * 6 + 3] = g;
            out[i * 6 + 4] = out[i * 6 + 5] = b;
        }
    }
    else if(modeNum->colorType == LCT_PALETTE)
    {
        int index = ColorTreeGet(tree, r, g, b, a);
        if (index < 0) return 82; // colorMode not in palette

        if (modeNum->bitDepth == 8) out[i] = index;
        else AddColorBits(out, i, modeNum->bitDepth, (unsigned int)index);
    }
    else if(modeNum->colorType == LCT_GREY_ALPHA)
    {
        unsigned char grey = r;
        if (modeNum->bitDepth == 8)
        {
            out[i * 2 + 0] = grey;
            out[i * 2 + 1] = a;
        }
        else if (modeNum->bitDepth == 16)
        {
            out[i * 4 + 0] = out[i * 4 + 1] = grey;
            out[i * 4 + 2] = out[i * 4 + 3] = a;
        }
    }
    else if (modeNum->colorType == LCT_RGBA)
    {
        if (modeNum->bitDepth == 8)
        {
            out[i * 4 + 0] = r;
            out[i * 4 + 1] = g;
            out[i * 4 + 2] = b;
            out[i * 4 + 3] = a;
        }
        else
        {
            out[i * 8 + 0] = out[i * 8 + 1] = r;
            out[i * 8 + 2] = out[i * 8 + 3] = g;
            out[i * 8 + 4] = out[i * 8 + 5] = b;
            out[i * 8 + 6] = out[i * 8 + 7] = a;
        }
    }

    return 0;
}

// put a pixel, given its RGBA16 colorMode, into data of any colorMode 16-bitDepth type
void RGBA16ToPixel(unsigned char* out, int i, const PNGColorMode* modeNum, unsigned int short r, unsigned int short g, unsigned int short b, unsigned int short a)
{
    if (modeNum->colorType == LCT_GREY)
    {
        unsigned int short grey = r;
        out[i * 2 + 0] = (grey >> 8) & 255;
        out[i * 2 + 1] = grey & 255;
    }
    else if (modeNum->colorType == LCT_RGB)
    {
        out[i * 6 + 0] = (r >> 8) & 255;
        out[i * 6 + 1] = r & 255;
        out[i * 6 + 2] = (g >> 8) & 255;
        out[i * 6 + 3] = g & 255;
        out[i * 6 + 4] = (b >> 8) & 255;
        out[i * 6 + 5] = b & 255;
    }
    else if (modeNum->colorType == LCT_GREY_ALPHA)
    {
        unsigned int short grey = r;
        out[i * 4 + 0] = (grey >> 8) & 255;
        out[i * 4 + 1] = grey & 255;
        out[i * 4 + 2] = (a >> 8) & 255;
        out[i * 4 + 3] = a & 255;
    }
    else if (modeNum->colorType == LCT_RGBA)
    {
        out[i * 8 + 0] = (r >> 8) & 255;
        out[i * 8 + 1] = r & 255;
        out[i * 8 + 2] = (g >> 8) & 255;
        out[i * 8 + 3] = g & 255;
        out[i * 8 + 4] = (b >> 8) & 255;
        out[i * 8 + 5] = b & 255;
        out[i * 8 + 6] = (a >> 8) & 255;
        out[i * 8 + 7] = a & 255;
    }
}

// Get RGBA8 colorMode of pixel with index i (y * width + x) from the raw data with given colorMode type.
void GetPixelColorRGBA8(unsigned char* r, unsigned char* g, unsigned char* b, unsigned char* a, const unsigned char* in, unsigned int i, const PNGColorMode* modeNum)
{
    if (modeNum->colorType == LCT_GREY)
    {
        if (modeNum->bitDepth == 8)
        {
            *r = *g = *b = in[i];
            if (modeNum->keyDefined && *r == modeNum->keyR) *a = 0;
            else *a = 255;
        }
        else if (modeNum->bitDepth == 16)
        {
            *r = *g = *b = in[i * 2 + 0];
            if (modeNum->keyDefined && 256U * in[i * 2 + 0] + in[i * 2 + 1] == modeNum->keyR) *a = 0;
            else *a = 255;
        }
        else
        {
            unsigned int highest = ((1U << modeNum->bitDepth) - 1U); // highest possible value for this bit depth
            unsigned int j = i * modeNum->bitDepth;
            unsigned int value = ReadBitsFromReversedStream(&j, in, modeNum->bitDepth);
            *r = *g = *b = (value * 255) / highest;

            if (modeNum->keyDefined && value == modeNum->keyR) *a = 0;
            else *a = 255;
        }
    }
    else if (modeNum->colorType == LCT_RGB)
    {
        if (modeNum->bitDepth == 8)
        {
            *r = in[i * 3 + 0]; *g = in[i * 3 + 1]; *b = in[i * 3 + 2];
            if (modeNum->keyDefined && *r == modeNum->keyR && *g == modeNum->keyG && *b == modeNum->keyB) *a = 0;
            else *a = 255;
        }
        else
        {
            *r = in[i * 6 + 0];
            *g = in[i * 6 + 2];
            *b = in[i * 6 + 4];
            if (modeNum->keyDefined && 256U * in[i * 6 + 0] + in[i * 6 + 1] == modeNum->keyR && 256U * in[i * 6 + 2] + in[i * 6 + 3] == modeNum->keyG && 256U * in[i * 6 + 4] + in[i * 6 + 5] == modeNum->keyB) *a = 0;
            else *a = 255;
        }
    }
    else if(modeNum->colorType == LCT_PALETTE)
    {
        unsigned int index;
        if (modeNum->bitDepth == 8) index = in[i];
        else
        {
            unsigned int j = i * modeNum->bitDepth;
            index = ReadBitsFromReversedStream(&j, in, modeNum->bitDepth);
        }

        if (index >= modeNum->paletteSize)
        {
            // This is an error according to the PNG spec, but common PNG decoders make it black instead.
            // Done here too, slightly faster due to no error handling needed.
            *r = *g = *b = 0;
            *a = 255;
        }
        else
        {
            *r = modeNum->palette[index * 4 + 0];
            *g = modeNum->palette[index * 4 + 1];
            *b = modeNum->palette[index * 4 + 2];
            *a = modeNum->palette[index * 4 + 3];
        }
    }
    else if (modeNum->colorType == LCT_GREY_ALPHA)
    {
        if (modeNum->bitDepth == 8)
        {
            *r = *g = *b = in[i * 2 + 0];
            *a = in[i * 2 + 1];
        }
        else
        {
            *r = *g = *b = in[i * 4 + 0];
            *a = in[i * 4 + 2];
        }
    }
    else if (modeNum->colorType == LCT_RGBA)
    {
        if (modeNum->bitDepth == 8)
        {
            *r = in[i * 4 + 0];
            *g = in[i * 4 + 1];
            *b = in[i * 4 + 2];
            *a = in[i * 4 + 3];
        }
        else
        {
            *r = in[i * 8 + 0];
            *g = in[i * 8 + 2];
            *b = in[i * 8 + 4];
            *a = in[i * 8 + 6];
        }
    }
}

// Similar to GetPixelColorRGBA8, but with all the for loops inside of the colorMode
// modeNum test cases, optimized to convert the colors much faster, when converting
// to RGBA or RGB with 8 bit per cannel. buffer must be RGBA or RGB dst with
// enough memory, if hasAlpha is true the dst is RGBA. modeNum has the colorMode modeNum
// of the src buffer.
void GetPixelColorsRGBA8(unsigned char* buffer, unsigned int numPixels, unsigned int hasAlpha, const unsigned char* in, const PNGColorMode* modeNum)
{
    unsigned int i, numChannels = hasAlpha ? 4 : 3;

    if (modeNum->colorType == LCT_GREY)
    {
        if (modeNum->bitDepth == 8)
        {
            for (i = 0; i != numPixels; i++, buffer += numChannels)
            {
                buffer[0] = buffer[1] = buffer[2] = in[i];
                if (hasAlpha) buffer[3] = modeNum->keyDefined && in[i] == modeNum->keyR ? 0 : 255;
            }
        }
        else if (modeNum->bitDepth == 16)
        {
            for (i = 0; i != numPixels; i++, buffer += numChannels)
            {
                buffer[0] = buffer[1] = buffer[2] = in[i * 2];
                if (hasAlpha) buffer[3] = modeNum->keyDefined && 256U * in[i * 2 + 0] + in[i * 2 + 1] == modeNum->keyR ? 0 : 255;
            }
        }
        else
        {
            unsigned int highest = ((1U << modeNum->bitDepth) - 1U); // highest possible value for this bit depth
            unsigned int j = 0;

            for (i = 0; i != numPixels; i++, buffer += numChannels)
            {
                unsigned int value = ReadBitsFromReversedStream(&j, in, modeNum->bitDepth);
                buffer[0] = buffer[1] = buffer[2] = (value * 255) / highest;
                if (hasAlpha) buffer[3] = modeNum->keyDefined && value == modeNum->keyR ? 0 : 255;
            }
        }
    }
    else if (modeNum->colorType == LCT_RGB)
    {
        if (modeNum->bitDepth == 8)
        {
            for (i = 0; i != numPixels; i++, buffer += numChannels)
            {
                buffer[0] = in[i * 3 + 0];
                buffer[1] = in[i * 3 + 1];
                buffer[2] = in[i * 3 + 2];
                if (hasAlpha) buffer[3] = modeNum->keyDefined && buffer[0] == modeNum->keyR && buffer[1]== modeNum->keyG && buffer[2] == modeNum->keyB ? 0 : 255;
            }
        }
        else
        {
            for (i = 0; i != numPixels; i++, buffer += numChannels)
            {
                buffer[0] = in[i * 6 + 0];
                buffer[1] = in[i * 6 + 2];
                buffer[2] = in[i * 6 + 4];
                if (hasAlpha) buffer[3] = modeNum->keyDefined && 256U * in[i * 6 + 0] + in[i * 6 + 1] == modeNum->keyR && 256U * in[i * 6 + 2] + in[i * 6 + 3] == modeNum->keyG && 256U * in[i * 6 + 4] + in[i * 6 + 5] == modeNum->keyB ? 0 : 255;
            }
        }
    }
    else if (modeNum->colorType == LCT_PALETTE)
    {
        unsigned int index;
        unsigned int j = 0;
        for (i = 0; i != numPixels; i++, buffer += numChannels)
        {
            if (modeNum->bitDepth == 8) index = in[i];
            else index = ReadBitsFromReversedStream(&j, in, modeNum->bitDepth);

            if (index >= modeNum->paletteSize)
            {
                // This is an error according to the PNG spec, but most PNG decoders make it black instead.
                // Done here too, slightly faster due to no error handling needed.
                buffer[0] = buffer[1] = buffer[2] = 0;
                if (hasAlpha) buffer[3] = 255;
            }
            else
            {
                buffer[0] = modeNum->palette[index * 4 + 0];
                buffer[1] = modeNum->palette[index * 4 + 1];
                buffer[2] = modeNum->palette[index * 4 + 2];
                if (hasAlpha) buffer[3] = modeNum->palette[index * 4 + 3];
            }
        }
    }
    else if (modeNum->colorType == LCT_GREY_ALPHA)
    {
        if (modeNum->bitDepth == 8)
        {
            for (i = 0; i != numPixels; i++, buffer += numChannels)
            {
                buffer[0] = buffer[1] = buffer[2] = in[i * 2 + 0];
                if (hasAlpha) buffer[3] = in[i * 2 + 1];
            }
        }
        else
        {
            for (i = 0; i != numPixels; i++, buffer += numChannels)
            {
                buffer[0] = buffer[1] = buffer[2] = in[i * 4 + 0];
                if (hasAlpha) buffer[3] = in[i * 4 + 2];
            }
        }
    }
    else if (modeNum->colorType == LCT_RGBA)
    {
        if (modeNum->bitDepth == 8)
        {
            for (i = 0; i != numPixels; i++, buffer += numChannels)
            {
                buffer[0] = in[i * 4 + 0];
                buffer[1] = in[i * 4 + 1];
                buffer[2] = in[i * 4 + 2];
                if (hasAlpha) buffer[3] = in[i * 4 + 3];
            }
        }
        else
        {
            for (i = 0; i != numPixels; i++, buffer += numChannels)
            {
                buffer[0] = in[i * 8 + 0];
                buffer[1] = in[i * 8 + 2];
                buffer[2] = in[i * 8 + 4];
                if (hasAlpha) buffer[3] = in[i * 8 + 6];
            }
        }
    }
}

// Get RGBA16 colorMode of pixel with index i (y * width + x) from the raw data with
// given colorMode type, but the given colorMode type must be 16-bit itself.
void GetPixelColorRGBA16(unsigned int short* r, unsigned int short* g, unsigned int short* b, unsigned int short* a, const unsigned char* in, unsigned int i, const PNGColorMode* modeNum)
{
    if (modeNum->colorType == LCT_GREY)
    {
        *r = *g = *b = 256 * in[i * 2 + 0] + in[i * 2 + 1];
        if (modeNum->keyDefined && 256U * in[i * 2 + 0] + in[i * 2 + 1] == modeNum->keyR) *a = 0;
        else *a = 65535;
    }
    else if (modeNum->colorType == LCT_RGB)
    {
        *r = 256 * in[i * 6 + 0] + in[i * 6 + 1];
        *g = 256 * in[i * 6 + 2] + in[i * 6 + 3];
        *b = 256 * in[i * 6 + 4] + in[i * 6 + 5];
        if (modeNum->keyDefined && 256U * in[i * 6 + 0] + in[i * 6 + 1] == modeNum->keyR && 256U * in[i * 6 + 2] + in[i * 6 + 3] == modeNum->keyG && 256U * in[i * 6 + 4] + in[i * 6 + 5] == modeNum->keyB) *a = 0;
        else *a = 65535;
    }
    else if (modeNum->colorType == LCT_GREY_ALPHA)
    {
        *r = *g = *b = 256 * in[i * 4 + 0] + in[i * 4 + 1];
        *a = 256 * in[i * 4 + 2] + in[i * 4 + 3];
    }
    else if (modeNum->colorType == LCT_RGBA)
    {
        *r = 256 * in[i * 8 + 0] + in[i * 8 + 1];
        *g = 256 * in[i * 8 + 2] + in[i * 8 + 3];
        *b = 256 * in[i * 8 + 4] + in[i * 8 + 5];
        *a = 256 * in[i * 8 + 6] + in[i * 8 + 7];
    }
}

unsigned int PNGConvert(unsigned char* out, const unsigned char* in, PNGColorMode* modeOut, const PNGColorMode* modeIn, unsigned int w, unsigned int h)
{
    ColorTree tree;
    unsigned int i, error = 0;
    unsigned int numPixels = w * h;

    if (PNGColorModeEqual(modeOut, modeIn))
    {
        unsigned int numBytes = PNGGetRawSize(w, h, modeIn);
        for (i = 0; i != numBytes; i++) out[i] = in[i];
        return 0;
    }

    if (modeOut->colorType == LCT_PALETTE)
    {
        unsigned int paletteSize = modeOut->paletteSize;
        const unsigned char* palette = modeOut->palette;
        unsigned int palSize = (unsigned int)1u << modeOut->bitDepth;
        // if the user specified output palette but did not give the values, assume
        // they want the values of the input color type (assuming that one is palette).
        // Note that we never create a new palette ourselves.
        if (paletteSize == 0)
        {
            paletteSize = modeIn->paletteSize;
            palette = modeIn->palette;
            // if the input was also palette with same bitdepth, then the color types are also
            // equal, so copy literally. This to preserve the exact indices that were in the PNG
            // even in case there are duplicate colors in the palette.
            if (modeIn->colorType == LCT_PALETTE && modeIn->bitDepth == modeOut->bitDepth)
            {
                unsigned int numBytes = PNGGetRawSize(w, h, modeIn);
                for (i = 0; i != numBytes; ++i) out[i] = in[i];
                return 0;
            }
        }

        if (modeOut->paletteSize < palSize) palSize = modeOut->paletteSize;
        ColorTreeInit(&tree);
        for (i = 0; i != palSize; i++)
        {
            unsigned char* p = &modeOut->palette[i * 4];
            ColorTreeAdd(&tree, p[0], p[1], p[2], p[3], (unsigned int)i);
        }
    }

    if (modeIn->bitDepth == 16 && modeOut->bitDepth == 16)
    {
        for (i = 0; i != numPixels; i++)
        {
            unsigned int short r = 0, g = 0, b = 0, a = 0;
            GetPixelColorRGBA16(&r, &g, &b, &a, in, i, modeIn);
            RGBA16ToPixel(out, i, modeOut, r, g, b, a);
        }
    }
    else if (modeOut->bitDepth == 8 && modeOut->colorType == LCT_RGBA)
    {
        GetPixelColorsRGBA8(out, numPixels, 1, in, modeIn);
    }
    else if (modeOut->bitDepth == 8 && modeOut->colorType == LCT_RGB)
    {
        GetPixelColorsRGBA8(out, numPixels, 0, in, modeIn);
    }
    else
    {
        unsigned char r = 0, g = 0, b = 0, a = 0;
        for (i = 0; i != numPixels; i++)
        {
            GetPixelColorRGBA8(&r, &g, &b, &a, in, i, modeIn);
            error = RGBA8ToPixel(out, i, modeOut, &tree, r, g, b, a);
            if (error) break;
        }
    }

    if (modeOut->colorType == LCT_PALETTE)
    {
        ColorTreeCleanup(&tree);
    }

    return error;
}

void PNGColorProfileInit(PNGColorProfile* profile)
{
    profile->colored = 0;
    profile->key = 0;
    profile->alpha = 0;
    profile->keyR = profile->keyG = profile->keyB = 0;
    profile->numColors = 0;
    profile->bits = 1;
}

// Returns how many bits needed to represent given value (max 8 bit)
unsigned int GetValueRequiredBits(unsigned char value)
{
    if (value == 0 || value == 255) return 1;

    // The scaling of 2-bit and 4-bit values uses multiples of 85 and 17
    if (value % 17 == 0) return value % 85 == 0 ? 2 : 4;

    return 8;
}

// profile must already have been inited with modeNum.
// It's ok to set some parameters of profile to done already.
unsigned int GetColorProfile(PNGColorProfile* profile, const unsigned char* in, unsigned int w, unsigned int h, const PNGColorMode* modeNum)
{
    ColorTree tree;
    unsigned int i, error = 0;
    unsigned int numPixels = w * h;

    unsigned int coloredDone = PNGIsGreyScaleType(modeNum) ? 1 : 0;
    unsigned int alphaDone = PNGCanHaveAlpha(modeNum) ? 0 : 1;
    unsigned int numColorsDone = 0;
    int bpp = PNGGetBPP(modeNum);
    unsigned int bitsDone = bpp == 1 ? 1 : 0;
    unsigned int maxNumColors = 257;
    unsigned int sixteen = 0;

    if (bpp <= 8) maxNumColors = bpp == 1 ? 2 : (bpp == 2 ? 4 : (bpp == 4 ? 16 : 256));

    ColorTreeInit(&tree);

    // Check if the 16-bit src is truly 16-bit
    if (modeNum->bitDepth == 16)
    {
        unsigned int short r, g, b, a;
        for (i = 0; i != numPixels; i++)
        {
            GetPixelColorRGBA16(&r, &g, &b, &a, in, i, modeNum);
            if ((r & 255) != ((r >> 8) & 255) || (g & 255) != ((g >> 8) & 255) || (b & 255) != ((b >> 8) & 255) || (a & 255) != ((a >> 8) & 255)) // first and second byte differ
            {
                sixteen = 1;
                break;
            }
        }
    }

    if (sixteen)
    {
        unsigned int short r = 0, g = 0, b = 0, a = 0;
        profile->bits = 16;
        bitsDone = numColorsDone = 1; // counting colors no longer useful, palette doesn't support 16-bit

        for (i = 0; i != numPixels; i++)
        {
            GetPixelColorRGBA16(&r, &g, &b, &a, in, i, modeNum);

            if (!coloredDone && (r != g || r != b))
            {
                profile->colored = 1;
                coloredDone = 1;
            }

            if (!alphaDone)
            {
                unsigned int matchKey = (r == profile->keyR && g == profile->keyG && b == profile->keyB);
                if (a != 65535 && (a != 0 || (profile->key && !matchKey)))
                {
                    profile->alpha = 1;
                    profile->key = 0;
                    alphaDone = 1;
                }
                else if (a == 0 && !profile->alpha && !profile->key)
                {
                    profile->key = 1;
                    profile->keyR = r;
                    profile->keyG = g;
                    profile->keyB = b;
                }
                else if (a == 65535 && profile->key && matchKey)
                {
                    //  col key cannot be used if an opaque pixel also has that RGB colorMode. 
                    profile->alpha = 1;
                    profile->key = 0;
                    alphaDone = 1;
                }
            }

            if (alphaDone && numColorsDone && coloredDone && bitsDone) break;
        }

        if (profile->key && !profile->alpha)
        {
            for (i = 0; i != numPixels; ++i)
            {
                GetPixelColorRGBA16(&r, &g, &b, &a, in, i, modeNum);
                if (a != 0 && r == profile->keyR && g == profile->keyG && b == profile->keyB)
                {
                    // Color key cannot be used if an opaque pixel also has that RGB color.
                    profile->alpha = 1;
                    profile->key = 0;
                    alphaDone = 1;
                }
            }
        }
    }
    else //  < 16-bit 
    {
        unsigned char r = 0, g = 0, b = 0, a = 0;
        for (i = 0; i != numPixels; i++)
        {
            GetPixelColorRGBA8(&r, &g, &b, &a, in, i, modeNum);

            if (!bitsDone && profile->bits < 8)
            {
                // only r is checked, < 8 bits is only relevant for greyscale
                unsigned int bits = GetValueRequiredBits(r);
                if (bits > profile->bits) profile->bits = bits;
            }
            bitsDone = (profile->bits >= bpp);

            if (!coloredDone && (r != g || r != b))
            {
                profile->colored = 1;
                coloredDone = 1;
                if (profile->bits < 8) profile->bits = 8; // PNG has no colored modes with less than 8-bit per channel
            }

            if (!alphaDone)
            {
                unsigned int matchKey = (r == profile->keyR && g == profile->keyG && b == profile->keyB);
                if (a != 255 && (a != 0 || (profile->key && !matchKey)))
                {
                    profile->alpha = 1;
                    profile->key = 0;
                    alphaDone = 1;
                    if (profile->bits < 8) profile->bits = 8; // PNG has no alphachannel modes with less than 8-bit per channel
                }
                else if (a == 0 && !profile->alpha && !profile->key)
                {
                    profile->key = 1;
                    profile->keyR = r;
                    profile->keyG = g;
                    profile->keyB = b;
                }
                else if (a == 255 && profile->key && matchKey)
                {
                    //  col key cannot be used if an opaque pixel also has that RGB colorMode. 
                    profile->alpha = 1;
                    profile->key = 0;
                    alphaDone = 1;
                    if (profile->bits < 8) profile->bits = 8; // PNG has no alphachannel modes with less than 8-bit per channel
                }
            }

            if (!numColorsDone)
            {
                if (!ColorTreeHas(&tree, r, g, b, a))
                {
                    ColorTreeAdd(&tree, r, g, b, a, profile->numColors);
                    if (profile->numColors < 256)
                    {
                        unsigned char* p = profile->palette;
                        unsigned int n = profile->numColors;
                        p[n * 4 + 0] = r;
                        p[n * 4 + 1] = g;
                        p[n * 4 + 2] = b;
                        p[n * 4 + 3] = a;
                    }

                    profile->numColors++;
                    numColorsDone = profile->numColors >= maxNumColors;
                }
            }

            if (alphaDone && numColorsDone && coloredDone && bitsDone) break;
        }

        if (profile->key && !profile->alpha)
        {
            for (i = 0; i != numPixels; ++i)
            {
                GetPixelColorRGBA8(&r, &g, &b, &a, in, i, modeNum);
                if (a != 0 && r == profile->keyR && g == profile->keyG && b == profile->keyB)
                {
                // col key cannot be used if an opaque pixel also has that RGB color.
                    profile->alpha = 1;
                    profile->key = 0;
                    alphaDone = 1;
                    if (profile->bits < 8) profile->bits = 8; // PNG has no alphachannel modes with less than 8-bit per channel
                }
            }
        }

        // make the profile's key always 16-bit for consistency - repeat each byte twice
        profile->keyR += (profile->keyR << 8);
        profile->keyG += (profile->keyG << 8);
        profile->keyB += (profile->keyB << 8);
    }

    ColorTreeCleanup(&tree);
    return error;
}

// Automatically chooses colorMode type that gives smallest amount of bits in the
// dst data, e.g. grey if there are only greyscale pixels, palette if there
// are less than 256 colors, ...
// Updates values of modeNum with a potentially smaller colorMode model. modeOut should
// contain the user chosen colorMode model, but will be overwritten with the new chosen one.
unsigned int PNGAutoChooseColor(PNGColorMode* modeOut, const unsigned char* data, unsigned int w, unsigned int h, const PNGColorMode* modeIn)
{
    unsigned int error = 0;
    PNGColorProfile prof;
    unsigned int i, n, palBits, palOK;
    unsigned int numPixels = w * h;

    PNGColorProfileInit(&prof);

    error = GetColorProfile(&prof, data, w, h, modeIn);
    if (error) return error;

    modeOut->keyDefined = 0;

    if (prof.key && numPixels <= 16)
    {
        prof.alpha = 1; // too few pixels to justify tRNS chunk overhead
        prof.key = 0;
        if (prof.bits < 8) prof.bits = 8; // PNG has no alphachannel modes with less than 8-bit per channel
    }

    n = prof.numColors;

    palBits = n <= 2 ? 1 : (n <= 4 ? 2 : (n <= 16 ? 4 : 8));
    palOK = n <= 256 && prof.bits <= 8;

    if (numPixels < n * 2) palOK = 0; // don't add palette overhead if data has only a few pixels
    if (!prof.colored && prof.bits <= palBits) palOK = 0; // grey is less overhead

    if (palOK)
    {
        unsigned char* p = prof.palette;
        PNGPaletteClear(modeOut); // remove potential earlier palette

        for (i = 0; i != prof.numColors; i++)
        {
            error = PNGPaletteAdd(modeOut, p[i * 4 + 0], p[i * 4 + 1], p[i * 4 + 2], p[i * 4 + 3]);
            if (error) break;
        }

        modeOut->colorType = LCT_PALETTE;
        modeOut->bitDepth = palBits;

        if (modeIn->colorType == LCT_PALETTE && modeIn->paletteSize >= modeOut->paletteSize && modeIn->bitDepth == modeOut->bitDepth)
        {
            // If src should have same palette colors, keep original to preserve its order and prevent conversion
            PNGColorModeCleanup(modeOut);
            PNGColorModeCopy(modeOut, modeIn);
        }
    }
    else // 8-bit or 16-bit per channel
    {
        modeOut->bitDepth = prof.bits;
        modeOut->colorType = prof.alpha ? (prof.colored ? LCT_RGBA : LCT_GREY_ALPHA) : (prof.colored ? LCT_RGB : LCT_GREY);

        if (prof.key)
        {
            unsigned int mask = (1u << modeOut->bitDepth) - 1u; // profile always uses 16-bit, mask converts it
            modeOut->keyR = prof.keyR & mask;
            modeOut->keyG = prof.keyG & mask;
            modeOut->keyB = prof.keyB & mask;
            modeOut->keyDefined = 1;
        }
    }

    return error;
}

// Paeth predicter, used by PNG Filter type 4
// The parameters are of type short, but should come from unsigned int chars, the shorts
// are only needed to make the paeth calculation correct.
unsigned char PaethPredictor(short a, short b, short c)
{
    short pa = abs(b - c);
    short pb = abs(a - c);
    short pc = abs(a + b - c - c);

    if (pc < pa && pc < pb) return (unsigned char)c;
    if (pb < pa) return (unsigned char)b;
    return (unsigned char)a;
}

// shared values used by multiple Adam7 related functions
const unsigned int ADAM7_IX[7] = { 0, 4, 0, 2, 0, 1, 0 }; // x start values
const unsigned int ADAM7_IY[7] = { 0, 0, 4, 0, 2, 0, 1 }; // y start values
const unsigned int ADAM7_DX[7] = { 8, 8, 4, 4, 2, 2, 1 }; // x delta values
const unsigned int ADAM7_DY[7] = { 8, 8, 8, 4, 4, 2, 2 }; // y delta values

// Outputs various dimensions and positions in the data related to the Adam7 reduced images.
// passw: dst containing the width of the 7 passes
// passh: dst containing the bmHeight of the 7 passes
// fillterPassStart: dst containing the index of the start and end of each
// reduced data with Filter bytes
// paddedPassStart dst containing the index of the start and end of each
// reduced data when without Filter bytes but with padded scanLines
// passStart: dst containing the index of the start and end of each reduced
// data without padding between scanLines, but still padding between the images
// w, h: width and bmHeight of non-interlaced data
// bpp: bits per pixel
// "padded" is only relevant if bpp is less than 8 and a scanline or data does not
// end at a full byte

void Adam7GetPassValues(unsigned int passw[7], unsigned int passh[7], unsigned int fillterPassStart[8], unsigned int paddedPassStart[8], unsigned int passStart[8], unsigned int w, unsigned int h, unsigned int bpp)
{
    // the passStart values have 8 values: the 8th one indicates the byte after the end of the 7th (= last) pass
    unsigned int i;

    // calculate width and bmHeight in pixels of each pass
    for (i = 0; i != 7; i++)
    {
        passw[i] = (w + ADAM7_DX[i] - ADAM7_IX[i] - 1) / ADAM7_DX[i];
        passh[i] = (h + ADAM7_DY[i] - ADAM7_IY[i] - 1) / ADAM7_DY[i];
        if (passw[i] == 0) passh[i] = 0;
        if (passh[i] == 0) passw[i] = 0;
    }

    fillterPassStart[0] = paddedPassStart[0] = passStart[0] = 0;
    for (i = 0; i != 7; i++)
    {
        // if passw[i] is 0, it's 0 bytes, not 1 (no filtertype-byte)
        fillterPassStart[i + 1] = fillterPassStart[i] + ((passw[i] && passh[i]) ? passh[i] * (1 + (passw[i] * bpp + 7) / 8) : 0);

        // bits padded if needed to fill full byte at end of each scanline
        paddedPassStart[i + 1] = paddedPassStart[i] + passh[i] * ((passw[i] * bpp + 7) / 8);

        // only padded at end of reduced data
        passStart[i + 1] = passStart[i] + (passh[i] * passw[i] * bpp + 7) / 8;
    }
}

/////////////////
// PNG decoder // 
/////////////////

// read the information from the header and store it in the PNGInfo. return value is error
unsigned int PNGInspect(unsigned int* w, unsigned int* h, PNGState* state, const unsigned char* in, unsigned int inSize)
{
    PNGInfo* info = &state->pngInfo;

    if (inSize == 0 || in == 0)
    {
        CERROR_RETURN_ERROR(state->error, 48); // error: the given data is empty
    }

    if (inSize < 33)
    {
        CERROR_RETURN_ERROR(state->error, 27); // error: the data len is smaller than the len of a PNG header
    }

    // when decoding a new PNG data, make sure all parameters created after previous decoding are reset
    PNGInfoCleanup(info);
    PNGInfoInit(info);

    if (in[0] != 137 || in[1] != 80 || in[2] != 78 || in[3] != 71 || in[4] != 13 || in[5] != 10 || in[6] != 26 || in[7] != 10)
    {
        CERROR_RETURN_ERROR(state->error, 28); // error: the first 8 bytes are not the correct PNG signature
    }

    if (PNGChunkLength(in + 8) != 13)
    {
        CERROR_RETURN_ERROR(state->error, 94); // error: header size must be 13 bytes
    }

    if (!PNGChunkTypeEquals(in + 8, "IHDR"))
    {
        CERROR_RETURN_ERROR(state->error, 29); // error: it doesn't start with a IHDR chunk!
    }

    // read the values given in the header
    *w = PNGRead32bitInt(&in[16]);
    *h = PNGRead32bitInt(&in[20]);

    info->colorMode.bitDepth = in[24];
    info->colorMode.colorType = (PNGColorType)in[25];
    info->compressionMethod = in[26];
    info->filterMethod = in[27];
    info->interlaceMethod = in[28];

    if (*w == 0 || *h == 0)
    {
        CERROR_RETURN_ERROR(state->error, 93);
    }

    if (!state->decoder.ignoreCRC)
    {
        unsigned int crc = PNGRead32bitInt(&in[29]);
        unsigned int checksum = CRC32(&in[12], 17);
        if (crc != checksum)
        {
            CERROR_RETURN_ERROR(state->error, 57); // invalid crc
        }
    }

    // error: only compression method 0 is allowed in the specification
    if (info->compressionMethod != 0) CERROR_RETURN_ERROR(state->error, 32);

    // error: only Filter method 0 is allowed in the specification
    if (info->filterMethod != 0) CERROR_RETURN_ERROR(state->error, 33);

    // error: only interlace methods 0 and 1 exist in the specification
    if (info->interlaceMethod > 1) CERROR_RETURN_ERROR(state->error, 34);

    state->error = CheckColorValidity(info->colorMode.colorType, info->colorMode.bitDepth);
    return state->error;
}

unsigned int UnfilterScanline(unsigned char* recon, const unsigned char* scanline, const unsigned char* precon, unsigned int byteWidth, unsigned char filterType, unsigned int len)
{
    // For PNG Filter method 0
    // Unfilter a PNG data scanline by scanline. when the pixels are smaller than 1 byte,
    // the Filter works byte per byte (byteWidth = 1)
    // precon is the previous unfiltered scanline, recon the result, scanline the current one
    // the incoming scanLines do NOT include the filtertype byte, that one is given in the parameter filterType instead
    // recon and scanline MAY be the same memory address! precon must be disjoint.

    unsigned int i;
    switch (filterType)
    {
    case 0:
        for (i = 0; i != len; i++) recon[i] = scanline[i];
        break;

    case 1:
        for (i = 0; i != byteWidth; i++) recon[i] = scanline[i];
        for (i = byteWidth; i != len; i++) recon[i] = scanline[i] + recon[i - byteWidth];
        break;

    case 2:
        if (precon)
        {
            for (i = 0; i != len; i++) recon[i] = scanline[i] + precon[i];
        }
        else
        {
            for (i = 0; i != len; i++) recon[i] = scanline[i];
        }
        break;

    case 3:
        if (precon)
        {
            for (i = 0; i != byteWidth; i++) recon[i] = scanline[i] + (precon[i] >> 1);
            for (i = byteWidth; i != len; i++) recon[i] = scanline[i] + ((recon[i - byteWidth] + precon[i]) >> 1);
        }
        else
        {
            for (i = 0; i != byteWidth; i++) recon[i] = scanline[i];
            for (i = byteWidth; i != len; i++) recon[i] = scanline[i] + (recon[i - byteWidth] >> 1);
        }
        break;

    case 4:
        if (precon)
        {
            for (i = 0; i != byteWidth; i++)
            {
                recon[i] = (scanline[i] + precon[i]); // PaethPredictor(0, precon[i], 0) is always precon[i]
            }
            for (i = byteWidth; i != len; i++)
            {
                recon[i] = (scanline[i] + PaethPredictor(recon[i - byteWidth], precon[i], precon[i - byteWidth]));
            }
        }
        else
        {
            for (i = 0; i != byteWidth; i++)
            {
                recon[i] = scanline[i];
            }
            for (i = byteWidth; i != len; i++)
            {
                // PaethPredictor(recon[i - byteWidth], 0, 0) is always recon[i - byteWidth]
                recon[i] = (scanline[i] + recon[i - byteWidth]);
            }
        }
        break;

    default: return 36; // error: unexisting Filter type given
    }

    return 0;
}

unsigned int Unfilter(unsigned char* out, const unsigned char* in, unsigned int w, unsigned int h, unsigned int bpp)
{
    // For PNG Filter method 0
    // this function unfilters a single data (e.g. without interlacing this is called once, with Adam7 seven times)
    // out must have enough bytes allocated already, in must have the scanLines + 1 filtertype byte per scanline
    // w and h are data dimensions or dimensions of reduced data, bpp is bits per pixel
    // in and out are allowed to be the same memory address (but aren't the same size since in has the extra Filter bytes)

    unsigned int y;
    unsigned char* prevLine = 0;

    // byteWidth is used for filtering, is 1 when bpp < 8, number of bytes per pixel otherwise
    unsigned int byteWidth = (bpp + 7) / 8;
    unsigned int lineBytes = (w * bpp + 7) / 8;

    for (y = 0; y < h; y++)
    {
        unsigned int outIndex = lineBytes * y;
        unsigned int inIndex = (1 + lineBytes) * y; // the extra filterbyte added to each row
        unsigned char filterType = in[inIndex];

        CERROR_TRY_RETURN(UnfilterScanline(&out[outIndex], &in[inIndex + 1], prevLine, byteWidth, filterType, lineBytes));

        prevLine = &out[outIndex];
    }

    return 0;
}

// in: Adam7 interlaced data, with no padding bits between scanLines, but between
// reduced images so that each reduced data starts at a byte.
// out: the same pixels, but re-ordered so that they're now a non-interlaced data with size w*h
// bpp: bits per pixel
// out has the following size in bits: w * h * bpp.
// in is possibly bigger due to padding bits between reduced images.
// out must be big enough AND must be 0 everywhere if bpp < 8 in the current implementation
// (because that's likely a little bit faster)
// NOTE: comments about padding bits are only relevant if bpp < 8

void Adam7Deinterlace(unsigned char* out, const unsigned char* in, unsigned int w, unsigned int h, unsigned int bpp)
{
    unsigned int i;
    unsigned int passw[7], passh[7];
    unsigned int fillterPassStart[8], paddedPassStart[8], passStart[8];

    Adam7GetPassValues(passw, passh, fillterPassStart, paddedPassStart, passStart, w, h, bpp);

    if (bpp >= 8)
    {
        for (i = 0; i != 7; i++)
        {
            unsigned int x, y, b;
            unsigned int byteWidth = bpp / 8;
            for (y = 0; y < passh[i]; y++)
            {
                for (x = 0; x < passw[i]; x++)
                {
                    unsigned int pixelInstart = passStart[i] + (y * passw[i] + x) * byteWidth;
                    unsigned int pixelOutstart = ((ADAM7_IY[i] + y * ADAM7_DY[i]) * w + ADAM7_IX[i] + x * ADAM7_DX[i]) * byteWidth;
                    for (b = 0; b < byteWidth; b++)
                    {
                        out[pixelOutstart + b] = in[pixelInstart + b];
                    }
                }
            }
        }
    }
    else // bpp < 8: Adam7 with pixels < 8 bit is a bit trickier: with bit pointers
    {
        for (i = 0; i != 7; i++)
        {
            unsigned int x, y, b;
            unsigned int inLineBits = bpp * passw[i];
            unsigned int outLineBits = bpp * w;
            unsigned int obp, ibp; // bit pointers (for out and in buffer)
            for (y = 0; y < passh[i]; y++)
            {
                for (x = 0; x < passw[i]; x++)
                { 
                    ibp = (8 * passStart[i]) + (y * inLineBits + x * bpp);
                    obp = (ADAM7_IY[i] + y * ADAM7_DY[i]) * outLineBits + (ADAM7_IX[i] + x * ADAM7_DX[i]) * bpp;
                    for (b = 0; b < bpp; b++)
                    {
                        unsigned char bit = ReadBitFromReversedStream(&ibp, in);
                        // note that this function assumes the out buffer is completely 0, use SetBitOfReversedStream otherwise
                        SetBitOfReversedStream0(&obp, out, bit);
                    }
                }
            }
        }
    }
}

void RemovePaddingBits(unsigned char* out, const unsigned char* in, unsigned int outLineBits, unsigned int inLineBits, unsigned int h)
{
    // After filtering there are still padding bits if scanLines have non multiple of 8 bit amounts. They need
    // to be removed (except at last scanline of (Adam7-reduced) data) before working with pure data buffers
    // for the Adam7 code, the colorMode convert code and the dst to the user.
    // in and out are allowed to be the same buffer, in may also be higher but still overlapping; in must
    // have >= inLineBits*h bits, out must have >= outLineBits*h bits, outLineBits must be <= inLineBits
    // also used to move bits after earlier such operations happened, e.g. in a sequence of reduced images from Adam7
    // only useful if (inLineBits - outLineBits) is a value in the range 1..7

    unsigned int x, y;
    unsigned int diff = inLineBits - outLineBits;
    unsigned int ibp = 0, obp = 0; // src and dst bit pointers

    for (y = 0; y < h; y++)
    {
        for (x = 0; x < outLineBits; x++)
        {
            unsigned char bit = ReadBitFromReversedStream(&ibp, in);
            SetBitOfReversedStream(&obp, out, bit);
        }

        ibp += diff;
    }
}

// out must be buffer big enough to contain full data, and in must contain the full decompressed data from
// the IDAT chunks (with Filter index bytes and possible padding bits)
// return value is error
unsigned int PostProcessScanlines(unsigned char* out, unsigned char* in, unsigned int w, unsigned int h, const PNGInfo* info)
{
    // This function converts the filtered-padded-interlaced data into pure 2D data buffer with the PNG's colorType.
    // Steps:
    // *) if no Adam7: 1) Unfilter 2) remove padding bits (= posible extra bits per scanline if bpp < 8)
    // *) if adam7: 1) 7x Unfilter 2) 7x remove padding bits 3) Adam7Deinterlace
    // NOTE: the in buffer will be overwritten with intermediate data!

    unsigned int bpp = PNGGetBPP(&info->colorMode);
    if (bpp == 0) return 31; // error: invalid colorType

    if (info->interlaceMethod == 0)
    {
        if (bpp < 8 && w * bpp != ((w * bpp + 7) / 8) * 8)
        {
            CERROR_TRY_RETURN(Unfilter(in, in, w, h, bpp));
            RemovePaddingBits(out, in, w * bpp, ((w * bpp + 7) / 8) * 8, h);
        }

        // we can immediatly Filter into the out buffer, no other steps needed
        else CERROR_TRY_RETURN(Unfilter(out, in, w, h, bpp));
    }
    else // interlaceMethod is 1 (Adam7)
    {
        unsigned int i;
        unsigned int passw[7], passh[7]; unsigned int fillterPassStart[8], paddedPassStart[8], passStart[8];

        Adam7GetPassValues(passw, passh, fillterPassStart, paddedPassStart, passStart, w, h, bpp);

        for (i = 0; i != 7; i++)
        {
            CERROR_TRY_RETURN(Unfilter(&in[paddedPassStart[i]], &in[fillterPassStart[i]], passw[i], passh[i], bpp));

            // TODO: possible efficiency improvement: if in this reduced data the bits fit nicely in 1 scanline,
            // move bytes instead of bits or move not at all
            if (bpp < 8)
            {
                // remove padding bits in scanLines; after this there still may be padding
                // bits between the different reduced images: each reduced data still starts nicely at a byte
                RemovePaddingBits(&in[passStart[i]], &in[paddedPassStart[i]], passw[i] * bpp, ((passw[i] * bpp + 7) / 8) * 8, passh[i]);
            }
        }

        Adam7Deinterlace(out, in, w, h, bpp);
    }

    return 0;
}

unsigned int ReadChunk_PLTE(PNGColorMode* colorMode, const unsigned char* data, unsigned int chunkLength)
{
    unsigned int pos = 0, i;
    if (colorMode->palette) free(colorMode->palette);

    colorMode->paletteSize = chunkLength / 3;
    colorMode->palette = (unsigned char*)malloc(4 * colorMode->paletteSize);

    if (!colorMode->palette && colorMode->paletteSize)
    {
        colorMode->paletteSize = 0;
        return 83;
    }

    if (colorMode->paletteSize > 256) return 38; // error: palette too big

    for (i = 0; i != colorMode->paletteSize; i++)
    {
        colorMode->palette[4 * i + 0] = data[pos++]; // R
        colorMode->palette[4 * i + 1] = data[pos++]; // g
        colorMode->palette[4 * i + 2] = data[pos++]; // b
        colorMode->palette[4 * i + 3] = 255; // alpha
    }

    return 0;
}

unsigned int ReadChunk_tRNS(PNGColorMode* colorMode, const unsigned char* data, unsigned int chunkLength)
{
    unsigned int i;

    if (colorMode->colorType == LCT_PALETTE)
    {
        // error: more alpha values given than there are palette entries
        if (chunkLength > colorMode->paletteSize) return 38;
        for (i = 0; i != chunkLength; i++) colorMode->palette[4 * i + 3] = data[i];
    }
    else if (colorMode->colorType == LCT_GREY)
    {
        // error: this chunk must be 2 bytes for greyscale data
        if (chunkLength != 2) return 30;
        colorMode->keyDefined = 1;
        colorMode->keyR = colorMode->keyG = colorMode->keyB = 256u * data[0] + data[1];
    }
    else if (colorMode->colorType == LCT_RGB)
    {
        // error: this chunk must be 6 bytes for RGB data
        if (chunkLength != 6) return 41;
        colorMode->keyDefined = 1;
        colorMode->keyR = 256u * data[0] + data[1];
        colorMode->keyG = 256u * data[2] + data[3];
        colorMode->keyB = 256u * data[4] + data[5];
    }
    else return 42; // error: tRNS chunk not allowed for other colorMode models

    return 0;
}

// background colorMode chunk (bKGD)
unsigned int ReadChunk_bKGD(PNGInfo* info, const unsigned char* data, unsigned int chunkLength)
{
    if (info->colorMode.colorType == LCT_PALETTE)
    {
        // error: this chunk must be 1 byte for indexed colorMode data
        if (chunkLength != 1) return 43;

        info->backgroundDefined = 1;
        info->backgroundR = info->backgroundG = info->backgroundB = data[0];
    }
    else if (info->colorMode.colorType == LCT_GREY || info->colorMode.colorType == LCT_GREY_ALPHA)
    {
        // error: this chunk must be 2 bytes for greyscale data
        if (chunkLength != 2) return 44;

        info->backgroundDefined = 1;
        info->backgroundR = info->backgroundG = info->backgroundB = 256u * data[0] + data[1];
    }
    else if (info->colorMode.colorType == LCT_RGB || info->colorMode.colorType == LCT_RGBA)
    {
        // error: this chunk must be 6 bytes for greyscale data
        if (chunkLength != 6) return 45;

        info->backgroundDefined = 1;
        info->backgroundR = 256u * data[0] + data[1];
        info->backgroundG = 256u * data[2] + data[3];
        info->backgroundB = 256u * data[4] + data[5];
    }

    return 0;
}

// text chunk (tEXt)
unsigned int ReadChunk_tEXt(PNGInfo* info, const unsigned char* data, unsigned int chunkLength)
{
    unsigned int i, error = 0;
    char *key = 0, *str = 0;

    while (!error) // not really a while loop, only used to break on error
    {
        unsigned int len, strBegin;

        len = 0;
        while (len < chunkLength && data[len] != 0) len++;

        // even though it's not allowed by the standard, no error is thrown if
        // there's no null termination char, if the text is empty
        if (len < 1 || len > 79) CERROR_BREAK(error, 89); // keyword too short or long

        key = (char*)malloc(len + 1);
        if (!key) CERROR_BREAK(error, 83); // alloc fail

        key[len] = 0;
        for (i = 0; i != len; i++) key[i] = (char)data[i];

        strBegin = len + 1; // skip keyword null terminator

        len = (unsigned int)(chunkLength < strBegin ? 0 : chunkLength - strBegin);
        str = (char*)malloc(len + 1);
        if (!str) CERROR_BREAK(error, 83); // alloc fail

        str[len] = 0;
        for (i = 0; i != len; i++) str[i] = (char)data[strBegin + i];

        error = PNGAddText(info, key, str);
        break;
    }

    free(key);
    free(str);

    return error;
}

// compressed text chunk (zTXt)
unsigned int ReadChunk_zTXt(PNGInfo* info, const PNGDecompressSettings* zlibSettings, const unsigned char* data, unsigned int chunkLength)
{
    unsigned int i, error = 0;
    unsigned int len, strBegin;
    char *key = 0;
    ucVector decoded;

    ucVectorInit(&decoded);

    while (!error) // not really a while loop, only used to break on error
    {
        for (len = 0; len < chunkLength && data[len] != 0; len++);

        if (len + 2 >= chunkLength) CERROR_BREAK(error, 75); // no null termination, corrupt?
        if (len < 1 || len > 79) CERROR_BREAK(error, 89); // keyword too short or long

        key = (char*)malloc(len + 1);
        if (!key) CERROR_BREAK(error, 83);

        key[len] = 0;
        for (i = 0; i != len; i++) key[i] = (char)data[i];

        if (data[len + 1] != 0) CERROR_BREAK(error, 72); // the 0 byte indicating compression must be 0

        strBegin = len + 2;
        if (strBegin > chunkLength) CERROR_BREAK(error, 75); // no null termination, corrupt?

        len = (unsigned int)chunkLength - strBegin;

        // will fail if zlib error, e.g. if len is too small
        error = ZlibDecompress(&decoded.data, &decoded.size, (unsigned char*)(&data[strBegin]), len, zlibSettings);
        if (error) break;

        ucVectorPushBack(&decoded, 0);
        error = PNGAddText(info, key, (char*)decoded.data);

        break;
    }

    free(key);
    ucVectorCleanup(&decoded);

    return error;
}

// international text chunk (iTXt)
unsigned int ReadChunk_iTXt(PNGInfo* info, const PNGDecompressSettings* zlibSettings, const unsigned char* data, unsigned int chunkLength)
{
    unsigned int i, error = 0;
    unsigned int len, begin, compressed;
    char *key = 0, *langTag = 0, *transKey = 0;

    ucVector decoded;
    ucVectorInit(&decoded);

    while (!error) // not really a while loop, only used to break on error
    {
        // Quick check if the chunk len isn't too small. Even without check
        // it'd still fail with other error checks below if it's too short. This just gives a different error code.
        if (chunkLength < 5) CERROR_BREAK(error, 30); // iTXt chunk too short

        // read the key
        for (len = 0; len < chunkLength && data[len] != 0; len++);

        if (len + 3 >= chunkLength) CERROR_BREAK(error, 75); // no null termination char, corrupt?
        if (len < 1 || len > 79) CERROR_BREAK(error, 89); // keyword too short or long

        key = (char*)malloc(len + 1);
        if (!key) CERROR_BREAK(error, 83); // alloc fail

        key[len] = 0;
        for (i = 0; i != len; i++) key[i] = (char)data[i];

        // read the compression method
        compressed = data[len + 1];
        if (data[len + 2] != 0) CERROR_BREAK(error, 72); // the 0 byte indicating compression must be 0

        // even though it's not allowed by the standard, no error is thrown if
        // there's no null termination char, if the text is empty for the next 3 texts

        // read the langTag
        begin = len + 3;
        len = 0;
        for (i = begin; i != chunkLength && data[i] != 0; i++) len++;

        langTag = (char*)malloc(len + 1);
        if (!langTag) CERROR_BREAK(error, 83);

        langTag[len] = 0;
        for (i = 0; i != len; i++) langTag[i] = (char)data[begin + i];

        // read the transKey
        begin += len + 1;
        len = 0;
        for (i = begin; i != chunkLength && data[i] != 0; i++) len++;

        transKey = (char*)malloc(len + 1);
        if (!transKey) CERROR_BREAK(error, 83);

        transKey[len] = 0;
        for (i = 0; i != len; i++) transKey[i] = (char)data[begin + i];

        // read the actual text
        begin += len + 1;

        len = (unsigned int)chunkLength < begin ? 0 : (unsigned int)chunkLength - begin;

        if (compressed)
        {
            // will fail if zlib error, e.g. if len is too small
            error = ZlibDecompress(&decoded.data, &decoded.size, (unsigned char*)(&data[begin]), len, zlibSettings);
            if (error) break;

            if (decoded.allocSize < decoded.size) decoded.allocSize = decoded.size;
            ucVectorPushBack(&decoded, 0);
        }
        else
        {
            if (!ucVectorResize(&decoded, len + 1)) CERROR_BREAK(error, 83);

            decoded.data[len] = 0;
            for (i = 0; i != len; i++) decoded.data[i] = data[begin + i];
        }

        error = PNGAddItext(info, key, langTag, transKey, (char*)decoded.data);
        break;
    }

    free(key);
    free(langTag);
    free(transKey);
    ucVectorCleanup(&decoded);

    return error;
}

unsigned int ReadChunk_tIME(PNGInfo* info, const unsigned char* data, unsigned int chunkLength)
{
    if (chunkLength != 7) return 73; // invalid tIME chunk size

    info->timeDefined = 1;
    info->time.year = 256u * data[0] + data[1];
    info->time.month = data[2];
    info->time.day = data[3];
    info->time.hour = data[4];
    info->time.minute = data[5];
    info->time.second = data[6];

    return 0;
}

unsigned int ReadChunk_pHYs(PNGInfo* info, const unsigned char* data, unsigned int chunkLength)
{
    if (chunkLength != 9) return 74; // invalid pHYs chunk size

    info->physDefined = 1;
    info->physX = 16777216u * data[0] + 65536u * data[1] + 256u * data[2] + data[3];
    info->physY = 16777216u * data[4] + 65536u * data[5] + 256u * data[6] + data[7];
    info->physUnit = data[8];

    return 0;
}

// read a PNG, the result will be in the same colorMode type as the PNG (hence "generic")
void DecodeGeneric(unsigned char** out, unsigned int* w, unsigned int* h, PNGState* state, const unsigned char* in, unsigned int inSize)
{
    unsigned char IEND = 0;
    const unsigned char* chunk;

    unsigned int i;
    ucVector idat; // the data from idat chunks
    ucVector scanLines;
    unsigned int predict;
    unsigned int outSize = 0;

    // for unknown chunk order
    unsigned int unknown = 0;
    unsigned int criticalPos = 1; // 1 = after IHDR, 2 = after PLTE, 3 = after IDAT

    // provide some proper dst values if error will happen
    *out = 0;

    state->error = PNGInspect(w, h, state, in, inSize); // reads header and resets other parameters in state->PNGInfo
    if (state->error) return;

    if (PNGPixelOverflow(*w, *h, &state->pngInfo.colorMode, &state->rawInfo))
    {
        CERROR_RETURN(state->error, 92); // overflow possible due to amount of pixels
    }

    ucVectorInit(&idat);
    chunk = &in[33]; // first byte of the first chunk after the header

    // loop through the chunks, ignoring unknown chunks and stopping at IEND chunk.
    // IDAT data is put at the start of the in buffer
    while (!IEND && !state->error)
    {
        unsigned int chunkLength;
        const unsigned char* data; // the data in the chunk

        // error: size of the in buffer too small to contain next chunk
        if ((unsigned int)((chunk - in) + 12) > inSize || chunk < in)
        {
            if (state->decoder.ignoreEnd) break; // other errors may still happen though
            CERROR_BREAK(state->error, 30);
        }
        // len of the data of the chunk, excluding the len bytes, chunk type and crc bytes
        chunkLength = PNGChunkLength(chunk);

        // error: chunk len larger than the max PNG chunk size
        if (chunkLength > 2147483647)
        {
            if (state->decoder.ignoreEnd) break; // other errors may still happen though
            CERROR_BREAK(state->error, 63);
        }       

        if ((unsigned int)((chunk - in) + chunkLength + 12) > inSize || (chunk + chunkLength + 12) < in)
        {
            CERROR_BREAK(state->error, 64); // error: size of the in buffer too small to contain next chunk
        }

        data = PNGChunkDataConst(chunk);

        // IDAT chunk, containing compressed data data
        if (PNGChunkTypeEquals(chunk, "IDAT"))
        {
            unsigned int oldSize = idat.size;
            unsigned int newSize;
              if (PNGAddOfL(oldSize, chunkLength, &newSize)) CERROR_BREAK(state->error, 95);
              if (!ucVectorResize(&idat, newSize)) CERROR_BREAK(state->error, 83); // alloc fail

            for (i = 0; i != chunkLength; i++) idat.data[oldSize + i] = data[i];
            criticalPos = 3;
        }

        // IEND chunk
        else if (PNGChunkTypeEquals(chunk, "IEND"))
        {
            IEND = 1;
        }
        // palette chunk (PLTE)
        else if(PNGChunkTypeEquals(chunk, "PLTE"))
        {
            state->error = ReadChunk_PLTE(&state->pngInfo.colorMode, data, chunkLength);
            if (state->error) break;
            criticalPos = 2;
        }
        // palette transparency chunk (tRNS)
        else if (PNGChunkTypeEquals(chunk, "tRNS"))
        {
            state->error = ReadChunk_tRNS(&state->pngInfo.colorMode, data, chunkLength);
            if (state->error) break;
        }
        // background colorMode chunk (bKGD)
        else if (PNGChunkTypeEquals(chunk, "bKGD"))
        {
            state->error = ReadChunk_bKGD(&state->pngInfo, data, chunkLength);
            if (state->error) break;
        }
        // text chunk (tEXt)
        else if (PNGChunkTypeEquals(chunk, "tEXt"))
        {
            if (state->decoder.readTextChunks)
            {
                state->error = ReadChunk_tEXt(&state->pngInfo, data, chunkLength);
                if (state->error) break;
            }
        }
        // compressed text chunk (zTXt)
        else if (PNGChunkTypeEquals(chunk, "zTXt"))
        {
            if (state->decoder.readTextChunks)
            {
                state->error = ReadChunk_zTXt(&state->pngInfo, &state->decoder.zlibSettings, data, chunkLength);
                if (state->error) break;
            }
        }
        // international text chunk (iTXt)
        else if (PNGChunkTypeEquals(chunk, "iTXt"))
        {
            if (state->decoder.readTextChunks)
            {
                state->error = ReadChunk_iTXt(&state->pngInfo, &state->decoder.zlibSettings, data, chunkLength);
                if (state->error) break;
            }
        }
        else if (PNGChunkTypeEquals(chunk, "tIME"))
        {
            state->error = ReadChunk_tIME(&state->pngInfo, data, chunkLength);
            if (state->error) break;
        }
        else if (PNGChunkTypeEquals(chunk, "pHYs"))
        {
            state->error = ReadChunk_pHYs(&state->pngInfo, data, chunkLength);
            if (state->error) break;
        }
        else // it's not an implemented chunk type, so ignore it: skip over the data
        {
            // error: unknown critical chunk (5th bit of first byte of chunk type is 0)
            if (!state->decoder.ignoreCritical && !PNGChunkAncillary(chunk))
            {
                CERROR_BREAK(state->error, 69);
            }
            unknown = 1;
            if (state->decoder.rememberUnknownChunks)
            {
                state->error = PNGChunkAppend(&state->pngInfo.unknownChunksData[criticalPos - 1], &state->pngInfo.unknownChunksSize[criticalPos - 1], chunk);
                if (state->error) break;
            }
        }

        if (!state->decoder.ignoreCRC && !unknown) // check crc if wanted, only on known chunk types
        {
            if (PNGChunkCheckCRC(chunk)) CERROR_BREAK(state->error, 57); // invalid crc
        }

        if (!IEND) chunk = PNGChunkNextConst(chunk);
    }

    ucVectorInit(&scanLines);

    // predict dst size, to allocate exact size for dst buffer to avoid more dynamic allocation.
    // The prediction is currently not correct for interlaced PNG images.
    if (state->pngInfo.interlaceMethod == 0)
    {
        // The extra *h is added because this are the filter bytes every scanline starts with
        predict = PNGGetRawSizeIDAT(*w, *h, &state->pngInfo.colorMode);
    }
    else
    {
        // Adam-7 interlaced: predicted size is the sum of the 7 sub-images sizes
        const PNGColorMode* color = &state->pngInfo.colorMode;
        predict = 0;
        predict += PNGGetRawSizeIDAT((*w + 7) >> 3, (*h + 7) >> 3, color);
        if (*w > 4) predict += PNGGetRawSizeIDAT((*w + 3) >> 3, (*h + 7) >> 3, color);
        predict += PNGGetRawSizeIDAT((*w + 3) >> 2, (*h + 3) >> 3, color);
        if (*w > 2) predict += PNGGetRawSizeIDAT((*w + 1) >> 2, (*h + 3) >> 2, color);
        predict += PNGGetRawSizeIDAT((*w + 1) >> 1, (*h + 1) >> 2, color);
        if (*w > 1) predict += PNGGetRawSizeIDAT((*w + 0) >> 1, (*h + 1) >> 1, color);
        predict += PNGGetRawSizeIDAT((*w + 0), (*h + 0) >> 1, color);
    }

    if (!state->error && !ucVectorReserve(&scanLines, predict)) state->error = 83;

    if (!state->error)
    {
        state->error = ZlibDecompress(&scanLines.data, &scanLines.size, idat.data, idat.size, &state->decoder.zlibSettings);
        if (!state->error && scanLines.size != predict) state->error = 91; // decompressed size doesn't match prediction
    }

    ucVectorCleanup(&idat);

    if (!state->error)
    {
        outSize = PNGGetRawSize(*w, *h, &state->pngInfo.colorMode);
        *out = (unsigned char*)malloc(outSize);
        if (!*out) state->error = 83;
    }

    if (!state->error)
    {
        for (i = 0; i != outSize; i++) (*out)[i] = 0;
        state->error = PostProcessScanlines(*out, scanLines.data, *w, *h, &state->pngInfo);
    }

    ucVectorCleanup(&scanLines);
}

void PNGDecoderSettingsInit(PNGDecoderSettings* settings)
{
    settings->colorConvert = 1;
    settings->readTextChunks = 1;
    settings->rememberUnknownChunks = 0;
    settings->ignoreCRC = 0;
    settings->ignoreCritical = 0;
    settings->ignoreEnd = 0;
    PNGDecompressSettingsInit(&settings->zlibSettings);
}

void PNGCompressSettingsInit(PNGCompressSettings* settings)
{
    // compress with dynamic huffman tree (not in the mathematical sense, just not the predefined one)
    settings->type = 2;
    settings->useLZ77 = 1;
    settings->windowSize = DEFAULT_WINDOWSIZE;
    settings->minMatch = 3;
    settings->niceMatch = 128;
    settings->lazyMatching = 1;

    settings->customZlib = 0;
    settings->CustomDeflate = 0;
    settings->customContext = 0;
}

void PNGEncoderSettingsInit(PNGEncoderSettings* settings)
{
    PNGCompressSettingsInit(&settings->zlibSettings);
    settings->fillterPaletteZero = 1;
    settings->fillterStrategy = LFS_MINSUM;
    settings->autoConvert = 1;
    settings->forcePalette = 0;
    settings->predefinedFilters = 0;
    settings->addID = 0;
    settings->textCompression = 1;
}

void PNGStateInit(PNGState* state)
{
    PNGDecoderSettingsInit(&state->decoder);
    PNGEncoderSettingsInit(&state->encoder);
    PNGColorModeInit(&state->rawInfo);
    PNGInfoInit(&state->pngInfo);
    state->error = 1;
}

void PNGStateCleanup(PNGState* state)
{
    PNGColorModeCleanup(&state->rawInfo);
    PNGInfoCleanup(&state->pngInfo);
}

//////////////////
// PNG encoder  // 
//////////////////
// chunkName must be string of 4 characters
unsigned int AddChunk(ucVector* out, const char* chunkName, const unsigned char* data, unsigned int len)
{
    CERROR_TRY_RETURN(PNGChunkCreate(&out->data, &out->size, (unsigned int)len, chunkName, data));
    out->allocSize = out->size; // fix the allocSize again
    return 0;
}

void WriteSignature(ucVector* out)
{
    // 8 bytes PNG signature, aka the magic bytes
    ucVectorPushBack(out, 137);
    ucVectorPushBack(out, 80);
    ucVectorPushBack(out, 78);
    ucVectorPushBack(out, 71);
    ucVectorPushBack(out, 13);
    ucVectorPushBack(out, 10);
    ucVectorPushBack(out, 26);
    ucVectorPushBack(out, 10);
}

unsigned int AddChunk_IHDR(ucVector* out, unsigned int w, unsigned int h, PNGColorType colorType, unsigned int bitDepth, unsigned int interlaceMethod)
{
    unsigned int error = 0;
    ucVector header;
    ucVectorInit(&header);

    PNGAdd32bitInt(&header, w); // width
    PNGAdd32bitInt(&header, h); // bmHeight
    ucVectorPushBack(&header, (unsigned char)bitDepth); // bit depth
    ucVectorPushBack(&header, (unsigned char)colorType); // colorMode type
    ucVectorPushBack(&header, 0); // compression method
    ucVectorPushBack(&header, 0); // Filter method
    ucVectorPushBack(&header, (unsigned char)interlaceMethod); // interlace method

    error = AddChunk(out, "IHDR", header.data, header.size);
    ucVectorCleanup(&header);

    return error;
}

unsigned int AddChunk_PLTE(ucVector* out, const PNGColorMode* info)
{
    unsigned int i, error = 0;
    ucVector PLTE;
    ucVectorInit(&PLTE);

    for (i = 0; i != info->paletteSize * 4; i++)
    {
        // add all channels except alpha channel
        if (i % 4 != 3) ucVectorPushBack(&PLTE, info->palette[i]);
    }

    error = AddChunk(out, "PLTE", PLTE.data, PLTE.size);
    ucVectorCleanup(&PLTE);

    return error;
}

unsigned int AddChunk_tRNS(ucVector* out, const PNGColorMode* info)
{
    unsigned int i, error = 0;
    ucVector tRNS;
    ucVectorInit(&tRNS);

    if (info->colorType == LCT_PALETTE)
    {
        unsigned int amount = info->paletteSize;

        // the tail of palette values that all have 255 as alpha, does not have to be encoded
        for (i = info->paletteSize; i > 0; i--)
        {
            if (info->palette[4 * (i - 1) + 3] == 255) amount--;
            else break;
        }

        // add only alpha channel
        for (i = 0; i != amount; i++) ucVectorPushBack(&tRNS, info->palette[4 * i + 3]);
    }
    else if (info->colorType == LCT_GREY)
    {
        if (info->keyDefined)
        {
            ucVectorPushBack(&tRNS, (unsigned char)(info->keyR >> 8));
            ucVectorPushBack(&tRNS, (unsigned char)(info->keyR & 255));
        }
    }
    else if (info->colorType == LCT_RGB)
    {
        if (info->keyDefined)
        {
            ucVectorPushBack(&tRNS, (unsigned char)(info->keyR >> 8));
            ucVectorPushBack(&tRNS, (unsigned char)(info->keyR & 255));
            ucVectorPushBack(&tRNS, (unsigned char)(info->keyG >> 8));
            ucVectorPushBack(&tRNS, (unsigned char)(info->keyG & 255));
            ucVectorPushBack(&tRNS, (unsigned char)(info->keyB >> 8));
            ucVectorPushBack(&tRNS, (unsigned char)(info->keyB & 255));
        }
    }

    error = AddChunk(out, "tRNS", tRNS.data, tRNS.size);
    ucVectorCleanup(&tRNS);

    return error;
}

unsigned int AddChunk_IDAT(ucVector* out, const unsigned char* data, unsigned int dataSize, PNGCompressSettings* zlibSettings)
{
    ucVector zlibdata;
    unsigned int error = 0;

    // compress with the Zlib compressor
    ucVectorInit(&zlibdata);
    error = ZlibCompress(&zlibdata.data, &zlibdata.size, data, dataSize, zlibSettings);
    if (!error) error = AddChunk(out, "IDAT", zlibdata.data, zlibdata.size);

    ucVectorCleanup(&zlibdata);

    return error;
}

unsigned int AddChunk_IEND(ucVector* out)
{
    unsigned int error = 0;
    error = AddChunk(out, "IEND", 0, 0);
    return error;
}

unsigned int AddChunk_tEXt(ucVector* out, const char* keyword, const char* textstring)
{
    unsigned int i, error = 0;
    ucVector text;

    ucVectorInit(&text);
    for (i = 0; keyword[i] != 0; i++) ucVectorPushBack(&text, (unsigned char)keyword[i]);

    if (i < 1 || i > 79) return 89; // error: invalid keyword size

    ucVectorPushBack(&text, 0); // 0 termination char

    for (i = 0; textstring[i] != 0; i++) ucVectorPushBack(&text, (unsigned char)textstring[i]);

    error = AddChunk(out, "tEXt", text.data, text.size);
    ucVectorCleanup(&text);

    return error;
}

unsigned int AddChunk_zTXt(ucVector* out, const char* keyword, const char* textstring, PNGCompressSettings* zlibSettings)
{
    ucVector data, compressed;
    unsigned int i, textsize = strlen(textstring), error = 0;

    ucVectorInit(&data);
    ucVectorInit(&compressed);

    for (i = 0; keyword[i] != 0; i++) ucVectorPushBack(&data, (unsigned char)keyword[i]);

    if (i < 1 || i > 79) return 89; // error: invalid keyword size

    ucVectorPushBack(&data, 0); // 0 termination char
    ucVectorPushBack(&data, 0); // compression method: 0

    error = ZlibCompress(&compressed.data, &compressed.size, (unsigned char*)textstring, textsize, zlibSettings);
    if (!error)
    {
        for (i = 0; i != compressed.size; i++) ucVectorPushBack(&data, compressed.data[i]);
        error = AddChunk(out, "zTXt", data.data, data.size);
    }

    ucVectorCleanup(&compressed);
    ucVectorCleanup(&data);
    return error;
}

unsigned int AddChunk_iTXt(ucVector* out, unsigned int compressed, const char* keyword, const char* langTag, const char* transKey, const char* textstring, PNGCompressSettings* zlibSettings)
{
    ucVector data;
    unsigned int i, textsize = strlen(textstring), error = 0;

    ucVectorInit(&data);

    for (i = 0; keyword[i] != 0; i++) ucVectorPushBack(&data, (unsigned char)keyword[i]);
    if (i < 1 || i > 79) return 89; // error: invalid keyword size

    ucVectorPushBack(&data, 0); // null termination char
    ucVectorPushBack(&data, compressed ? 1 : 0); // compression flag
    ucVectorPushBack(&data, 0); // compression method

    for (i = 0; langTag[i] != 0; i++) ucVectorPushBack(&data, (unsigned char)langTag[i]);
    ucVectorPushBack(&data, 0); // null termination char

    for (i = 0; transKey[i] != 0; i++) ucVectorPushBack(&data, (unsigned char)transKey[i]);
    ucVectorPushBack(&data, 0); // null termination char

    if (compressed)
    {
        ucVector compressed_data;
        ucVectorInit(&compressed_data);
        error = ZlibCompress(&compressed_data.data, &compressed_data.size, (unsigned char*)textstring, textsize, zlibSettings);
        if (!error)
        {
            for (i = 0; i != compressed_data.size; i++) ucVectorPushBack(&data, compressed_data.data[i]);
        }
        ucVectorCleanup(&compressed_data);
    }
    else // not compressed
    {
        for (i = 0; textstring[i] != 0; i++) ucVectorPushBack(&data, (unsigned char)textstring[i]);
    }

    if (!error) error = AddChunk(out, "iTXt", data.data, data.size);
    ucVectorCleanup(&data);

    return error;
}

unsigned int AddChunk_bKGD(ucVector* out, const PNGInfo* info)
{
    unsigned int error = 0;
    ucVector bKGD;
    ucVectorInit(&bKGD);

    if (info->colorMode.colorType == LCT_GREY || info->colorMode.colorType == LCT_GREY_ALPHA)
    {
        ucVectorPushBack(&bKGD, (unsigned char)(info->backgroundR >> 8));
        ucVectorPushBack(&bKGD, (unsigned char)(info->backgroundR & 255));
    }
    else if (info->colorMode.colorType == LCT_RGB || info->colorMode.colorType == LCT_RGBA)
    {
        ucVectorPushBack(&bKGD, (unsigned char)(info->backgroundR >> 8));
        ucVectorPushBack(&bKGD, (unsigned char)(info->backgroundR & 255));
        ucVectorPushBack(&bKGD, (unsigned char)(info->backgroundG >> 8));
        ucVectorPushBack(&bKGD, (unsigned char)(info->backgroundG & 255));
        ucVectorPushBack(&bKGD, (unsigned char)(info->backgroundB >> 8));
        ucVectorPushBack(&bKGD, (unsigned char)(info->backgroundB & 255));
    }
    else if (info->colorMode.colorType == LCT_PALETTE)
    {
        ucVectorPushBack(&bKGD, (unsigned char)(info->backgroundR & 255)); // palette index
    }

    error = AddChunk(out, "bKGD", bKGD.data, bKGD.size);
    ucVectorCleanup(&bKGD);

    return error;
}

unsigned int AddChunk_tIME(ucVector* out, const PNGTime* time)
{
    unsigned int error = 0;
    unsigned char data[7];

    data[0] = (unsigned char)(time->year >> 8);
    data[1] = (unsigned char)(time->year & 255);
    data[2] = (unsigned char)time->month;
    data[3] = (unsigned char)time->day;
    data[4] = (unsigned char)time->hour;
    data[5] = (unsigned char)time->minute;
    data[6] = (unsigned char)time->second;

    error = AddChunk(out, "tIME", data, 7);
    return error;
}

unsigned int AddChunk_pHYs(ucVector* out, const PNGInfo* info)
{
    unsigned int error = 0;
    ucVector data;
    ucVectorInit(&data);

    PNGAdd32bitInt(&data, info->physX);
    PNGAdd32bitInt(&data, info->physY);
    ucVectorPushBack(&data, (unsigned char)info->physUnit);

    error = AddChunk(out, "pHYs", data.data, data.size);
    ucVectorCleanup(&data);

    return error;
}

void FilterScanline(unsigned char* out, const unsigned char* scanline, const unsigned char* prevLine, unsigned int len, unsigned int byteWidth, unsigned char filterType)
{
    unsigned int i;
    switch(filterType)
    {
    case 0: // None
        for (i = 0; i != len; i++) out[i] = scanline[i];
        break;

    case 1: // Sub
        if (prevLine)
        {
            for (i = 0; i != byteWidth; i++) out[i] = scanline[i];
            for (i = byteWidth; i != len; i++) out[i] = scanline[i] - scanline[i - byteWidth];
        }
        else
        {
            for (i = 0; i != byteWidth; i++) out[i] = scanline[i];
            for (i = byteWidth; i != len; i++) out[i] = scanline[i] - scanline[i - byteWidth];
        }
        break;

    case 2: // Up
        if (prevLine)
        {
            for (i = 0; i != len; i++) out[i] = scanline[i] - prevLine[i];
        }
        else
        {
            for (i = 0; i != len; i++) out[i] = scanline[i];
        }
        break;

    case 3: // Average
        if (prevLine)
        {
            for (i = 0; i != byteWidth; i++) out[i] = scanline[i] - (prevLine[i] >> 1);
            for (i = byteWidth; i != len; i++) out[i] = scanline[i] - ((scanline[i - byteWidth] + prevLine[i]) >> 1);
        }
        else
        {
            for (i = 0; i != byteWidth; i++) out[i] = scanline[i];
            for (i = byteWidth; i != len; i++) out[i] = scanline[i] - (scanline[i - byteWidth] >> 1);
        }
        break;

    case 4: // Paeth
        if (prevLine)
        {
            // PaethPredictor(0, prevLine[i], 0) is always prevLine[i]
            for (i = 0; i != byteWidth; i++) out[i] = (scanline[i] - prevLine[i]);
            for (i = byteWidth; i != len; i++)
            {
                out[i] = (scanline[i] - PaethPredictor(scanline[i - byteWidth], prevLine[i], prevLine[i - byteWidth]));
            }
        }
        else
        {
            for (i = 0; i != byteWidth; i++) out[i] = scanline[i];
            // PaethPredictor(scanline[i - byteWidth], 0, 0) is always scanline[i - byteWidth]
            for (i = byteWidth; i != len; i++) out[i] = (scanline[i] - scanline[i - byteWidth]);
        }
        break;

        default: return; // unexisting Filter type given
    }
}

// log2 approximation. A slight bit faster than std::log. 
float flog2(float f)
{
    float result = 0.0;
    while (f > 32) { result += 4; f /= 16; }
    while (f > 2) { result++; f /= 2; }
    return result + 1.442695f * (f * f * f / 3 - 3 * f * f / 2 + 3 * f - 1.83333f);
}

unsigned int Filter(unsigned char* out, const unsigned char* in, unsigned int w, unsigned int h, const PNGColorMode* info, const PNGEncoderSettings* settings)
{
    // For PNG Filter method 0
    // out must be a buffer with as size: h + (w * h * bpp + 7) / 8, because there are
    // the scanLines with 1 extra byte per scanline
    unsigned int bpp = PNGGetBPP(info), error = 0;

    // the width of a scanline in bytes, not including the Filter type
    unsigned int lineBytes = (w * bpp + 7) / 8;

    // byteWidth is used for filtering, is 1 when bpp < 8, number of bytes per pixel otherwise
    unsigned int byteWidth = (bpp + 7) / 8;
    const unsigned char* prevLine = 0;
    unsigned int x, y;

    PNGFilterStrategy strategy = settings->fillterStrategy;

    // There is a heuristic called the minimum sum of absolute differences heuristic, suggested by the PNG standard:
    // * If the data type is palette, or the bit depth is smaller than 8, then do not Filter the data (i.e.
    //  use fixed filtering, with the Filter None).
    // * (The other case) If the data type is Grayscale or RGB (with or without alpha), and the bit depth is
    //  not smaller than 8, then use adaptive filtering heuristic as follows: independently for each row, apply
    //  all five filters and select the Filter that produces the smallest sum of absolute values per row.
    // This heuristic is used if Filter strategy is LFS_MINSUM and fillterPaletteZero is true.

    // If fillterPaletteZero is true and fillterStrategy is not LFS_MINSUM, the above heuristic is followed,
    // but for "the other case", whatever strategy fillterStrategy is set to instead of the minimum sum
    // heuristic is used.
    if (settings->fillterPaletteZero && (info->colorType == LCT_PALETTE || info->bitDepth < 8)) strategy = LFS_ZERO;

    if (bpp == 0) return 31; // error: invalid colorMode type

    if (strategy == LFS_ZERO)
    {
        for (y = 0; y < h; y++)
        {
            unsigned int outIndex = (1 + lineBytes) * y; // the extra filterbyte added to each row
            unsigned int inIndex = lineBytes * y;
            out[outIndex] = 0; // Filter type byte
            FilterScanline(&out[outIndex + 1], &in[inIndex], prevLine, lineBytes, byteWidth, 0);
            prevLine = &in[inIndex];
        }
    }
    else if (strategy == LFS_MINSUM)
    {
        // adaptive filtering
        unsigned int sum[5];
        unsigned char* attempt[5]; // five filtering attempts, one for each Filter type
        unsigned int smallest = 0;
        unsigned char type, bestType = 0;

        for (type = 0; type != 5; type++)
        {
            attempt[type] = (unsigned char*)malloc(lineBytes);
            if (!attempt[type]) return 83;
        }

        if (!error)
        {
            for (y = 0; y < h; y++)
            {
                // try the 5 Filter types
                for (type = 0; type != 5; type++)
                {
                    FilterScanline(attempt[type], &in[y * lineBytes], prevLine, lineBytes, byteWidth, type);

                    // calculate the sum of the result
                    sum[type] = 0;
                    if (type == 0)
                    {
                        for (x = 0; x != lineBytes; x++) sum[type] += (unsigned char)(attempt[type][x]);
                    }
                    else
                    {
                        for (x = 0; x != lineBytes; x++)
                        {
                            // For differences, each byte should be treated as signed, values above 127 are negative
                            // (converted to signed char). Filtertype 0 isn't a difference though, so use unsigned int there.
                            // This means filtertype 0 is almost never chosen, but that is justified.
                            unsigned char s = attempt[type][x];
                            sum[type] += s < 128 ? s : (255U - s);
                        }
                    }

                    // check if this is smallest sum (or if type == 0 it's the first case so always store the values)
                    if (type == 0 || sum[type] < smallest)
                    {
                        bestType = type;
                        smallest = sum[type];
                    }
                }

                prevLine = &in[y * lineBytes];

                // now fill the out values
                out[y * (lineBytes + 1)] = bestType; // the first byte of a scanline will be the Filter type
                for (x = 0; x < lineBytes; x++) out[y * (lineBytes + 1) + 1 + x] = attempt[bestType][x];
            }
        }

        for (type = 0; type != 5; ++type) free(attempt[type]);
    }
    else if(strategy == LFS_ENTROPY)
    {
        float sum[5], smallest = 0.0;
        unsigned char* attempt[5]; // five filtering attempts, one for each Filter type
        unsigned int type, bestType = 0;
        unsigned int count[256];

        for (type = 0; type < 5; type++)
        {
            attempt[type] = (unsigned char*)malloc(lineBytes);
            if (!attempt[type]) return 83;  
        }

        for (y = 0; y != h; y++)
        {
            // try the 5 Filter types
            for (type = 0; type != 5; type++)
            {
                FilterScanline(attempt[type], &in[y * lineBytes], prevLine, lineBytes, byteWidth, (unsigned char)type);
                for (x = 0; x != 256; x++) count[x] = 0;
                for (x = 0; x != lineBytes; x++) count[attempt[type][x]]++;

                count[type]++; // the Filter type itself is part of the scanline
                sum[type] = 0;

                for (x = 0; x != 256; x++)
                {
                    float p = count[x] / (float)(lineBytes + 1);
                    sum[type] += count[x] == 0 ? 0 : flog2(1 / p) * p;
                }

                // check if this is smallest sum (or if type == 0 it's the first case so always store the values)
                if (type == 0 || sum[type] < smallest)
                {
                    bestType = type;
                    smallest = sum[type];
                }
            }

            prevLine = &in[y * lineBytes];

            // now fill the out values
            out[y * (lineBytes + 1)] = bestType; // the first byte of a scanline will be the Filter type
            for (x = 0; x != lineBytes; x++) out[y * (lineBytes + 1) + 1 + x] = attempt[bestType][x];
        }

        for (type = 0; type != 5; type++) free(attempt[type]);
    }
    else if (strategy == LFS_PREDEFINED)
    {
        for (y = 0; y != h; y++)
        {
            unsigned int outIndex = (1 + lineBytes) * y; // the extra filterbyte added to each row
            unsigned int inIndex = lineBytes * y;
            unsigned char type = settings->predefinedFilters[y];
            out[outIndex] = type; // Filter type byte
            FilterScanline(&out[outIndex + 1], &in[inIndex], prevLine, lineBytes, byteWidth, type);
            prevLine = &in[inIndex];
        }
    }
    else if (strategy == LFS_BRUTE_FORCE)
    {
        // brute force Filter chooser.
        // Deflate the scanline after every Filter attempt to see which one deflates best.
        // This is very slow and gives only slightly smaller, sometimes even larger, result
        unsigned int size[5];
        unsigned char* attempt[5]; // five filtering attempts, one for each Filter type
        unsigned int smallest = 0;
        unsigned int type = 0, bestType = 0;
        unsigned char* i;
        PNGCompressSettings zlibSettings = settings->zlibSettings;

        // use fixed tree on the attempts so that the tree is not adapted to the filtertype on purpose,
        // to simulate the true case where the tree is the same for the whole data. Sometimes it gives
        // better result with dynamic tree anyway. Using the fixed tree sometimes gives worse, but in rare
        // cases better compression. It does make this a bit less slow, so it's worth doing this.
        zlibSettings.type = 1;

        // a custom encoder likely doesn't read the type setting and is optimized for complete PNG
        // images only, so disable it
        zlibSettings.customZlib = 0;
        zlibSettings.CustomDeflate = 0;

        for (type = 0; type != 5; type++)
        {
            attempt[type] = (unsigned char*)malloc(lineBytes);
            if (!attempt[type]) return 83;
        }

        for (y = 0; y != h; y++) // try the 5 Filter types
        {
            for (type = 0; type != 5; type++)
            {
                unsigned int testSize = lineBytes;

                FilterScanline(attempt[type], &in[y * lineBytes], prevLine, lineBytes, byteWidth, type);

                size[type] = 0;
                i = 0;

                ZlibCompress(&i, &size[type], attempt[type], testSize, &zlibSettings);
                free(i);

                // check if this is smallest size (or if type == 0 it's the first case so always store the values)
                if(type == 0 || size[type] < smallest)
                {
                    bestType = type;
                    smallest = size[type];
                }
            }

            prevLine = &in[y * lineBytes];
            out[y * (lineBytes + 1)] = bestType; // the first byte of a scanline will be the Filter type

            for (x = 0; x != lineBytes; x++) out[y * (lineBytes + 1) + 1 + x] = attempt[bestType][x];
        }

        for (type = 0; type != 5; type++) free(attempt[type]);
    }
    else return 88; //  unknown Filter strategy 

    return error;
}

void AddPaddingBits(unsigned char* out, const unsigned char* in, unsigned int outLineBits, unsigned int inLineBits, unsigned int h)
{
    // The opposite of the RemovePaddingBits function
    // outLineBits must be >= inLineBits
    unsigned int y;
    unsigned int diff = outLineBits - inLineBits;
    unsigned int obp = 0, ibp = 0; // bit pointers

    for (y = 0; y != h; y++)
    {
        unsigned int x;
        for (x = 0; x < inLineBits; x++)
        {
            unsigned char bit = ReadBitFromReversedStream(&ibp, in);
            SetBitOfReversedStream(&obp, out, bit);
        }

        // obp += diff; --> no, fill in some value in the padding bits too, to avoid
        // "Use of uninitialised value of size ###" warning from valgrind
        for (x = 0; x != diff; x++) SetBitOfReversedStream(&obp, out, 0);
    }
}

// in: non-interlaced data with size w*h
// out: the same pixels, but re-ordered according to PNG's Adam7 interlacing, with
// no padding bits between scanLines, but between reduced images so that each
// reduced data starts at a byte.
// bpp: bits per pixel
// there are no padding bits, not between scanLines, not between reduced images
// in has the following size in bits: w * h * bpp.
// out is possibly bigger due to padding bits between reduced images
// NOTE: comments about padding bits are only relevant if bpp < 8
void Adam7Interlace(unsigned char* out, const unsigned char* in, unsigned int w, unsigned int h, unsigned int bpp)
{
    unsigned int i;
    unsigned int passw[7], passh[7];
    unsigned int fillterPassStart[8], paddedPassStart[8], passStart[8];

    Adam7GetPassValues(passw, passh, fillterPassStart, paddedPassStart, passStart, w, h, bpp);

    if (bpp >= 8)
    {
        for (i = 0; i != 7; i++)
        {
            unsigned int x, y, b;
            unsigned int byteWidth = bpp >> 3;
            for (y = 0; y < passh[i]; y++)
            {
                for (x = 0; x < passw[i]; x++)
                {
                    unsigned int pixelInstart = ((ADAM7_IY[i] + y * ADAM7_DY[i]) * w + ADAM7_IX[i] + x * ADAM7_DX[i]) * byteWidth;
                    unsigned int pixelOutstart = passStart[i] + (y * passw[i] + x) * byteWidth;

                    for (b = 0; b < byteWidth; b++)
                    {
                        out[pixelOutstart + b] = in[pixelInstart + b];
                    }
                }
            }
        }
    }
    else // bpp < 8: Adam7 with pixels < 8 bit is a bit trickier: with bit pointers
    {
        for (i = 0; i != 7; i++)
        {
            unsigned int x, y, b;
            unsigned int inLineBits = bpp * passw[i];
            unsigned int outLineBits = bpp * w;
            unsigned int obp, ibp; // bit pointers (for out and in buffer)

            for (y = 0; y < passh[i]; y++)
            {
                for (x = 0; x < passw[i]; x++)
                {
                    ibp = (ADAM7_IY[i] + y * ADAM7_DY[i]) * outLineBits + (ADAM7_IX[i] + x * ADAM7_DX[i]) * bpp;
                    obp = (8 * passStart[i]) + (y * inLineBits + x * bpp);

                    for (b = 0; b < bpp; b++)
                    {
                        unsigned char bit = ReadBitFromReversedStream(&ibp, in);
                        SetBitOfReversedStream(&obp, out, bit);
                    }
                }
            }
        }
    }
}

// out must be buffer big enough to contain uncompressed IDAT chunk data, and in must contain the full data.
// return value is error*
unsigned int PreProcessScanlines(unsigned char** out, unsigned int* outSize, const unsigned char* in, unsigned int w, unsigned int h, const PNGInfo* info, const PNGEncoderSettings* settings)
{
    // This function converts the pure 2D data with the PNG's colorType, into filtered-padded-interlaced data. Steps:
    // *) if no Adam7: 1) add padding bits (= posible extra bits per scanline if bpp < 8) 2) Filter
    // *) if adam7: 1) Adam7Interlace 2) 7x add padding bits 3) 7x Filter
    unsigned int error = 0;
    unsigned int bpp = PNGGetBPP(&info->colorMode);

    if (info->interlaceMethod == 0)
    {
        *outSize = h + (h * ((w * bpp + 7) / 8)); // data size plus an extra byte per scanline + possible padding bits

        *out = (unsigned char*)malloc(*outSize);
        if(!(*out) && (*outSize)) error = 83;

        if (!error)
        {
            // non multiple of 8 bits per scanline, padding bits needed per scanline
            if (bpp < 8 && w * bpp != ((w * bpp + 7) / 8) * 8)
            {
                unsigned char* padded = (unsigned char*)malloc(h * ((w * bpp + 7) / 8));
                if (!padded) error = 83;

                if (!error)
                {
                    AddPaddingBits(padded, in, ((w * bpp + 7) / 8) * 8, w * bpp, h);
                    error = Filter(*out, padded, w, h, &info->colorMode, settings);
                }

                free(padded);
            }
            else
            {
                // we can immediatly Filter into the out buffer, no other steps needed
                error = Filter(*out, in, w, h, &info->colorMode, settings);
            }
        }
    }
    else // interlaceMethod is 1 (Adam7)
    {
        unsigned int passw[7], passh[7];
        unsigned int fillterPassStart[8], paddedPassStart[8], passStart[8];
        unsigned char* adam7;

        Adam7GetPassValues(passw, passh, fillterPassStart, paddedPassStart, passStart, w, h, bpp);

        *outSize = fillterPassStart[7]; // data size plus an extra byte per scanline + possible padding bits
        *out = (unsigned char*)malloc(*outSize);
        if (!(*out)) error = 83;

        adam7 = (unsigned char*)malloc(passStart[7]);
        if (!adam7 && passStart[7]) error = 83;

        if (!error)
        {
            int i;

            Adam7Interlace(adam7, in, w, h, bpp);
            for (i = 0; i != 7; i++)
            {
                if (bpp < 8)
                {
                    unsigned char* padded = (unsigned char*)malloc(paddedPassStart[i + 1] - paddedPassStart[i]);
                    if (!padded) ERROR_BREAK(83);
                    AddPaddingBits(padded, &adam7[passStart[i]], ((passw[i] * bpp + 7) / 8) * 8, passw[i] * bpp, passh[i]);
                    error = Filter(&(*out)[fillterPassStart[i]], padded, passw[i], passh[i], &info->colorMode, settings);
                    free(padded);
                }
                else
                {
                    error = Filter(&(*out)[fillterPassStart[i]], &adam7[paddedPassStart[i]], passw[i], passh[i], &info->colorMode, settings);
                }
                if (error) break;
            }
        }

        free(adam7);
    }

    return error;
}

// palette must have 4 * size bytes allocated, and given in format RGBARGBARGBARGBA...
// returns 0 if the palette is opaque,
// returns 1 if the palette has a single colorMode with alpha 0 ==> colorMode key
// returns 2 if the palette is semi-translucent.
unsigned int GetPaletteTranslucency(const unsigned char* palette, unsigned int size)
{
    unsigned int i;
    unsigned int key = 0;
    unsigned int r = 0, g = 0, b = 0; // the value of the colorMode with alpha 0, so long as colorMode keying is possible

    for (i = 0; i != size; i++)
    {
        if (!key && palette[4 * i + 3] == 0)
        {
            r = palette[4 * i + 0]; g = palette[4 * i + 1]; b = palette[4 * i + 2];
            key = 1;
            i = (unsigned int)(-1); // restart from beginning, to detect earlier opaque colors with key's value
        }
        else if (palette[4 * i + 3] != 255) return 2;

        // when key, no opaque RGB may have key's RGB
        else if (key && r == palette[i * 4 + 0] && g == palette[i * 4 + 1] && b == palette[i * 4 + 2]) return 2;
    }

    return key;
}

unsigned int AddUnknownChunks(ucVector* out, unsigned char* data, unsigned int dataSize)
{
    unsigned char* inchunk = data;
    while ((unsigned int)(inchunk - data) < dataSize)
    {
        CERROR_TRY_RETURN(PNGChunkAppend(&out->data, &out->size, inchunk));
        out->allocSize = out->size; // fix the allocSize again
        inchunk = PNGChunkNext(inchunk);
    }
    return 0;
}

unsigned int PNGDecode(unsigned char** out, unsigned int* w, unsigned int* h, PNGState* state, const unsigned char* in, unsigned int inSize)
{
    *out = 0;
    DecodeGeneric(out, w, h, state, in, inSize);

    if (state->error) return state->error;

    if (!state->decoder.colorConvert || PNGColorModeEqual(&state->rawInfo, &state->pngInfo.colorMode))
    {
        // same colorMode type, no copying or converting of data needed
        // store the info colorMode settings on the rawInfo so that the rawInfo still reflects what colorType
        // the raw data has to the end user
        if (!state->decoder.colorConvert)
        {
            state->error = PNGColorModeCopy(&state->rawInfo, &state->pngInfo.colorMode);
            if (state->error) return state->error;
        }
    }
    else
    {
        // colorMode conversion needed; sort of copy of the data
        unsigned char* data = *out;
        unsigned int outSize;

        // TODO: check if this works according to the statement in the documentation: "The converter can convert
        // from greyscale src colorMode type, to 8-bit greyscale or greyscale with alpha"
        if (!(state->rawInfo.colorType == LCT_RGB || state->rawInfo.colorType == LCT_RGBA) && !(state->rawInfo.bitDepth == 8))
        {
            return 56; // unsupported colorMode modeNum conversion
        }

        outSize = PNGGetRawSize(*w, *h, &state->rawInfo);
        *out = (unsigned char*)malloc(outSize);

        if (!(*out))
        {
            state->error = 83;
        }
        else state->error = PNGConvert(*out, data, &state->rawInfo, &state->pngInfo.colorMode, *w, *h);

        free(data);
    }

    return state->error;
}

unsigned int PNGEncode(unsigned char** out, unsigned int* outSize, const unsigned char* in, unsigned int w, unsigned int h, PNGState* state)
{
    PNGInfo info;
    ucVector outv;
    unsigned char* data = 0; // uncompressed version of the IDAT chunk data
    unsigned int dataSize = 0;

    // provide some proper dst values if error will happen
    *out = 0;
    *outSize = 0;
    state->error = 0;

    if ((state->pngInfo.colorMode.colorType == LCT_PALETTE || state->encoder.forcePalette) && (state->pngInfo.colorMode.paletteSize == 0 || state->pngInfo.colorMode.paletteSize > 256))
    {
        CERROR_RETURN_ERROR(state->error, 68); // invalid palette size, it is only allowed to be 1-256
    }

    if (state->encoder.zlibSettings.type > 2)
    {
        CERROR_RETURN_ERROR(state->error, 61); // error: unexisting type
    }

    if (state->pngInfo.interlaceMethod > 1)
    {
        CERROR_RETURN_ERROR(state->error, 71); // error: unexisting interlace modeNum
    }

    state->error = CheckColorValidity(state->pngInfo.colorMode.colorType, state->pngInfo.colorMode.bitDepth);
    if (state->error) return state->error; // error: unexisting colorMode type given

    state->error = CheckColorValidity(state->rawInfo.colorType, state->rawInfo.bitDepth);
    if (state->error) return state->error; // error: unexisting colorMode type given

    // color convert and compute scanline filter types
    PNGInfoInit(&info);
    PNGInfoCopy(&info, &state->pngInfo);

    if (state->encoder.autoConvert) state->error = PNGAutoChooseColor(&info.colorMode, in, w, h, &state->rawInfo);

    if (!state->error)
    {
        if (!PNGColorModeEqual(&state->rawInfo, &info.colorMode))
        {
            unsigned char* converted;
            unsigned int size = (w * h * PNGGetBPP(&info.colorMode) + 7) / 8;
            converted = (unsigned char*)malloc(size);

            if (!converted && size) state->error = 83; // alloc fail
        
            if (!state->error)
            {
                state->error = PNGConvert(converted, in, &info.colorMode, &state->rawInfo, w, h);
            }

            if (!state->error) PreProcessScanlines(&data, &dataSize, converted, w, h, &info, &state->encoder);
            free(converted);
        }
        else PreProcessScanlines(&data, &dataSize, in, w, h, &info, &state->encoder);
    }
    
    // output all PNG chunks
    ucVectorInit(&outv);
    while (!state->error) // while only executed once, to break on error
    {
        int i;

        // write signature and chunks
        WriteSignature(&outv);

        // IHDR
        AddChunk_IHDR(&outv, w, h, info.colorMode.colorType, info.colorMode.bitDepth, info.interlaceMethod);

        // unknown chunks between IHDR and PLTE
        if (info.unknownChunksData[0])
        {
            state->error = AddUnknownChunks(&outv, info.unknownChunksData[0], info.unknownChunksSize[0]);
            if (state->error) break;
        }

        // PLTE
        if (info.colorMode.colorType == LCT_PALETTE)
        {
            AddChunk_PLTE(&outv, &info.colorMode);
        }

        if (state->encoder.forcePalette && (info.colorMode.colorType == LCT_RGB || info.colorMode.colorType == LCT_RGBA))
        {
            AddChunk_PLTE(&outv, &info.colorMode);
        }

        // tRNS
        if (info.colorMode.colorType == LCT_PALETTE && GetPaletteTranslucency(info.colorMode.palette, info.colorMode.paletteSize) != 0)
        {
            AddChunk_tRNS(&outv, &info.colorMode);
        }

        if ((info.colorMode.colorType == LCT_GREY || info.colorMode.colorType == LCT_RGB) && info.colorMode.keyDefined)
        {
            AddChunk_tRNS(&outv, &info.colorMode);
        }

        // bKGD (must come between PLTE and the IDAt chunks
        if (info.backgroundDefined) AddChunk_bKGD(&outv, &info);

        // pHYs (must come before the IDAT chunks)
        if (info.physDefined) AddChunk_pHYs(&outv, &info);

        // unknown chunks between PLTE and IDAT
        if (info.unknownChunksData[1])
        {
            state->error = AddUnknownChunks(&outv, info.unknownChunksData[1], info.unknownChunksSize[1]);
            if (state->error) break;
        }

        // IDAT (multiple IDAT chunks must be consecutive)
        state->error = AddChunk_IDAT(&outv, data, dataSize, &state->encoder.zlibSettings);
        if (state->error) break;

        // tIME
        if (info.timeDefined) AddChunk_tIME(&outv, &info.time);

        // tEXt and/or zTXt
        for (i = 0; i != info.textNum; i++)
        {
            if (strlen(info.textKeys[i]) > 79)
            {
                state->error = 66; // text chunk too large
                break;
            }
            if (strlen(info.textKeys[i]) < 1)
            {
                state->error = 67; // text chunk too small
                break;
            }
            if (state->encoder.textCompression)
            {
                AddChunk_zTXt(&outv, info.textKeys[i], info.textStrings[i], &state->encoder.zlibSettings);
            }
            else
            {
                AddChunk_tEXt(&outv, info.textKeys[i], info.textStrings[i]);
            }
        }

        // LodePNG version id in text chunk
        if (state->encoder.addID)
        {
            unsigned int addedText = 0;
            for (i = 0; i != info.textNum; i++)
            {
                if (!strcmp(info.textKeys[i], "PNG"))
                {
                    addedText = 1;
                    break; 
                }
            }

            if (addedText == 0)
            {
                AddChunk_tEXt(&outv, "PNG", PNG_VERSION_STRING); // it's shorter as tEXt than as zTXt chunk
            }
        }

        // iTXt
        for (i = 0; i != info.itextNum; i++)
        {
            if (strlen(info.itextKeys[i]) > 79)
            {
                state->error = 66; // text chunk too large
                break;
            }

            if (strlen(info.itextKeys[i]) < 1)
            {
                state->error = 67; // text chunk too small
                break;
            }

            AddChunk_iTXt(&outv, state->encoder.textCompression, info.itextKeys[i], info.itextLangTags[i], info.itextTransKeys[i], info.itextStrings[i], &state->encoder.zlibSettings);
        }

        // unknown chunks between IDAT and IEND
        if (info.unknownChunksData[2])
        {
            state->error = AddUnknownChunks(&outv, info.unknownChunksData[2], info.unknownChunksSize[2]);
            if (state->error) break;
        }

        AddChunk_IEND(&outv);

        break; // this isn't really a while loop; no error happened so break out now!
    }

    PNGInfoCleanup(&info);
    free(data);

    // instead of cleaning the vector up, give it to the dst
    *out = outv.data;
    *outSize = outv.size;

    return state->error;
}

// Load PNG file as IMAGE texture
unsigned int LoadPNG(char *fname, IMAGE *img)
{
    FILE *fp;

    PNGState state;
    unsigned int error;
    unsigned int size;
    unsigned char *buffer;

    fp = fopen(fname, "rb");
    if (!fp) return 83;

    size = GetFileSize(fp);
    if (!size) return 83;

    buffer = (unsigned char*)malloc(size);
    if (!buffer) return 83;

    fread(buffer, size, 1, fp);
    fclose(fp);

    PNGStateInit(&state);
    state.rawInfo.colorType = LCT_RGBA;
    state.rawInfo.bitDepth = 8;
    error = PNGDecode(&img->mData, &img->mWidth, &img->mHeight, &state, buffer, size);
    PNGStateCleanup(&state);
    
    img->mPixels   = 4;
    img->mSize     = img->mWidth * img->mHeight * img->mPixels;
    img->mRowBytes = img->mWidth * img->mPixels;

    free(buffer);

    buffer = img->mData;
    size   = img->mWidth * img->mHeight;

    // Make image buffer as texture (swap r, b to match screen ARGB)
    _asm {
        mov    edi, buffer
        mov    ecx, size
    next:
        mov    al, [edi]
        xchg   al, [edi + 2]
        mov    [edi], al
        add    edi, 4
        loop   next
    }

    return error;
}

unsigned int SavePNG(char* fname, IMAGE *img)
{
    FILE *fp;

    unsigned int error = 0;
    PNGState state;
    unsigned char* buffer;
    unsigned int size;
    PNGColorType colorType = (img->mPixels == 4) ?  LCT_RGBA : LCT_RGB;

    PNGStateInit(&state);
    state.rawInfo.colorType = colorType;
    state.rawInfo.bitDepth = 8;
    state.pngInfo.colorMode.colorType = colorType;
    state.pngInfo.colorMode.bitDepth = 8;
    PNGEncode(&buffer, &size, img->mData, img->mWidth, img->mHeight, &state);
    error = state.error;
    PNGStateCleanup(&state);

    if (error)
    {
        free(buffer);
        return error;
    }

    if (!(fp = fopen(fname, "wb")))
    {
        free(buffer);
        return 83;
    }

    fwrite(buffer, size, 1, fp);
    fclose(fp);

    free(buffer);
    return error;
}

// This returns the description of a numerical error code in English. This is also
// the documentation of all the error codes.
const char* PNGErrorText(unsigned int code)
{
    switch (code)
    {
    case 0:  return "no error, everything went ok";
    case 1:  return "nothing done yet"; // the encoder/decoder has done nothing yet, error checking makes no sense yet
    case 10: return "end of src memory reached without huffman end code"; // while huffman decoding
    case 11: return "error in code tree made it jump outside of huffman tree"; // while huffman decoding
    case 13: return "problem while processing dynamic Deflate block";
    case 14: return "problem while processing dynamic Deflate block";
    case 15: return "problem while processing dynamic Deflate block";
    case 16: return "unexisting code while processing dynamic Deflate block";
    case 17: return "end of out buffer memory reached while inflating";
    case 18: return "invalid distance code while inflating";
    case 19: return "end of out buffer memory reached while inflating";
    case 20: return "invalid Deflate block btype encountered while decoding";
    case 21: return "nlen is not ones complement of len in a Deflate block";
    // end of out buffer memory reached while inflating:
    // This can happen if the inflated Deflate data is longer than the amount of bytes required to fill up
    // all the pixels of the data, given the colorMode depth and data dimensions. Something that doesn't
    // happen in a normal, well encoded, PNG data.
    case 22: return "end of out buffer memory reached while inflating";
    case 23: return "end of in buffer memory reached while inflating";
    case 24: return "invalid FCHECK in zlib header";
    case 25: return "invalid compression method in zlib header";
    case 26: return "FDICT encountered in zlib header while it's not used for PNG";
    case 27: return "PNG file is smaller than a PNG header";
    // Checks the magic file header, the first 8 bytes of the PNG file
    case 28: return "incorrect PNG signature, it's no PNG or corrupted";
    case 29: return "first chunk is not the header chunk";
    case 30: return "chunk len too large, chunk broken off at end of file";
    case 31: return "illegal PNG colorMode type or bpp";
    case 32: return "illegal PNG compression method";
    case 33: return "illegal PNG Filter method";
    case 34: return "illegal PNG interlace method";
    case 35: return "chunk len of a chunk is too large or the chunk too small";
    case 36: return "illegal PNG Filter type encountered";
    case 37: return "illegal bit depth for this colorMode type given";
    case 38: return "the palette is too big"; // more than 256 colors
    case 39: return "more palette alpha values given in tRNS chunk than there are colors in the palette";
    case 40: return "tRNS chunk has wrong size for greyscale data";
    case 41: return "tRNS chunk has wrong size for RGB data";
    case 42: return "tRNS chunk appeared while it was not allowed for this colorMode type";
    case 43: return "bKGD chunk has wrong size for palette data";
    case 44: return "bKGD chunk has wrong size for greyscale data";
    case 45: return "bKGD chunk has wrong size for RGB data";
    // the src data is empty, maybe a PNG file doesn't exist or is in the wrong path
    case 48: return "empty src or file doesn't exist";
    case 49: return "jumped past memory while generating dynamic huffman tree";
    case 50: return "jumped past memory while generating dynamic huffman tree";
    case 51: return "jumped past memory while inflating huffman block";
    case 52: return "jumped past memory while inflating";
    case 53: return "size of zlib data too small";
    case 54: return "repeat symbol in tree while there was no value symbol yet";
    // jumped past tree while generating huffman tree, this could be when the
    // tree will have more leaves than symbols after generating it out of the
    // given lenghts. They call this an oversubscribed dynamic bit lenghts tree in zlib.
    case 55: return "jumped past tree while generating huffman tree";
    case 56: return "given dst data colorType or bitDepth not supported for colorMode conversion";
    case 57: return "invalid CRC encountered (checking CRC can be disabled)";
    case 58: return "invalid adler32 encountered (checking adler32 can be disabled)";
    case 59: return "requested colormode conversion not supported";
    case 60: return "invalid window size given in the settings of the encoder (must be 0-32768)";
    case 61: return "invalid btype given in the settings of the encoder (only 0, 1 and 2 are allowed)";
    // LodePNG leaves the choice of RGB to greyscale conversion formula to the user.
    case 62: return "conversion from colorMode to greyscale not supported";
    case 63: return "len of a chunk too long, max allowed for PNG is 2147483647 bytes per chunk"; // (2^31-1)
    // this would result in the inability of a deflated block to ever contain an end code. It must be at least 1.
    case 64: return "the len of the END symbol 256 in the Huffman tree is 0";
    case 66: return "the len of a text chunk keyword given to the encoder is longer than the maximum of 79 bytes";
    case 67: return "the len of a text chunk keyword given to the encoder is smaller than the minimum of 1 byte";
    case 68: return "tried to encode a PLTE chunk with a palette that has less than 1 or more than 256 colors";
    case 69: return "unknown chunk type with 'critical' flag encountered by the decoder";
    case 71: return "unexisting interlace mode given to encoder (must be 0 or 1)";
    case 72: return "while decoding, unexisting compression method encountering in zTXt or iTXt chunk (it must be 0)";
    case 73: return "invalid tIME chunk size";
    case 74: return "invalid pHYs chunk size";
    // len could be wrong, or data chopped off
    case 75: return "no null termination char found while decoding text chunk";
    case 76: return "iTXt chunk too short to contain required bytes";
    case 77: return "integer overflow in buffer size";
    case 78: return "failed to open file for reading"; // file doesn't exist or couldn't be opened for reading
    case 79: return "failed to open file for writing";
    case 80: return "tried creating a tree of 0 symbols";
    case 81: return "lazy matching at pos 0 is impossible";
    case 82: return "colormode conversion to palette requested while a colorMode isn't in palette";
    case 83: return "memory allocation failed";
    case 84: return "given data too small to contain all pixels to be encoded";
    case 86: return "impossible offset in lz77 encoding (internal bug)";
    case 87: return "must provide custom zlib function pointer if LODEPNG_COMPILE_ZLIB is not defined";
    case 88: return "invalid filter strategy given for PNGEncoderSettings.fillterStrategy";
    case 89: return "text chunk keyword too short or long: must have size 1-79";
    // the windowSize in the PNGCompressSettings. Requiring POT(==> & instead of %) makes encoding 12% faster.
    case 90: return "windowsize must be a power of two";
    case 91: return "invalid decompressed idat size";
    case 92: return "integer overflow due to too many pixels";
    case 93: return "zero bmWidth or bmHeight is invalid";
    case 94: return "header chunk must have a size of 13 bytes";
    case 95: return "integer overflow with combined idat chunk size";
    }

    return "Unknown error code";
}

void ColorTypeString(PNGColorType type, char *buff)
{
    char *name;
    switch (type)
    {
    case LCT_GREY:       name = "Grey"; break;
    case LCT_RGB:        name = "RGB"; break;
    case LCT_PALETTE:    name = "Palette"; break;
    case LCT_GREY_ALPHA: name = "Grey With Alpha"; break;
    case LCT_RGBA:       name = "RGBA"; break;
    default:             name = "Invalid"; break;
    }

    sprintf(buff, "%d (%s)", type, name);
}

// Display general info about the PNG.
void PNGShowInfo(PNGInfo* info)
{
    unsigned int i;
    char ctype[128] = {0};
    PNGColorMode colorMode = info->colorMode;

    printf("Compression Method: %u\n", info->compressionMethod);
    printf("Filter Method: %u\n", info->filterMethod);

    ColorTypeString(colorMode.colorType, ctype);

    printf("Color Type: %s\n", ctype);
    printf("Bit Depth: %u\n", colorMode.bitDepth);

    printf("Bits per Pixel: %u\n", PNGGetBPP(&colorMode));
    printf("Channels per Pixel: %u\n", PNGGetChannels(&colorMode));
    printf("Is Greyscale Type: %d\n", PNGIsGreyScaleType(&colorMode));
    printf("Can Have Alpha: %d\n", PNGCanHaveAlpha(&colorMode));

    printf("Palette Size: %u\n", colorMode.paletteSize);
    printf("Has Color Mode Key: %d\n", colorMode.keyDefined);

    if (colorMode.keyDefined)
    {
        printf("Color Key R: %d\n", colorMode.keyR);
        printf("Color Key G: %d\n", colorMode.keyG);
        printf("Color Key B: %d\n", colorMode.keyB);
    }

    printf("Interlace Method: %d\n", info->interlaceMethod);
    printf("Texts: %d\n", info->textNum);

    for (i = 0; i != info->textNum; i++)
    { 
        printf("Text: %s: %s\n", info->textKeys[i], info->textStrings[i]);
    }

    printf("International Texts: %u\n", info->itextNum);

    for (i = 0; i != info->itextNum; i++)
    {
        printf("Text: %s, %s, %s, %s\n", info->itextKeys[i], info->itextLangTags[i], info->itextTransKeys[i], info->itextStrings[i]);
    }

    printf("Time Defined: %u\n", info->timeDefined);

    if (info->timeDefined)
    {
        PNGTime time = info->time;
        printf("Year: %u\n", time.year);
        printf("Month: %u\n", time.month);
        printf("Day: %u\n", time.day);
        printf("Hour: %u\n", time.hour);
        printf("Minute: %u\n", time.minute);
        printf("Second: %u\n", time.second);
    }

    printf("Physics Defined: %u\n", info->physDefined);
    if (info->physDefined)
    {
        printf("Physics X: %u\n", info->physX);
        printf("Physics Y: %u\n", info->physY);
        printf("Physics Unit: %u\n", info->physUnit);
    }
}

void PNGFileInfo(char* fname)
{
    FILE *fp;
    PNGState state;

    unsigned w, h, error;
    unsigned int inSize;
    unsigned char* in;
    unsigned char* out;

    if (!(fp = fopen(fname, "rb"))) return;
    if (!(inSize = GetFileSize(fp))) return;
    if (!(in = (unsigned char*)malloc(inSize))) return;

    fread(in, inSize, 1, fp);
    fclose(fp);

    PNGStateInit(&state);
    state.rawInfo.colorType = LCT_RGBA;
    state.rawInfo.bitDepth = 8;
    error = PNGDecode(&out, &w, &h, &state, in, inSize);
    if (error)
    {
        printf("PNGFileInfo: PNG decode error: %s\n", PNGErrorText(error));
        free(in);
        free(out);
        PNGStateCleanup(&state);
        return;
    }

    printf("File Size: %u\n", inSize);
    printf("Width: %d\n", w);
    printf("Height: %d\n", h);
    printf("Num Pixels: %d\n", w * h);

    PNGShowInfo(&state.pngInfo); 
    PNGStateCleanup(&state);

    free(out);
    free(in);
}

// load image from file (support BMP, PNG)
// this use file extension to determine image type
int LoadImage(char *fname, IMAGE *im)
{
    char *pos = strrchr(fname, '.');
    if (!pos) return 0;

    // load and convert bitmap to image (screen texture)
    if (!strcmpi(pos, ".bmp"))
    {
        BITMAP bm;
        if (!LoadBitmap(fname, &bm)) return 0;
        ConvertBitmap(&bm, im);
        CloseBitmap(&bm);
        return 1;
    }
    else if (!strcmpi(pos, ".png"))
    {
        // load and convert PNG to image
        if (LoadPNG(fname, im)) return 0;
        return 1;
    }
    else FatalError("LoadImage: load image unknown type!\n");
    return 0;
}

// Render PNG file (24/32 bits colors)
void ShowPNG(char *file)
{
    IMAGE img;
    unsigned int error = 0;
    unsigned char *pngData;
    unsigned int pngWidth, pngHeight;

    // try to load png file
    error = LoadPNG(file, &img);
    if (error) FatalError("ShowPNG: cannot load PNG file: %s\n", PNGErrorText(error));

    // make local use for asm code
    pngData   = img.mData;
    pngWidth  = img.mWidth;
    pngHeight = img.mHeight;

    // render to screen
    switch (bitsPerPixel)
    {
    case 32:
        _asm {
            mov    eax, pageOffset
            mov    ebx, bytesPerScanline
            mul    ebx
            mov    edi, lfbPtr
            add    edi, eax
            mov    esi, pngData
            mov    ecx, pngWidth
            mov    eax, bytesPerScanline
            mov    ebx, pngWidth
            shl    ebx, 2
            sub    eax, ebx
            push   eax
        next:
            xor    edx, edx
            mov    eax, ecx
            shr    eax, 4
            mov    ebx, 2
            div    ebx
            push   edx
            xor    edx, edx
            mov    eax, pngHeight
            shr    eax, 4
            mov    ebx, 2
            div    ebx
            pop    ebx
            cmp    edx, ebx
            jne    setbk
            mov    ebx, 255
            jmp    quit
        setbk:
            mov    ebx, 191
        quit:
            mov    eax, 255
            sub    al, [esi + 3]
            mul    ebx
            mov    ebx, eax
            xor    eax, eax
            mov    al, [esi + 3]
            mul    byte ptr[esi]
            add    eax, ebx
            shr    eax, 8
            stosb
            mov    al, [esi + 3]
            mul    byte ptr[esi + 1]
            add    eax, ebx
            shr    eax, 8
            stosb
            mov    al, [esi + 3]
            mul    byte ptr[esi + 2]
            add    eax, ebx
            shr    eax, 8
            stosb
            inc    edi
            add    esi, 4
            loop   next
            mov    ecx, pngWidth
            add    edi, [esp]
            dec    pngHeight
            jnz    next
            pop    eax
        }
        break;

    case 24:
        _asm {
            mov    eax, pageOffset
            mov    ebx, bytesPerScanline
            mul    ebx
            mov    edi, lfbPtr
            add    edi, eax
            mov    esi, pngData
            mov    ecx, pngWidth
            mov    eax, bytesPerScanline
            sub    eax, pngWidth
            sub    eax, pngWidth
            sub    eax, pngWidth
            push   eax
        next:
            xor    edx, edx
            mov    ebx, 2
            mov    eax, ecx
            shr    eax, 4
            div    ebx
            push   edx
            xor    edx, edx
            mov    eax, pngHeight
            shr    eax, 4
            div    ebx
            pop    ebx
            cmp    edx, ebx
            jne    setbk
            mov    ebx, 255
            jmp    quit
        setbk:
            mov    ebx, 191
        quit:
            mov    eax, 255
            sub    al, [esi + 3]
            mul    ebx
            mov    ebx, eax
            mov    al, [esi + 3]
            mul    byte ptr[esi]
            add    eax, ebx
            shr    eax, 8
            stosb
            mov    al, [esi + 3]
            mul    byte ptr[esi + 1]
            add    eax, ebx
            shr    eax, 8
            stosb
            mov    al, [esi + 3]
            mul    byte ptr[esi + 2]
            add    eax, ebx
            shr    eax, 8
            stosb
            add    esi, 4
            loop   next
            mov    ecx, pngWidth
            add    edi, [esp]
            dec    pngHeight
            jnz    next
            pop    eax
        }
        break;
    }

    CloseImage(&img);
}

// Capture display screen and save to file
void SaveScreen(char *fname)
{
    // check for image type (use file extension)
    char *pos = strrchr(fname, '.');
    if (!pos) return;

    // Save BMP file
    if (!strcmpi(pos, ".bmp"))
    {
        BITMAP bmp;
        bmp.bmWidth    = xres;
        bmp.bmHeight   = yres;
        bmp.bmData     = lfbPtr;
        bmp.bmPixels   = bytesPerPixel;
        bmp.bmRowBytes = bytesPerScanline;
        if (!SaveBitmap(fname, &bmp)) FatalError("SaveScreen: cannot create BMP file: %s\n", fname);
    }

    // Save PNG file
    else if (!strcmpi(pos, ".png"))
    {
        IMAGE img;
        unsigned int error = 0;
        unsigned char *pngData;
        unsigned int pngWidth, pngHeight;

        // init bitmap data
        img.mWidth  = xres;
        img.mHeight = yres;
        img.mPixels = bytesPerPixel;
        img.mData   = (unsigned char*)malloc(yres * bytesPerScanline);
        if (!img.mData) FatalError("SaveScreen: cannot alloc PNG buffer.\n");

        // make local to use asm
        pngData   = img.mData;
        pngWidth  = img.mWidth;
        pngHeight = img.mHeight;

        // copy screen pixel to png data buffer
        switch(bitsPerPixel)
        {
        case 32:
            _asm {
                mov    eax, pageOffset
                mul    bytesPerScanline
                mov    esi, lfbPtr
                add    esi, eax
                mov    edi, pngData
                mov    ecx, pngWidth
            next:
                lodsd
                mov    ebx, eax
                mov    edx, eax
                shr    eax, 16
                shr    ebx, 8
                mov    [edi]    , al  // r
                mov    [edi + 1], bl  // g
                mov    [edi + 2], dl  // b
                mov    [edi + 3], 255 // a
                add    edi, 4
                loop   next
                mov    ecx, pngWidth
                dec    pngHeight
                jnz    next
                pop    eax
            }
            break;

        case 24:
            _asm {
                mov    eax, pageOffset
                mul    bytesPerScanline
                mov    esi, lfbPtr
                add    esi, eax
                mov    edi, pngData
                mov    ecx, pngWidth
            next:
                lodsw
                mov    [edi + 2], al  // r
                mov    [edi + 1], ah  // g
                lodsb
                mov    [edi]    , al  // b
                add    edi, 3
                loop   next
                mov    ecx, pngWidth
                dec    pngHeight
                jnz    next
                pop    eax
            }
            break;
        }

        // encode buffer to file
        error = SavePNG(fname, &img);
        if (error) FatalError("SaveScreen: PNG save error: %s\n", PNGErrorText(error));
        free(img.mData);
    }
}

// Initialize mouse driver
int InitMouse()
{
    short state = 0;
    short button = 0;

    _asm {
        xor    ax, ax
        xor    bx, bx
        int    33h
        mov    state, ax
        mov    button, bx
    }
    return (state != 0) && (button > 0);
}

// Initalize mouse driver and bitmap mouse image
int InitMouseButton(MOUSE_IMAGE *mi)
{
    // initialize mouse first
    short state = 0;
    short button = 0;
    
    _asm {
        xor    ax, ax
        xor    bx, bx
        int    33h
        mov    state, ax
        mov    button, bx
    }

    // initialize mouse image value
    mi->msState    = state;
    mi->msNumBtn   = button;
    mi->msPosX     = centerx;
    mi->msPosY     = centery;
    mi->msWidth    = 0;
    mi->msHeight   = 0;
    mi->msPixels   = 0;
    mi->msUnder    = NULL;
    mi->msBitmap   = NULL;
    return (state != 0) && (button > 0);
}

// mouse callback handler
#pragma aux MouseHandler parm [eax] [ebx] [ecx] [edx]
void __loadds __far MouseHandler(int max, int mbx, int mcx, int mdx)
{
    mcd.max = max;
    mcd.mbx = mbx;
    mcd.mcx = mcx;
    mcd.mdx = mdx;
}

// install hardware interrupt handler
void InstallMouseHandler()
{
    _asm {
        lea    edx, MouseHandler
        mov    ax, seg MouseHandler
        mov    es, ax
        mov    ax, 0Ch
        mov    cx, 0FFh
        int    33h
    }
}

// uninstall hardware interrupt hander
void UnInstallMouseHandler()
{
    _asm {
        xor    edx, edx
        mov    ax, 0Ch
        xor    cx, cx
        mov    es, cx
        int    33h
    }
}

// set new mouse postion
void SetMousePos(short posx, short posy)
{
    _asm {
        mov    ax, 04h
        mov    cx, posx
        mov    dx, posy
        int    33h
    }
}

// set mouse limit range
void SetMouseRange(short x1, short y1, short x2, short y2)
{
    _asm {
        mov    ax, 07h
        mov    cx, x1
        mov    dx, x2
        int    33h
        inc    ax
        mov    cx, y1
        mov    dx, y2
        int    33h
    }
}

// set mouse sensity and drag speed
void SetMouseSensitivity(short sx, short sy, short dspeed)
{
    if (sx > 100) sx = 100;
    if (sy > 100) sy = 100;
    if (dspeed > 100) dspeed = 100;

    _asm {
        mov    ax, 01Ah
        mov    bx, sx
        mov    cx, sy
        mov    dx, dspeed
        int    33h
    }
}

// draw mouse cursor
void DrawMouseCursor(MOUSE_IMAGE *mi)
{
    int mbWidth  = mi->msWidth;
    int mbHeight = mi->msHeight;

    int mx = mi->msPosX - mi->msBitmap->mbHotX;
    int my = mi->msPosY - mi->msBitmap->mbHotY;

    unsigned char *msUnder = mi->msUnder;
    unsigned char *mbImage = mi->msBitmap->mbData;

    // check color channel
    if (bytesPerPixel != mi->msPixels) return;

    // check clip boundary
    if (mx < cminx) mx = cminx;
    if (mx > cmaxx) mx = cmaxx;
    if (my < cminy) my = cminy;
    if (my > cmaxy) my = cmaxy;

    // render mouse cursor to screen
    switch(bytesPerPixel)
    {
    case 1:
        _asm {
            mov    eax, my
            add    eax, pageOffset
            mul    bytesPerScanline
            add    eax, mx
            mov    esi, lfbPtr
            add    esi, eax
            mov    edi, msUnder
            mov    ebx, bytesPerScanline
            sub    ebx, mbWidth
            push   ebx
            xor    edx, edx
        next:
            xor    ecx, ecx
        step:
            // check mouse boundary
            mov    ebx, mx
            add    ebx, ecx
            cmp    ebx, cminx
            jb     skip
            cmp    ebx, cmaxx
            ja     skip
            mov    ebx, my
            add    ebx, edx
            cmp    ebx, cminy
            jb     skip
            cmp    ebx, cmaxy
            ja     skip

            // copy screen background to mouse under
            movsb   
            push   esi
            push   edi

            // render mouse cursor to screen
            mov    eax, edi
            sub    eax, msUnder
            mov    edi, esi
            dec    edi
            mov    esi, mbImage
            add    esi, eax
            dec    esi

            // don't render color key
            lodsb
            test   al, al
            jz     quit
            stosb
            jmp    quit
        skip:
            inc    esi
            inc    edi
            jmp    cycle
        quit:
            pop    edi
            pop    esi
        cycle:
            inc    ecx
            cmp    ecx, mbWidth
            jb     step
            add    esi, [esp]
            inc    edx
            cmp    edx, mbHeight
            jb     next
            pop    ebx
        }
        break;

    case 2:
        _asm {
            mov    eax, my
            add    eax, pageOffset
            mul    bytesPerScanline
            add    eax, mx
            add    eax, mx
            mov    esi, lfbPtr
            add    esi, eax
            mov    edi, msUnder
            mov    ebx, bytesPerScanline
            sub    ebx, mbWidth
            sub    ebx, mbWidth
            push   ebx
            xor    edx, edx
        next:
            xor    ecx, ecx
        step:
            // check mouse boundary
            mov    ebx, mx
            add    ebx, ecx
            cmp    ebx, cminx
            jb     skip
            cmp    ebx, cmaxx
            ja     skip
            mov    ebx, my
            add    ebx, edx
            cmp    ebx, cminy
            jb     skip
            cmp    ebx, cmaxy
            ja     skip

            // copy screen background to mouse under
            movsw
            push   esi
            push   edi

            // render mouse cursor to screen
            mov    eax, edi
            sub    eax, msUnder
            mov    edi, esi
            sub    edi, 2
            mov    esi, mbImage
            add    esi, eax
            sub    esi, 2

            // don't render color key
            lodsw
            test   ax, ax
            jz     quit
            stosw
            jmp    quit
        skip:
            add    esi, 2
            add    edi, 2
            jmp    cycle
        quit:
            pop    edi
            pop    esi
        cycle:
            inc    ecx
            cmp    ecx, mbWidth
            jb     step
            add    esi, [esp]
            inc    edx
            cmp    edx, mbHeight
            jb     next
            pop    ebx
        }
        break;

    case 3:
        _asm {
            mov    eax, my
            add    eax, pageOffset
            mul    bytesPerScanline
            add    eax, mx
            add    eax, mx
            add    eax, mx
            mov    esi, lfbPtr
            add    esi, eax
            mov    edi, msUnder
            mov    ebx, bytesPerScanline
            sub    ebx, mbWidth
            sub    ebx, mbWidth
            sub    ebx, mbWidth
            push   ebx
            xor    edx, edx
        next:
            xor    ecx, ecx
        step:
            // check mouse boundary
            mov    ebx, mx
            add    ebx, ecx
            cmp    ebx, cminx
            jb     skip
            cmp    ebx, cmaxx
            ja     skip
            mov    ebx, my
            add    ebx, edx
            cmp    ebx, cminy
            jb     skip
            cmp    ebx, cmaxy
            ja     skip

            // copy screen background to mouse under
            movsw
            movsb
            push   esi
            push   edi

            // render mouse cursor to screen
            mov    eax, edi
            sub    eax, msUnder
            mov    edi, esi
            sub    edi, 3
            mov    esi, mbImage
            add    esi, eax
            sub    esi, 3

            // don't render color key
            lodsw
            mov    ebx, eax
            lodsb
            shl    eax, 16
            or     eax, ebx
            test   eax, eax
            jz     quit
            stosw
            shr    eax, 16
            stosb
            jmp    quit
        skip:
            add    esi, 3
            add    edi, 3
            jmp    cycle
        quit:
            pop    edi
            pop    esi
        cycle:
            inc    ecx
            cmp    ecx, mbWidth
            jb     step
            add    esi, [esp]
            inc    edx
            cmp    edx, mbHeight
            jb     next
            pop    ebx
        }
        break;

    case 4:
        _asm {
            mov    eax, my
            add    eax, pageOffset
            mul    bytesPerScanline
            mov    ebx, mx
            shl    ebx, 2
            add    eax, ebx
            mov    esi, lfbPtr
            add    esi, eax
            mov    edi, msUnder
            mov    eax, bytesPerScanline
            mov    ebx, mbWidth
            shl    ebx, 2
            sub    eax, ebx
            push   eax
            xor    edx, edx
        next:
            xor    ecx, ecx
        step:
            // check mouse boundary
            mov    ebx, mx
            add    ebx, ecx
            cmp    ebx, cminx
            jb     skip
            cmp    ebx, cmaxx
            ja     skip
            mov    ebx, my
            add    ebx, edx
            cmp    ebx, cminy
            jb     skip
            cmp    ebx, cmaxy
            ja     skip

            // copy screen background to mouse under
            movsd
            push   esi
            push   edi

            // render mouse cursor to screen
            mov    eax, edi
            sub    eax, msUnder
            mov    edi, esi
            sub    edi, 4
            mov    esi, mbImage
            add    esi, eax
            sub    esi, 4

            // don't render color key
            lodsd
            and    eax, 00FFFFFFh
            test   eax, eax
            jz     quit
            stosd
            jmp    quit
        skip:
            add    esi, 4
            add    edi, 4
            jmp    cycle
        quit:
            pop    edi
            pop    esi
        cycle:
            inc    ecx
            cmp    ecx, mbWidth
            jb     step
            add    esi, [esp]
            inc    edx
            cmp    edx, mbHeight
            jb     next
            pop    ebx
        }
        break;
    }
}

// hide mouse cursor
void ClearMouseCursor(MOUSE_IMAGE *mi)
{ 
    int mbWidth  = mi->msWidth;
    int mbHeight = mi->msHeight;

    unsigned char *msUnder = mi->msUnder;
    int mx = mi->msPosX - mi->msBitmap->mbHotX;
    int my = mi->msPosY - mi->msBitmap->mbHotY;

    // check color channel
    if (bytesPerPixel != mi->msPixels) return;

    // check clip boundary
    if (mx < cminx) mx = cminx;
    if (mx > cmaxx) mx = cmaxx;
    if (my < cminy) my = cminy;
    if (my > cmaxy) my = cmaxy;

    // render mouse under to screen
    switch(bytesPerPixel)
    {
    case 1:
        // copy mouse under to screen
        _asm {
            mov    eax, my
            add    eax, pageOffset
            mul    bytesPerScanline
            add    eax, mx
            mov    edi, lfbPtr
            add    edi, eax
            mov    esi, msUnder
            mov    ebx, bytesPerScanline
            sub    ebx, mbWidth
            xor    edx, edx
        next:
            xor    ecx, ecx
        step:
            // check mouse boundary
            mov    eax, mx
            add    eax, ecx
            cmp    eax, cminx
            jb     skip
            cmp    eax, cmaxx
            ja     skip
            mov    eax, my
            add    eax, edx
            cmp    eax, cminy
            jb     skip
            cmp    eax, cmaxy
            ja     skip
            movsb
            jmp    quit
        skip:
            inc    edi
            inc    esi
        quit:
            inc    ecx
            cmp    ecx, mbWidth
            jb     step
            add    edi, ebx
            inc    edx
            cmp    edx, mbHeight
            jb     next
        }
        break;

    case 2:
        // copy mouse under to screen
        _asm {
            mov    eax, my
            add    eax, pageOffset
            mul    bytesPerScanline
            add    eax, mx
            add    eax, mx
            mov    edi, lfbPtr
            add    edi, eax
            mov    esi, msUnder
            mov    ebx, bytesPerScanline
            sub    ebx, mbWidth
            sub    ebx, mbWidth
            xor    edx, edx
        next:
            xor    ecx, ecx
        step:
            // check mouse boundary
            mov    eax, mx
            add    eax, ecx
            cmp    eax, cminx
            jb     skip
            cmp    eax, cmaxx
            ja     skip
            mov    eax, my
            add    eax, edx
            cmp    eax, cminy
            jb     skip
            cmp    eax, cmaxy
            ja     skip
            movsw
            jmp    quit
        skip:
            add    edi, 2
            add    esi, 2
        quit:
            inc    ecx
            cmp    ecx, mbWidth
            jb     step
            add    edi, ebx
            inc    edx
            cmp    edx, mbHeight
            jb     next
        }
        break;

    case 3:
        // copy mouse under to screen
        _asm {
            mov    eax, my
            add    eax, pageOffset
            mul    bytesPerScanline
            add    eax, mx
            add    eax, mx
            add    eax, mx
            mov    edi, lfbPtr
            add    edi, eax
            mov    esi, msUnder
            mov    ebx, bytesPerScanline
            sub    ebx, mbWidth
            sub    ebx, mbWidth
            sub    ebx, mbWidth
            xor    edx, edx
        next:
            xor    ecx, ecx
        step:
            // check mouse boundary
            mov    eax, mx
            add    eax, ecx
            cmp    eax, cminx
            jb     skip
            cmp    eax, cmaxx
            ja     skip
            mov    eax, my
            add    eax, edx
            cmp    eax, cminy
            jb     skip
            cmp    eax, cmaxy
            ja     skip
            movsw
            movsb
            jmp    quit
        skip:
            add    edi, 3
            add    esi, 3
        quit:
            inc    ecx
            cmp    ecx, mbWidth
            jb     step
            add    edi, ebx
            inc    edx
            cmp    edx, mbHeight
            jb     next
        }
        break;

    case 4:
        _asm {
            mov    eax, my
            add    eax, pageOffset
            mul    bytesPerScanline
            mov    ebx, mx
            shl    ebx, 2
            add    eax, ebx
            mov    edi, lfbPtr
            add    edi, eax
            mov    esi, msUnder
            mov    ebx, bytesPerScanline
            mov    eax, mbWidth
            shl    eax, 2
            sub    ebx, eax
            xor    edx, edx
        next:
            xor    ecx, ecx
        step:
            // check mouse boundary
            mov    eax, mx
            add    eax, ecx
            cmp    eax, cminx
            jb     skip
            cmp    eax, cmaxx
            ja     skip
            mov    eax, my
            add    eax, edx
            cmp    eax, cminy
            jb     skip
            cmp    eax, cmaxy
            ja     skip
            movsd
            jmp    quit
        skip:
            add    edi, 4
            add    esi, 4
        quit:
            inc    ecx
            cmp    ecx, mbWidth
            jb     step
            add    edi, ebx
            inc    edx
            cmp    edx, mbHeight
            jb     next
        }
        break;
    }
}

// Draw bitmap button
void DrawButton(BUTTON_BITMAP *btn)
{
    int x1, y1, x2, y2, lfbWidth, lfbHeight;
    int lfbMinX, lfbMinY, lfbMaxX, lfbMaxY;
    
    // Initialize button data
    int btnWidth  = btn->btWidth;
    int btnHeight = btn->btHeight;
    void *btnData = btn->btData[btn->btState % BUTTON_BITMAPS];
    unsigned int btnRowBytes = btnWidth * btn->btPixels;

    // check color channel
    if (bytesPerPixel != btn->btPixels) return;

    // Calculate coordinator
    x1 = btn->btPosX;
    y1 = btn->btPosY;
    x2 = (x1 + btnWidth) - 1;
    y2 = (y1 + btnHeight) - 1;

    // Clip button image to context boundaries
    lfbMinX = (x1 >= cminx) ? x1 : cminx;
    lfbMinY = (y1 >= cminy) ? y1 : cminy;
    lfbMaxX = (x2 <= cmaxx) ? x2 : cmaxx;
    lfbMaxY = (y2 <= cmaxy) ? y2 : cmaxy;

    // Validate boundaries
    if (lfbMinX >= lfbMaxX) return;
    if (lfbMinY >= lfbMaxY) return;

    // Initialize loop variables
    lfbWidth  = (lfbMaxX - lfbMinX) + 1;
    lfbHeight = (lfbMaxY - lfbMinY) + 1;

    // Check for loop
    if (!lfbWidth || !lfbHeight) return;

    switch (bytesPerPixel)
    {
    case 1:
        _asm {
            mov    edi, lfbPtr
            mov    eax, lfbMinY
            add    eax, pageOffset
            mul    bytesPerScanline
            add    eax, lfbMinX
            add    edi, eax
            mov    esi, btnData
            mov    eax, lfbMinY
            sub    eax, y1
            mul    btnRowBytes
            mov    ebx, lfbMinX
            sub    ebx, x1
            add    eax, ebx
            add    esi, eax
            mov    ebx, bytesPerScanline
            sub    ebx, lfbWidth
            mov    edx, btnWidth
            sub    edx, lfbWidth
        next:
            mov    ecx, lfbWidth
        plot:
            lodsb
            test    al, al
            jz      skip
            stosb
            jmp    quit
        skip:
            inc    edi
        quit:
            loop   plot
            add    edi, ebx
            add    esi, edx
            dec    lfbHeight
            jnz    next
        }
        break;

    case 2:
        _asm {
            mov    edi, lfbPtr
            mov    eax, lfbMinY
            add    eax, pageOffset
            mul    bytesPerScanline
            mov    ebx, lfbMinX
            shl    ebx, 1
            add    eax, ebx
            add    edi, eax
            mov    esi, btnData
            mov    eax, lfbMinY
            sub    eax, y1
            mul    btnRowBytes
            mov    ebx, lfbMinX
            sub    ebx, x1
            shl    ebx, 1
            add    eax, ebx
            add    esi, eax
            mov    ebx, bytesPerScanline
            mov    edx, lfbWidth
            shl    edx, 1
            sub    ebx, edx
            mov    edx, btnWidth
            sub    edx, lfbWidth
            shl    edx, 1
        next:
            mov    ecx, lfbWidth
        plot:
            lodsw
            test    ax, ax
            je     skip
            stosw
            jmp    quit
        skip:
            add    edi, 2
        quit:
            loop   plot
            add    edi, ebx
            add    esi, edx
            dec    lfbHeight
            jnz    next
        }
        break;

    case 3:
        _asm {
            mov    edi, lfbPtr
            mov    eax, lfbMinY
            add    eax, pageOffset
            mul    bytesPerScanline
            mov    ebx, lfbMinX
            shl    ebx, 1
            add    ebx, lfbMinX
            add    eax, ebx
            add    edi, eax
            mov    esi, btnData
            mov    eax, lfbMinY
            sub    eax, y1
            mul    btnRowBytes
            mov    ebx, lfbMinX
            sub    ebx, x1
            mov    ecx, ebx
            shl    ebx, 1
            add    ebx, ecx
            add    eax, ebx
            add    esi, eax
            mov    ebx, bytesPerScanline
            mov    edx, lfbWidth
            shl    edx, 1
            add    edx, lfbWidth
            sub    ebx, edx
            push   ebx
            mov    edx, btnWidth
            sub    edx, lfbWidth
            mov    eax, edx
            shl    edx, 1
            add    edx, eax
        next:
            mov    ecx, lfbWidth
        plot:
            lodsw
            mov    ebx, eax
            lodsb
            shl    eax, 16
            or     eax, ebx
            test   eax, eax
            jz     skip
            stosw
            shr    eax, 16
            stosb
            jmp    quit
        skip:
            add    edi, 3
        quit:
            loop   plot
            add    edi, [esp]
            add    esi, edx
            dec    lfbHeight
            jnz    next
            pop    ebx
        }
        break;

    case 4:
        _asm {
            mov    edi, lfbPtr
            mov    eax, lfbMinY
            add    eax, pageOffset
            mul    bytesPerScanline
            mov    ebx, lfbMinX
            shl    ebx, 2
            add    eax, ebx
            add    edi, eax
            mov    esi, btnData
            mov    eax, lfbMinY
            sub    eax, y1
            mul    btnRowBytes
            mov    ebx, lfbMinX
            sub    ebx, x1
            shl    ebx, 2
            add    eax, ebx
            add    esi, eax
            mov    ebx, bytesPerScanline
            mov    edx, lfbWidth
            shl    edx, 2
            sub    ebx, edx
            mov    edx, btnWidth
            sub    edx, lfbWidth
            shl    edx, 2
        next:
            mov    ecx, lfbWidth
        plot:
            lodsd
            and    eax, 00FFFFFFh
            test   eax, eax
            jz     skip
            stosd
            jmp    quit
        skip:
            add    edi, 4
        quit:
            loop   plot
            add    edi, ebx
            add    esi, edx
            dec    lfbHeight
            jnz    next
        }
        break;
    }
}

// load the bitmap mouse button
int LoadMouseButton(char *fname, MOUSE_IMAGE *mi, MOUSE_BITMAP *mbm, BUTTON_BITMAP *btn)
{
    BITMAP bmp;
    int i, j, y;
    unsigned char *src, *dst;

    // load mouse bitmap and animation
    if (!LoadBitmap(fname, &bmp)) return 0;

    // allocate memory for mouse under background
    if (!(mi->msUnder = (unsigned char*)malloc(MOUSE_SIZE * bmp.bmPixels))) return 0;

    // init mouse image width and height
    mi->msWidth  = MOUSE_WIDTH;
    mi->msHeight = MOUSE_HEIGHT;
    mi->msPixels = bmp.bmPixels;

    // copy mouse cursors
    for (i = 0; i != NUM_MOUSE_BITMAPS; i++)
    {
        mbm[i].mbData = (unsigned char*)malloc(MOUSE_SIZE * bmp.bmPixels);
        if (!mbm[i].mbData) return 0;
        mbm[i].mbHotX = 12;
        mbm[i].mbHotY = 12;
        mbm[i].mbNext = &mbm[i + 1];
        for (y = 0; y != MOUSE_HEIGHT; y++)
        {
            dst = &mbm[i].mbData[y * MOUSE_WIDTH * bmp.bmPixels];
            src = &bmp.bmData[(i * MOUSE_WIDTH + y * bmp.bmWidth) * bmp.bmPixels];
            memcpy(dst, src, MOUSE_WIDTH * bmp.bmPixels);
        }
    }

    // init current and next mouse animated
    mbm[0].mbHotX = 7;
    mbm[0].mbHotY = 2;
    mbm[0].mbNext = &mbm[0];
    mbm[8].mbNext = &mbm[1];

    // copy button bitmaps
    for (i = 0; i != NUM_BUTTONS; i++)
    {
        btn[i].btWidth = BUTTON_WIDTH;
        btn[i].btHeight = BUTTON_HEIGHT;
        btn[i].btPixels = bmp.bmPixels;
        for (j = 0; j != BUTTON_BITMAPS; j++)
        {
            btn[i].btData[j] = (unsigned char*)malloc(BUTTON_SIZE * bmp.bmPixels);
            if (!btn[i].btData[j]) return 0;
            for (y = 0; y != BUTTON_HEIGHT; y++)
            {
                dst = &btn[i].btData[j][y * BUTTON_WIDTH * bmp.bmPixels];
                src = &bmp.bmData[(i * (bmp.bmWidth >> 1) + j * BUTTON_WIDTH + (BUTTON_HEIGHT + y) * bmp.bmWidth) * bmp.bmPixels];
                memcpy(dst, src, BUTTON_WIDTH * bmp.bmPixels);
            }
        }
    }

    // init button 'click me'
    btn[0].btPosX  = centerx - BUTTON_WIDTH - 20;
    btn[0].btPosY  = centery - (BUTTON_HEIGHT >> 1);
    btn[0].btState = STATE_NORM;

    // init button 'exit'
    btn[1].btPosX  = centerx + BUTTON_WIDTH + 10;
    btn[1].btPosY  = centery - (BUTTON_HEIGHT >> 1);
    btn[1].btState = STATE_NORM;

    // set palette for 8bits bitmap
    if (bmp.bmPixels == 1) SetPalette(bmp.bmExtra);
    CloseBitmap(&bmp);
    return 1;
}

// release mouse button
void CloseMouseButton(MOUSE_IMAGE *mi, MOUSE_BITMAP *mbm, BUTTON_BITMAP *btn)
{
    int i, j;

    // cleanup mouse bitmap
    for (i = 0; i != NUM_MOUSE_BITMAPS; i++)
    {
        if (mbm[i].mbData)
        {
            free(mbm[i].mbData);
            mbm[i].mbData = NULL;
        }
    }

    // cleanup button bitmap
    for (i = 0; i != NUM_BUTTONS; i++)
    {
        for (j = 0; j != BUTTON_BITMAPS; j++)
        {
            if (btn[i].btData[j])
            {
                free(btn[i].btData[j]);
                btn[i].btData[j] = NULL;
            }
        }
    }

    // cleanup mouse underground
    ClearMouseCursor(mi);
    if (mi->msUnder)
    {
        free(mi->msUnder);
        mi->msUnder = NULL;
    }
}

// automatic mouse event handler
void HandleMouse(char *fname)
{
    MOUSE_IMAGE mi;

    MOUSE_BITMAP *msNormal = NULL;
    MOUSE_BITMAP *msWait = NULL;
    MOUSE_BITMAP *msNew = NULL;

    BUTTON_BITMAP btn[NUM_BUTTONS] = {0};
    MOUSE_BITMAP mbm[NUM_MOUSE_BITMAPS] = {0};

    int i, done = 0;
    int lastx = 0, lasty = 0;
    unsigned long lastTime = 0;
    unsigned int needDraw = 0xFFFF;
    char *bkg[] = {"1lan8.bmp", "1lan16.bmp", "1lan24.bmp", "1lan32.bmp"};

    // init and setup bitmap mouse and button
    if (!InitMouseButton(&mi)) FatalError("HandleMouse: cannot init mouse driver.\n");
    if (!LoadMouseButton(fname, &mi, mbm, btn)) FatalError("HandleMouse: cannot load mouse bitmap: %s\n", fname);

    // install user-define mouse handler
    InstallMouseHandler();
    SetMousePos(centerx, centery);
    SetMouseRange(centerx - 100, centery - 100, centerx + 110, centery + 100);
    SetMouseSensitivity(100, 100, 100);

    // init mouse normal and wait cursor bitmap
    msNormal    = &mbm[0];
    msWait      = &mbm[1];
    mi.msBitmap = msNormal;

    // setup screen background
    ShowBitmap(bkg[bytesPerPixel - 1]);
    DrawMouseCursor(&mi);

    // update last mouse pos
    lastx = mcd.mcx = centerx;
    lasty = mcd.mdx = centery;
    lastTime = GetTicks();

    // remove keyboard buffer
    while (kbhit()) getch();
    while (!kbhit() && !done)
    {
        // only draw if needed
        if (needDraw)
        {
            // clear old mouse position
            WaitRetrace();
            ClearMouseCursor(&mi);

            // draw buttons
            if (needDraw > 1)
            {
                for (i = 0; i != NUM_BUTTONS; i++)
                {
                    if (needDraw & (2 << i)) DrawButton(&btn[i]);
                }
            }

            // update new button bitmap
            if (msNew) mi.msBitmap = msNew;

            // update mouse position and draw new mouse position
            mi.msPosX = mcd.mcx;
            mi.msPosY = mcd.mdx;
            DrawMouseCursor(&mi);

            // update last ticks count and turn off drawable
            lastTime = GetTicks();
            needDraw = 0;
            msNew = NULL;
        }

        // check for draw new state button
        if (GetTicks() != lastTime)
        {
            if (mi.msBitmap != mi.msBitmap->mbNext)
            {
                needDraw = 1;
                mi.msBitmap = mi.msBitmap->mbNext;
            }
            else
            {
                lastTime = GetTicks();
            }
        }

        // update drawable when position changing
        if (lastx != mcd.mcx || lasty != mcd.mdx)
        {
            lastx = mcd.mcx;
            lasty = mcd.mdx;
            needDraw = 1;
        }

        // check for new button state
        for (i = 0; i != NUM_BUTTONS; i++)
        {
            // check if mouse inside the button region
            if (mcd.mcx >= btn[i].btPosX && mcd.mcx <= btn[i].btPosX + BUTTON_WIDTH && mcd.mdx >= btn[i].btPosY && mcd.mdx <= btn[i].btPosY + BUTTON_HEIGHT)
            {
                if (mcd.mbx == 0 && btn[i].btState == STATE_PRESSED)
                {
                    btn[i].btState = STATE_ACTIVE;
                    needDraw |= (2 << i);
                    if (i == 0)
                    {
                        if (mi.msBitmap == msNormal) msNew = msWait;
                        else msNew = msNormal;
                    }
                    else if (i == 1) done = 1;
                }
                else if (mcd.mbx == 1)
                {
                    btn[i].btState = STATE_PRESSED;
                    needDraw |= (2 << i);
                }
                else if (btn[i].btState == STATE_NORM && mcd.mbx == 0)
                {
                    btn[i].btState = STATE_ACTIVE;
                    needDraw |= (2 << i);
                }
                else if (btn[i].btState == STATE_WAITING)
                {
                    if (mcd.mbx == 1)
                    {
                        btn[i].btState = STATE_PRESSED;
                    }
                    else
                    {
                        btn[i].btState = STATE_ACTIVE;
                    }
                    needDraw |= (2 << i);
                }
            }
            else if (btn[i].btState == STATE_ACTIVE)
            {
                btn[i].btState = STATE_NORM;
                needDraw |= (2 << i);
            }
            else if (btn[i].btState == STATE_PRESSED && mcd.mbx == 1)
            {
                btn[i].btState = STATE_WAITING;
                needDraw |= (2 << i);
            }
            else if (btn[i].btState == STATE_WAITING && mcd.mbx == 0)
            {
                btn[i].btState = STATE_NORM;
                needDraw |= (2 << i);
            }
        }
    }

    // release mouse bitmap and callback handler
    CloseMouseButton(&mi, mbm, btn);
    UnInstallMouseHandler();
}

///////////////////// END OF GFXLIB.C ///////////////////////////////////
int main()
{
    VBE_FAR_CALL        fcall;
    VBE_DRIVER_INFO     drvInfo;
    VBE_PM_INFO_BLOCK   *pmInfo;

    unsigned int val = 0;
    unsigned int i = 0;
    unsigned char biosCheckSum = 0;

    unsigned char *biosCode;
    unsigned char *biosData;
    unsigned char *biosStack;
    unsigned char *biosPtr;

    unsigned short biosDataSel;
    unsigned short biosCodeSel;
    unsigned short a0000Sel;
    unsigned short b0000Sel;
    unsigned short b8000Sel;

    unsigned short biosInitSel;
    unsigned short biosStackSel;

    unsigned short vbeInfoSel;

    // copy ROM BIOS code from physical address 0xC0000 to RAM
    biosCode = (unsigned char*)malloc(VBE_CODE_SIZE);
    if (!biosCode) return 1;
    memcpy(biosCode, (unsigned char*)0xC0000, VBE_CODE_SIZE);

    // find VESA 3.0 protected mode info block signature
    biosPtr = biosCode;
    while ((biosPtr <= biosCode + VBE_CODE_SIZE - sizeof(VBE_PM_INFO_BLOCK)) && memcmp(((VBE_PM_INFO_BLOCK*)biosPtr)->Signature, "PMID", 4)) biosPtr++;

    // check for correct signature
    pmInfo = (VBE_PM_INFO_BLOCK*)biosPtr;
    if (memcmp(pmInfo->Signature, "PMID", 4))
    {
        printf("VESA PMID not found!\n");
        return 1;
    }

    // calculate BIOS checksum
    for (i = 0; i != sizeof(VBE_PM_INFO_BLOCK); i++) biosCheckSum += *biosPtr++;
    if (biosCheckSum)
    {
        printf("VESA BIOS checksum error!\n");
        return 1;
    }

    // setup structure (provide selectors, map video mem, ...)
    biosData = (unsigned char *)malloc(VBE_DATA_SIZE);
    if (!biosData) return 1;
    memset(biosData, 0, VBE_DATA_SIZE);

    // setup BIOS data selector
    biosDataSel = AllocSelector();
    if (biosDataSel == 0 || biosDataSel == 0xFFFF) return 1;
    if (!SetSelectorRights(biosDataSel, 0x8092)) return 1;
    if (!SetSelectorBase(biosDataSel, (unsigned int)biosData)) return 1;
    if (!SetSelectorLimit(biosDataSel, VBE_DATA_SIZE - 1)) return 1;
    pmInfo->BIOSDataSel = biosDataSel;

    // map video memory
    a0000Sel = AllocSelector();
    if (a0000Sel == 0 || a0000Sel == 0xFFFF) return 1;
    if (!SetSelectorRights(a0000Sel, 0x8092)) return 1;
    if (!SetSelectorBase(a0000Sel, (unsigned int)0xA0000)) return 1;
    if (!SetSelectorLimit(a0000Sel, 0xFFFF)) return 1;
    pmInfo->A0000Sel = a0000Sel;

    b0000Sel = AllocSelector();
    if (b0000Sel == 0 || b0000Sel == 0xFFFF) return 1;
    if (!SetSelectorRights(b0000Sel, 0x8092)) return 1;
    if (!SetSelectorBase(b0000Sel, (unsigned int)0xB0000)) return 1;
    if (!SetSelectorLimit(b0000Sel, 0xFFFF)) return 1;
    pmInfo->B0000Sel = b0000Sel;

    b8000Sel = AllocSelector();
    if (b8000Sel == 0 || b8000Sel == 0xFFFF) return 1;
    if (!SetSelectorRights(b8000Sel, 0x8092)) return 1;
    if (!SetSelectorBase(b8000Sel, (unsigned int)0xB8000)) return 1;
    if (!SetSelectorLimit(b8000Sel, 0x7FFF)) return 1;
    pmInfo->B8000Sel = b8000Sel;

    // setup BIOS code selector
    biosCodeSel = AllocSelector();
    if (biosCodeSel == 0 || biosCodeSel == 0xFFFF) return 1;
    if (!SetSelectorRights(biosCodeSel, 0x8092)) return 1;
    if (!SetSelectorBase(biosCodeSel, (unsigned int)biosCode)) return 1;
    if (!SetSelectorLimit(biosCodeSel, VBE_CODE_SIZE - 1)) return 1;
    pmInfo->CodeSegSel = biosCodeSel;

    // put BIOS code run in protect mode
    pmInfo->InProtectMode = 1;

    // alloc code segment selector for initialize function
    biosInitSel = AllocSelector();
    if (biosInitSel == 0 || biosInitSel == 0xFFFF) return 1;
    if (!SetSelectorRights(biosInitSel, 0x8092)) return 1;
    if (!SetSelectorBase(biosInitSel, (unsigned int)biosCode)) return 1;
    if (!SetSelectorLimit(biosInitSel, VBE_CODE_SIZE - 1)) return 1;

    // alloc stack selector
    biosStack = (unsigned char *)malloc(VBE_STACK_SIZE);
    if (!biosStack) return 1;
    biosStackSel = AllocSelector();
    if (biosStackSel == 0 || biosStackSel == 0xFFFF) return 1;
    if (!SetSelectorRights(biosStackSel, 0x8092)) return 1;
    if (!SetSelectorBase(biosStackSel, (unsigned int)biosStack)) return 1;
    if (!SetSelectorLimit(biosStackSel, VBE_STACK_SIZE - 1)) return 1;

    // call initialize protect mode function first
    fcall.offset = pmInfo->PMInitialize;
    fcall.segment = biosInitSel;
    _asm {
        pusha
        mov    ax, biosStackSel
        mov    ss, ax
        mov    sp, 0
        lea    esi, fcall
        call   fword ptr [esi]
        popa
    }

    // call initialize VBE controller
    vbeInfoSel = AllocSelector();
    if (vbeInfoSel == 0 || vbeInfoSel == 0xFFFF) return 1;
    if (!SetSelectorRights(vbeInfoSel, 0x8092)) return 1;
    if (!SetSelectorBase(vbeInfoSel, (unsigned int)&drvInfo)) return 1;
    if (!SetSelectorLimit(vbeInfoSel, sizeof(VBE_DRIVER_INFO) - 1)) return 1;

    fcall.offset = pmInfo->EntryPoint;
    fcall.segment = pmInfo->CodeSegSel;
    _asm {
        mov    ax, vbeInfoSel
        mov    es, ax
        mov    eax, 0x4F00
        xor    edi, edi
        lea    esi, fcall
        call   fword ptr [esi]
        mov    val, eax
    }

    if (val == 0x004F && !memcmp(drvInfo.VBESignature, "VESA", 4) && drvInfo.VBEVersion >= 0x0200) printf("OK!\n");
    else printf("VESA 3.0 INIT FAILED!\n");

    return 0;
}
