/****************************************************************************
*
*                            Open Watcom Project
*
* Copyright (c) 2015-2016 The Open Watcom Contributors. All Rights Reserved.
*    Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
*
*  ========================================================================
*
*    This file contains Original Code and/or Modifications of Original
*    Code as defined in and that are subject to the Sybase Open Watcom
*    Public License version 1.0 (the 'License'). You may not use this file
*    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
*    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
*    provided with the Original Code and Modifications, and is also
*    available at www.sybase.com/developer/opensource.
*
*    The Original Code and all software distributed under the License are
*    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
*    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
*    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
*    NON-INFRINGEMENT. Please see the License for the specific language
*    governing rights and limitations under the License.
*
*  ========================================================================
*
* Description:  Definition of editor's global variables.
*
****************************************************************************/


#include "vi.h"
#include "win.h"
#include "menu.h"
#include "ex.h"
#include "rxsupp.h"
#include "memdmp.h"

/* strings */
const char          _NEAR MSG_CHARACTERS[] = "characters";
const char          _NEAR MSG_LINES[] = "lines";
const char          _NEAR MSG_PRESSANYKEY[] = "Press any key";
const char          _NEAR MSG_DELETEDINTOBUFFER[] = " deleted into buffer ";
const char          _NEAR MEMORIZE_MODE[] = "Memorize Mode ";
const char          _NEAR CONFIG_FILE[] = CFG_NAME;
const char          _NEAR SingleBlank[] = " ";
const char          _NEAR SingleSlash[] = "/";
const char          _NEAR SingleQuote[] = "\"";
const char _NEAR    * _NEAR BoolStr[] = {
    CONST_NEAR_STRING( "FALSE" ),
    CONST_NEAR_STRING( "TRUE" )
};
const char          _NEAR SpinData[] = { '-', '\\', '|', '/' };

/* event data */
event _NEAR EventList[] = {
    #define vi_pick( enum, modeless, insert, command, nm_bits, bits ) \
        modeless, insert, command, nm_bits, bits,
    #include "events.h"
    #undef vi_pick
};

/* mouse data */
#if !defined( __UNIX__ )
windim          MouseRow;
windim          MouseCol;
int             MouseStatus;
#endif
vi_mouse_event  LastMouseEvent = VI_MOUSE_NONE;

/* generic editing data */
long            NextAutoSave;
int             HalfPageLines;
vi_key          LastEvent;
int             SpinCount;
char            VideoPage;
char            *EXEName;
char            *BndMemory;
long            SystemRC;
int             FcbBlocksInUse;
mark            *MarkList;
fcb             *FcbThreadHead;
fcb             *FcbThreadTail;
info            *InfoHead;
info            *InfoTail;
info            *CurrentInfo;
file            *CurrentFile;
fcb             *CurrentFcb;
line            *CurrentLine;
line            *WorkLine = NULL;
i_mark          CurrentPos = { 1, 1 };
i_mark          LeftTopPos = { 1, 0 };
int             VirtualColumnDesired = 1;
window_id       current_window_id = NO_WINDOW;
window_id       message_window_id = NO_WINDOW;
window_id       status_window_id = NO_WINDOW;
window_id       menu_window_id = NO_WINDOW;
window_id       linenum_current_window_id = NO_WINDOW;
window_id       repeat_window_id = NO_WINDOW;
select_rgn      SelRgn;

/*
 * directory data
 */
direct_ent      * _NEAR DirFiles[MAX_FILES];
list_linenum    DirFileCount;

/*
 * key map data
 */
int             CurrentKeyMapCount;
vi_key          *CurrentKeyMap;
key_map         *KeyMaps;
key_map         *InputKeyMaps;

/*
 * savebuf data
 */
vi_key          _NEAR SavebufBound[MAX_SAVEBUFS] = {
    VI_KEY( CTRL_F1 ),
    VI_KEY( CTRL_F2 ),
    VI_KEY( CTRL_F3 ),
    VI_KEY( CTRL_F4 ),
    VI_KEY( CTRL_F5 ),
    VI_KEY( CTRL_F6 ),
    VI_KEY( CTRL_F7 ),
    VI_KEY( CTRL_F8 ),
    VI_KEY( CTRL_F9 )
};

int             CurrentSavebuf = 0;     /* this is 0 based */
char            LastSavebuf;    /* this is 1 based - users see it */
savebuf         _NEAR Savebufs[MAX_SAVEBUFS];
savebuf         _NEAR SpecialSavebufs[MAX_SPECIAL_SAVEBUFS + 1];
savebuf         *WorkSavebuf = &SpecialSavebufs[MAX_SPECIAL_SAVEBUFS];
int             SavebufNumber = NO_SAVEBUF; /* this is 0 based */

/*
 * undo data
 */
undo_stack      *UndoStack;
undo_stack      *UndoUndoStack;

/*
 * windows data
 */

