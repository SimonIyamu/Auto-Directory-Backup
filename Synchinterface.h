#include "SupplementaryStructs.h"

Tree AddNode(Tree *ptr,char *Name, inode **inodelist ,const char *path);

void DeleteNode(Tree *ptr,char *Name, inode **inodelist);

int isDirectory(const char *path);

void do_ls(char  dirname [] , Tree *Mytree , inode **inodelist);

void DeleteTree(Tree *Root);

void Traversal(Tree *Root);

void Delete(Tree *Root, inode **inodelist );

void InodePrint(inode **inodelist);

void Myrm(Tree *Father,Tree *Root,char *Path,inode **inodelist);

void DirectoryDeletion(Tree *Origin,Tree *Backup,char *Path, inode **inodelist);

void DirectorySychronization(Tree *Origin,Tree *Backup,char *Path, inode **inodelist);

void InitializeNames(char *Original,char *Backup);

Tree Treefind(Tree Root,const char *Path);

int FileCopy(char *Original, char *Backup, int BUFFSIZE);

void InitiallizePointers(Tree *Original,Tree *Backup);

Tree FindById(Tree *Root,int id);

char *OriginalFileName;

char *DestFileName;
