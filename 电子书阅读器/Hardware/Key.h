#ifndef __KEY_H
#define __KEY_H

#include <stdint.h>

#define KEY_NONE	0
#define KEY_NEXT	1		/* PB1：下一页 */
#define KEY_PREV	2		/* PB11：上一页 */

void Key_Init(void);
uint8_t Key_GetRaw(void);

#endif
