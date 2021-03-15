/** @file
  Copyright (C) 2021, Mike Beaton. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef OC_TYPING_LIB_H
#define OC_TYPING_LIB_H

#include <Uefi.h>
#include <Protocol/AppleEvent.h>

//
// Max. num. keystrokes buffered is one less than buffer size
//
#define OC_TYPING_BUFFER_SIZE    21

/**
  Typing buffer entry.
**/
typedef struct {
  APPLE_EVENT_TYPE            EventType;
  APPLE_MODIFIER_MAP          Modifiers;
  APPLE_KEY_CODE              KeyCode;
} OC_TYPING_BUFFER_ENTRY;

typedef struct {
  OC_TYPING_BUFFER_ENTRY      Buffer[OC_TYPING_BUFFER_SIZE];
  APPLE_EVENT_HANDLE          Handle;
  UINTN                       Head;
  UINTN                       Tail;
  APPLE_MODIFIER_MAP          CurrentModifiers;
  UINT16                      Pad1;
  UINT32                      Pad2;
} OC_TYPING_CONTEXT;

/**
  Register typing handler with Apple Event protocol.

  @param[out]     Context             Typing handler context.

  @retval EFI_SUCCESS                 Registered successfully.
  @retval EFI_OUT_OF_RESOURCES        Could not allocate buffer memory.
  @retval other                       An error returned by a sub-operation.
**/
EFI_STATUS
OcRegisterTypingHandler (
  OUT OC_TYPING_CONTEXT   **Context
  );

/**
  Unregister typing handler.

  @param[in]      Context             Typing handler context.

  @retval EFI_SUCCESS                 Unregistered successfully.
  @retval other                       An error returned by a sub-operation.
**/
EFI_STATUS
OcUnregisterTypingHandler (
   IN OC_TYPING_CONTEXT           *Context
  );

/**
  Get next keystroke from typing buffer. Will always return immediately.

  @param[in]      Context             Typing handler context.
  @param[out]     Modifiers       Current key modifiers, returned even if no key is available.
  @param[out]     KeyCode         Next keycode if one is available, zero otherwsie.
**/
VOID
OcGetNextKeystroke (
   IN OC_TYPING_CONTEXT           *Context,
  OUT APPLE_MODIFIER_MAP          *Modifiers,
  OUT APPLE_KEY_CODE              *KeyCode
  );

/**
  Flush typing buffer.

  @param[in]      Context             Typing handler context.

**/
VOID
OcFlushTypingBuffer (
   IN OC_TYPING_CONTEXT           *Context
  );

#endif // OC_TYPING_LIB_H
