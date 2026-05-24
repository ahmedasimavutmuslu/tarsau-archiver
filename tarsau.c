#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_FILES 32
#define MAX_TOTAL_SIZE (200 * 1024 * 1024) // 200 MB

// Function prototypes
void archive_files(int argc, char *argv[]);
void extract_archive(int argc, char *argv[]);
int is_text_file(const char *filename);

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
        fprintf(stderr, "Hata: Gecersiz parametre.\n");
        return 1;
    }

    return 0;
}

// Helper function to check if a file is purely ASCII text
int is_text_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return 0;
    
    int ch;
    int count = 0;
    // Read up to the first 1000 bytes to verify ASCII compliance
    while ((ch = fgetc(file)) != EOF && count < 1000) {
        if (ch > 127) { // Standard ASCII characters are 0-127 
            fclose(file);
            return 0; // Contains non-ASCII bytes
        }
        count++;
    }
    fclose(file);
    return 1;
}

void archive_files(int argc, char *argv[]) {
    char *output_filename = "a.sau"; // Varsayilan arsiv adi [cite: 17]
    char *input_files[MAX_FILES];
    int file_count = 0;
    long long total_size = 0;

    // Ayrıştırma işlemi: Hangi argüman dosya, hangisi -o parametresi?
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                output_filename = argv[i + 1];
                i++; // -o'dan sonraki argümanı atla (çünkü o dosya adı) [cite: 16]
            }
        } else {
            if (file_count >= MAX_FILES) {
                fprintf(stderr, "Hata: En fazla 32 dosya girilebilir.\n"); // 
                exit(1);
            }
            input_files[file_count++] = argv[i];
        }
    }

    if (file_count == 0) {
        fprintf(stderr, "Hata: Arsivlenecek dosya belirtilmedi.\n");
        exit(1);
    }

    // Dosyaların format ve boyut doğrulamasını yap
    for (int i = 0; i < file_count; i++) {
        struct stat st;
        if (stat(input_files[i], &st) != 0) {
            fprintf(stderr, "Hata: %s dosyasi bulunamadi veya erisilemiyor.\n", input_files[i]);
            exit(1);
        }

        if (!is_text_file(input_files[i])) {
            // İstenen tam hata mesajı ve sorunsuz çıkış 
            printf("%s giriş dosyasının formatı uyumsuzdur!\n", input_files[i]);
            exit(0); 
        }

        total_size += st.st_size;
    }

    if (total_size > MAX_TOTAL_SIZE) {
        fprintf(stderr, "Hata: Toplam dosya boyutu 200 MB'i gecemez.\n"); // 
        exit(1);
    }

    printf("Basarili: %d adet metin dosyasi (Toplam %lld byte) '%s' icine arsivlenecek.\n", file_count, total_size, output_filename);
    
    // TODO: Organizasyon bolumunu ve dosya iceriklerini .sau dosyasina yaz
}

void extract_archive(int argc, char *argv[]) {
    printf("Cikartma islemi hazirlaniyor...\n");
}