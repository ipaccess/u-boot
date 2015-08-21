/*
 * Copyright(c) 2007-2014 Intel Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 * The full GNU General Public License is included in this distribution 
 * in the file called LICENSE.GPL.
 *
 * Contact Information:
 * Intel Corporation
 */


static const unsigned char AD9548Register100BankValues[] =
{
    0x48, /* Register:&h100 Value: Binary:&b01001000 Decimal: 072 */
    0x34, /* Register:&h101 Value: Binary:&b00110100 Decimal: 052 */
    0x45, /* Register:&h102 Value: Binary:&b01000101 Decimal: 069 */
    0x83, /* Register:&h103 Value: Binary:&b10000011 Decimal: 131 */
    0x48, /* Register:&h104 Value: Binary:&b01001000 Decimal: 072 */
    0x0F, /* Register:&h105 Value: Binary:&b00001111 Decimal: 015 */
    0x01, /* Register:&h106 Value: Binary:&b00000001 Decimal: 001 */
    0x00, /* Register:&h107 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h108 Value: Binary:&b00000000 Decimal: 000 */
};

static const unsigned char AD9548Register300BankValues[] =
{
    0xD9, /* Register:&h300 Value: Binary:&b11011001 Decimal: 217 */
    0x89, /* Register:&h301 Value: Binary:&b10001001 Decimal: 137 */
    0x9D, /* Register:&h302 Value: Binary:&b10011101 Decimal: 157 */
    0xD8, /* Register:&h303 Value: Binary:&b11011000 Decimal: 216 */
    0x89, /* Register:&h304 Value: Binary:&b10001001 Decimal: 137 */
    0x3D, /* Register:&h305 Value: Binary:&b00111101 Decimal: 061 */
    0x00, /* Register:&h306 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h307 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h308 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h309 Value: Binary:&b00000000 Decimal: 000 */
    0xFF, /* Register:&h30A Value: Binary:&b11111111 Decimal: 255 */
    0xFF, /* Register:&h30B Value: Binary:&b11111111 Decimal: 255 */
    0xFF, /* Register:&h30C Value: Binary:&b11111111 Decimal: 255 */
    0x00, /* Register:&h30D Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h30E Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h30F Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h310 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h311 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h312 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h313 Value: Binary:&b00000000 Decimal: 000 */
    0xE8, /* Register:&h314 Value: Binary:&b11101000 Decimal: 232 */
    0x03, /* Register:&h315 Value: Binary:&b00000011 Decimal: 003 */
    0x00, /* Register:&h316 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h317 Value: Binary:&b00000000 Decimal: 000 */
    0x30, /* Register:&h318 Value: Binary:&b00110000 Decimal: 048 */
    0x75, /* Register:&h319 Value: Binary:&b01110101 Decimal: 117 */
    0x00, /* Register:&h31A Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h31B Value: Binary:&b00000000 Decimal: 000 */
};

static const unsigned char AD9548Register400BankValues[] =
{
    0x04, // 0x400: Output 2 (clock to AD9553 on T2200 EVM) powered down
    0x0B, // 0x401: Enable all but output 2
    0x00, // 0x402:
    0x01, // 0x403: Sync on DPLL frequency lock
    0x08, // 0x404: OUT0 control, CMOS both pins, normal power
    0x08, // 0x405: OUT1 control, CMOS both pins, normal power
    0x03, // 0x406: OUT2 control, both pins tristated
    0x03, // 0x407: OUT3 control, both pins tristated

    // OUT0 divisor:
    0x17, /* Register:&h408 Value: Binary:&b00010111 Decimal: 023 */
    0x00, /* Register:&h409 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h40A Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h40B Value: Binary:&b00000000 Decimal: 000 */

#define AD9548_DIVISOR_100HZ ((240000000 / 100) - 1)
    // OUT1 divisor: Divide by 240 MHz by 100, then minus 1 to get divisor for 100 Hz output
    ((AD9548_DIVISOR_100HZ >>  0) & 0xFF), /* Register:&h40C */
    ((AD9548_DIVISOR_100HZ >>  8) & 0xFF), /* Register:&h40D  */
    ((AD9548_DIVISOR_100HZ >> 16) & 0xFF), /* Register:&h40E  */
    ((AD9548_DIVISOR_100HZ >> 24) & 0xFF), /* Register:&h40F  */

    // OUT2 divisor:
    0x05, /* Register:&h410 Value: Binary:&b00000101 Decimal: 005 */
    0x00, /* Register:&h411 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h412 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h413 Value: Binary:&b00000000 Decimal: 000 */

    // OUT3 divisor:
    0x17, /* Register:&h414 Value: Binary:&b00010111 Decimal: 023 */
    0x00, /* Register:&h415 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h416 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h417 Value: Binary:&b00000000 Decimal: 000 */

    0x00, /* Register:&h418 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h419 Value: Binary:&b00000000 Decimal: 000 */
};

