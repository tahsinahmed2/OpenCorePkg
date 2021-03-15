/** @file

Keyboard debug display.

Copyright (c) 2021, Mike Beaton. All rights reserved.

This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Uefi.h>
#include <IndustryStandard/AppleHid.h>
#include <Library/BaseLib.h>
#include <Library/OcKbDebugDisplayLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

STATIC UINT64               mTscFrequency = 0;

STATIC INT32                mRunningColumn;

STATIC UINT64               mPreviousTick;

STATIC UINT64               mLoopDelayStart;
STATIC UINT64               mLoopDelayEnd;

STATIC UINT64               mFlushDelayStart;
STATIC UINT64               mFlushDelayEnd;

#define OC_KB_DEBUG_MAX_COLUMN           80
#define OC_KB_DEBUG_DELTA_SAMPLE_COLUMN  0 //40

#define OC_KB_DEBUG_PRINT_ROW            2

#define OC_KB_DEBUG_UP_DOWN_ROW          7
#define OC_KB_DEBUG_FLUSH_ROW            OC_KB_DEBUG_UP_DOWN_ROW
#define OC_KB_DEBUG_X_ROW                8
#define OC_KB_DEBUG_MODIFIERS_ROW        9

VOID
OcInitKbDebugDisplay (
  UINT64 TscFrequency
  )
{
  mTscFrequency = TscFrequency;

  mRunningColumn = 0;

  mLoopDelayStart = 0;
  mLoopDelayEnd = 0;

  mFlushDelayStart = 0;
  mFlushDelayEnd = 0;
}

VOID
OcInstrumentLoopDelay (
  UINT64 LoopDelayStart,
  UINT64 LoopDelayEnd
  )
{
  mLoopDelayStart     = LoopDelayStart;
  mLoopDelayEnd       = LoopDelayEnd;
}

VOID
OcInstrumentFlushDelay (
  UINT64 FlushDelayStart,
  UINT64 FlushDelayEnd
  )
{
  mFlushDelayStart    = FlushDelayStart;
  mFlushDelayEnd      = FlushDelayEnd;
}

STATIC
VOID
ShowDeltas (
  UINT64                    CurrentTick,
  INT32                     RestoreRow
  )
{
  CONST CHAR16 *ClearSpace = L"      ";

  gST->ConOut->SetCursorPosition (gST->ConOut, 0, RestoreRow + OC_KB_DEBUG_PRINT_ROW);

  Print (
    L"mTscFrequency  = %,Lu\n",
    mTscFrequency);

  Print (
    L"Called delta   = %,Lu%s\n",
    CurrentTick - mPreviousTick,
    ClearSpace);

  Print (L"Loop delta     = %,Lu (@ -%,Lu)%s%s\n",
    mLoopDelayEnd == 0 ? 0 : mLoopDelayEnd - mLoopDelayStart,
    mLoopDelayEnd == 0 ? 0 : CurrentTick - mLoopDelayEnd,
    ClearSpace,
    ClearSpace);

  Print (L"Flush delta    = %,Lu (@ -%,Lu)%s%s\n",
    mFlushDelayEnd == 0 ? 0 : mFlushDelayEnd - mFlushDelayStart,
    mFlushDelayEnd == 0 ? 0 : CurrentTick - mFlushDelayEnd,
    ClearSpace,
    ClearSpace);
}

VOID
OcShowKbDebugDisplay (
  UINTN                     NumKeysUp,
  UINTN                     NumKeysDown,
  UINTN                     NumKeysHeld,
  APPLE_MODIFIER_MAP        Modifiers,
  CHAR16                    CallerID,
  BOOLEAN                   UsingDownkeys
  )
{
  UINT64                            CurrentTick;
  INT32                             RestoreRow;
  INT32                             RestoreColumn;

  //
  // Is initialised? Only initialised in OcShowSimpleBootMenu, to avoid spamming console with text in OpenCanopy.
  //
  if (mTscFrequency == 0) {
    return;
  }

  CHAR16                            Code[3]; // includes flush-ahead space, to make progress visible

  Code[1]         = L' ';
  Code[2]         = L'\0';

  RestoreRow      = gST->ConOut->Mode->CursorRow;
  RestoreColumn   = gST->ConOut->Mode->CursorColumn;

  CurrentTick     = AsmReadTsc();
  if (mRunningColumn == OC_KB_DEBUG_DELTA_SAMPLE_COLUMN) {
    ShowDeltas(CurrentTick, RestoreRow);
  }
  mPreviousTick = CurrentTick;

  if (UsingDownkeys) {
    //
    // Show downkeys info when in use
    //
    gST->ConOut->SetCursorPosition (gST->ConOut, mRunningColumn, RestoreRow + OC_KB_DEBUG_UP_DOWN_ROW);
    if (NumKeysUp > 0) {
      Code[0] = L'U';
    } else if (NumKeysDown > 0) {
      Code[0] = L'D';
    } else if (NumKeysHeld > 0) {
      Code[0] = L'-';
    } else {
      Code[0] = L' ';
    }
    gST->ConOut->OutputString (gST->ConOut, Code);
  } else {
    //
    // Show caller ID for flush or non-flush
    //
    gST->ConOut->SetCursorPosition (gST->ConOut, mRunningColumn, RestoreRow + OC_KB_DEBUG_FLUSH_ROW);
    Code[0] = CallerID;
    gST->ConOut->OutputString (gST->ConOut, Code);
  }

  //
  // Key held info
  //
  gST->ConOut->SetCursorPosition (gST->ConOut, mRunningColumn, RestoreRow + OC_KB_DEBUG_X_ROW);
  if (NumKeysHeld > 0) {
    Code[0] = L'X';
  } else {
    Code[0] = L'.';
  }
  gST->ConOut->OutputString (gST->ConOut, Code);

  //
  // Modifiers info
  //
  gST->ConOut->SetCursorPosition (gST->ConOut, mRunningColumn, RestoreRow + OC_KB_DEBUG_MODIFIERS_ROW);
  if (Modifiers == 0) {
    Code[0] = L' ';
    gST->ConOut->OutputString (gST->ConOut, Code);
  } else {
    Print(L"%d", Modifiers);
  }

  if (++mRunningColumn >= OC_KB_DEBUG_MAX_COLUMN) {
    mRunningColumn = 0;
  }

  gST->ConOut->SetCursorPosition (gST->ConOut, RestoreColumn, RestoreRow);
}
