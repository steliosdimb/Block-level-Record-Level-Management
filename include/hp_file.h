#ifndef HP_FILE_H
#define HP_FILE_H
#define HP_ERROR -1
#include <record.h>

/* Η δομή HP_info κρατάει μεταδεδομένα που σχετίζονται με το αρχείο σωρού*/
typedef struct
{
    int file_desc;             // file desc
    int last_block_id;         // id of last block
    int block_record_capacity; // number of records that can be stored in each block
} HP_info;

// domh hp_block_info krataei metadedomena pou sxetizontai me to kathe block
typedef struct
{
    int records_in_block; // number of records in this block
    BF_Block *next_block; // pointer to next block
} HP_block_info;

/*Η συνάρτηση HP_CreateFile χρησιμοποιείται για τη δημιουργία και
κατάλληλη αρχικοποίηση ενός άδειου αρχείου σωρού με όνομα fileName.
Σε περίπτωση που εκτελεστεί επιτυχώς, επιστρέφεται 0, ενώ σε
διαφορετική περίπτωση -1.*/
int HP_CreateFile(
    char *fileName /*όνομα αρχείου*/);

/* Η συνάρτηση HP_OpenFile ανοίγει το αρχείο με όνομα filename και
διαβάζει από το πρώτο μπλοκ την πληροφορία που αφορά το αρχείο σωρού.
Κατόπιν, ενημερώνεται μια δομή που κρατάτε όσες πληροφορίες κρίνονται
αναγκαίες για το αρχείο αυτό προκειμένου να μπορείτε να επεξεργαστείτε
στη συνέχεια τις εγγραφές του. Η μεταβλητή *file_desc αναφέρεται στο
αναγνωριστικό ανοίγματος του συγκεκριμένου αρχείου όπως προκύπτει από την
BF_OpenFile.
*/
HP_info *HP_OpenFile(char *fileName, /* όνομα αρχείου */
                     int *file_desc  /* προσδιοριστικό ανοίγματος αρχείου όπως δίνεται από την BF_OpenFile */
);

/* Η συνάρτηση HP_CloseFile κλείνει το αρχείο που προσδιορίζεται
από το αναγνωριστικό file_desc. Σε περίπτωση που εκτελεστεί επιτυχώς,
επιστρέφεται 0, ενώ σε διαφορετική περίπτωση -1. Η συνάρτηση είναι
υπεύθυνη και για την αποδέσμευση της μνήμης που καταλαμβάνει η δομή
που περάστηκε ως παράμετρος, στην περίπτωση που το κλείσιμο
πραγματοποιήθηκε επιτυχώς.
*/
int HP_CloseFile(int file_desc,       /* προσδιοριστικό ανοίγματος αρχείου όπως δίνεται από την BF_OpenFile */
                 HP_info *header_info /* η κεφαλή που περιέχει τα δεδομένα του αρχείου */
);

/* Η συνάρτηση HP_InsertEntry χρησιμοποιείται για την εισαγωγή μιας
εγγραφής στο αρχείο σωρού. Το αναγνωριστικό για το αρχείο είναι file_desc,
τα μεταδεδομένα του αρχείου βρίσκονται στη δομή header_info, ενώ η εγγραφή προς εισαγωγή
προσδιορίζεται από τη δομή record. Σε περίπτωση που εκτελεστεί
επιτυχώς, επιστρέφετε τον αριθμό του block στο οποίο έγινε η εισαγωγή
(blockId) , ενώ σε διαφορετική περίπτωση -1.
*/
int HP_InsertEntry(
    int file_desc,
    HP_info *header_info, /* επικεφαλίδα του αρχείου*/
    Record record /* δομή που προσδιορίζει την εγγραφή */);

/*Η συνάρτηση αυτή χρησιμοποιείται για την εκτύπωση όλων των εγγραφών
που υπάρχουν στο αρχείο σωρού οι οποίες έχουν τιμή στο
πεδίο-κλειδί ίση με value. Η πρώτη δομή δίνει πληροφορία για το αρχείο
σωρού, όπως αυτή είχε επιστραφεί από την HP_OpenFile.
Για κάθε εγγραφή που υπάρχει στο αρχείο και έχει τιμή στο πεδίο id
ίση με value, εκτυπώνονται τα περιεχόμενά της (συμπεριλαμβανομένου
και του πεδίου-κλειδιού). Να επιστρέφεται επίσης το πλήθος των blocks που
διαβάστηκαν μέχρι να βρεθούν όλες οι εγγραφές. Σε περίπτωση επιτυχίας
επιστρέφει το πλήθος των blocks που διαβάστηκαν, ενώ σε περίπτωση λάθους επιστρέφει -1.
*/
int HP_GetAllEntries(
    int file_desc,
    HP_info *header_info, /* επικεφαλίδα του αρχείου*/
    int id /* η τιμή id της εγγραφής στην οποία πραγματοποιείται η αναζήτηση*/);

// it iterates through the block and returns a pointer to the section of the metadata

HP_block_info *find_metadata(HP_info *hp_info, HP_block_info *hp_block_info);

// inserts record into the desired block
// if it returns 0 the block can store the record else it returns -1

int insert_record(HP_info *hp_info, HP_block_info *hp_block_info, void *block, Record record);

#endif // HP_FILE_H
