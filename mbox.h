#ifndef MBOX_H
#define MBOX_H

#include <stdint.h>

// Mailbox channel 8: Property tags (ARM -> VC)
#define MBOX_CH_PROP  8

// Mailbox status codes returned by mbox_call
#define MBOX_SUCCESS  1
#define MBOX_ERROR    0

// Property tag identifiers
#define MBOX_TAG_GETSERIAL      0x00010004
#define MBOX_TAG_GETARMMEM      0x00010005
#define MBOX_TAG_GETVCMEM       0x00010006
#define MBOX_TAG_GETBOARDREV    0x00010002
#define MBOX_TAG_LAST           0x00000000

// Shared aligned mailbox buffer (must be 16-byte aligned)
extern volatile uint32_t mbox[36];

// Make a mailbox call on the given channel.
// Returns MBOX_SUCCESS on success, MBOX_ERROR on failure.
int mbox_call(uint8_t ch);

#endif // MBOX_H
