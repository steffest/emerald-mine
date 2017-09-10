/* handle files in emerald mine */

char *global = "emerald";
static char *local = "file";

#include "proto.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

/* ---------------------------------------------------------------------- */

/* global arguments */

char *arg_display;
char *arg_geometry;
int arg_install;
int arg_fullscreen;

struct hash arg_cave; /* sorted list of all cave filenames */
struct hash arg_path; /* cave paths */
struct hash arg_audio; /* audio devices */
struct hash arg_joystick; /* joystick devices */
struct hash arg_keyboard; /* keyboard keycode names */
struct hash arg_name; /* player names */
int arg_select;

static char *def_audio[] = {
	"/dev/audio", "/dev/dsp", /* audio */
	"default", /* alsa */
};
static char *def_joystick[] = {
	"/dev/input/js0", "/dev/input/js1", /* linux */
	"/dev/joy0", "/dev/joy1", /* bsd */
};

/* ---------------------------------------------------------------------- */

/* handle a hash table of strings */

static void hash_add(struct hash *arg, char *str)
{
	if(arg->length >= arg->alloc) {
		char **ptr = arg->name;
		int size = 10;
		while(arg->length >= size) size = size * 8 / 5;
		if(ptr = realloc(ptr, size * sizeof(*ptr)), ptr == 0) return;
		arg->name = ptr;
		arg->alloc = size;
	}
	if(str = strdup(str), str == 0) return;
	arg->name[arg->length++] = str;
}

static void hash_free(struct hash *arg)
{
	char *str;
	if(arg->name) {
		while(arg->length) {
			str = arg->name[--arg->length];
			free(str);
		}
		free(arg->name);
	}
	memset(arg, 0, sizeof(*arg));
}

/* ---------------------------------------------------------------------- */

/* handle a string of tokens */

static char token_buffer[4096];

void file_token_read(struct hash *arg, int index, char **token, int cnt)
{
	char *str;
	int i;
	memset(token_buffer, 0, sizeof(token_buffer));
	if(index >= 0 && index < arg->length) strncpy(token_buffer, arg->name[index], sizeof(token_buffer) - 1);
	str = token_buffer;
	for(i = 0; i < cnt; i++) {
		while(*str && *str == ' ') str++;
		token[i] = str;
		while(*str && *str != ' ') str++;
		if(*str) *str++ = 0;
	}
}

void file_token_write(struct hash *arg, int index, char **token, int cnt)
{
	char *out = token_buffer, *end = token_buffer + sizeof(token_buffer) - 1;
	char *str;
	int i;
	memset(token_buffer, 0, sizeof(token_buffer));
	for(i = 0; i < cnt; i++) {
		if(i) if(out < end) *out++ = ' ';
		for(str = token[i]; *str; str++) if(out < end) *out++ = *str;
	}
	if(index >= 0 && index < arg->length) {
		if(str = strdup(token_buffer), str) {
			free(arg->name[index]);
			arg->name[index] = str;
		}
	}
}

/* ---------------------------------------------------------------------- */

/* handle environment variables, temporary files, read config */

static char *home;
static int homelen;

static char *config;

static FILE *cave_fp;
static long cave_len;

static char cave_template[16];
static int cave_remove;

static int cave_cache = -1;
static int level_cache = -1;

static struct unzip *zstate;

extern const char *unzip(struct unzip *, char **indin, char **indout, char *inend, char *outend);
extern void unzip_init(struct unzip **);
extern void unzip_free(struct unzip **);

