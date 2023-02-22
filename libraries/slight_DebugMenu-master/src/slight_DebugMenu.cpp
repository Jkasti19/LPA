// NOLINT(legal/copyright)
/******************************************************************************

    slight_DebugMenu Library
        helper library to build a simple debug menu for the stream_out terminal.

    written by stefan krueger (s-light),
        git@s-light.eu, http://s-light.eu, https://github.com/s-light/

******************************************************************************/
/******************************************************************************
    The MIT License (MIT)

    Copyright (c) 2015 Stefan Krüger

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    stream_out OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
******************************************************************************/


// Include yourselfs header file
// NOLINTNEXTLINE(build/include)
#include "./slight_DebugMenu.h"

// Includes Core Arduino functionality
#if ARDUINO < 100
    #include <WProgram.h>
#else
    #include <Arduino.h>
#endif


// slight_DebugMenu :: slight_DebugMenu(
//     Print &out_ref
// ) {
//     ready = false;
//     stream_out = out_ref;
// }

// slight_DebugMenu :: slight_DebugMenu(
//     Stream &inout_ref,
//     uint8_t input_length_new
// ) {
//     slight_DebugMenu(inout_ref, inout_ref, input_length_new);
// }

slight_DebugMenu :: slight_DebugMenu(
    // NOLINTNEXTLINE(runtime/references)
    Stream &stream_input_ref,
    // NOLINTNEXTLINE(runtime/references)
    Print &stream_out_ref,
    uint8_t input_length_new
) :
    stream_input(stream_input_ref),
    stream_out(stream_out_ref)
