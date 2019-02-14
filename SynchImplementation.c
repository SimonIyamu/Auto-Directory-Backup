#include   <stdio.h>

#include   <string.h>

#include   <sys/types.h>

#include   <dirent.h>

#include   <stdlib.h>

#include   <sys/stat.h>

#include   <unistd.h>

#include   <time.h>

#include "Synchinterface.h"

#include <fcntl.h>

#define   SIZE             30
#define   PERM              0644

Tree *HeadBackup;
Tree *HeadOrigin;

void InitializeNames(char *Original,char *Backup){              /*Function that initiallizes the names of our parent directories in catholic variables*/

    OriginalFileName=Original;
    DestFileName=Backup;

}

void InitiallizePointers(Tree *Original,Tree *Backup){          /*Function that stores the head of our two head tree pointers in catholic variables*/

    HeadBackup=Backup;

    HeadOrigin=Original;

}

void DirectorySychronization(Tree *Origin,Tree *Backup,char *Path, inode **inodelist);      /*Just an Initiallization of a function we will implement later on*/


Tree Treefind(Tree Root,const char *Path){                                          /*A function that finds a node based on the path given*/
    if(Root==NULL)                                                                  /*If we dont find it return NULL*/
        return NULL;
    if(strcmp(Path,Root->path)==0){                                                 /*If we find it return the TreeNode Pointer*/
        return Root;
    }
  
    /* Recursive call for all children */ 
    struct subthings *child = Root->list;
    Tree s;
    while(child!=NULL){                                                     /*Use the function Reccursively for every child in the list pf child of the current examined tree node pointer*/
        s = Treefind(child->item,Path);
        if(s!=NULL)                                                         /*If the solution is found and is not NULL return it*/
            return s;
        child=child->next;
    }
}

Tree FindById(Tree *Root,int id){                                             /*A function that finds a node by ID of i node*/

    children temp;
    Tree success;
    temp=(*Root)->list;

        while(temp!=NULL){                                                     /*Reccursively call the function for each child of the current node*/

            success=FindById(&(temp->item),id);
            if(success!=NULL)                                                   /*If a solution is found , return it*/
                return success;

            if(id==temp->item->inodeptr->id)                                /*if we find the node with the wanted i-node , return it*/
                return temp->item;
            temp=temp->next;
        }
        return NULL;
}

