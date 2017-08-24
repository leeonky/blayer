#ifndef VDECODE_H
#define VDECODE_H

extern int vdecode_main(int, char **, FILE *, FILE *, FILE *);

typedef struct vdecode_args {
	int track_index;
	const char *file_name;
	int buffer_bits;
} vdecode_args;

extern int process_args(vdecode_args *, int, char **, FILE *);

#endif
