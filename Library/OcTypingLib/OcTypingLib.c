/** @file

Apple Event typing.

Copyright (c) 2021, Mike Beaton. All rights reserved.

This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#if defined(OC_TARGET_DEBUG) || defined(OC_TARGET_NOOPT)
//#define DEBUG_DETAIL
#endif

#include <Uefi.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/OcDebugLogLib.h>
#include <Library/OcTypingLib.h>
#include <Library/OcAppleEventLib.h>

STATIC APPLE_EVENT_PROTOCOL     *mProtocol;

STATIC
VOID
EFIAPI
HandleKeyEvent (
  IN APPLE_EVENT_INFORMATION    *Information,
  IN OC_TYPING_CONTEXT          *Context
  )
{
  APPLE_KEY_CODE        NewKeyCode;
  UINTN                 NewHead;

  NewKeyCode = 0;
  if (Information->EventData.KeyData != NULL) {
    NewKeyCode = Information->EventData.KeyData->AppleKeyCode;
    DEBUG_CODE_BEGIN();
    ASSERT (NewKeyCode != 0);
    DEBUG_CODE_END();
  }

  DETAIL_DEBUG ((DEBUG_INFO, "OCTY: HandleKeyEvent[%p] %x:%d[%x]\n", &HandleKeyEvent, Information->EventType, Information->Modifiers, NewKeyCode));
  //DEBUG ((DEBUG_INFO, "OCTY: HandleKeyEvent[%p] %x:%d[%x]\n", &HandleKeyEvent, Information->EventType, Information->Modifiers, NewKeyCode));

  NewHead = Context->Head + 1;
  if (NewHead >= OC_TYPING_BUFFER_SIZE) {
    NewHead = 0;
  }

  //
  // Intentionally overwrite last item if queue is full
  // TODO: Test with tiny queue and long delay
  //
  if (NewHead != Context->Tail) {
    Context->Head = NewHead;
  }

  DETAIL_DEBUG ((DEBUG_INFO, "OCTY: Writing to %d\n", Context->Head));

  Context->Buffer[Context->Head].EventType  = Information->EventType;
  Context->Buffer[Context->Head].KeyCode    = NewKeyCode;
  Context->Buffer[Context->Head].Modifiers  = Information->Modifiers;
}

EFI_STATUS
OcRegisterTypingHandler (
  OUT OC_TYPING_CONTEXT   **Context
  )
{
  EFI_STATUS    Status;

  DETAIL_DEBUG ((DEBUG_INFO, "OCTY: OcRegisterTypingHandler\n"));

  *Context = NULL;

  Status = gBS->LocateProtocol (
    &gAppleEventProtocolGuid,
    NULL,
    (VOID **) &mProtocol
    );

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "OCTY: Failed to locate apple event protocol - %r\n", Status));
    return Status;
  }

  DETAIL_DEBUG ((DEBUG_INFO, "OCTY: About to allocate %d x %d = %d + a bit\n", OC_TYPING_BUFFER_SIZE, sizeof(OC_TYPING_BUFFER_ENTRY), sizeof(OC_TYPING_CONTEXT)));

  *Context = AllocatePool (sizeof(OC_TYPING_CONTEXT));

  if (*Context == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    DEBUG ((DEBUG_ERROR, "OCTY: Failed to allocate context - %r\n", Status));
    return Status;
  }

  DETAIL_DEBUG ((DEBUG_INFO, "OCTY: RegisterHandler (F, [%p], %p, %p)\n", &HandleKeyEvent, &(*Context)->Handle, *Context));
  Status = mProtocol->RegisterHandler (
    APPLE_ALL_KEYBOARD_EVENTS,
    (APPLE_EVENT_NOTIFY_FUNCTION)&HandleKeyEvent,
    &(*Context)->Handle,
    *Context
  );

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "OCTY: Failed to register handler - %r\n", Status));
    FreePool (*Context);
    *Context = NULL;
    return Status;
  }

  DETAIL_DEBUG ((DEBUG_INFO, "OCTY: C, H = %p, %p\n", *Context, (*Context)->Handle));

  //
  // Init
  //
  OcFlushTypingBuffer (*Context);

  DEBUG ((DEBUG_INFO, "OCTY: Registered\n"));

  return EFI_SUCCESS;
}

EFI_STATUS
OcUnregisterTypingHandler (
  IN OC_TYPING_CONTEXT   *Context
  )
{
  EFI_STATUS    Status;

  DETAIL_DEBUG ((DEBUG_INFO, "OCTY: OcUnregisterTypingHandler\n"));

  Status = mProtocol->UnregisterHandler(
    Context->Handle
  );

  FreePool (Context);

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "OCTY: Failed to unregister handler - %r\n", Status));
  }

  return Status;
}

VOID
OcGetNextKeystroke (
   IN OC_TYPING_CONTEXT           *Context,
  OUT APPLE_MODIFIER_MAP          *Modifiers,
  OUT APPLE_KEY_CODE              *KeyCode
  )
{
  APPLE_MODIFIER_MAP          UpdatedModifiers;
  APPLE_MODIFIER_MAP          IncomingModifiers;

  OC_TYPING_BUFFER_ENTRY      *Entry;

  DETAIL_DEBUG ((DEBUG_INFO, "OCTY: OcGetNextKeystroke(%p, M, K)\n", Context));

  *KeyCode    = 0;

  if (Context->Tail == Context->Head) {
    DETAIL_DEBUG ((DEBUG_INFO, "-"));
    *Modifiers  = Context->CurrentModifiers;
    return;
  }

  DETAIL_DEBUG ((DEBUG_INFO, "OCTY: %d != %d\n", Context->Tail, Context->Head));

  ++(Context->Tail);
  if (Context->Tail >= OC_TYPING_BUFFER_SIZE) {
    Context->Tail = 0;
  }

  DETAIL_DEBUG ((DEBUG_INFO, "OCTY: Reading from %d\n", Context->Tail));

  Entry = &Context->Buffer[Context->Tail];

  if ((Entry->EventType & APPLE_EVENT_TYPE_KEY_DOWN) != 0) {
    *KeyCode = Entry->KeyCode;
  }

  IncomingModifiers   = Entry->Modifiers;
  UpdatedModifiers    = Context->CurrentModifiers;

  if ((Entry->EventType & APPLE_EVENT_TYPE_MODIFIER_UP) != 0) {
    UpdatedModifiers &= ~IncomingModifiers;
  } else {
    UpdatedModifiers = IncomingModifiers;
  }

  Context->CurrentModifiers = UpdatedModifiers;
  *Modifiers = UpdatedModifiers;

  //DEBUG ((DEBUG_INFO, "OCTY: OcGetNextKeystroke @%d %x:%d[%x] => %d[%x]\n", Context->Tail, Entry->EventType, Entry->Modifiers, Entry->KeyCode, *Modifiers, *KeyCode));
}

VOID
OcFlushTypingBuffer (
   IN OC_TYPING_CONTEXT           *Context
  )
{
  Context->Tail = 0;
  Context->Head = 0;
  Context->CurrentModifiers = 0;
  DETAIL_DEBUG ((DEBUG_INFO, "OCTY: OcFlushTypingBuffer %d %d %d\n", Context->Tail, Context->Head, Context->CurrentModifiers));
}