Tree AddNode(Tree *ptr,char *Name, inode **inodelist ,const char *path){            /*This dunction creates a file on our tree*/

    inode *pointer;

    inode *pointer2;
    
    if((*ptr)==NULL){                                                                   /*If it the file is the father directory*/

        (*ptr)=(Tree)malloc(sizeof(struct nametree));                                       /*Dynamically create the node Tree*/

        (*ptr)->list=NULL;                                                              /*It has no children yet*/
        
        (*ptr)->name=(char*)malloc(strlen(Name)+1);                                             /*Dynamically allocating the space for the file name*/

        (*ptr)->path=(char*)malloc(strlen(path)+1);

        strcpy((*ptr)->path,path);                                                          /*Except from the name we store the path towards it as well*/
        
        strcpy((*ptr)->name,Name);                                                      /*Giving the node the file name*/

        pointer=(*inodelist);                                                                  /*Storing the i node list in a local variable*/

        struct stat statbuf; 

        if (stat(path, &statbuf) ==  -1){                                                     /*Acessing the data of the file*/

            
            perror("Failed  to get  file  status");
            
            return NULL;
        
        }

        if(pointer!=NULL){                                                              /*If the inodelist is not empty*/

            while(pointer->nextinlist!=NULL){                                                   /*Reach for the end of the list*/

                pointer = pointer->nextinlist;

            }

            pointer->nextinlist=malloc(sizeof(inode));                                      /*Allocate memmory for the i-node*/

            pointer2 = pointer->nextinlist;                                                 /*Store the node on a local cariable*/
        
        }
        
        else{                                                                       /*If the node list is empty*/

            pointer=malloc(sizeof(inode));                                                  /*Make the inode*/

            pointer2=pointer;

            (*inodelist)=pointer2;                                                  /*Store it in the same variable so that we save time NOTE: I CAN MAKE A FUNCTION FOR THAT*/

        }

        pointer2->size=statbuf.st_size;                                             /*Storing the size*/

        pointer2->id=statbuf.st_ino;

        pointer2->copy=NULL;

        pointer2->ModifiedTime=ctime (& statbuf.st_mtime);                                  /*Saving the last modified time*/

        pointer2->ListOfNamesLinked=(names*)malloc(sizeof(names));                      /*Making the list of names linked and making the char pointer "contents",show to the name of the treenode/file*/

        pointer2->ListOfNamesLinked->contents=(*ptr)->name;

        pointer2->numberoffileslinked=1;                                                        /*Currently we have one name stored*/

        pointer2->ListOfNamesLinked->next=NULL;

        pointer2->modified=false;                                                           /*A pointer to see wheter its modified(used in the inotify events)*/

        pointer2->nextinlist=NULL;

        (*ptr)->inodeptr=pointer2;                                                              /*THe pointer of the tree node to the i node*/
        
        return *ptr;
    
    }
    else{                                                                                       /*The tree is not empty*/
        
        children child;

        children sort;

        children before;
        
        Tree successor;

        Tree temporary;
        
        successor=(Tree)malloc(sizeof(struct nametree));                                        /*Allcoating the tree node as before*/
        
        successor->list=NULL;
        
        successor->name=(char*)malloc(strlen(Name)+1);

        successor->path=(char*)malloc(strlen(path)+1);

        strcpy(successor->path,path);
        
        strcpy(successor->name,Name);

        pointer=(*inodelist);                                                                   /*Checking the new node we made , which child of the father node is it?*/

        struct stat statbuf; 
        
        if (stat(path, &statbuf) ==  -1){
            perror("Failed  to get  file  status");
            return NULL;
        }

        int flag=0;

        long int id;
        
        while(pointer->nextinlist!=NULL){                                                               /*if the next object we want to create has the same id as one already in the tree, it means its hardlinked so change the flag value*/

           id=pointer->id;

           if(id==statbuf.st_ino){

               flag=1;
               break;

           }

           pointer = pointer->nextinlist;

        }
        
        id=pointer->id;

        if(id==statbuf.st_ino){

            flag=1;

        }
        if(flag==0){                                                                        /*If flag=0 , which means that the inode of the object is new*/

            pointer->nextinlist=malloc(sizeof(inode));                                              /*Make a position for it in the child list of the father*/

            pointer2 = pointer->nextinlist;                                                         /*Same procedure as before*/

            pointer2->size=statbuf.st_size;

            pointer2->id=statbuf.st_ino;

            pointer2->copy=NULL;

            pointer2->ModifiedTime=ctime (& statbuf.st_mtime);

            pointer2->ListOfNamesLinked=(names*)malloc(sizeof(names));

            pointer2->ListOfNamesLinked->contents=successor->name;

            pointer2->numberoffileslinked=1;

            pointer2->ListOfNamesLinked->next=NULL;

            pointer2->modified=false;

            pointer2->nextinlist=NULL;
        }
        if((*ptr)->list==NULL){                                                                         /*If no TreeNode childs exist for this SubTree node ptr*/
        
            (*ptr)->list=(children)(malloc(sizeof(struct subthings)));                                  /*Make it his child at the start of the list*/
        
            (*ptr)->list->item=successor;
        
            (*ptr)->list->next=NULL;
        
        }
        
        else{                                                                                               /*Else we insert it at the end of it*/
        
            child=(*ptr)->list;

            sort=(*ptr)->list;

            
        
            while(child->next!=NULL){                                                                   /*Find the end of the list and insert it*/
        
                child=child->next;
        
            }
        
            child->next=(children)(malloc(sizeof(struct subthings)));
        
            child->next->item=successor;
        
            child->next->next=NULL;

            before=sort;

            while(sort->item!=successor){                                                       /*Here we sort the list of children in an alphabetical each time a child is inserted*/ 

                if(strcmp(sort->item->name,successor->name)>0){

                    temporary=sort->item;
                    
                    child->next->item=sort->item;

                    if(sort==before){

                        (*ptr)->list->item=successor;

                    }
                    else{

                        before->next->item=successor;

                    }
                    break;                   

                }
                before=sort;

                sort=sort->next;
            
            }
        
        }
        if(flag==0)                                                 /*Again flag=0 means the inode is new so directly connect it*/
            successor->inodeptr=pointer2;
        else{                                                       /*Else it means the inode already exists , and it is stored in the pointer value*/
            successor->inodeptr=pointer;
            pointer->numberoffileslinked++;                         /*So he create our hardlink to the already existing node and adding the name on the list*/
            names *Namelist;
            Namelist=pointer->ListOfNamesLinked;
            while(Namelist->next!=NULL){

                Namelist=Namelist->next;

            }
            Namelist->next=(names*)malloc(sizeof(struct namelist));
            Namelist->next->contents=successor->name;
            Namelist->next->next=NULL;
        }
        
        return successor;
    }

}
void DeleteNode(Tree *ptr,char *Name, inode **inodelist){                                           /*This function deletes a Node from the tree .Note that we find it based on the father node and the nodes name*/

    children target;

    children prev;
    
    target=(*ptr)->list;

    prev=target;

    while(target!=NULL){                                                            /*Traversing the tree untill we reach the file/node we want to delete*/

        if(strcmp(target->item->name,Name)==0){

            break;

        }

        prev=target;

        target=target->next;

    }

    if(target==prev)                                                            /*Making the necessary connections to omit this node*/

        (*ptr)->list=target->next;

    else

        prev->next=target->next;

    inode *wanted;

    inode *previous;

    names *temp;

    names *helping;

    names *deleter;

    wanted=(*inodelist);

    previous=wanted;

    while(wanted!=NULL){                                                                    /*We do the same thing to find the wanted inode*/

        if(wanted==target->item->inodeptr)

            break;

        previous=wanted;

        wanted=wanted->nextinlist;

    }
    if(wanted->numberoffileslinked>1){                                                              /*If we have more than one name linked , then we dont need to delete the inode completely*/
       
        wanted->numberoffileslinked--;                                                              /*Decrease the number of files connected*/

        temp=wanted->ListOfNamesLinked;

        if(strcmp(temp->contents,Name)==0){                                                         /*If it is at the beggining of the list make the next name as the beggining*/

            wanted->ListOfNamesLinked=wanted->ListOfNamesLinked->next;

            free(temp);

        }
        else{                                                                                   /*we need to find it in the list , omit it and delete it*/
            helping=temp;
            while(temp!=NULL){

                if(strcmp(temp->contents,Name)==0){

                    helping->next=temp->next;

                    deleter=temp;

                    temp=temp->next;

                    free(deleter);
                
                }
                else{

                    helping=temp;
                    temp=temp->next;

                }
            }
        }
    }
    else{                                                                           /*Being here means that we only have one name connected , so we need to delete the i node*/
        if(wanted==(*inodelist))

            (*inodelist)=wanted->nextinlist;

        else

            previous->nextinlist=wanted->nextinlist;

        while(wanted->ListOfNamesLinked!=NULL){                                     /*AFter we find it we delete every name linked to it */

            temp=wanted->ListOfNamesLinked;

            wanted->ListOfNamesLinked=wanted->ListOfNamesLinked->next;

            free(temp);

        }
        free(wanted);
    }

    free(target->item->name);                                                       /*We free all the dynamically created components , namely , the Path , name , child position and the node itself*/

    free(target->item->path);

    free((target->item));

    target->item=NULL;

    free(target);

    target=NULL;

}

