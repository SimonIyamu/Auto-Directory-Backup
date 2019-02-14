#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>

#include "Synchinterface.h"
#include "listInterface.h"
#include "watchInterface.h"

#define EVENTS_OF_INTEREST (IN_CREATE | IN_ATTRIB | IN_MODIFY | IN_CLOSE_WRITE | IN_DELETE | IN_DELETE_SELF | IN_MOVED_FROM | IN_MOVED_TO)

uint32_t cookie=0;
bool moved_from_flag=false;  /* Indicates whether the previous event was IN_MOVED_FROM or not */

/* Given the relative path of a directory, the following function
 * adds inotify watch to every subdirectory. */
void add_watch_rec(char *root_dir){ 
    DIR *dir_ptr;
    char *sub_dir_name;
    int wd;
    char *sub_dir; /* This variable holds the relative path of a sub directory */
    struct dirent *direntp;

    /* Adding a watch for the original directory */
    wd = inotify_add_watch(fd, root_dir, EVENTS_OF_INTEREST);
    if( wd == -1 ) 
        printf("Error: Failed to add watch %s\n", root_dir);
    else{
        list_push(wd,root_dir);
        printf("Watching %s\n", root_dir);
    }

    /* Open the directory */
    if((dir_ptr=opendir(root_dir))==NULL){
        printf("Error in opening %s\n",root_dir);
        return;
    }

    /* Iterate all subdirectories */
    while((direntp=readdir(dir_ptr))!=NULL){
        sub_dir_name=direntp->d_name;
        if(strcmp(sub_dir_name,".") && strcmp(sub_dir_name,"..")){ /* Ignores previous and current dir */
            sub_dir = (char *)malloc(sizeof(char)*(strlen(root_dir)+strlen(sub_dir_name) + 2)); /* +2 because of the "/" and the terminating null. */
            strcpy(sub_dir,root_dir);
            strcat(sub_dir,"/");
            strcat(sub_dir,sub_dir_name);
            if(isDirectory(sub_dir)){
                /* Recursive call */
                add_watch_rec(sub_dir);
            }
            free(sub_dir);
        }
    }
    closedir(dir_ptr);
}

/* Concatinate two paths */
char *pathcat(const char *a,const char *b){
    char *path = (char*) malloc(sizeof(char)*(strlen(a) + strlen(b) + 2));
    strcpy(path,a);
    strcat(path,"/");
    strcat(path,b);
    return path;
}

/* Change the root directory name of a path */
char *change_path_root(const char *path,const char *old_root,char *new_root){
    /* If its only a name */
    if(strchr(path,'/')==NULL)
            return new_root;

    char *new_path = (char*) malloc(sizeof(char)*(strlen(path) + strlen(new_root) - strlen(old_root) + 2));
    strcpy(new_path,new_root);
    strcat(new_path,path + strlen(old_root));
    return new_path;
}

/* If there is a slash in the end of the path, it gets removed */
/* E.x. Dir1/Dir2/ will become Dir1/Dir2 */
char *remove_last_slash(char *path){
    if(path[strlen(path)-1]=='/')
        path[strlen(path)-1]='\0';
    return path;
}

const char * target_type(struct inotify_event *event) {
    if( event->len == 0 )
        return "";
    else
        return event->mask & IN_ISDIR ? "directory" : "file";
}

const char * target_name(struct inotify_event *event) {
    return event->len > 0 ? event->name : NULL;
}

const char * event_name(struct inotify_event *event) {
    if (event->mask & IN_ACCESS)
        return "IN_ACCESS";
    else if (event->mask & IN_ATTRIB)
        return "IN_ATTRIB";
    else if (event->mask & IN_CLOSE_WRITE)
        return "IN_CLOSE_WRITE";
    else if (event->mask & IN_CREATE)
        return "IN_CREATE";
    else if (event->mask & IN_DELETE)
        return "IN_DELETE";
    else if (event->mask & IN_DELETE_SELF)
        return "IN_DELETE_SELF";
    else if (event->mask & IN_MODIFY)
        return "IN_MODIFY";
    else if (event->mask & IN_MOVED_FROM)
        return "IN_MOVED_FROM";
    else if (event->mask & IN_MOVED_TO)
        return "IN_MOVED_TO";
    else if (event->mask & IN_IGNORED)
        return "IN_IGNORED";
    else
        return "unknown";
}


