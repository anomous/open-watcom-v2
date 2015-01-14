/****************************************************************************
*
*                            Open Watcom Project
*
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
* Description:  Declaration of RFX functions for local side.
*
****************************************************************************/

extern rc_erridx LocalMkDir( const char * );
extern rc_erridx LocalRmDir( const char * );
extern rc_erridx LocalGetCwd( int, char * );
extern rc_erridx LocalSetCWD( const char * );
extern int       LocalGetDrv( void );
extern rc_erridx LocalSetDrv( int );
extern rc_erridx LocalFindFirst( const char *, void *, unsigned, int );
extern int       LocalFindNext( void *, unsigned );
extern long      LocalGetFileAttr( const char * );
extern rc_erridx LocalSetFileAttr( const char *, long );
extern long      LocalGetFreeSpace( int );
extern rc_erridx LocalRename( const char *, const char * );
extern void      LocalTime( int *hour, int *min, int *sec, int *hundredths );
extern void      LocalDate( int *year, int *month, int *day, int *weekday );
extern rc_erridx LocalDateTime( sys_handle, int *, int *, int );
extern int       LocalInteractive( sys_handle );
extern void      LocalGetBuff( char *, unsigned );
