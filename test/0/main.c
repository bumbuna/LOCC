//bumbuna <developer@devbumbuna.com>
//2022
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#ifdef DEBUG
#define D(...) fprintf(stderr, __VA_ARGS__)
#define PS(...) time_t starttime = time(NULL), endtime;
#define PE(...) do {\
    endtime = time(NULL); \
    D("time taken: %u\n", (int)(endtime-starttime)); \
    } while(0);
#else
#define D(...)
#define PS(...)
#define PE(...)
#endif

enum FILE_TYPE {
    TYPE_C,
    TYPE_OTHER
};

struct locrecord {
    enum FILE_TYPE type;
    int loc;
    time_t last_update;
};

static char *TARGET_DIR = ".";
#define CURRENT_DIR  "."
#define PARENT_DIR  ".."
#define abort(...) exit(__VA_ARGS__)
#define save(f,c) printf("%s: %d\n", f, c)
#define MAX_DEPTH 16
#define MIN_DEPTH 1
#define SUPPORTED_TYPES 1
#define C_KEY 1
static int SESSION_DEPTH = MAX_DEPTH;
static struct locrecord LOC_TBL[SUPPORTED_TYPES];
static char READ_BUFFER[1024];

// static void (*store_locrecord)(enum FILE_TYPE, int);
// static void (*foreach_locrecord)(void(*)(struct locrecord *));
// static struct locrecord (*get_locrecord)(enum FILE_TYPE);

//enum FILE_TYPE represents a file type
//An integer represents recent LOCs
//enum FILE_TYPE, Integer -> void
//increment the LOC for files of type <b>type</b> by <b>loc</>
void locrecords_store(enum FILE_TYPE type, int loc) {
    switch(type) {
        case TYPE_C: {
#if defined(ARRAY_STORAGE)
            LOC_TBL[C_KEY].loc += loc;
            LOC_TBL[C_KEY].last_update = time(NULL);
#endif
            break;
        }
    }

}

//void(*)(struct locrecord*) represents a procedure signature
//void(*)(struct locrecord*) -> void
//apply procedure <b>to_apply</b> on all locrecords found 
static void locrecords_foreach(void (*to_apply)(struct locrecord *)) {
#if defined(ARRAY_STORAGE)
    static int i = 0;
    for(; i < SUPPORTED_TYPES; i++) {
        (*to_apply)(&LOC_TBL[i]);
    }
#endif
}

//enum FILE_TYPE represents a file type
//enum FILE_TYPE -> struct locrecord *
//find and return a locrecord got files of type <b>type</b> 
static struct locrecord *locrecords_search(enum FILE_TYPE type) {
#if defined(ARRAY_STORAGE)
    switch(type) {
        case TYPE_C: return &LOC_TBL[C_KEY];
    }
#endif
return NULL;
}

//A string represents a file name
//String -> enum FILE_TYPE
//determine the file type of file <b>file</b>
//using its extension
enum FILE_TYPE file_type(char *file) {
    char *extension = NULL;
    char *temp = file;
    while((temp = strchr(temp, '.'))) {
        extension = ++temp;
    }
    if(!extension) {
        return TYPE_OTHER;
    }
    char *extdup = strdup(extension);
    enum FILE_TYPE f;
    for(int i = 0; i<strlen(extension); i++) {
        extdup[i] = tolower(extdup[i]);
    }
    if(!strcmp(extdup, "c")) f =  TYPE_C;
    else f = TYPE_OTHER;
    return f;
}

//An integer represents a file descriptor
//Integer -> Integer
//count the LOC in file <b>fd</b>
int loc_in_file(int fd) {
    int loc = 0;
    char c;
    int rd = 0;
    while((rd = read(fd, READ_BUFFER, sizeof(READ_BUFFER)))) {
        if(rd == -1) {
            D("read error:");
            return 0;
        }
        for(int i = 0; i < rd; i++) {
            if(READ_BUFFER[i] == '\n') loc++;
        }
    }
    return loc;
}

//A string represents a directory name/path
//An integer represent the depth from project directory
//String, Integer -> void
//traverse directory <b>dir</b> applying <i>loc_in_file</i>
//on regular files and opening subdirectories recursively.
//if <b>depth</b> is greater than SESSION_DEPTH <b>dir</b> is
//not processed
void traverse_dir(char *dir, int depth) {
    if(depth > SESSION_DEPTH) return;
    if(!dir) return;
    if(chdir(dir)) {
        return;
    }
    DIR *r = opendir(CURRENT_DIR);
    if(!r) goto _exit;
    struct dirent *d;
    errno = 0;
    while((d = readdir(r))) {
        switch(d->d_type) {
            case DT_REG: {
                enum FILE_TYPE t = file_type(d->d_name);
                if(t == TYPE_C) {
                    int fd = open(d->d_name, O_RDONLY);
                    locrecords_store(t, loc_in_file(fd));
                    close(fd);
                }
                break;
            }
            case DT_DIR: {
                if(strcmp(d->d_name, CURRENT_DIR)
                && strcmp(d->d_name, PARENT_DIR))
                    traverse_dir(d->d_name, depth+1);
                break;
            }
        }
    }
    if(errno) {
        perror("readdir");
    }
_exit:
    chdir(PARENT_DIR);
    closedir(r);
}

//Integer,String[] -> Integer
//Launch
int main(int argc, char **argv) {
    PS();
    if(argc >= 2) {
        TARGET_DIR = argv[1];
    }
    if(argc == 3) {
        sscanf(argv[2], "%d", &SESSION_DEPTH);
        SESSION_DEPTH = SESSION_DEPTH > MAX_DEPTH ? MAX_DEPTH : SESSION_DEPTH;
        SESSION_DEPTH = SESSION_DEPTH < MIN_DEPTH ? MIN_DEPTH : SESSION_DEPTH;
    }
    D("depth: %d\n", SESSION_DEPTH);
    traverse_dir(TARGET_DIR, 0);
    struct locrecord *r = locrecords_search(TYPE_C);
    printf("C: %d\n", r->loc);
    PE();
    return 0;
}