void handle_event(struct inotify_event *event, Tree *Original, Tree *Backup, inode **inodelist, inode **inodelistBackup){
    char *moved_parent_dir;
    char *moved_file_path;
    char *moved_file_name;

    /* Ignore hidden files */
    if(event->name[0] == '.')
        return;

    printf("Event: %s   WD:%i   Type: %s    Name: %s   Cookie: %u\n",event_name(event),event->wd,target_type(event),target_name(event),event->cookie);

    if (event->mask & IN_ATTRIB){
        on_attrib(event,Original,Backup);
        printf("IN_ATTRIB event actions are complete.\n");
    }else if (event->mask & IN_CLOSE_WRITE){
        on_close_write(event,Original,Backup,inodelist,inodelistBackup);
        printf("IN_CLOSE_WRITE event actions are complete.\n");
    }else if (event->mask & IN_CREATE){
        on_create(event,Original,Backup,inodelist,inodelistBackup);
        printf("IN_CREATE event actions are complete.\n");
    }else if (event->mask & IN_DELETE){
        on_delete(event,Original,Backup,inodelist,inodelistBackup);
        printf("IN_DELETE event actions are complete.\n");
    }else if (event->mask & IN_DELETE_SELF){
        on_delete_self(event,Original,Backup,inodelist,inodelistBackup);
        printf("IN_DELETE_SELF event actions are complete.\n");
    }else if (event->mask & IN_MODIFY){
        on_modify(event,Original);
        printf("IN_MODIFY event actions are complete.\n");
    }else if (event->mask & IN_MOVED_FROM){
        /* The event will be handled on the next call */
        /* Save cookie.*/
        cookie = event->cookie; 
        /* Save the current file path and it's parent path*/
        moved_parent_dir=list_find(event->wd);
        moved_file_path=pathcat(moved_parent_dir,event->name);
        moved_file_name=(char*)malloc(sizeof(char)*(strlen(event->name)+1));
        strcpy(moved_file_name,event->name);
        printf("IN_MOVED_FROM event will be handled after the next event\n");
    }else if (event->mask & IN_MOVED_TO){
        /* If movement begins and ends within the monitored hierarchy */
        if(moved_from_flag && event->cookie == cookie){
            printf("IN_MOVED_FROM event actions are complete.\n");
            on_moved_to(event,moved_file_path,moved_parent_dir,Original,Backup,inodelist,inodelistBackup);
        }else{
            /* A file is coming from outside of the monitored hierarchy */
            on_create(event,Original,Backup,inodelist,inodelistBackup); }
        printf("IN_MOVED_TO event actions are complete.\n");
    }

    /* If next event of IN_MOVED_FROM was not IN_MOVED_TO */
    /* or it was IN_MOVED_TO but with different cookie */
    if (moved_from_flag && (!(event->mask & IN_MOVED_TO) || 
       ((event->mask & IN_MOVED_TO) && event->cookie != cookie))){
        on_moved_from(moved_file_path,moved_file_name,moved_parent_dir,Original,Backup,inodelist,inodelistBackup);
        free(moved_file_name);
        printf("IN_MOVED_FROM event actions are complete.\n");
    }

    /* This flag indicates whether the previous event was 
     * IN_MOVED_FROM or not */
    if (event->mask & IN_MOVED_FROM)
        moved_from_flag=true; 
    else
        moved_from_flag=false;
}

void on_attrib(struct inotify_event *event,Tree *Original,Tree *Backup){
    const char *parent_dir = list_find(event->wd);

    /* The file path will be parent dir + file name */
    char *file_path = pathcat(parent_dir,event->name);
    
    /* If its a file */ 
    if(!isDirectory(file_path)){
        /* The destination's file path will be the same as file_path */
        /* but with diffrent initial directory (DestFileName) */
        char *dest_file_path = change_path_root(file_path,OriginalFileName,DestFileName);
        
        Tree file_tree_node = Treefind(*Original,file_path);
        inode *file_inode = file_tree_node->inodeptr;

        Tree dest_file_tree_node = Treefind(*Backup,dest_file_path);
        inode *dest_file_inode = dest_file_tree_node->inodeptr;
       
        struct stat file_stat_buf,dest_file_stat_buf;
        if(stat(file_path,&file_stat_buf)==-1 || stat(dest_file_path,&dest_file_stat_buf)==-1){
            perror("Failed to get file status\n");
            return;
        }

        /* If modification time has changed */
        if(file_stat_buf.st_mtime != dest_file_stat_buf.st_mtime){
            /* Update  inode modification time */
            struct utimbuf times;
            times.modtime = times.actime = file_stat_buf.st_mtime;
            utime(dest_file_path,&times);
            
            /* Update inode table entries*/
            file_inode->ModifiedTime=ctime(&file_stat_buf.st_mtime);
            dest_file_inode->ModifiedTime=ctime(&file_stat_buf.st_mtime);
        }
    }
}