int file_open(void)
{
	char *path;
	int pathlen;
	char *str;
	int i;

/* use the current directory as home on windows and EMSCRIPTEN*/
	home = "";

#if ! (defined WIN32 || defined MSDOS || defined __EMSCRIPTEN__)
	if(path = getenv("HOME"), path) {
		if(pathlen = strlen(path), pathlen) {
			if(str = malloc(pathlen + 2), str) {
				home = str;
				homelen = pathlen;
				memcpy(home, path, homelen);
#if (defined WIN32 || defined MSDOS || defined __EMSCRIPTEN__)
				for(str = home; str < home + homelen; str++) if(*str == '\\') *str = '/';
#endif
				if(home[homelen - 1] != '/') home[homelen++] = '/';
				home[homelen] = 0;
			}
		}
	}
#endif

	cave_fp = tmpfile();
	if(cave_fp == 0) {
		strcpy(cave_template, "XXXXXX"); /* try harder */
		mktemp(cave_template);
		cave_fp = fopen(cave_template, "w+b");
		if(cave_fp) cave_remove = 1;
	}
	if(cave_fp == 0) {
		fprintf(stderr, "%s[%s] %s: %s\n", global, local, "failed to create temporary cave file", strerror(errno));
		return(1);
	}

	unzip_init(&zstate);

#if ! (defined WIN32 || defined MSDOS)
	path = ".emcrc";
#else
	path = "emc.rc";
#endif
	pathlen = strlen(path);
	if(config = malloc(homelen + pathlen + 1), config) {
		sprintf(config, "%s%s", home, path);
	}

	file_config_read();

/* add default paths */

	if(arg_path.length == 0) {
		path = "cave";
		pathlen = strlen(path);
		if(homelen + pathlen < (int)(sizeof(token_buffer))) {
			sprintf(token_buffer, "%s%s", home, path);
			hash_add(&arg_path, token_buffer);
		}
	}
	if(arg_audio.length == 0) {
		for(i = 0; i < (int)(sizeof(def_audio) / sizeof(*def_audio)); i++) hash_add(&arg_audio, def_audio[i]);
	}
	if(arg_joystick.length == 0) {
		for(i = 0; i < (int)(sizeof(def_joystick) / sizeof(*def_joystick)); i++) hash_add(&arg_joystick, def_joystick[i]);
	}

/* maintain minimum size */

	for(i = arg_keyboard.length; i < 2; i++) {
		hash_add(&arg_keyboard, "");
	}
	for(i = arg_name.length; i < 10; i++) {
		hash_add(&arg_name, "none");
	}

	return(0);
}

int file_close(void)
{
	cave_cache = -1;
	level_cache = -1;

	file_config_write();

	hash_free(&arg_cave);
	hash_free(&arg_path);
	hash_free(&arg_audio);
	hash_free(&arg_joystick);
	hash_free(&arg_keyboard);
	hash_free(&arg_name);

	if(config) { free(config); config = 0; }

	unzip_free(&zstate);

	if(cave_fp) { fclose(cave_fp); cave_fp = 0; }
	if(cave_remove) { remove(cave_template); cave_remove = 0; }

	if(homelen) { free(home); homelen = 0; }
	return(0);
}

/* ---------------------------------------------------------------------- */

/* handle config file */

void file_config_read(void)
{
	FILE *fp = 0;
	if(config) {
		if(fp = fopen(config, "r"), fp == 0) {
			if(errno != ENOENT) {
				fprintf(stderr, "%s[%s] %s: %s: %s\n", global, local, config, "failed to open config file", strerror(errno));
			}
		}
	}
	if(fp) {
		while(fgets(token_buffer, sizeof(token_buffer), fp)) {
			char *opt = strtok(token_buffer, "=\r\n");
			char *str = strtok(0, "\r\n");
			if(opt == 0 || str == 0) continue;
			if(strcmp(opt, "cave") == 0) hash_add(&arg_path, str);
			if(strcmp(opt, "audio") == 0) hash_add(&arg_audio, str);
			if(strcmp(opt, "joystick") == 0) hash_add(&arg_joystick, str);
			if(strcmp(opt, "keyboard") == 0) hash_add(&arg_keyboard, str);
			if(strcmp(opt, "name") == 0) hash_add(&arg_name, str);
			if(strcmp(opt, "select") == 0) arg_select = atoi(str);
		}
		fclose(fp);
	}
}

void file_config_write(void)
{
	FILE *fp = 0;
	if(config) {
		if(fp = fopen(config, "w"), fp == 0) {
			fprintf(stderr, "%s[%s] %s: %s: %s\n", global, local, config, "failed to create config file", strerror(errno));
		}
	}
	if(fp) {
		int i;
		for(i = 0; i < arg_path.length; i++) fprintf(fp, "cave=%s\n", arg_path.name[i]);
		for(i = 0; i < arg_audio.length; i++) fprintf(fp, "audio=%s\n", arg_audio.name[i]);
		for(i = 0; i < arg_joystick.length; i++) fprintf(fp, "joystick=%s\n", arg_joystick.name[i]);
		for(i = 0; i < arg_keyboard.length; i++) fprintf(fp, "keyboard=%s\n", arg_keyboard.name[i]);
		for(i = 0; i < arg_name.length; i++) fprintf(fp, "name=%s\n", arg_name.name[i]);
		fprintf(fp, "select=%d\n", arg_select);
		if(ferror(fp)) {
			fprintf(stderr, "%s[%s] %s: %s: %s\n", global, local, config, "failed to write config file", strerror(errno));
		}
		if(fclose(fp)) {
			fprintf(stderr, "%s[%s] %s: %s: %s\n", global, local, config, "failed to flush config file", strerror(errno));
		}
	}
}

/* ---------------------------------------------------------------------- */

