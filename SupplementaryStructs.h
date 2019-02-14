struct subthings;

struct namelist;

struct nametree;

typedef enum { false, true } bool;

typedef struct inode{
    long int id;                        /*Added this*/
    off_t size;                     /*The size in bytes of the file .To print it you need to cast it as Long Int*/
    char *ModifiedTime;                /*Dynamically create a string returned for the ctime(&statresult.mtim)*/
    struct namelist *ListOfNamesLinked;
    int numberoffileslinked;
    struct inode *copy;

    bool modified;
    struct inode *nextinlist;
}inode;

struct nametree{
    struct subthings *list;
    char *name;
    char *path;
    struct inode *inodeptr;
};

typedef struct subthings{
    struct nametree *item;
    struct subthings *next;
} *children;

typedef struct nametree *Tree;

typedef struct namelist{
    char *contents;
    struct namelist *next;
} names;