void on_close_write(struct inotify_event *event,Tree *Original, Tree *Backup, inode **inodelist, inode **inodelistBackup){
    const char *parent_dir = list_find(event->wd);

    /* The file path will be parent dir + file name */
    char *file_path = pathcat(parent_dir,event->name);

    Tree file_tree_node = Treefind(*Original,file_path);
    inode *file_inode = file_tree_node->inodeptr;

    /* If it has been marked as modified */
    if(file_inode->modified == true){
        char *dest_parent_dir = change_path_root(parent_dir,OriginalFileName,DestFileName);
        char *dest_file_path = pathcat(dest_parent_dir,event->name);

        /* Copy inode */
        FileCopy(file_path,dest_file_path,1);

        /* Update the Original Tree Node */
        Tree parent_tree_node = Treefind(*Original,parent_dir);
        DeleteNode(&parent_tree_node,event->name,inodelist);
        Tree file_tree_node =AddNode(&parent_tree_node,event->name,inodelist,file_path);
        file_inode = file_tree_node->inodeptr;

        /* Update the Backup Tree Node*/
        Tree parent_backuptree_node = Treefind(*Backup,dest_parent_dir);
        DeleteNode(&parent_backuptree_node,event->name,inodelistBackup);
        Tree dest_file_tree_node = AddNode(&parent_backuptree_node,event->name,inodelistBackup,dest_file_path);
        
        file_inode->copy = dest_file_tree_node->inodeptr;
        
        /* The modification has been taken care of */
        file_inode->modified = false;
    }
}

void on_create(struct inotify_event *event,Tree *Original, Tree *Backup, inode **inodelist, inode **inodelistBackup){
    const char *parent_dir = list_find(event->wd);
    const char *dest_parent_dir = change_path_root(parent_dir,OriginalFileName,DestFileName);


    /* The file path will be parent dir + file name */
    char *file_path = pathcat(parent_dir,event->name);
    char *dest_file_path = pathcat(dest_parent_dir,event->name);

    /* If it's a directory */
    if(isDirectory(file_path)){
        /* Create the new directory in the destination */
        mkdir(dest_file_path,0777);

        Tree parent_tree_node = Treefind(*Original,parent_dir);
        Tree parent_backuptree_node = Treefind(*Backup,dest_parent_dir);

        AddNode(&parent_tree_node,event->name,inodelist,file_path);
        AddNode(&parent_backuptree_node,event->name,inodelistBackup,dest_file_path);

        /* Add watch to the new directory */
        add_watch_rec(file_path);
    }
    else{
        Tree parent_tree_node = Treefind(*Original,parent_dir);
        Tree parent_backuptree_node = Treefind(*Backup,dest_parent_dir);

        /* Add node to the Original Tree */
        AddNode(&parent_tree_node,event->name,inodelist,file_path);

        Tree file_tree_node = Treefind(*Original,file_path);
        inode *file_inode = file_tree_node->inodeptr;
        /* If there exists a copy */
        if(file_inode->copy!=NULL){
            /* Link */
            Tree sibling = FindById(Backup,file_inode->copy->id);
            link(sibling->path,dest_file_path);
            AddNode(&parent_backuptree_node,event->name,inodelistBackup,dest_file_path);
        }
        else{
            /* Create */
            FileCopy(file_path,dest_file_path,1024);
            Tree dest_file_tree_node = AddNode(&parent_backuptree_node,event->name,inodelistBackup,dest_file_path);
            file_inode->copy = dest_file_tree_node->inodeptr;
        }
    }
}

void on_delete(struct inotify_event *event,Tree *Original, Tree *Backup, inode **inodelist, inode **inodelistBackup){
    const char *parent_dir = list_find(event->wd);
    const char *dest_parent_dir = change_path_root(parent_dir,OriginalFileName,DestFileName);

    /* The file path will be parent dir + file name */
    char *file_path = pathcat(parent_dir,event->name);
    char *dest_file_path = pathcat(dest_parent_dir,event->name);

    /* If it is a file (and file exists) */
    if(access(dest_file_path, F_OK)!=-1 && !isDirectory(dest_file_path)){
        /* Delete it from the Original Tree */
        Tree parent_tree_node = Treefind(*Original,parent_dir);
        DeleteNode(&parent_tree_node,event->name,inodelist);

        /* Delete it from the Backup Tree */
        Tree parent_backuptree_node = Treefind(*Backup,dest_parent_dir);
        DeleteNode(&parent_backuptree_node,event->name,inodelistBackup);

        /* Unlink */
        unlink(dest_file_path);
    }
}

