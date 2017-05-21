#ifndef VDECODE_H
#define VDECODE_H

extern int vdecode_main(int, char **, FILE *, FILE *, FILE *);

typedef struct vdecode_args {
	int video_index;
	char *file_name;
} vdecode_args;

extern int process_args(vdecode_args *, int, char **, FILE *);

#endif
