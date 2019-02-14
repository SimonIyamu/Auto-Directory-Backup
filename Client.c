#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "Synchinterface.h"
#include "listInterface.h"
#include "watchInterface.h"

#define EVENT_SIZE ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN ( 1024 * ( EVENT_SIZE + 16 ) )
#define EVENTS_OF_INTEREST (IN_CREATE | IN_ATTRIB | IN_MODIFY | IN_CLOSE_WRITE | IN_DELETE | IN_DELETE_SELF | IN_MOVED_FROM | IN_MOVED_TO)

void fail(const char *message);

int main(int argc, char **argv){

    int i=0;                                                                        /*Making the necessary initializations*/

    char *Original;                                                                 /*Pointers that will hold the name of the Original and Backup folders names*/

    char *Backup;

    inode *inodelist=NULL;                                                             /*Initiallizing the inodes list*/

    inode *inodelistBackup=NULL;

    Tree Origin=NULL;                                                                  /*The Original Folder tree*/

    Tree Copy=NULL;                                                                     /*The Backup Folder tree*/

    if(argc==3){                                                                        /*Storing the Original and Backup Folder Names*/

        Original= remove_last_slash(argv[++i]);

        Backup= remove_last_slash(argv[++i]);

    }

    else{                                                                               /*More arguements than the ones needed*/

        printf("Wrong Input\n");

        return 1;

    }
    InitializeNames(Original,Backup);

    char implement[strlen(Backup)+4];

    
    strcpy(implement,".");
    strcat(implement,"/");
    strcat(implement,Backup);
    if(!isDirectory(implement)){
        mkdir(implement,0777);
    }

    AddNode(&Copy,Backup,&inodelistBackup,Backup);                                            /*Adding the head of the tree , the father catalog*/

    AddNode(&Origin,Original,&inodelist,Original);

    InitiallizePointers(&Origin,&Copy);                                                  /*Added that*/

    do_ls(Backup,&Copy,&inodelistBackup);                                                     /*The construction of the catalog tree and the i node*/

    do_ls(Original,&Origin,&inodelist);

    //InodePrint(&inodelist);                                                            /*Print the inode*/

    //InodePrint(&inodelistBackup);

    DirectorySychronization(&Origin,&Copy,NULL,&inodelistBackup);

    DirectoryDeletion(&Origin,&Copy,NULL,&inodelistBackup);

    int length, read_ptr, read_offset;
    int wd;
    char buffer[EVENT_BUF_LEN];
    char *root_dir_name;

    /* Creating the INOTIFY instance*/
    fd = inotify_init();
    if (fd < 0)
        fail("inotify_init");

    if(argc==3){
        root_dir_name = argv[1];
    }
    else{
        printf("Error: Wrong ammount of arguments\n");
        exit(1);
    }

    /* Add inotify watch to the original directory and all subdirectories */
    add_watch_rec(root_dir_name);

    read_offset = 0; /* Remaining number of bytes from previous read */
    while (1) {
        /* Read next series of events */
        length = read(fd, buffer + read_offset, sizeof(buffer) - read_offset);
        if (length < 0)
            fail("read");
        length += read_offset;
        read_ptr = 0;
        
        /* Process each event making sure that at least the fixed part of the event in included in the buffer */
        while (read_ptr + EVENT_SIZE <= length ) { 
            /* Points to the fixed part of next inotify_event */
            struct inotify_event *event = (struct inotify_event *) &buffer[ read_ptr ];
            
            /* If the dynamic part exceeds the buffer */
            if( read_ptr + EVENT_SIZE + event->len > length ) 
                break;

            /* Undertake the necessary actions on the mirror directory */   
            handle_event(event,&Origin,&Copy,&inodelist,&inodelistBackup);

            /* Advance read_ptr to the beginning of the next event*/
            read_ptr += EVENT_SIZE + event->len;
        }
        /* Check to see if a partial event remains at the end */
        if( read_ptr < length ) {
            /* Copy the remaining bytes from the end of the buffer to the beginning of it */
            memcpy(buffer, buffer + read_ptr, length - read_ptr);
            /* The next read will begin immediatelly after them	*/
            read_offset = length - read_ptr;
        } else
            read_offset = 0;
    }

    close(fd); // TODO rm watchers klp  se sighandler

    /*Free the tree and the inode list*/
    Delete(&Copy,&inodelistBackup); 
    Delete(&Origin,&inodelist);
}

void fail(const char *message) {
    perror(message);
    exit(1);
}