/* build the cave list, very slow
 *
 * special code to avoid stat because of ridiculous performance
 */

static int cmp_cave(const void *aa, const void *bb)
{
	char *a = *(void **)(aa), *b = *(void **)(bb);
	return(strcmp(a, b));
}

#ifdef __DJGPP__
#include <dir.h>
#endif

void file_cave_refresh(void)
{
	int i;

	hash_free(&arg_cave);
	for(i = 0; i < arg_path.length; i++) {
		int cnt = arg_cave.length;

		char *path = arg_path.name[i], *sep = "/", *file = "*.*";
		int pathlen = strlen(path), seplen = strlen(sep), filelen = strlen(file);
		if(pathlen < seplen || strcmp(path + pathlen - seplen, sep) == 0) { sep++; seplen--; }

#ifdef __DJGPP__
		{
			char *dirp;
			struct ffblk dp;
			int err;
			if(dirp = malloc(pathlen + seplen + filelen + 1), dirp) {
				sprintf(dirp, "%s%s%s", path, sep, file);
				for(err = findfirst(dirp, &dp, (FA_RDONLY | FA_HIDDEN | FA_SYSTEM | FA_LABEL | FA_DIREC | FA_ARCH)); err == 0; err = findnext(&dp)) {
					file = dp.ff_name;
					filelen = strlen(file);
					if(pathlen + seplen + filelen < (int)(sizeof(token_buffer))) {
						sprintf(token_buffer, "%s%s%s", path, sep, file);
						if(
						   (dp.ff_attrib & FA_DIREC) == 0
						) hash_add(&arg_cave, token_buffer); /* add only regular files */
					}
				}
				free(dirp);
			}
		}
#else
		{
			DIR *dirp;
			struct dirent *dp;
			struct stat st;
			if(dirp = opendir(path), dirp) {
				while(dp = readdir(dirp), dp) {
					file = dp->d_name;
					filelen = strlen(file);
					if(pathlen + seplen + filelen < (int)(sizeof(token_buffer))) {
						sprintf(token_buffer, "%s%s%s", path, sep, file);
						if((
#ifdef _DIRENT_HAVE_D_TYPE
						   dp->d_type == DT_REG) || (dp->d_type == DT_UNKNOWN &&
#endif
						   stat(token_buffer, &st) == 0 && S_ISREG(st.st_mode)
						)) hash_add(&arg_cave, token_buffer); /* add only regular files */
					}
				}
				closedir(dirp);
			} else fprintf(stderr, "%s[%s] %s: %s: %s\n", global, local, path, "failed to open cave directory", strerror(errno));
		}
#endif

		if(cnt < arg_cave.length) qsort(arg_cave.name + cnt, arg_cave.length - cnt, sizeof(*arg_cave.name), cmp_cave);
	}
	cave_cache = -1;
	level_cache = -1;
}

/* ---------------------------------------------------------------------- */

/* select a cave, select a level, read binary emc cave */

char file_cave_buffer[4096];