static const unsigned char AD9548Register500BankValues[] =
{
    0x00, /* Register:&h500 Value: Binary:&b10111111 Decimal: 191 */ // set to 0 (allow all inputs)
    // Register 0x501 Ref A, AA, B, BB logic family
    0xFA, /* Register:&h501 Value: Binary:&b11111010 Decimal: 250 */
    // Register 0x501 Ref C, CC, D, DD logic family
    0x9A, /* Register:&h502 Value: Binary:&b10011010 Decimal: 154 */
    // Register 0x503 Ref A and AA profile selection
    0xC0, /* Ref AA manual profile 4 (25 MHz), Ref A automatic: Register:&h503 Value: Binary:&b11000000 Decimal: 192 */
    // Register 0x504 Ref B and BB profile selection
    0x08, /* Ref BB automatic, Ref B profile 0 (1 Hz): Register:&h504 Value: Binary:&b10001000 Decimal: 008 */
    // Register 0x505 Ref C and CC profile selection
    0x0B, /* Ref CC automatic, Ref C manual profile 3 (19.2MHz), Register:&h505 Value: Binary:&b00001011 Decimal: 011 */
    // Register 0x506 Ref D and DD profile selection
    0x00, /* Ref D and DD automatic profile select: Register:&h506 Value: Binary:&b00000000 Decimal: 000 */
    // Register 0x507 Threshold priority level
    0x00, /* Register:&h507 Value: Binary:&b00000000 Decimal: 000 */ // Forced to 7 (allow all priorities)
};