int isDirectory(const char *path) {                             /*Function that checks if the visited structure is a file*/

   struct stat statbuf;

   if (stat(path, &statbuf) != 0){
       return 0;
   }

   return S_ISDIR(statbuf.st_mode);

}

void     do_ls(char  dirname [] , Tree *Mytree , inode **inodelist){                            /*Function that makes the directory tree*/

    Tree     mychild=NULL;

    DIR      *dir_ptr;

    char    *directory;

    struct   dirent *direntp;

    char     final[50];                                     /*Better use realloc instead , in the end*/

    strcpy(final,"");

    if ( ( dir_ptr = opendir( dirname ) ) == NULL )                                         /*Open the directory*/

        fprintf(stderr , "cannot  open %s \n",dirname);

    else {

        while ( ( direntp=readdir(dir_ptr) ) != NULL ){                                 /*If it succeeds read the directory*/

            directory=direntp->d_name;                                                          /*Read the name*/

            if(strcmp(directory,".") && strcmp(directory,"..")){                            /*Ignore the directories . .. which mean the current and previous directories*/

                strcat(final,dirname);

                strcat(final,"/");

                strcat(final,directory);                                                    /*Making the path so that we can give it to the nexts reccursion*/

                (mychild)=AddNode(Mytree,directory,inodelist,final);                            /*Add the child to the Directory tree*/

                if(isDirectory(final))                                                       /*If it is a directory then use it for the reccursion*/

                    do_ls(final,&mychild,inodelist);

                strcpy(final,"");

            }

        }

        closedir(dir_ptr);

    }

}