int file_select_cave_level(int cave, int level)
{
	FILE *fp;
	char *file = "*.*";
	if(cave >= 0 && cave < arg_cave.length) file = arg_cave.name[cave];

/* cache cave */

	if(cave == cave_cache) goto cave;
	cave_len = 0;
	cave_cache = -1;
	level_cache = -1;
	if(!(cave >= 0 && cave < arg_cave.length)) goto cave;

	unzip_init(&zstate);

	if(fp = fopen(file, "rb"), fp == 0) {
		fprintf(stderr, "%s[%s] %s: %s: %s\n", global, local, file, "failed to open cave", strerror(errno));
		goto cave;
	}
	rewind(cave_fp);
	{
		char *inend = token_buffer + sizeof(token_buffer), *in = inend;
		char *outend = file_cave_buffer + sizeof(file_cave_buffer), *out = file_cave_buffer;
		int type = 0;
		while(cave_len < 250000) {
			if(in == inend && in != 0) {
				int len = fread(token_buffer, 1, in - token_buffer, fp);
				in = token_buffer; inend = token_buffer + len;
				if(ferror(fp)) {
					fprintf(stderr, "%s[%s] %s: %s: %s\n", global, local, file, "failed to read cave", strerror(errno));
					break;
				}
			}
			if(in == inend) { in = 0; inend = 0; }
			if(type == 0 || type == 2) {
				const char *err = unzip(zstate, &in, &out, inend, outend);
				if(type == 0) {
					if(err) {
						in = token_buffer; /* rewind */
					} else {
						type = 2;
					}
				} else if(err) {
					fprintf(stderr, "%s[%s] %s: %s: %s\n", global, local, file, "failed to uncompress cave", err);
					break;
				}
			}
			if(type == 0 || type == 1) {
				if(in) {
					int len = inend - in;
					if(len > outend - out) len = outend - out;
					memcpy(out, in, len);
					in += len; out += len;
				}
				type = 1;
			}
			if(out == outend || in == 0) {
				int len = fwrite(file_cave_buffer, 1, out - file_cave_buffer, cave_fp);
				out = file_cave_buffer; outend = file_cave_buffer + len;
				if(ferror(cave_fp)) {
					fprintf(stderr, "%s[%s] %s: %s: %s\n", global, local, file, "failed to write temporary cave file", strerror(errno));
					break;
				}
				cave_len += len;
			}
			if(out == outend) break;
		}
	}
	if(fclose(fp)) {
		fprintf(stderr, "%s[%s] %s: %s: %s\n", global, local, file, "failed to close cave", strerror(errno));
	}

	cave_cache = cave;
cave:

/* cache level */

	if(level == level_cache) goto level;
	memset(file_cave_buffer, 0, 2172);
	level_cache = -1;
	if(!(level >= 0 && level < cave_len / 2172)) goto level;

	if(fseek(cave_fp, level * 2172, SEEK_SET)) {
		fprintf(stderr, "%s[%s] %s: %s: %s\n", global, local, file, "failed to open level", strerror(errno));
		goto level;
	}
	fread(file_cave_buffer, 2172, 1, cave_fp);
	if(ferror(cave_fp)) {
		fprintf(stderr, "%s[%s] %s: %s: %s\n", global, local, file, "failed to read level", strerror(errno));
		goto level;
	}

	level_cache = level;
level:

	return(cave_cache == -1 || level_cache == -1);
}

/* ---------------------------------------------------------------------- */

#if (defined AMIGA)
char *optarg; int optind = 1; int getopt(int argc, char * const argv[], const char *optstring) { return -1; }
#endif

extern long seed;

void quit(void);

int main(int argc, char **argv)
{
	int ch = ' ';

	fputs("X11 Emerald Mine © David Tritscher\n", stdout);
	fputs("EMScripten port by Steffest\n", stdout);
parse:
	switch(getopt(argc, argv, "-d:g:ifc:a:j:")) {
help:
	default:
		fputs("\nusage: emerald [options]\n", stderr);
		fputs("\t-d [arg]    display\n", stderr);
		fputs("\t-g [arg]    geometry\n", stderr);
		fputs("\t-i          install colourmap\n", stderr);
		fputs("\t-f          full screen\n", stderr);
		fputs("\t-c [...]    paths to search for caves\n", stderr);
		fputs("\t-a [...]    audio devices\n", stderr);
		fputs("\t-j [...]    joystick devices\n", stderr);
		fputs("[...] is a list of one or more arguments\n", stderr);
		return(1);
	case 'd': ch = 'd'; arg_display = optarg; goto parse;
	case 'g': ch = 'g'; arg_geometry = optarg; goto parse;
	case 'i': ch = 'i'; arg_install = 1; goto parse;
	case 'f': ch = 'f'; arg_fullscreen = 1; goto parse;
	case 'c': ch = 'c'; goto list;
	case 'a': ch = 'a'; goto list;
	case 'j': ch = 'j'; goto list;
list:
	case 1: /* gnu rubbish */
		switch(ch) {
		case 'c': hash_add(&arg_path, optarg); goto parse;
		case 'a': hash_add(&arg_audio, optarg); goto parse;
		case 'j': hash_add(&arg_joystick, optarg); goto parse;
		}
		fprintf(stderr, "%s: unexpected list of arguments -- %c\n", argv[0], ch);
		goto help;
	case -1:
		if(optind < argc) {
			fprintf(stderr, "%s: too many arguments\n", argv[0]);
			goto help;
		}
		argc -= optind;
		argv += optind;
	}

	seed += time(0);

	fprintf(stdout, "Seed: %d\n", seed);

	atexit(quit);

	fputs("Opening File\n", stdout);
	if(!file_open())
	fputs("Opening Display\n", stdout);
	if(!display_open())
	fputs("Opening Keyboard\n", stdout);
	if(!keyboard_open(), 1)
	fputs("Opening Joystick\n", stdout);
	if(!joystick_open(), 1)
	fputs("Opening Audio\n", stdout);
	if(!audio_open(), 1)
	fputs("Opening Screen\n", stdout);
	if(!screen_open())

	menu();

	return(0);
}

void quit(void)
{
	screen_close();
	audio_close();
	joystick_close();
	keyboard_close();
	display_close();
	file_close();
}
