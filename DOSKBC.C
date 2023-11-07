#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "vga.h"

#define IRQ_PORT     0x20
#define KBC_IRQ      0x09
#define KBC_PORT     0x60
#define KBC_SCP      0x61
#define KBC_BREAK    0x80
#define SCP_KBC_DIS  0x80

#define KEY_ENHANCED 0xE0
#define KEY_ESC      0x01
#define KEY_W        0x11
#define KEY_A        0x1E
#define KEY_S        0x1F
#define KEY_D        0x20
#define KEY_SHIFT    0x2A
#define KEY_CTRL     0x1D

volatile byte keys[256];

void interrupt ( *old_irq )();

void interrupt kbc_irq_handler()
{
    byte code, state, val = 1;
    disable();
    code = inportb( KBC_PORT );
    state = inportb( KBC_PORT + 1);
    if( code & KBC_BREAK ) {
        val = 0;
        code -= KBC_BREAK;
    } else {
        val = 1;
    }
    keys[code] = val;
    /* Acknowledge scancode by toggling bit 7 of SCP */
    outportb( KBC_SCP, state | SCP_KBC_DIS );
    outportb( KBC_SCP, state );
    /* EOI - End Of Interrupt code */
    /* Acknowledges that IRQ has been served */
    outportb( IRQ_PORT, 0x20);
    enable();
}

int main()
{
    word x = 160, y = 160;
    byte col = 15;
    memset(keys, 0, 256);
    set_graphics_mode();
    old_irq = getvect( KBC_IRQ );
    setvect( KBC_IRQ, kbc_irq_handler );
    while(keys[KEY_ESC] == 0) {
        SETPIX(x,y,col);
        if( keys[KEY_W] ) y = (y >   0 ? y - 1 : y);
        if( keys[KEY_S] ) y = (y < 199 ? y + 1 : y);
        if( keys[KEY_A] ) x = (x >   0 ? x - 1 : x);
        if( keys[KEY_D] ) x = (x < 319 ? x + 1 : x);
        if( keys[KEY_SHIFT] ) col = (col < 255 ? col + 1 : col );
	if( keys[KEY_CTRL] ) col = (col > 0 ? col - 1 : col );
        wait_for_retrace();
    }
    set_text_mode();
    setvect( KBC_IRQ, old_irq );

    return 0;
}
