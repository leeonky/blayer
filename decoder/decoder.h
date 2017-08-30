#ifndef VDECODE_H
#define VDECODE_H

extern int decoder_main(int, char **, FILE *, FILE *, FILE *);

typedef struct decoder_args {
	int track_index;
	int track_type;
	const char *file_name;
	int buffer_bits;
} decoder_args;

extern int process_args(decoder_args *, int, char **, FILE *);

#endif