void Traversal(Tree *Root){                             /*DFS traversal of the tree (preorder)*/
    
    Tree ptr=NULL;

    ptr=*Root;

    printf("%s\n",ptr->path);

    children offspring;

    offspring=ptr->list;

    while (offspring!=NULL){                                /*While children do exist*/
    
        Traversal(&(offspring->item));                      /*Visit them*/
    
        offspring=offspring->next;                          /*Move on to the next child*/
    
    } 
}

void DeleteTree(Tree *Root ){                           /*Function that deletes the tree*/

    Tree ptr=NULL;

    ptr=*Root;                                          /*Ptr points to the head of the tree*/

    children offspring;

    children temp;

    offspring=ptr->list;                                /*Storing ptr->list in a local temporary variable*/

    while (offspring!=NULL){                            /*While children exist in the (sub)tree*/
    
        DeleteTree(&(offspring->item));                 /*Reccursively call the function for each child node*/

        temp=offspring;                                 /*The classic routine to delete the childnode position , since we have a list of children*/

        offspring=offspring->next;

        free(temp);
    
    } 

    free(ptr->path);

    free(ptr->name);                                   /*Free the name which is dynamically allocated*/

    free(ptr);                                          /*Free the tree node*/

}

void Delete(Tree *Root, inode **inodelist ){                                         /*Function in charge of i node and tree deletion*/

    inode *temp;

    names *temp2;

    while((*inodelist)!=NULL){                                                         /*While nodes exist in the inode list*/

        while((*inodelist)->ListOfNamesLinked!=NULL){                                  /*Search the List of names linked*/

            temp2=(*inodelist)->ListOfNamesLinked;                                     /*Store the node in a temporary variable*/

            (*inodelist)->ListOfNamesLinked=(*inodelist)->ListOfNamesLinked->next;        /*Move on to the next node*/

            free(temp2);                                                            /*Free the previous node that was stored in the temp variable*/

        }

        temp=(*inodelist);                                                                 /*Store in temporary variable the inodelist node*/

        (*inodelist)=(*inodelist)->nextinlist;                                                /*Move on to the next*/

        free(temp);                                                                     /*Free the previous node stored in the temporary value*/

    }
    (*inodelist)=NULL;

    if(*Root!=NULL)

        DeleteTree(Root);                                                               /*Call the routine that removes the tree*/

}

