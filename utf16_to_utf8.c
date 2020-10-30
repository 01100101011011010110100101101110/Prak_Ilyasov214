//ИЗ ЮТФ-16 В ЮТФ-8
#include <stdio.h>
#include <stdlib.h>

FILE *inp, *out;

void get_io(int argc, char **argv) {
	inp = stdin, out = stdout;
	switch (argc) {
		case 3:
			if ((out = fopen(argv[2], "w")) == NULL) {
				fprintf(stderr, "Can't open file for writing\n");
				exit(1);
			}
		case 2:
			if ((inp = fopen(argv[1], "r")) == NULL) {
				fprintf(stderr, "Can't open file for reading\n");
				exit(1);
			}
			break;
	}
				
				
}

int check_endian(FILE *inp) {
	unsigned short utf16ch;
	if (!fread(&utf16ch, 1, 2, inp)) {
		fprintf(stderr, "Empty file or only 1 readable byte\n");
		return 1;
	}
	utf16ch = (utf16ch << 8) | ((utf16ch >> 8) & 0xFF);
	if (utf16ch == 0xFEFF) {
		return 0;
	} else if (utf16ch == 0xFFFE) {
		return 1;	
	} else {				//BOM doesn't provided
		fseek(inp, 0, SEEK_SET);
		return 1;
	}
}

void symbol_processing(int le) {
	unsigned short utf16ch, symbol;
	char byte[3];
	int i, j, info;
	if ((info = fread(&utf16ch, 1, 2, inp)) != 2) {
		if (info == 1) {
			fprintf(stderr, "Odd number of bytes\n");
		} 
		return;
	}
	if (!le) {
		symbol = (utf16ch << 8) | ((utf16ch >> 8) & 0xFF);
	} else {
		symbol = utf16ch;
	}
	if (symbol < 128) {
		i = 1;
		byte[0] = symbol;
	} else if (symbol < 2048) {
		i = 2;
		byte[0] = (symbol >> 6) | 0xC0;
		byte[1] = (symbol & 0x3F) | 0x80;
	} else if (symbol < 65536) {
		i = 3;
		byte[0] = (symbol >> 12) | 0xE0;
		byte[1] = ((symbol & 0xFC0) >> 6) | 0x80;
		byte[2] = (symbol & 0x3F) | 0x80;
	}
	for (j = 0; j < i; j++) {
		fwrite(&byte[j], sizeof(char), 1, out);	
	}
}
int main(int argc, char **argv) {
	int le;	
	get_io(argc, argv);
	le = check_endian(inp);
	//skip_utf8_bom(out);
	while (!feof(inp)) {
		symbol_processing(le);
	}
	fclose(inp);
	fclose(out);
}	
