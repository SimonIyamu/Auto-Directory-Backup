# Auto-Directory-Backup
This application monitors dynamically a hierarchy of files and directories, and when there are changes, it updates a complete copy of the hierarchy in another directory. 

During the initial synchronization, we create structures that reflect the hierarchy of names and i-nodes, as it is in the file management subsystem of the operating system. After the directories are synched, the watching begins. The program watches the events that happen on the source directory, using the inotify system call interface, and then does any neccessary actions on the backup directory in order to keep the directories synched.

Compilation:  make

Execution: ./mirr source backup  
,where source and backup is the name of the source and backup directory respectively.

-----------------
Contributors: Eugene Kladis, Simon Iyamu.  
This project was an assignments in the Operating Systems course in 2018.