void InodePrint(inode **inodelist){                                                 /*Function that prints the i node*/

    inode *ptr=(*inodelist);
    
    while(ptr!=NULL){                                                                   /*While the node of the list exists print the wanted contents*/


        printf("%s , %ld , %s\n",ptr->ListOfNamesLinked->contents,ptr->size,ptr->ModifiedTime);
        ptr=ptr->nextinlist;

    }

}
void Myrm(Tree *Father,Tree *Root,char *Path,inode **inodelist){                    /*This function using the DeleteNode whipes whole directories one by one*/

    char *WholePath;
    
    children Child;

    children temp;

    Child=(*Root)->list;

    while(Child!=NULL){                                                             /*For every child*/

        temp=Child;                                                         /*Save the pointer of the child because deletes change the lists form*/

        Child=Child->next;

        WholePath=(char *)malloc(sizeof(char)*(strlen(Path)+6+strlen(temp->item->name)));           /*Dynamically allocate the path to the next child*/

        strcpy(WholePath,Path);

        strcat(WholePath,"/");

        strcat(WholePath,temp->item->name);

        if(isDirectory(WholePath)){

            Myrm(Root,&(temp->item),WholePath,inodelist);                           /*If it is a directory call rm for the current directory so we delete all its contents*/

        }
        else{

            unlink(WholePath);                                                     /*Else Delete the file */

            DeleteNode(Root,(temp)->item->name,inodelist);                          /*And remove it from the TreeNode*/

        }

        free(WholePath);                                        /*Free the temporary path we used to delete the child directory*/

    }

    rmdir(Path);                                                        /*We delete the root folder*/

    DeleteNode(Father,(*Root)->name,inodelist);                         /*We remove it from the tree and inode*/

}

void DirectoryDeletion(Tree *Origin,Tree *Backup,char *Path, inode **inodelist){           /*This is the Synchronization function that Deletes Directories*/
    
    int flag=1;

    char *Proxy;
    
    Tree tempOrigin;
    
    Tree tempBackup;
    
    children OList;
    
    children BList;

    children TempChild;

    char *DestinationPath;

    tempOrigin=(*Origin);

    tempBackup=(*Backup);

    OList=tempOrigin->list;
    
    BList=tempBackup->list;

    while(BList!=NULL){                                                     /*For each subfolder or subfile of the backup folder in the TreeNode*/

        while(OList!=NULL){                                                 /*For each subfolder or subfile in the original folder in the TReeNode*/
            

            if(Path!=NULL){                                                 /*Make dynamically a temporary path to the currently inspecting folder or file Without the roor folder in it*/

                Proxy=(char *)malloc(sizeof(char)*(strlen(BList->item->name)+2+strlen(Path)));

                strcpy(Proxy,Path);
            
            }

            else{

                Proxy=(char *)malloc(sizeof(char)*(strlen((BList)->item->name)+3));

                strcpy(Proxy,"");

            }


            strcat(Proxy,(BList)->item->name);

            strcat(Proxy,"/");

            DestinationPath=(char *)malloc(sizeof(char)*(strlen(Proxy)+strlen(DestFileName)+2));            /*Make the temporary destination path which is the path from the backup folder to the curently inspecting item*/
                        
            strcpy(DestinationPath,DestFileName);
            
            strcat(DestinationPath,"/");
            
            strcat(DestinationPath,Proxy);

            if(strcmp(BList->item->name,OList->item->name)==0){                                     /*If the file or directory exists in both Backup Folder and Origin Folder*/

                if(isDirectory(DestinationPath)){                                                               /*If it is a directory*/

                    DirectoryDeletion(&(OList->item),&(BList->item),Proxy,inodelist);                       /*Inspect it*/

                }

                flag=0;                                                                                     /*Flag that determines if the curren Backup file or directory exists*/
                
            }

            free(DestinationPath);                                                                      /*Free the temporary paths*/

            free(Proxy);

            OList=OList->next;

        }

        if(flag==1 && BList!=NULL){                                                             /*If the flag is 1 , its means that the Directory or file of the backup doesnt exist so it need to be deleted*/

            if(Path!=NULL){                                                                         /*In the same way we make a temporary path without the Root folder in it*/

                Proxy=(char *)malloc(sizeof(char)*(2+strlen(Path)+strlen((BList)->item->name)));

                strcpy(Proxy,Path);
            
            }

            else{

                Proxy=(char *)malloc(sizeof(char)*(strlen((BList)->item->name)+2));

                strcpy(Proxy,"");

            }

            strcat(Proxy,(BList)->item->name);

            DestinationPath=(char *)malloc(sizeof(char)*(strlen(Proxy)+strlen(DestFileName)+5));                       /*In the DestinationPath we make the full temporary path towards the inspected file or directory*/

            strcpy(DestinationPath,".");

            strcat(DestinationPath,"/");
            
            strcat(DestinationPath,DestFileName);
            
            strcat(DestinationPath,"/");
            
            strcat(DestinationPath,Proxy);

            TempChild=BList;

            BList=BList->next;

            if(isDirectory(DestinationPath))                                                   /*If it is a directory , whipe it*/
            
                Myrm(Backup,&(TempChild->item),DestinationPath,inodelist);
            
            else{                                                                           /*If it is a file , unlink it*/

                unlink(DestinationPath);

                DeleteNode(Backup,TempChild->item->name,inodelist);
            
            }

            free(Proxy);                                                                /*Freeing the temporary paths*/

            free(DestinationPath);


        }
        else{

            BList=BList->next;                                                  /*On to the next child*/

        }

        flag=1;

        OList=tempOrigin->list;

    
    }

}

