#include <iostream>
#include <fstream>
#include <string.h>
#include <stdio.h>



using namespace std;

// this is a stand in look up table.
// hint: we only need the lower half since the upper half characters are valid utf-8 characters, too.
// source: https://en.wikipedia.org/wiki/Windows-1256
// ps: to make life easier, I wrote a javascript function to extract the unicodes and format them as hex numbers
// so that I can just copy from Chrome console to this code.
// $('table.chset.nounderlines.nowrap tr').each((i,d) => { let str = ""; $(d).find("small").each((ii,dd) => { str += "0x" + $(dd).text() + ","; }); console.log(str); });
// I ignored the first half of the table (range from 0x00 to 0x7F)

static const uint16_t lookup[128] = {
    
    0x20AC,0x067E,0x201A,0x0192,0x201E,0x2026,0x2020,0x2021,0x02C6,0x2030,0x0679,0x2039,0x0152,0x0686,0x0698,0x0688,
    0x06AF,0x2018,0x2019,0x201C,0x201D,0x2022,0x2013,0x2014,0x06A9,0x2122,0x0691,0x203A,0x0153,0x200C,0x200D,0x06BA,
    0x00A0,0x060C,0x00A2,0x00A3,0x00A4,0x00A5,0x00A6,0x00A7,0x00A8,0x00A9,0x06BE,0x00AB,0x00AC,0x00AD,0x00AE,0x00AF,
    0x00B0,0x00B1,0x00B2,0x00B3,0x00B4,0x00B5,0x00B6,0x00B7,0x00B8,0x00B9,0x061B,0x00BB,0x00BC,0x00BD,0x00BE,0x061F,
    0x06C1,0x0621,0x0622,0x0623,0x0624,0x0625,0x0626,0x0627,0x0628,0x0629,0x062A,0x062B,0x062C,0x062D,0x062E,0x062F,
    0x0630,0x0631,0x0632,0x0633,0x0634,0x0635,0x0636,0x00D7,0x0637,0x0638,0x0639,0x063A,0x0640,0x0641,0x0642,0x0643,
    0x00E0,0x0644,0x00E2,0x0645,0x0646,0x0647,0x0648,0x00E7,0x00E8,0x00E9,0x00EA,0x00EB,0x0649,0x064A,0x00EE,0x00EF,
    0x064B,0x064C,0x064D,0x064E,0x00F4,0x064F,0x0650,0x00F7,0x0651,0x00F9,0x0652,0x00FB,0x00FC,0x200E,0x200F,0x06D2

    };


char* set_output_file_name(char* in);
    
int main(int argc, char **argv) {
    

    // check if the file exits
    // and that the file can be opened
    // display an error message if not
    FILE* input_file = fopen(argv[1], "rb");
    if(input_file == NULL) {
        printf("Could not open %s\n", argv[1]);
        return 1;
    }

    // split file name to (name + extension)
    char* input_file_name = strtok(argv[1], ".");
    const char* input_file_ext = argv[1];
    //const char* test = get_filename_ext(argv[1]);

    // new file name will be (input_file_name)_arb.(input_file_extension)
    // for example: if input file was (the_village.srt)
    // then output file name will be (the_village_arb.srt)

    char* output_filename = 0;
    output_filename = set_output_file_name(argv[1]);

    // prepare the output file handle
    FILE* output_file = fopen(output_filename, "wb");

    // check if the file was opened for writing
    if(output_file==NULL) {
        fclose(input_file);
        printf("Could not write to [%s]", output_filename);
        return 2;
    }

    // will hold the byte read from the input file
    unsigned char win1256_byte = 0;

    // will hold the conversion from lookup table
    uint16_t      utf8_byte = 0;
    
    // to track number of bytes written
    size_t written_elements = 0;

    // loop through all of the bytes in the input file, one byte at a time
    while(fread(&win1256_byte, sizeof(unsigned char), 1, input_file)) {
        // print out the byte for debuggin...
        // printf("win-1256:  %2x  ", win1256_byte);
        
        // if the upper nibble is 7 or lower, then there is no need
        // to do anything with the byte.. just write it to the output file
        // as is.
        if(win1256_byte<0x80) {
            // ascii bytes. do nothing with it.
            // just write it to the output file as is 
             written_elements = fwrite(&win1256_byte, sizeof(unsigned char), 1, output_file);
            
        } else {
            // conversion is needed.

            // first, we will need 2 bytes to hold the utf-8 conversion 
            //  of a single windows-1256 byte

            // get the equivilent 2 bytes from lookup
            utf8_byte = lookup[win1256_byte & 0x7f];

            // upper half is obtained by shifting the conversion by 8 bits
            unsigned char upper = utf8_byte >> 8;
            // lower half is masked with 0b0000000011111111 to get rid of the upper bytes
            unsigned char lower = utf8_byte & 0x00FF;
            
            // preparing the conversion place holders
            unsigned char byte1 = 0;
            unsigned char byte2 = 0;
            
            // take the first 6 bits of the lower byte and add them to 10_ _ _ _ _ _
            byte1 = 0b10000000 + ( lower & 0b00111111 );
            
            // shift-left lower 6 bits so we are left with the remaining 2 bits that we still did not use
            // and shift-right the upper bytes 2 bits to make room for the unused 2 bits from lower byte
            // the result is added to 110 _ _ _ _ _

            byte2 = 0b11000000 + (lower >> 6) + (upper << 2);

            // now we have a valid 2-byte utf-8 representation. We can write them to the output file

            written_elements = fwrite(&byte2, sizeof(unsigned char), 1, output_file);
            written_elements += fwrite(&byte1, sizeof(unsigned char), 1, output_file);
        
            // print out the converted byte(s) for debugging.
            // printf("  utf-8   :  %2x\n",utf8_byte);
        }
        
    }
    
    // close the file before exiting
    fclose(input_file);
    fclose(output_file);
    
    // print a confirmation message:
    cout << "Written: " << output_filename << "\n";
    
    return 0;
}

char* set_output_file_name(char* in) {
    char* out = 0;

    // fine last dot in the filename 
    int last_dot = -1;

    for(int i=0; i<strlen(in); i++) {
        if(in[i]=='.') {
            last_dot = i;
        }
    }

    // terminate the filename at its last dot location
    in[last_dot] = '\0';
    
    // append _arb to the filename and assign extension .srt
    asprintf(&out,"%s_arb.srt",in);

    return out;
}