#ifdef __WIN__
/* the ONE TRUE configuration - do not change this or nothing will work */
window_info cmdlinew_info =         { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 0, 23, 69, 25 },  true };
window_info statusw_info =          { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 69, 24, 79, 24 }, false };
window_info repcntw_info =          { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 28, 20, 43, 32 }, true };
window_info editw_info =            { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 0, 0, 79, 23 },   false };
window_info extraw_info =           { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 26, 2, 51, 18 },  true };
window_info filecw_info =           { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 4, 8, 75, 17 },   true };
window_info linenumw_info =         { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 1, 21, 8, 24 },   false };
window_info dirw_info =             { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 10, 2, 69, 17 },  true };
window_info filelistw_info =        { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 26, 2, 78, 18 },  true };
window_info setw_info =             { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 12, 2, 40, 21 },  true };
window_info setvalw_info =          { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 43, 6, 70, 9 },   true };
window_info messagew_info =         { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 0, 25, 69, 25 },  false };
window_info menuw_info =            { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 0, 1, 0, 0 },     true };
window_info menubarw_info =         { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 0, 0, 79, 0 },    false };
window_info defaultw_info =         { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 0, 1, 79, 22 },   true };
window_info activemenu_info =       { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 0, 0, 0, 0 },     false };
window_info greyedmenu_info =       { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 0, 0, 0, 0 },     false };
window_info activegreyedmenu_info = { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 0, 0, 0, 0 },     false };
#else
window_info cmdlinew_info =         { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 0, 24, 79, 24 },  false };
window_info statusw_info =          { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 69, 24, 79, 24 }, false };
window_info repcntw_info =          { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 28, 20, 43, 32 }, true };
window_info editw_info =            { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 0, 0, 79, 24 },   false };
window_info extraw_info =           { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 26, 2, 51, 18 },  true };
window_info filecw_info =           { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 4, 8, 75, 17 },   true };
window_info linenumw_info =         { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 1, 21, 8, 24 },   false };
window_info dirw_info =             { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 10, 2, 69, 17 },  true };
window_info filelistw_info =        { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 26, 2, 78, 18 },  true };
window_info setw_info =             { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 12, 2, 40, 21 },  true };
window_info setvalw_info =          { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 43, 6, 70, 9 },   true };
window_info messagew_info =         { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 0, 24, 79, 24 },  false };
window_info menuw_info =            { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 0, 1, 0, 0 },     true };
window_info menubarw_info =         { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 0, 0, 79, 0 },    false };
window_info defaultw_info =         { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 0, 1, 79, 22 },   false };
window_info activemenu_info =       { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 0, 0, 0, 0 },     false };
window_info greyedmenu_info =       { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 0, 0, 0, 0 },     false };
window_info activegreyedmenu_info = { WHITE, BLACK, DEF_TEXT_STYLE, DEF_HILIGHT_STYLE, { 0, 0, 0, 0 },     false };
#endif

/*
 * file io data
 */
char            *CommandBuffer = NULL;
char            *CurrentDirectory = NULL;
char            *HomeDirectory = NULL;
char            *ReadBuffer = NULL;
char            *WriteBuffer = NULL;
unsigned char   *SwapBlocks = NULL;
int             SwapBlockArraySize;
int             SwapBlocksInUse;
char            *Comspec = NULL;

/*
 * misc data
 */
vi_rc           LastRetCode;
vi_rc           LastRC;
long            MaxMemFree;
long            MaxMemFreeAfterInit;
int             RegExpError;
regexp          *CurrentRegularExpression = NULL;
char            * _NEAR MatchData[MAX_SEARCH_STRINGS * 2];
int             MatchCount = INITIAL_MATCH_COUNT;
vi_rc           LastError;
int             maxdotbuffer = 1024;
vi_key          *DotBuffer = NULL;
vi_key          *AltDotBuffer = NULL;
vi_key          *DotCmd = NULL;
int             DotDigits;
int             DotCount;
int             DotCmdCount;
int             AltDotDigits;
int             AltDotCount;
volatile long   ClockTicks;
int             RepeatDigits;
bool            NoRepeatInfo;
char            _NEAR RepeatString[MAX_REPEAT_STRING];
int             SourceErrCount;
bool            BoundData = false;
bool            RCSActive = false;

#define INITVARS
/*
 * edit flags
 */
eflags EditFlags = {
    #define PICK( a,b,c,d,e )   d,
    #include "setb.h"
    #undef PICK

    /*
     * internal booleans are here
     */
    #define PICK( a,b )         b,
    #include "setbi.h"
    #undef PICK
};

/*
 * edit vars
 */
evars EditVars = {
    #define PICK( a,b,c,d,e,f ) e,
    #include "setnb.h"
    #undef PICK

    /*
     * internal vars are here
     */
    #define PICK( a,b,c )       c,
    #include "setnbi.h"
    #undef PICK
};
#undef INITVARS