void on_delete_self(struct inotify_event *event,Tree *Original, Tree *Backup, inode **inodelist, inode **inodelistBackup){

    char *dir_path = list_find(event->wd);
    char *parent_dir = (char*)malloc(sizeof(char)*(strlen(dir_path)+1));

    /* For parent_dir: copy dir_path until the last / */
    int i;
    for(i=0; i<strlen(dir_path) ; i++){
        if(&(dir_path[i])==strrchr(dir_path,'/'))
            break;
        parent_dir[i] = dir_path[i];
    }
    parent_dir[i]='\0';

    /* Find dir name */
    char *dir_name=(char*)malloc(sizeof(char)*(strlen(strrchr(dir_path,'/'))+2));
    strcpy(dir_name,strrchr(dir_path,'/'));
    for(i=1; i<=strlen(dir_name) ; i++){
        dir_name[i-1] = dir_name[i];
    }
    dir_name[i]='\0';

    const char *dest_parent_dir = change_path_root(parent_dir,OriginalFileName,DestFileName);
    char *dest_dir_path = pathcat(dest_parent_dir,dir_name);

    /* Delete it from the Original and Backup Tree */
    Tree parent_tree_node = Treefind(*Original,parent_dir);
    Tree parent_backuptree_node = Treefind(*Backup,dest_parent_dir);

    /* Delete it from the Backup Tree */
    DeleteNode(&parent_tree_node,dir_name,inodelist);
    DeleteNode(&parent_backuptree_node,dir_name,inodelistBackup);

    /* Remove directory in destination */
    rmdir(dest_dir_path);

    list_remove(event->wd);
    inotify_rm_watch(fd,event->wd);
}

void on_modify(struct inotify_event *event, Tree *Original){
    const char *parent_dir = list_find(event->wd);

    /* The file path will be parent dir + file name */
    char *file_path = pathcat(parent_dir,event->name);

    /* Find its Tree Node */
    Tree file_tree_node = Treefind(*Original,file_path);

    /* Mark it as modified */
    file_tree_node->inodeptr->modified = true;
}

/* This function is only called in case the file is moved outside 
 * the hierarchy */
void on_moved_from(char *file_path, char *file_name, char *parent_dir,Tree *Original, Tree *Backup, inode **inodelist, inode **inodelistBackup){
    char *dest_parent_dir = change_path_root(parent_dir,OriginalFileName,DestFileName);
    char *dest_file_path = pathcat(dest_parent_dir,file_name);

    /* If it is a file (and file exists) */
    if(access(dest_file_path, F_OK)!=-1 && !isDirectory(dest_file_path)){
        /* Delete it from the Original Tree */
        Tree parent_tree_node = Treefind(*Original,parent_dir);
        DeleteNode(&parent_tree_node,file_name,inodelist);

        /* Delete it from the Backup Tree */
        Tree parent_backuptree_node = Treefind(*Backup,dest_parent_dir);
        DeleteNode(&parent_backuptree_node,file_name,inodelistBackup);

        /* Unlink */
        unlink(dest_file_path);
    }
}

/* This function is only called in case the movement is between 
 * directories in the monitored hierarchy */
void on_moved_to(struct inotify_event *event, char *prev_file_path, char *prev_parent_dir,  Tree *Original, Tree *Backup, inode **inodelist, inode **inodelistBackup){
    const char *parent_dir = list_find(event->wd);
    const char *dest_parent_dir = change_path_root(parent_dir,OriginalFileName,DestFileName);

    /* The file path will be parent dir + file name */
    char *file_path = pathcat(parent_dir,event->name);
    char *dest_file_path = pathcat(dest_parent_dir,event->name);

    char *prev_dest_file_path = change_path_root(prev_file_path,OriginalFileName,DestFileName);
    char *prev_dest_parent_dir = change_path_root(prev_parent_dir,OriginalFileName,DestFileName);

    /* Move the file name */
    link(prev_dest_file_path,dest_file_path);
    unlink(prev_dest_file_path);

    /* Update the Original Tree */
    Tree parent_tree_node = Treefind(*Original,prev_parent_dir);
    DeleteNode(&parent_tree_node,event->name,inodelist);
    AddNode(&parent_tree_node,event->name,inodelist,file_path);

    /* Update the Backup Tree */
    Tree parent_backuptree_node = Treefind(*Backup,prev_dest_parent_dir);
    DeleteNode(&parent_backuptree_node,event->name,inodelistBackup);
    AddNode(&parent_backuptree_node,event->name,inodelistBackup,dest_file_path);
}
