/** @file
  Copyright (C) 2021, Mike Beaton. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef OC_KB_DEBUG_DISPLAY_LIB_H
#define OC_KB_DEBUG_DISPLAY_LIB_H

/**
  Initialise running display of held keys, for builtin picker only.

  @param[in]      TscFrequency      Send in current understood TSC frequency.
**/
VOID
OcInitKbDebugDisplay (
  UINT64 TscFrequency
  );

/**
  Instrument old path kb loop delay.

  @param[in]      LoopDelayStart    Delay start in TSC asm ticks.
  @param[in]      LoopDelayEnd      Delay end in TSC asm ticks.
**/
VOID
OcInstrumentLoopDelay (
  UINT64 LoopDelayStart,
  UINT64 LoopDelayEnd
  );

/**
  Instrument old path kb flush delay.

  @param[in]      FlushDelayStart   Delay start in TSC asm ticks.
  @param[in]      FlushDelayEnd     Delay end in TSC asm ticks.
**/
VOID
OcInstrumentFlushDelay (
  UINT64 FlushDelayStart,
  UINT64 FlushDelayEnd
  );

/**
  Running display of held keys, in builtin picker only.

  @param[in]      NumKeysUp       Number of keys that went up.
  @param[in]      NumKeysDown     Number of keys that went down.
  @param[in]      NumKeysHeld     Number of keys held.
  @param[in]      Modifiers       Key modifiers.
  @param[in]      CallerID        Caller ID to display if not using downkeys handler.
  @param[in]      UsingDownkeys   Set to true if using downkeys handler in the kb loop.
**/
VOID
OcShowKbDebugDisplay (
  UINTN                     NumKeysUp,
  UINTN                     NumKeysDown,
  UINTN                     NumKeysHeld,
  APPLE_MODIFIER_MAP        Modifiers,
  CHAR16                    CallerID,
  BOOLEAN                   UsingDownkeys
  );

#endif // OC_KB_DEBUG_DISPLAY_LIB_H
