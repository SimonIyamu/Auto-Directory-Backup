typedef struct ListNodeTag{ /* A list that maps a WD to a file's path */
    int key;
    char *data; /* Relative path */
    struct ListNodeTag *next;
} ListNode;