static const unsigned char AD9548Register600BankValues[] =
{
    0x00, /* Register:&h600 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h601 Value: Binary:&b00000000 Decimal: 000 */
    0x80, /* Register:&h602 Value: Binary:&b10000000 Decimal: 128 */
    0xC6, /* Register:&h603 Value: Binary:&b11000110 Decimal: 198 */
    0xA4, /* Register:&h604 Value: Binary:&b10100100 Decimal: 164 */
    0x7E, /* Register:&h605 Value: Binary:&b01111110 Decimal: 126 */
    0x8D, /* Register:&h606 Value: Binary:&b10001101 Decimal: 141 */
    0x03, /* Register:&h607 Value: Binary:&b00000011 Decimal: 003 */
    0xC8, /* Register:&h608 Value: Binary:&b11001000 Decimal: 200 */
    0x00, /* Register:&h609 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h60A Value: Binary:&b00000000 Decimal: 000 */
    0xC8, /* Register:&h60B Value: Binary:&b11001000 Decimal: 200 */
    0x00, /* Register:&h60C Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h60D Value: Binary:&b00000000 Decimal: 000 */
    0xB8, /* Register:&h60E Value: Binary:&b10111000 Decimal: 184 */
    0x0B, /* Register:&h60F Value: Binary:&b00001011 Decimal: 011 */
    0xB8, /* Register:&h610 Value: Binary:&b10111000 Decimal: 184 */
    0x0B, /* Register:&h611 Value: Binary:&b00001011 Decimal: 011 */
    0xDA, /* Register:&h612 Value: Binary:&b11011010 Decimal: 218 */
    0xA6, /* Register:&h613 Value: Binary:&b10100110 Decimal: 166 */
    0x05, /* Register:&h614 Value: Binary:&b00000101 Decimal: 005 */
    0x5E, /* Register:&h615 Value: Binary:&b01011110 Decimal: 094 */
    0xF2, /* Register:&h616 Value: Binary:&b11110010 Decimal: 242 */
    0x62, /* Register:&h617 Value: Binary:&b01100010 Decimal: 098 */
    0x26, /* Register:&h618 Value: Binary:&b00100110 Decimal: 038 */
    0x86, /* Register:&h619 Value: Binary:&b10000110 Decimal: 134 */
    0x31, /* Register:&h61A Value: Binary:&b00110001 Decimal: 049 */
    0x60, /* Register:&h61B Value: Binary:&b01100000 Decimal: 096 */
    0xEC, /* Register:&h61C Value: Binary:&b11101100 Decimal: 236 */
    0x0B, /* Register:&h61D Value: Binary:&b00001011 Decimal: 011 */
    0x00, /* Register:&h61E Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h61F Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h620 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h621 Value: Binary:&b00000000 Decimal: 000 */
    0xFF, /* Register:&h622 Value: Binary:&b11111111 Decimal: 255 */
    0x1B, /* Register:&h623 Value: Binary:&b00011011 Decimal: 027 */
    0x4E, /* Register:&h624 Value: Binary:&b01001110 Decimal: 078 */
    0x0E, /* Register:&h625 Value: Binary:&b00001110 Decimal: 014 */
    0x00, /* Register:&h626 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h627 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h628 Value: Binary:&b00000000 Decimal: 000 */
    0xE8, /* Register:&h629 Value: Binary:&b11101000 Decimal: 232 */
    0xFD, /* Register:&h62A Value: Binary:&b11111101 Decimal: 253 */
    0x32, /* Register:&h62B Value: Binary:&b00110010 Decimal: 050 */
    0x32, /* Register:&h62C Value: Binary:&b00110010 Decimal: 050 */
    0xA0, /* Register:&h62D Value: Binary:&b10100000 Decimal: 160 */
    0x25, /* Register:&h62E Value: Binary:&b00100101 Decimal: 037 */
    0x26, /* Register:&h62F Value: Binary:&b00100110 Decimal: 038 */
    0x20, /* Register:&h630 Value: Binary:&b00100000 Decimal: 032 */
    0x44, /* Register:&h631 Value: Binary:&b01000100 Decimal: 068 */
    0x00, /* Register:&h632 Value: Binary:&b00000000 Decimal: 000 */
    0x40, /* Register:&h633 Value: Binary:&b01000000 Decimal: 064 */
    0x78, /* Register:&h634 Value: Binary:&b01111000 Decimal: 120 */
    0x7D, /* Register:&h635 Value: Binary:&b01111101 Decimal: 125 */
    0x01, /* Register:&h636 Value: Binary:&b00000001 Decimal: 001 */
    0x00, /* Register:&h637 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h638 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h639 Value: Binary:&b00000000 Decimal: 000 */
    0xC8, /* Register:&h63A Value: Binary:&b11001000 Decimal: 200 */
    0x00, /* Register:&h63B Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h63C Value: Binary:&b00000000 Decimal: 000 */
    0xC8, /* Register:&h63D Value: Binary:&b11001000 Decimal: 200 */
    0x00, /* Register:&h63E Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h63F Value: Binary:&b00000000 Decimal: 000 */
    0xB8, /* Register:&h640 Value: Binary:&b10111000 Decimal: 184 */
    0x0B, /* Register:&h641 Value: Binary:&b00001011 Decimal: 011 */
    0xB8, /* Register:&h642 Value: Binary:&b10111000 Decimal: 184 */
    0x0B, /* Register:&h643 Value: Binary:&b00001011 Decimal: 011 */
    0xDB, /* Register:&h644 Value: Binary:&b11011011 Decimal: 219 */
    0xBE, /* Register:&h645 Value: Binary:&b10111110 Decimal: 190 */
    0x40, /* Register:&h646 Value: Binary:&b01000000 Decimal: 064 */
    0x4F, /* Register:&h647 Value: Binary:&b01001111 Decimal: 079 */
    0xAB, /* Register:&h648 Value: Binary:&b10101011 Decimal: 171 */
    0x37, /* Register:&h649 Value: Binary:&b00110111 Decimal: 055 */
    0x09, /* Register:&h64A Value: Binary:&b00001001 Decimal: 009 */
    0xDF, /* Register:&h64B Value: Binary:&b11011111 Decimal: 223 */
    0x1B, /* Register:&h64C Value: Binary:&b00011011 Decimal: 027 */
    0xD6, /* Register:&h64D Value: Binary:&b11010110 Decimal: 214 */
    0x69, /* Register:&h64E Value: Binary:&b01101001 Decimal: 105 */
    0x06, /* Register:&h64F Value: Binary:&b00000110 Decimal: 006 */
    0x0F, /* Register:&h650 Value: Binary:&b00001111 Decimal: 015 */
    0x27, /* Register:&h651 Value: Binary:&b00100111 Decimal: 039 */
    0x00, /* Register:&h652 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h653 Value: Binary:&b00000000 Decimal: 000 */
    0x5F, /* Register:&h654 Value: Binary:&b01011111 Decimal: 095 */
    0xEA, /* Register:&h655 Value: Binary:&b11101010 Decimal: 234 */
    0x00, /* Register:&h656 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h657 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h658 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h659 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h65A Value: Binary:&b00000000 Decimal: 000 */
    0xE8, /* Register:&h65B Value: Binary:&b11101000 Decimal: 232 */
    0xFD, /* Register:&h65C Value: Binary:&b11111101 Decimal: 253 */
    0x32, /* Register:&h65D Value: Binary:&b00110010 Decimal: 050 */
    0x32, /* Register:&h65E Value: Binary:&b00110010 Decimal: 050 */
    0xC4, /* Register:&h65F Value: Binary:&b11000100 Decimal: 196 */
    0x09, /* Register:&h660 Value: Binary:&b00001001 Decimal: 009 */
    0x00, /* Register:&h661 Value: Binary:&b00000000 Decimal: 000 */
    0x20, /* Register:&h662 Value: Binary:&b00100000 Decimal: 032 */
    0x44, /* Register:&h663 Value: Binary:&b01000100 Decimal: 068 */
};