int FileCopy(char *Original, char *Backup, int BUFFSIZE){                               /*Function crreated based on the function given in the lessons slides*/

    int  infile , outfile;
    ssize_t  nread;
    char  buffer[BUFFSIZE ];

    if ( (infile=open(Original ,O_RDONLY)) ==  -1 )
        return (-1);

    if ( (outfile=open(Backup, O_WRONLY|O_CREAT|O_TRUNC , PERM)) ==  -1){
        close(infile);
        return (-2);
    }

    while ( (nread=read(infile , buffer , BUFFSIZE) ) > 0 ){
        if ( write(outfile ,buffer ,nread) < nread ){
            close(infile); close(outfile); return (-3);
        }
    }

    close(infile); close(outfile);
    
    if (nread  ==  -1 ) 
        return (-4);
    else     
        return (0);
}

void DirectorySychronization(Tree *Origin,Tree *Backup,char *Path, inode **inodelist){              /*Function very similar to the deletion that creates files*/
    
    int flag=1,temp;

    char *Proxy;

    char *Wholepath;

    char *CreationPath;
    
    Tree tempOrigin;
    
    Tree tempBackup;

    Tree temp2;
    
    children OList;
    
    children BList;

    char *DestinationPath;

    tempOrigin=(*Origin);

    tempBackup=(*Backup);

    OList=tempOrigin->list;
    BList=tempBackup->list;

    children Childtemp;

    while(OList!=NULL){                                 /*For every child of Origin folder*/
        while(BList!=NULL){                                                 /*For every child of the backup folder*/
            if(Path!=NULL){                                             /*We make the path without the head folder*/
                Proxy=(char *)malloc(sizeof(char)*(strlen(OList->item->name)+2+strlen(Path)));
                strcpy(Proxy,Path);
            }
            else{
                Proxy=(char *)malloc(sizeof(char)*(strlen((OList)->item->name)+3));
                strcpy(Proxy,"");
            }

            strcat(Proxy,(OList)->item->name);
            strcat(Proxy,"/");

            Wholepath=(char *)malloc(sizeof(char)*(strlen(Proxy)+strlen(OriginalFileName)+2));          /*This is the path to the inspected item from the Original Folder*/
            strcpy(Wholepath,OriginalFileName);
            strcat(Wholepath,"/");
            strcat(Wholepath,Proxy);

            DestinationPath=(char *)malloc(sizeof(char)*(strlen(Proxy)+strlen(DestFileName)+2));        /*This is the path to the inspected item from the backup folder*/
            strcpy(DestinationPath,DestFileName);
            strcat(DestinationPath,"/");
            strcat(DestinationPath,Proxy);

            if(strcmp(BList->item->name,OList->item->name)==0){                                 /*We compare their names*/
                if(isDirectory(Wholepath)){                                                 /*If it is the same and it is a directory inspect it*/
                    DirectorySychronization(&(OList->item),&(BList->item),Proxy,inodelist);
                    flag=0;
                }
                else{                                                   /*If it is a file*/
                    if(BList->item->inodeptr->ModifiedTime==OList->item->inodeptr->ModifiedTime && BList->item->inodeptr->size==OList->item->inodeptr->size){/*If they have the same size and modified date then they are copies of each other*/
                        OList->item->inodeptr->copy=BList->item->inodeptr;          /*So the inode original should be pointing to the copy's i node*/
                        flag=0;
                    }
                    else{
                        Childtemp=BList;                                                /*Else we unlink the file and we unlink it.Because Delete Node changes the list of children , we use a temporary poinmter instead and we move on to the next child prematurely*/
                        BList=BList->next;
                        DeleteNode(Backup,Childtemp->item->name,inodelist);
                        unlink(DestinationPath);
                        temp=flag;
                        flag=3;
                    }

                }
            }

            free(DestinationPath);              /*Free the temporary paths and move on*/
            free(Proxy);
            free(Wholepath);
            if(flag!=3)
                BList=BList->next;
            else
                flag=temp;

        }

        if((flag==3 || flag==1) && OList!=NULL){                            /*If the flag is 1 that means that the Original Subfolder or subfile we are inspecting  doesnt exist , so we need to make it*/ 

            if(Path!=NULL){                                                         /*Making our temporary paths*/

                Proxy=(char *)malloc(sizeof(char)*(strlen((OList)->item->name)+2+strlen(Path)));

                strcpy(Proxy,Path);
            
            }

            else{
                

                Proxy=(char *)malloc(sizeof(char)*(strlen((OList)->item->name)+2));

                strcpy(Proxy,"");

            }

            strcat(Proxy,(OList)->item->name);
            Wholepath=(char *)malloc(sizeof(char)*(strlen(Proxy)+strlen(OriginalFileName)+2));
            strcpy(Wholepath,OriginalFileName);
            strcat(Wholepath,"/");
            strcat(Wholepath,Proxy);
            free(Proxy);

            if(Path!=NULL){
                Proxy=(char *)malloc(sizeof(char)*(2+strlen(Path)+strlen((OList)->item->name)));
                strcpy(Proxy,Path);
            }
            else{
                Proxy=(char *)malloc(sizeof(char)*(strlen((OList)->item->name)+2));
                strcpy(Proxy,"");
            }
            strcat(Proxy,(OList)->item->name);
            CreationPath=(char *)malloc(sizeof(char)*(strlen(Proxy)+strlen(DestFileName)+4));

            strcpy(CreationPath,".");
            strcat(CreationPath,"/");
            strcat(CreationPath,DestFileName);
            strcat(CreationPath,"/");
            strcat(CreationPath,Proxy);

            if(isDirectory(Wholepath))                                              /*If we want to make a directory we  do so with mkdir*/
                printf("%d\n",mkdir(CreationPath,0777));
            else{

                if(OList->item->inodeptr->copy!=NULL){                              /* If there exists a copy */

                    Tree Brother;

                    Brother=FindById(HeadBackup,OList->item->inodeptr->copy->id);/*Find the file that already is a copy of it*/

                    link(Brother->path,CreationPath);                                       /*And make a new one hard linked to it*/

                    flag=3;

                }
                else{                                                                       /*Else just make a copy*/
                    FileCopy(Wholepath,CreationPath,1);
                    flag=6;
                }
            }
            temp2=AddNode(Backup,OList->item->name,inodelist,CreationPath);                         /*Either way a new treenode is made*/
            if(flag==6){                                                                                /*If it is a copy make the original node show to the copy*/
                if(!temp2->inodeptr)
                    OList->item->inodeptr->copy=temp2->inodeptr;
            }

            strcat(Proxy,"/");
            DirectorySychronization(&(OList->item),&temp2,Proxy,inodelist);
            free(Proxy);
            free(Wholepath);
            free(CreationPath);
        }

        flag=1;

        BList=tempBackup->list;
    
        OList=OList->next;
    
    }

}
