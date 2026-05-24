#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void archive_files(int argc, char *argv[]);
void extract_archive(int argc, char *argv[]);

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Hata: Eksik parametre.\nKullanim: ./tarsau -b [dosyalar...] -o [arsiv.sau] VEYA ./tarsau -a [arsiv.sau] [dizin]\n");
        return 1;
    }

    if (strcmp(argv[1], "-b") == 0) {
        archive_files(argc, argv);
    } else if (strcmp(argv[1], "-a") == 0) {
        extract_archive(argc, argv);
    } else {
        fprintf(stderr, "Hata: Gecersiz parametre. -b veya -a kullanin.\n");
        return 1;
    }
    return 0;
}

void archive_files(int argc, char *argv[]) {
    printf("Arsivleme islemi baslatildi...\n");
    // -b logic will go here
}

void extract_archive(int argc, char *argv[]) {
    printf("Cikartma islemi baslatildi...\n");
    // -a logic will go here
}