static const unsigned char AD9548Register680BankValues[] =
{
    0x00, /* Register:&h680 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h681 Value: Binary:&b00000000 Decimal: 000 */
    0xE1, /* Register:&h682 Value: Binary:&b11100001 Decimal: 225 */
    0xF5, /* Register:&h683 Value: Binary:&b11110101 Decimal: 245 */
    0x05, /* Register:&h684 Value: Binary:&b00000101 Decimal: 005 */
    0x00, /* Register:&h685 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h686 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h687 Value: Binary:&b00000000 Decimal: 000 */
    0xE8, /* Register:&h688 Value: Binary:&b11101000 Decimal: 232 */
    0x03, /* Register:&h689 Value: Binary:&b00000011 Decimal: 003 */
    0x00, /* Register:&h68A Value: Binary:&b00000000 Decimal: 000 */
    0xE8, /* Register:&h68B Value: Binary:&b11101000 Decimal: 232 */
    0x03, /* Register:&h68C Value: Binary:&b00000011 Decimal: 003 */
    0x00, /* Register:&h68D Value: Binary:&b00000000 Decimal: 000 */
    0x64, /* Register:&h68E Value: Binary:&b01100100 Decimal: 100 */
    0x00, /* Register:&h68F Value: Binary:&b00000000 Decimal: 000 */
    0x64, /* Register:&h690 Value: Binary:&b01100100 Decimal: 100 */
    0x00, /* Register:&h691 Value: Binary:&b00000000 Decimal: 000 */
    0x93, /* Register:&h692 Value: Binary:&b10010011 Decimal: 147 */
    0xAA, /* Register:&h693 Value: Binary:&b10101010 Decimal: 170 */
    0x80, /* Register:&h694 Value: Binary:&b10000000 Decimal: 128 */
    0xFD, /* Register:&h695 Value: Binary:&b11111101 Decimal: 253 */
    0xB9, /* Register:&h696 Value: Binary:&b10111001 Decimal: 185 */
    0x33, /* Register:&h697 Value: Binary:&b00110011 Decimal: 051 */
    0x85, /* Register:&h698 Value: Binary:&b10000101 Decimal: 133 */
    0xE6, /* Register:&h699 Value: Binary:&b11100110 Decimal: 230 */
    0x19, /* Register:&h69A Value: Binary:&b00011001 Decimal: 025 */
    0x7D, /* Register:&h69B Value: Binary:&b01111101 Decimal: 125 */
    0xEB, /* Register:&h69C Value: Binary:&b11101011 Decimal: 235 */
    0x05, /* Register:&h69D Value: Binary:&b00000101 Decimal: 005 */
    0x4B, /* Register:&h69E Value: Binary:&b01001011 Decimal: 075 */
    0x04, /* Register:&h69F Value: Binary:&b00000100 Decimal: 004 */
    0x00, /* Register:&h6A0 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h6A1 Value: Binary:&b00000000 Decimal: 000 */
    0x1F, /* Register:&h6A2 Value: Binary:&b00011111 Decimal: 031 */
    0x67, /* Register:&h6A3 Value: Binary:&b01100111 Decimal: 103 */
    0x00, /* Register:&h6A4 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h6A5 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h6A6 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h6A7 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h6A8 Value: Binary:&b00000000 Decimal: 000 */
    0xE8, /* Register:&h6A9 Value: Binary:&b11101000 Decimal: 232 */
    0xFD, /* Register:&h6AA Value: Binary:&b11111101 Decimal: 253 */
    0x20, /* Register:&h6AB Value: Binary:&b00100000 Decimal: 032 */
    0x44, /* Register:&h6AC Value: Binary:&b01000100 Decimal: 068 */
    0xC4, /* Register:&h6AD Value: Binary:&b11000100 Decimal: 196 */
    0x09, /* Register:&h6AE Value: Binary:&b00001001 Decimal: 009 */
    0x00, /* Register:&h6AF Value: Binary:&b00000000 Decimal: 000 */
    0x20, /* Register:&h6B0 Value: Binary:&b00100000 Decimal: 032 */
    0x44, /* Register:&h6B1 Value: Binary:&b01000100 Decimal: 068 */
    0x00, /* Register:&h6B2 Value: Binary:&b00000000 Decimal: 000 */
    0x85, /* Register:&h6B3 Value: Binary:&b10000101 Decimal: 133 */
    0xBA, /* Register:&h6B4 Value: Binary:&b10111010 Decimal: 186 */
    0x1A, /* Register:&h6B5 Value: Binary:&b00011010 Decimal: 026 */
    0x03, /* Register:&h6B6 Value: Binary:&b00000011 Decimal: 003 */
    0x00, /* Register:&h6B7 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h6B8 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h6B9 Value: Binary:&b00000000 Decimal: 000 */
    0xE8, /* Register:&h6BA Value: Binary:&b11101000 Decimal: 232 */
    0x03, /* Register:&h6BB Value: Binary:&b00000011 Decimal: 003 */
    0x00, /* Register:&h6BC Value: Binary:&b00000000 Decimal: 000 */
    0xE8, /* Register:&h6BD Value: Binary:&b11101000 Decimal: 232 */
    0x03, /* Register:&h6BE Value: Binary:&b00000011 Decimal: 003 */
    0x00, /* Register:&h6BF Value: Binary:&b00000000 Decimal: 000 */
    0x64, /* Register:&h6C0 Value: Binary:&b01100100 Decimal: 100 */
    0x00, /* Register:&h6C1 Value: Binary:&b00000000 Decimal: 000 */
    0x64, /* Register:&h6C2 Value: Binary:&b01100100 Decimal: 100 */
    0x00, /* Register:&h6C3 Value: Binary:&b00000000 Decimal: 000 */
    0x88, /* Register:&h6C4 Value: Binary:&b10001000 Decimal: 136 */
    0xA1, /* Register:&h6C5 Value: Binary:&b10100001 Decimal: 161 */
    0x40, /* Register:&h6C6 Value: Binary:&b01000000 Decimal: 064 */
    0xFD, /* Register:&h6C7 Value: Binary:&b11111101 Decimal: 253 */
    0xB9, /* Register:&h6C8 Value: Binary:&b10111001 Decimal: 185 */
    0x33, /* Register:&h6C9 Value: Binary:&b00110011 Decimal: 051 */
    0x85, /* Register:&h6CA Value: Binary:&b10000101 Decimal: 133 */
    0xE6, /* Register:&h6CB Value: Binary:&b11100110 Decimal: 230 */
    0x19, /* Register:&h6CC Value: Binary:&b00011001 Decimal: 025 */
    0x7D, /* Register:&h6CD Value: Binary:&b01111101 Decimal: 125 */
    0xEB, /* Register:&h6CE Value: Binary:&b11101011 Decimal: 235 */
    0x05, /* Register:&h6CF Value: Binary:&b00000101 Decimal: 005 */
    0xE7, /* Register:&h6D0 Value: Binary:&b11100111 Decimal: 231 */
    0x03, /* Register:&h6D1 Value: Binary:&b00000011 Decimal: 003 */
    0x00, /* Register:&h6D2 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h6D3 Value: Binary:&b00000000 Decimal: 000 */
    0xD3, /* Register:&h6D4 Value: Binary:&b11010011 Decimal: 211 */
    0x30, /* Register:&h6D5 Value: Binary:&b00110000 Decimal: 048 */
    0x00, /* Register:&h6D6 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h6D7 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h6D8 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h6D9 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h6DA Value: Binary:&b00000000 Decimal: 000 */
    0xE8, /* Register:&h6DB Value: Binary:&b11101000 Decimal: 232 */
    0xFD, /* Register:&h6DC Value: Binary:&b11111101 Decimal: 253 */
    0x20, /* Register:&h6DD Value: Binary:&b00100000 Decimal: 032 */
    0x44, /* Register:&h6DE Value: Binary:&b01000100 Decimal: 068 */
    0xC4, /* Register:&h6DF Value: Binary:&b11000100 Decimal: 196 */
    0x09, /* Register:&h6E0 Value: Binary:&b00001001 Decimal: 009 */
    0x00, /* Register:&h6E1 Value: Binary:&b00000000 Decimal: 000 */
    0x20, /* Register:&h6E2 Value: Binary:&b00100000 Decimal: 032 */
    0x44, /* Register:&h6E3 Value: Binary:&b01000100 Decimal: 068 */
};