// NOLINTNEXTLINE(whitespace/braces)
{
    ready = false;
    // in = in_ref;
    // stream_out = out_ref;
    input_length = input_length_new;
    user_EOC_char_active = false;
    command_input = reinterpret_cast<char*>(malloc(input_length));
    command_current = reinterpret_cast<char*>(malloc(input_length));
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// functions
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// public

void slight_DebugMenu::begin() {
    if (ready == false) {
        memset(command_input, '\0', input_length);
        memset(command_current, '\0', input_length);

        flag_BF = false;  // BufferFull
        flag_EOC = false;
        flag_CR = false;
        flag_LF = false;
        // flag_LongLine = false;
        // flag_SkipRest = false;

        ready = true;
    }
}

void slight_DebugMenu::begin(bool print_on_start) {
    if (print_on_start) {
        begin();
        command_input[0] = '?';
        flag_EOC = true;
        update();
    }
}

void slight_DebugMenu::update() {
    if (ready) {
        handle_input_available();
        check_EOC();
    }
}


void slight_DebugMenu::set_callback(tCallbackFunction callback_on_EOC_new) {
    callback_on_EOC = callback_on_EOC_new;
}


void slight_DebugMenu::set_user_EOC_char(char user_EOC_char_new) {
    user_EOC_char = user_EOC_char_new;
    user_EOC_char_active = true;
}

char slight_DebugMenu::get_user_EOC_char() {
    return user_EOC_char;
}

void slight_DebugMenu::clear_user_EOC_char() {
    user_EOC_char_active = false;
}

bool slight_DebugMenu::get_flag_BF() {
    return flag_BF;
}

void slight_DebugMenu::set_flag_EOC(bool flag = true) {
    flag_EOC = flag;
}

Print& slight_DebugMenu::get_stream_out_ref() {
    return stream_out;
}


char* slight_DebugMenu::get_command_input_pointer() {
    return command_input;
}

char* slight_DebugMenu::get_command_current_pointer() {
    return command_current;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// public static

void slight_DebugMenu::print_Binary_8(Print &stream_out, uint8_t value)  {
    // for (size_t mask = 0b10000000; mask; mask >>= 1) {
    for (uint8_t mask = ((uint8_t)1 << 7); mask; mask >>= 1) {
        // check if this bit is set
        if (mask & value) {
            stream_out.print('1');
        } else {
            stream_out.print('0');
        }
    }
}

void slight_DebugMenu::print_Binary_12(Print &stream_out, uint16_t value)  {
    //                       B12345678   B12345678
    // for (unsigned int mask = 0x8000; mask; mask >>= 1) {
    for (uint16_t mask = ((uint16_t)1 << 11); mask; mask >>= 1) {
        // check if this bit is set
        if (mask & value) {
            stream_out.print('1');
        } else {
            stream_out.print('0');
        }
    }
}

void slight_DebugMenu::print_Binary_16(Print &stream_out, uint16_t value)  {
    for (uint16_t mask = ((uint16_t)1 << 15); mask; mask >>= 1) {
        // check if this bit is set
        if (mask & value) {
            stream_out.print('1');
        } else {
            stream_out.print('0');
        }
    }
}

void slight_DebugMenu::print_Binary_32(Print &stream_out, uint32_t value)  {
    for (uint32_t mask = ((uint32_t)1 << 31); mask; mask >>= 1) {
        // check if this bit is set
        if (mask & value) {
            stream_out.print('1');
        } else {
            stream_out.print('0');
        }
    }
}

void slight_DebugMenu::print_HEX(Print &stream_out, uint8_t value) {
    if (value < 0x10) {
        stream_out.print("0");
    }
    stream_out.print(value, HEX);
}



// uint8_t slight_DebugMenu::parse_HEX(char *input, uint8_t count){
uint8_t slight_DebugMenu::parse_HEX(char *input) {
    uint8_t count = 2;
    uint8_t value = 0;
    uint8_t temp = 0;
    uint8_t index = 0;

    for (index = 0; index < count; index++)  {
    if        ( (input[index] >= '0') && (input[index] <= '9') ) {
    temp = input[index] - '0';
    } else if ( (input[index] >= 'a') && (input[index] <= 'f') ) {
    temp = (input[index] - 'a' + 10);
    } else if ( (input[index] >= 'A') && (input[index] <= 'F') ) {
    temp = (input[index] - 'A' + 10);
    } else {
    // no HEX value..
    temp = 0;
    }
    value = value + ((16 - ((index%2) * 15)) * temp);
    }

    // // debug variant
    // index = 0;
    // Serial.print(F("input["));
    // Serial.print(index);
    // Serial.print(F("]: '"));
    // Serial.print(input[index]);
    // Serial.println(F("'"));
    // if ((input[index] >= '0') && (input[index] <= '9')) {
    //     temp = input[index] - '0';
    // } else if ( (input[index] >= 'a') && (input[index] <= 'f') ) {
    //     temp = (input[index] - 'a' + 10);
    // } else if ( (input[index] >= 'A') && (input[index] <= 'F') ) {
    //     temp = (input[index] - 'A' + 10);
    // } else {
    //     // no HEX value..
    //     temp = 0;
    // }
    // Serial.print(F("temp: "));
    // Serial.println(temp);
    //
    // value = value + ((16 - ((index%2) * 15)) * temp);
    // //value = value + (16 * temp);
    // Serial.print(F("16 - ((index%2) * 15): "));
    // Serial.println(16 - ((index%2) * 15));
    //
    // Serial.print(F("value: "));
    // Serial.println(value);
    // index = 1;
    // Serial.print(F("input["));
    // Serial.print(index);
    // Serial.print(F("]: '"));
    // Serial.print(input[index]);
    // Serial.println(F("'"));
    // if        ( (input[index] >= '0') && (input[index] <= '9') ) {
    //        temp = input[index] - '0';
    // } else if ( (input[index] >= 'a') && (input[index] <= 'f') ) {
    //        temp = (input[index] - 'a' + 10);
    // } else if ( (input[index] >= 'A') && (input[index] <= 'F') ) {
    //        temp = (input[index] - 'A' + 10) ;
    // } else {
    //        // no HEX value..
    //        temp = 0;
    // }
    // Serial.print(F("temp: "));
    // Serial.println(temp);
    //
    // value = value + ((16 - ((index%2) * 15)) * temp);
    //
    // Serial.print(F("16 - ((index%2) * 15): "));
    // Serial.println(16 - ((index%2) * 15));
    //
    // Serial.print(F("value: "));
    // Serial.println(value);

    return value;
}



uint8_t slight_DebugMenu::count_digits(
    uint32_t value
) {
    // based on https://stackoverflow.com/a/554603/574981
    // this covers the whole range of 32-bit unsigned values
    const uint32_t test[] = {
        1, 10, 100, 1000, 10000,
        100000, 1000000, 10000000, 100000000, 1000000000 };
    uint8_t digits = 10;
    while (value < test[digits]) {
        --digits;
    }
    return digits;
}

void slight_DebugMenu::print_uint8_align_right(
    Print &stream_out,
    uint8_t value
) {
    // uint8_t bOffset = 0;
    if (value < 100) {
        if (value < 10) {
            // bOffset = 2;
            stream_out.print(F("  "));
        } else {
            // bOffset = 1;
            stream_out.print(F(" "));
        }
    }
    stream_out.print(value);
}


void slight_DebugMenu::print_uint16_align_right(
    Print &stream_out,
    uint16_t value
) {
    // 0 .. 65,535

    // tests
    // out.println(F("test print_uint16_align_right: "));
    // uint16_t ui16[] = {
    //     0,
    //     100,
    //     999,
    //     1000,
    //     1001,
    //     10321,
    //     65535
    // };
    //
    // for (size_t i = 0; i < (sizeof(ui16)/sizeof(uint16_t)); i++) {
    //     slight_DebugMenu::print_uint32_align_right(
    //         out,
    //         ui16[i]
    //     );
    //     out.print(F(" ("));
    //     out.print(ui16[i]);
    //     out.print(F(")"));
    //     out.println();
    // }

    // uint8_t offset = 0;
    // if (value < 10000) {
    //     offset += 1;
    //     if (value < 1000) {
    //         offset += 1;
    //         if (value < 100) {
    //             offset += 1;
    //             if (value < 10) {
    //                 offset += 1;
    //             }
    //         }
    //     }
    // }

    uint8_t padding_width = 5;
    uint8_t offset = padding_width-1;
    uint16_t compare = 10;
    while (
        (value >= compare) &&
        (offset > 0)
    ) {
        offset -= 1;
        compare *= 10;
    }



    for (size_t i = 0; i < offset; i++) {
        stream_out.print(F(" "));
    }
    stream_out.print(value);
}


void slight_DebugMenu::print_uint32_align_right(
    Print &stream_out,
    uint32_t value
) {
    // 0 .. 4,294,967,295

    // tests:
    // out.println(F("test print_uint32_align_right: "));
    // uint32_t ui32[] = {
    //     0,
    //     100,
    //     999,
    //     10321,
    //     90321,
    //     1000321,
    //     9000321,
    //     100300321,
    //     900300321,
    //     1300300321,
    //     4294967295,
    // };
    //
    // for (size_t i = 0; i < (sizeof(ui32)/sizeof(uint32_t)); i++) {
    //     slight_DebugMenu::print_uint32_align_right(
    //         out,
    //         ui32[i]
    //     );
    //     out.print(F(" ("));
    //     out.print(ui32[i]);
    //     out.print(F(")"));
    //     out.println();
    // }

    uint8_t padding_width = 10;
    uint8_t offset = padding_width-1;
    uint32_t compare = 10;
    while (
        (value >= compare) &&
        (offset > 0)
    ) {
        offset -= 1;
        compare *= 10;
    }

    for (size_t i = 0; i < offset; i++) {
        stream_out.print(F(" "));
    }
    stream_out.print(value);
}


void slight_DebugMenu::print_int32_align_right(
    Print &stream_out,
    int32_t value
) {
    // -2,147,483,648 .. 2,147,483,647

    // tests:
    // out.println(F("test print_int32_align_right: "));
    // int32_t i32[] = {
    //     0,
    //     1,
    //     9,
    //     10,
    //     99,
    //     100,
    //     999,
    //     1000,
    //     9999,
    //     10000,
    //     99999,
    //     100000,
    //     999999,
    //     1000000,
    //     9999999,
    //     10000000,
    //     19999999,
    //     100000000,
    //     999999999,
    //     1000000000,
    //     2147483646,
    //     2147483647,
    //     1,
    //     0,
    //     -1,
    //     -9,
    //     -10,
    //     -99,
    //     -100,
    //     -999,
    //     -1000,
    //     -9999,
    //     -10000,
    //     -99999,
    //     -100000,
    //     -999999,
    //     -1000000,
    //     -9999999,
    //     -10000000,
    //     -19999999,
    //     -100000000,
    //     -999999999,
    //     -1000000000,
    //     -2147483646,
    //     -2147483647,
    //     -2147483648,
    // };
    //
    // for (size_t i = 0; i < (sizeof(i32)/sizeof(uint32_t)); i++) {
    //     slight_DebugMenu::print_int32_align_right(
    //         out,
    //         i32[i]
    //     );
    //     out.print(F(" ("));
    //     out.print(i32[i]);
    //     out.print(F(")"));
    //     out.println();
    // }

    // one more for sign.
    uint8_t padding_width = 10;
    uint8_t offset = padding_width;
    uint32_t compare = 1;

    uint32_t value_abs = abs(value);

    // we have a minimum of one digit:
    offset -= 1;
    compare *= 10;

    while (
        (compare <= value_abs) &&
        (offset > 0)
    ) {
        offset -= 1;
        if (offset > 0) {
            compare *= 10;
        }
    }

    // handle '-' sign
    if (value >= 0) {
        offset += 1;
    }

    for (size_t i = 0; i < offset; i++) {
        stream_out.print(F(" "));
    }
    stream_out.print(value);

    // stream_out.print("    (compare:");
    // stream_out.print(compare);
    // stream_out.print(" offset:");
    // stream_out.print(offset);
    // stream_out.print(") ");
}

void slight_DebugMenu::print_int16_align_right(
    Print &stream_out,
    int16_t value
) {
    // -32,768 .. 32,767

    uint8_t padding_width = 5;
    uint8_t offset = padding_width;
    uint16_t compare = 1;

    uint16_t value_abs = abs(value);

    // we have a minimum of one digit:
    offset -= 1;
    compare *= 10;

    while (
        (compare <= value_abs) &&
        (offset > 0)
    ) {
        offset -= 1;
        if (offset > 0) {
            compare *= 10;
        }
    }

    // handle '-' sign
    if (value >= 0) {
        offset += 1;
    }

    for (size_t i = 0; i < offset; i++) {
        stream_out.print(F(" "));
    }
    stream_out.print(value);
}

void slight_DebugMenu::print_int8_align_right(
    Print &stream_out,
    int16_t value
) {
    // -128 .. 127

    uint8_t padding_width = 3;
    uint8_t offset = padding_width;
    uint8_t compare = 1;

    uint8_t value_abs = abs(value);

    // we have a minimum of one digit:
    offset -= 1;
    compare *= 10;

    while (
        (compare <= value_abs) &&
        (offset > 0)
    ) {
        offset -= 1;
        if (offset > 0) {
            compare *= 10;
        }
    }

    // handle '-' sign
    if (value >= 0) {
        offset += 1;
    }

    for (size_t i = 0; i < offset; i++) {
        stream_out.print(F(" "));
    }
    stream_out.print(value);
}


#if defined(ARDUINO_ARCH_AVR)
    // AVR-specific code
    int8_t slight_DebugMenu::print_float_align_right(
        Print &stream_out,
        double value,
        int8_t total_width,
        uint8_t precision,
        int8_t clip
    ) {
        // reference / ideas:
        // dtostrf
        // https://www.microchip.com/webdoc/AVRLibcReferenceManual/group__avr__stdlib_1ga060c998e77fb5fc0d3168b3ce8771d42.html
        // printf
        // https://www.microchip.com/webdoc/AVRLibcReferenceManual/group__avr__stdio_1gaa3b98c0d17b35642c0f3e4649092b9f1.html
        // http://www.cplusplus.com/reference/cstdio/printf/
        // http://www.cplusplus.com/reference/cstdio/snprintf/
        // http://www.cplusplus.com/reference/cstring/memset/
        // http://www.cplusplus.com/reference/cstring/strlen/
        // https://stackoverflow.com/a/27652012/574981

        // sprintf(buffer1, "%*.*f", total_width, precision, value);
        // ^-- this does not work -
        //     avr lib has skipped the floating point formating..

        // i'm not sure that this is fine.. but i think for now it works
        const uint8_t int_width = count_digits((int32_t)value);

        const uint8_t float_widht = (int_width + 1 + precision);
        const uint8_t buffer_size = float_widht + 1;

        char buffer1[buffer_size];
        memset(buffer1, '\0', buffer_size);

        // char *dtostrf(
        //     double val, signed char width, unsigned char prec, char *s)
        dtostrf(value, total_width, precision, buffer1);
        uint8_t value_length = strlen(buffer1);
        uint8_t buffer_offset = 0;
        if ((value_length > abs(total_width)) && (clip != 0)) {
            if (clip > 0) {
                // clip on start
                buffer_offset = value_length - abs(total_width);
            }
            if (clip < 0) {
                // clip on end
                buffer1[total_width
                ] = '\0';
            }
        }

        uint8_t chars_written = stream_out.print(buffer1 + buffer_offset);

        return chars_written;
    }
#elif defined(ARDUINO_ARCH_SAM)
    // SAM-specific code
#else
    // generic, non-platform specific code
#endif



void slight_DebugMenu::print_int8_array(
    Print &stream_out,
    int8_t *array,
    size_t count
) {
    stream_out.print(F(" "));
    size_t index = 0;
    print_int8_align_right(stream_out, array[index]);
    for (index = 1; index < count; index++) {
        stream_out.print(F(", "));
        print_int8_align_right(stream_out, array[index]);
    }
}

void slight_DebugMenu::print_uint8_array(
    Print &stream_out,
    uint8_t *array,
    size_t count
) {
    stream_out.print(F(" "));
    uint8_t index = 0;
    print_uint8_align_right(stream_out, array[index]);
    for (index = 1; index < count; index++) {
        stream_out.print(F(", "));
        print_uint8_align_right(stream_out, array[index]);
    }
}

void slight_DebugMenu::print_uint8_array(
    Print &stream_out,
    char *array,
    size_t count
) {
    stream_out.print(F(" "));
    uint8_t index = 0;
    print_uint8_align_right(stream_out, array[index]);
    for (index = 1; index < count; index++) {
        stream_out.print(F(", "));
        print_uint8_align_right(stream_out, array[index]);
    }
}

void slight_DebugMenu::print_uint8_array_HEX(
    Print &stream_out,
    uint8_t *array,
    size_t count
) {
    stream_out.print(F(" "));
    uint8_t index = 0;
    print_HEX(stream_out, array[index]);
    for (index = 1; index < count; index++) {
        stream_out.print(F(", "));
        print_HEX(stream_out, array[index]);
    }
}

void slight_DebugMenu::print_uint16_array(
    Print &stream_out,
    uint16_t *array,
    size_t count
) {
    stream_out.print(F(" "));
    uint8_t index = 0;
    print_uint16_align_right(stream_out, array[index]);
    for (index = 1; index < count; index++) {
        stream_out.print(F(", "));
        print_uint16_align_right(stream_out, array[index]);
    }
}

void slight_DebugMenu::print_int16_array(
    Print &stream_out,
    int16_t *array,
    size_t count
) {
    stream_out.print(F(" "));
    uint8_t index = 0;
    print_int16_align_right(stream_out, array[index]);
    for (index = 1; index < count; index++) {
        stream_out.print(F(", "));
        print_int16_align_right(stream_out, array[index]);
    }
}


void slight_DebugMenu::print_MAC(Print &stream_out, uint8_t *array) {
    size_t index = 0;
    print_HEX(stream_out, array[index]);
    for (index = 1; index < 6; index++) {
        stream_out.print(F(":"));
        print_HEX(stream_out, array[index]);
    }
}

void slight_DebugMenu::print_IP(Print &stream_out, uint8_t *array) {
    size_t index = 0;
    print_uint8_align_right(stream_out, array[index]);
    for (index = 1; index < 4; index++) {
        stream_out.print(F("."));
        print_uint8_align_right(stream_out, array[index]);
    }
}




//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// private


void slight_DebugMenu::handle_input_available() {
    // collect next input text
    while ((!flag_EOC) && (stream_input.available())) {
        // get the new byte:
        char charNew = static_cast<char>(stream_input.read());
        // stream_out.print(F("charNew '"));
        // stream_out.print(charNew);
        // stream_out.print(F("' : "));
        // stream_out.println(charNew, DEC);

        // collect next full line
        // http://forums.codeguru.com/showthread.php?253826-C-String-What-is-the-difference-between-n-and-r-n
        //     '\n' == 10 == LineFeed == LF
        //     '\r' == 13 == Carriage Return == CR
        //     Windows: '\r\n'
        //     Linux: '\n'
        //     Apple: '\r'
        //
        // check for line end
        switch (charNew) {
            // case '\0': {
            //     stream_out.println(F("incoming char is \\0: set EOL"));
            //     flag_EOC = true;
            //     // flag_CR = false;
            //     // flag_LF = false;
            // } break;
            case '\r': {
                // stream_out.println(F("incoming char is \\r (CR): set EOL"));
                flag_EOC = true;
                flag_CR = true;
                // flag_LF = false;
            } break;
            case '\n': {
                // stream_out.println(F("incoming char is \\n (LF): set EOL"));
                // stream_out.println(F("  Flags:"));
                //     stream_out.print  (F("    flag_BF: '"));
                //     stream_out.println(flag_BF);
                //     stream_out.print  (F("    flag_CR: '"));
                //     stream_out.println(flag_CR);
                //     stream_out.print  (F("    flag_LF: '"));
                //     stream_out.println(flag_LF);
                //     stream_out.print  (F("    flag_EOC: '"));
                //     stream_out.println(flag_EOC);

                flag_LF = true;
                flag_EOC = true;

                // stream_out.println(F("  check for CR"));
                // check if last char was not CR -
                // if this is true set EOL - else ignore.
                // if(!flag_CR) {
                //     flag_EOC = true;
                // } else {
                //     flag_CR = false;
                // }
                //
                // stream_out.println(F("  Flags:"));
                //     stream_out.print  (F("    flag_BF: '"));
                //     stream_out.println(flag_BF);
                //     stream_out.print  (F("    flag_CR: '"));
                //     stream_out.println(flag_CR);
                //     stream_out.print  (F("    flag_LF: '"));
                //     stream_out.println(flag_LF);
                //     stream_out.print  (F("    flag_EOC: '"));
                //     stream_out.println(flag_EOC);

                // this check also works for windows double line ending
                // if (strlen(command_input) > 0) {
                //     flag_EOC = true;
                // }
            } break;
            default: {
                // new! check for user EOC charNew
                if (user_EOC_char_active) {
                    if (charNew == user_EOC_char) {
                        flag_EOC = true;
                        flag_USER = true;
                    }
                }

                if (!flag_EOC) {
                    // normal char -
                    // add it to the input:
                    // stream_out.print(F("input char '"));
                    // stream_out.print(charNew);
                    // stream_out.println(F("'"));

                    // stream_out.print(F("sizeof(command_input): "));
                    // stream_out.print(sizeof(command_input));
                    // stream_out.println();

                    // check for length
                    if ((uint8_t)strlen(command_input) < (input_length-1)) {
                        command_input[strlen(command_input)] = charNew;
                    } else {
                        // stream_out.println(F(
                        //     " line to long! ignoring rest of line"
                        // ));
                        // set complete flag so line will be parsed
                        // stream_out.println(F(
                        //     "Buffer is full: set EOL; set LongLine"
                        // ));
                        // flag_EOC = true;
                        flag_BF = true;
                        // skip rest of line
                        // flag_LongLine = true;
                    }
                }
            }  // case default
        }  // switch charNew
    }  // while input.available()
}

void slight_DebugMenu::check_EOC() {
    // complete line found:
    if (flag_EOC) {
        // Serial.println(F("check_EOC:"));
        // Serial.print(F("    command_input: '"));
        // Serial.print(command_input);
        // Serial.println(F("'"));

        // Serial.println(F("  Flags:"));
        // Serial.print  (F("    flag_BF: '"));
        // Serial.println(flag_BF);
        // Serial.print  (F("    flag_CR: '"));
        // Serial.println(flag_CR);
        // Serial.print  (F("    flag_LF: '"));
        // Serial.println(flag_LF);
        // Serial.print  (F("    flag_EOC: '"));
        // Serial.println(flag_EOC);


        // Serial.println(F("  copy command_input to command_current."));
        // copy to current buffer
        strcpy(command_current, command_input);

        // Serial.println(F("  clear command_input"));
        // reset memory
        memset(command_input, '\0', input_length-1);

        // Serial.println(F("  clear flag_EOC"));
        // reset flag
        flag_EOC = false;
        flag_LF = false;
        flag_CR = false;

        // parse line / run command
        if (callback_on_EOC) {
            callback_on_EOC(this);
        }

        // reset BufferFull
        flag_BF = false;

        // print info if things were skipped.
        // if (flag_BF) {
        //     stream_out.println(F(
        //         "input was to long. used first part - skipped rest."
        //     ));
        //     flag_BF = false;
        // }

        // Serial.print  (F("    command_input: '"));
        // Serial.print(command_input);
        // Serial.println(F("'"));
        // Serial.print  (F("    command_current: '"));
        // Serial.print(command_current);
        // Serial.println(F("'"));

        // // Serial.println(F("  check flag_SkipRest"));
        // if ( !flag_SkipRest) {
        //     // Serial.println(F("   parse Line:"));
        //
        //     if (flag_BF) {
        //         Serial.println(F("input was to long. used first part - skipped rest."));
        //         flag_BF = false;
        //     }
        //
        //     // parse line / run command
        //     menuSwitcher(Serial, command_current);
        //
        //     if(flag_LongLine) {
        //         flag_SkipRest = true;
        //         flag_LongLine = false;
        //     }
        // } else {
        //     // Serial.println(F("   skip rest of Line"));
        //     flag_SkipRest = false;
        // }
    }  // if Flag complete
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// DummyWriter
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

DummyWriter::DummyWriter() {
    // nothing to do.
}

size_t DummyWriter::write(uint8_t data) {
    // data is send to nirvana :-)
    return 1;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// THE END
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
