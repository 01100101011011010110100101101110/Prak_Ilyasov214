#include <stdio.h>
#include <stdlib.h>

FILE *inp, *out;

void get_io(int argc, char **argv) {
	inp = stdin, out = stdout;
	switch (argc) {
		case 3:
			if ((out = fopen(argv[2], "r")) == NULL) {
				;	//for processing BOM	
			}
		case 2:
			if ((inp = fopen(argv[1], "r")) == NULL) {
				fprintf(stderr, "Can't open input file for reading\n");
				exit(1);
			}
			break;
	}
}

void skip_utf8_bom(FILE *inp) {
	if (inp == stdin) {
		return;
	}
	unsigned char b1, b2, b3;
	fread(&b1, 1, 1, inp);
	fread(&b2, 1, 1, inp);
	fread(&b3, 1, 1, inp);
	if(!((b1==0xEF) && (b2==0xBB) && (b3==0xBF))) {
		fseek(inp, 0, SEEK_SET);
	}
}

int check_endian(char **argv, FILE *out) {
	int le = 1;
	unsigned short utf16ch;
	if (out == NULL) {
		fprintf(stderr, "Can't open output file for reading\n");
	} else {
		if (!fread(&utf16ch, 1, 2, out)) {
			le = 1;
		}
		utf16ch = (utf16ch << 8) | ((utf16ch >> 8) & 0xFF);
		if (utf16ch == 0xFEFF) {
			le = 0;
		} else if (utf16ch == 0xFFFE) {
			le = 1;
		}
		fclose(out);
	}
	if ((out = fopen(argv[2], "w"))	== NULL) {
		fprintf(stderr, "Can't open file for writing\n");
		exit(1);		
	}
	if ((utf16ch == 0xFFFE) || (utf16ch == 0xFEFF)) {
		utf16ch = (utf16ch << 8) | ((utf16ch >> 8) & 0xFF);
		fwrite(&utf16ch, sizeof(utf16ch), 1, out);
	} 
	return le;
}

void switch_encoding(int le) {
	unsigned char inp_byte1, inp_byte2, inp_byte3, out_byte1, out_byte2;
	while (fread(&inp_byte1, 1, 1, inp)) {
		out_byte1 = 0;
		out_byte2 = 0;
		if (!(inp_byte1 & 0x80)) {			//1byte set
			out_byte1 = inp_byte1;
		} else if ((inp_byte1 & 0xE0) == 0xC0) {	//2byte set
			inp_byte1 &= 0x1F;
			fread(&inp_byte2, 1, 1, inp);
			if((inp_byte2 & 0xC0) != 0x80) {
				fprintf(stderr, "Invalid UTF-8 byte '%x' in second byte of 2-byte set. In %ld\n pos\n", inp_byte2, ftell(inp));
				continue;
			}
			inp_byte2 &= 0x3F;
			out_byte1 = ((inp_byte1 & 0x3) << 6) | inp_byte2;
			out_byte2 = inp_byte1 >> 2;
		} else if ((inp_byte1 & 0xF0) == 0xE0) {	//3byte set
			inp_byte1 &= 0xF;
			fread(&inp_byte2, 1, 1, inp);
			if ((inp_byte2 & 0xC0) != 0x80) {
				fprintf(stderr, "Invalid UTF-8 byte '%x' in second byte of 3-byte set. In %ld\n pos\n", inp_byte2, ftell(inp));
				continue;
			}
			inp_byte2 &= 0x3F;
			fread(&inp_byte3, 1, 1, inp);
			if ((inp_byte3 & 0xC0) != 0x80) {
				fprintf(stderr, "Invalid UTF-8 byte '%x' in third byte of 3-byte set. In %ld\n pos\n", inp_byte3, ftell(inp));
				continue;
			}
			inp_byte3 &= 0x3F;
			out_byte1 = ((inp_byte2 & 0x3) << 6) | inp_byte3;
			out_byte2 = (inp_byte2 >> 2) | (inp_byte1 << 4);
		} else {
			fprintf(stderr, "Invalid byte.\n");
			continue;
		}
		if (le) {
			fwrite(&out_byte1, 1, 1, out);
			fwrite(&out_byte2, 1, 1, out);
		} else {
			fwrite(&out_byte2, 1, 1, out);
			fwrite(&out_byte1, 1, 1, out);
		}
	}
	
}

int main(int argc, char **argv) {
	int le;
	get_io(argc, argv);
	skip_utf8_bom(inp);
	le = check_endian(argv, out);
	switch_encoding(le);
	fclose(inp);
	fclose(out);
	
	return 0;
}

		