static const unsigned char AD9548Register700BankValues[] =
{
    0x00, /* Register:&h700 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h701 Value: Binary:&b00000000 Decimal: 000 */
    0x5A, /* Register:&h702 Value: Binary:&b01011010 Decimal: 090 */
    0x62, /* Register:&h703 Value: Binary:&b01100010 Decimal: 098 */
    0x02, /* Register:&h704 Value: Binary:&b00000010 Decimal: 002 */
    0x00, /* Register:&h705 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h706 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h707 Value: Binary:&b00000000 Decimal: 000 */
    0xE8, /* Register:&h708 Value: Binary:&b11101000 Decimal: 232 */
    0x03, /* Register:&h709 Value: Binary:&b00000011 Decimal: 003 */
    0x00, /* Register:&h70A Value: Binary:&b00000000 Decimal: 000 */
    0xE8, /* Register:&h70B Value: Binary:&b11101000 Decimal: 232 */
    0x03, /* Register:&h70C Value: Binary:&b00000011 Decimal: 003 */
    0x00, /* Register:&h70D Value: Binary:&b00000000 Decimal: 000 */
    0x64, /* Register:&h70E Value: Binary:&b01100100 Decimal: 100 */
    0x00, /* Register:&h70F Value: Binary:&b00000000 Decimal: 000 */
    0x64, /* Register:&h710 Value: Binary:&b01100100 Decimal: 100 */
    0x00, /* Register:&h711 Value: Binary:&b00000000 Decimal: 000 */
    0x12, /* Register:&h712 Value: Binary:&b00010010 Decimal: 018 */
    0x9B, /* Register:&h713 Value: Binary:&b10011011 Decimal: 155 */
    0x40, /* Register:&h714 Value: Binary:&b01000000 Decimal: 064 */
    0xFD, /* Register:&h715 Value: Binary:&b11111101 Decimal: 253 */
    0xB9, /* Register:&h716 Value: Binary:&b10111001 Decimal: 185 */
    0x33, /* Register:&h717 Value: Binary:&b00110011 Decimal: 051 */
    0x85, /* Register:&h718 Value: Binary:&b10000101 Decimal: 133 */
    0xE6, /* Register:&h719 Value: Binary:&b11100110 Decimal: 230 */
    0x19, /* Register:&h71A Value: Binary:&b00011001 Decimal: 025 */
    0x7D, /* Register:&h71B Value: Binary:&b01111101 Decimal: 125 */
    0xEB, /* Register:&h71C Value: Binary:&b11101011 Decimal: 235 */
    0x05, /* Register:&h71D Value: Binary:&b00000101 Decimal: 005 */
    0xE1, /* Register:&h71E Value: Binary:&b11100001 Decimal: 225 */
    0x04, /* Register:&h71F Value: Binary:&b00000100 Decimal: 004 */
    0x00, /* Register:&h720 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h721 Value: Binary:&b00000000 Decimal: 000 */
    0xDF, /* Register:&h722 Value: Binary:&b11011111 Decimal: 223 */
    0x2E, /* Register:&h723 Value: Binary:&b00101110 Decimal: 046 */
    0x00, /* Register:&h724 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h725 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h726 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h727 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h728 Value: Binary:&b00000000 Decimal: 000 */
    0xE8, /* Register:&h729 Value: Binary:&b11101000 Decimal: 232 */
    0xFD, /* Register:&h72A Value: Binary:&b11111101 Decimal: 253 */
    0x20, /* Register:&h72B Value: Binary:&b00100000 Decimal: 032 */
    0x44, /* Register:&h72C Value: Binary:&b01000100 Decimal: 068 */
    0xC4, /* Register:&h72D Value: Binary:&b11000100 Decimal: 196 */
    0x09, /* Register:&h72E Value: Binary:&b00001001 Decimal: 009 */
    0x00, /* Register:&h72F Value: Binary:&b00000000 Decimal: 000 */
    0x20, /* Register:&h730 Value: Binary:&b00100000 Decimal: 032 */
    0x44, /* Register:&h731 Value: Binary:&b01000100 Decimal: 068 */
    0x00, /* Register:&h732 Value: Binary:&b00000000 Decimal: 000 */
    0x62, /* Register:&h733 Value: Binary:&b01100010 Decimal: 098 */
    0xE0, /* Register:&h734 Value: Binary:&b11100000 Decimal: 224 */
    0x4A, /* Register:&h735 Value: Binary:&b01001010 Decimal: 074 */
    0x02, /* Register:&h736 Value: Binary:&b00000010 Decimal: 002 */
    0x00, /* Register:&h737 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h738 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h739 Value: Binary:&b00000000 Decimal: 000 */
    0xE8, /* Register:&h73A Value: Binary:&b11101000 Decimal: 232 */
    0x03, /* Register:&h73B Value: Binary:&b00000011 Decimal: 003 */
    0x00, /* Register:&h73C Value: Binary:&b00000000 Decimal: 000 */
    0xE8, /* Register:&h73D Value: Binary:&b11101000 Decimal: 232 */
    0x03, /* Register:&h73E Value: Binary:&b00000011 Decimal: 003 */
    0x00, /* Register:&h73F Value: Binary:&b00000000 Decimal: 000 */
    0x64, /* Register:&h740 Value: Binary:&b01100100 Decimal: 100 */
    0x00, /* Register:&h741 Value: Binary:&b00000000 Decimal: 000 */
    0x64, /* Register:&h742 Value: Binary:&b01100100 Decimal: 100 */
    0x00, /* Register:&h743 Value: Binary:&b00000000 Decimal: 000 */
    0x12, /* Register:&h744 Value: Binary:&b00010010 Decimal: 018 */
    0x9B, /* Register:&h745 Value: Binary:&b10011011 Decimal: 155 */
    0x40, /* Register:&h746 Value: Binary:&b01000000 Decimal: 064 */
    0xFD, /* Register:&h747 Value: Binary:&b11111101 Decimal: 253 */
    0xB9, /* Register:&h748 Value: Binary:&b10111001 Decimal: 185 */
    0x33, /* Register:&h749 Value: Binary:&b00110011 Decimal: 051 */
    0x85, /* Register:&h74A Value: Binary:&b10000101 Decimal: 133 */
    0xE6, /* Register:&h74B Value: Binary:&b11100110 Decimal: 230 */
    0x19, /* Register:&h74C Value: Binary:&b00011001 Decimal: 025 */
    0x7D, /* Register:&h74D Value: Binary:&b01111101 Decimal: 125 */
    0xEB, /* Register:&h74E Value: Binary:&b11101011 Decimal: 235 */
    0x05, /* Register:&h74F Value: Binary:&b00000101 Decimal: 005 */
    0x13, /* Register:&h750 Value: Binary:&b00010011 Decimal: 019 */
    0x05, /* Register:&h751 Value: Binary:&b00000101 Decimal: 005 */
    0x00, /* Register:&h752 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h753 Value: Binary:&b00000000 Decimal: 000 */
    0xDF, /* Register:&h754 Value: Binary:&b11011111 Decimal: 223 */
    0x2E, /* Register:&h755 Value: Binary:&b00101110 Decimal: 046 */
    0x00, /* Register:&h756 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h757 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h758 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h759 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h75A Value: Binary:&b00000000 Decimal: 000 */
    0xE8, /* Register:&h75B Value: Binary:&b11101000 Decimal: 232 */
    0xFD, /* Register:&h75C Value: Binary:&b11111101 Decimal: 253 */
    0x20, /* Register:&h75D Value: Binary:&b00100000 Decimal: 032 */
    0x44, /* Register:&h75E Value: Binary:&b01000100 Decimal: 068 */
    0xC4, /* Register:&h75F Value: Binary:&b11000100 Decimal: 196 */
    0x09, /* Register:&h760 Value: Binary:&b00001001 Decimal: 009 */
    0x00, /* Register:&h761 Value: Binary:&b00000000 Decimal: 000 */
    0x20, /* Register:&h762 Value: Binary:&b00100000 Decimal: 032 */
    0x44, /* Register:&h763 Value: Binary:&b01000100 Decimal: 068 */
};

static const unsigned char AD9548Register780BankValues[] =
{
    0x00, /* Register:&h780 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h781 Value: Binary:&b00000000 Decimal: 000 */
    0xA0, /* Register:&h782 Value: Binary:&b10100000 Decimal: 160 */
    0x72, /* Register:&h783 Value: Binary:&b01110010 Decimal: 114 */
    0x4E, /* Register:&h784 Value: Binary:&b01001110 Decimal: 078 */
    0x18, /* Register:&h785 Value: Binary:&b00011000 Decimal: 024 */
    0x09, /* Register:&h786 Value: Binary:&b00001001 Decimal: 009 */
    0x00, /* Register:&h787 Value: Binary:&b00000000 Decimal: 000 */
    0xE8, /* Register:&h788 Value: Binary:&b11101000 Decimal: 232 */
    0x03, /* Register:&h789 Value: Binary:&b00000011 Decimal: 003 */
    0x00, /* Register:&h78A Value: Binary:&b00000000 Decimal: 000 */
    0xE8, /* Register:&h78B Value: Binary:&b11101000 Decimal: 232 */
    0x03, /* Register:&h78C Value: Binary:&b00000011 Decimal: 003 */
    0x00, /* Register:&h78D Value: Binary:&b00000000 Decimal: 000 */
    0xB8, /* Register:&h78E Value: Binary:&b10111000 Decimal: 184 */
    0x0B, /* Register:&h78F Value: Binary:&b00001011 Decimal: 011 */
    0xB8, /* Register:&h790 Value: Binary:&b10111000 Decimal: 184 */
    0x0B, /* Register:&h791 Value: Binary:&b00001011 Decimal: 011 */
    0x69, /* Register:&h792 Value: Binary:&b01101001 Decimal: 105 */
    0x8F, /* Register:&h793 Value: Binary:&b10001111 Decimal: 143 */
    0x0E, /* Register:&h794 Value: Binary:&b00001110 Decimal: 014 */
    0x5E, /* Register:&h795 Value: Binary:&b01011110 Decimal: 094 */
    0xC9, /* Register:&h796 Value: Binary:&b11001001 Decimal: 201 */
    0x6B, /* Register:&h797 Value: Binary:&b01101011 Decimal: 107 */
    0x3D, /* Register:&h798 Value: Binary:&b00111101 Decimal: 061 */
    0x02, /* Register:&h799 Value: Binary:&b00000010 Decimal: 002 */
    0x33, /* Register:&h79A Value: Binary:&b00110011 Decimal: 051 */
    0x60, /* Register:&h79B Value: Binary:&b01100000 Decimal: 096 */
    0xEC, /* Register:&h79C Value: Binary:&b11101100 Decimal: 236 */
    0x0B, /* Register:&h79D Value: Binary:&b00001011 Decimal: 011 */
    0x00, /* Register:&h79E Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h79F Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7A0 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7A1 Value: Binary:&b00000000 Decimal: 000 */
    0xFF, /* Register:&h7A2 Value: Binary:&b11111111 Decimal: 255 */
    0x9E, /* Register:&h7A3 Value: Binary:&b10011110 Decimal: 158 */
    0x24, /* Register:&h7A4 Value: Binary:&b00100100 Decimal: 036 */
    0x00, /* Register:&h7A5 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7A6 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7A7 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7A8 Value: Binary:&b00000000 Decimal: 000 */
    0x10, /* Register:&h7A9 Value: Binary:&b00010000 Decimal: 016 */
    0x27, /* Register:&h7AA Value: Binary:&b00100111 Decimal: 039 */
    0x20, /* Register:&h7AB Value: Binary:&b00100000 Decimal: 032 */
    0x44, /* Register:&h7AC Value: Binary:&b01000100 Decimal: 068 */
    0xF4, /* Register:&h7AD Value: Binary:&b11110100 Decimal: 244 */
    0x01, /* Register:&h7AE Value: Binary:&b00000001 Decimal: 001 */
    0x00, /* Register:&h7AF Value: Binary:&b00000000 Decimal: 000 */
    0x20, /* Register:&h7B0 Value: Binary:&b00100000 Decimal: 032 */
    0x44, /* Register:&h7B1 Value: Binary:&b01000100 Decimal: 068 */
    0x00, /* Register:&h7B2 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7B3 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7B4 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7B5 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7B6 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7B7 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7B8 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7B9 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7BA Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7BB Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7BC Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7BD Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7BE Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7BF Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7C0 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7C1 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7C2 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7C3 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7C4 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7C5 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7C6 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7C7 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7C8 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7C9 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7CA Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7CB Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7CC Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7CD Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7CE Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7CF Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7D0 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7D1 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7D2 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7D3 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7D4 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7D5 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7D6 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7D7 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7D8 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7D9 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7DA Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7DB Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7DC Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7DD Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7DE Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7DF Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7E0 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7E1 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7E2 Value: Binary:&b00000000 Decimal: 000 */
    0x00, /* Register:&h7E3 Value: Binary:&b00000000 Decimal: 000 */
};
