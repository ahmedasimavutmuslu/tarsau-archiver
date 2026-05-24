#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_FILES 32
#define MAX_TOTAL_SIZE (200 * 1024 * 1024) // 200 MB

// Fonksiyon prototipleri
void archive_files(int argc, char *argv[]);
void extract_archive(int argc, char *argv[]);
int is_text_file(const char *filename);

int main(int argc, char *argv[]){
    if (argc < 3) {
        fprintf(stderr, "Hata: Eksik parametre.\nKullanim: ./tarsau -b [dosyalar...] -o [arsiv.sau] VEYA ./tarsau -a [arsiv.sau] [dizin]\n");
        return 1;
    }

    if (strcmp(argv[1], "-b") == 0){
        archive_files(argc, argv);
    }
    else if (strcmp(argv[1], "-a") == 0){
        extract_archive(argc, argv);
    } 
    else{
        fprintf(stderr, "Hata: Gecersiz parametre.\n");
        return 1;
    }

    return 0;
}

// Dosyanin tamamen ascii karakter icerip icermedigini kontrol eden yardimci fonksiyon
int is_text_file(const char *filename){
    FILE *file = fopen(filename, "r");
    if (!file) return 0;
    
    int ch;
    int count = 0;
    // Kontrol icin ilk 1000 bytei oku
    while ((ch = fgetc(file)) != EOF && count < 1000){
        if (ch > 127){ // standart ascii karakterleri 0-127 
            fclose(file);
            return 0; // ascii olmayan
        }
        count++;
    }
    fclose(file);
    return 1;
}

void archive_files(int argc, char *argv[]){
    char *output_filename = "a.sau"; // Varsayilan arsiv adi 
    char *input_files[MAX_FILES];
    int file_count = 0;
    long long total_size = 0;

    // Ayristirma islemleri
    for (int i = 2; i < argc; i++){
        if (strcmp(argv[i], "-o") == 0){
            if (i + 1 < argc){
                output_filename = argv[i + 1];
                i++; // -o'dan sonrakini atla (dosya adi)
            }
        } 
        else{
            if (file_count >= MAX_FILES){
                fprintf(stderr, "Hata: En fazla 32 dosya girilebilir.\n");
                exit(1);
            }
            input_files[file_count++] = argv[i];
        }
    }

    if (file_count == 0){
        fprintf(stderr, "Hata: Arsivlenecek dosya belirtilmedi.\n");
        exit(1);
    }

    // Dosyaların format ve boyut dogrulamasi
    for (int i = 0; i < file_count; i++){
        struct stat st;
        if (stat(input_files[i], &st) != 0){
            fprintf(stderr, "Hata: %s dosyasi bulunamadi veya erisilemiyor.\n", input_files[i]);
            exit(1);
        }

        if (!is_text_file(input_files[i])){
            // Istenen hata mesaji ve cikis
            printf("%s giris dosyasinin formati uygunsuzdur!\n", input_files[i]);
            exit(0); 
        }

        total_size += st.st_size;
    }

    if (total_size > MAX_TOTAL_SIZE){
        fprintf(stderr, "Hata: Toplam dosya boyutu 200 MB'i gecemez.\n"); // 
        exit(1);
    }

    printf("Basarili: %d adet metin dosyasi (Toplam %lld byte) '%s' icine arsivlenecek.\n", file_count, total_size, output_filename);
    
    // Asil arsivleme islemi
    FILE *out_file = fopen(output_filename, "w");
    if (!out_file) {
        fprintf(stderr, "Hata: Arsiv dosyasi olusturulamadi.\n");
        exit(1);
    }

    // Header (Baslik) kisminin ayarlanmasi
    char header_buffer[8192] = {0}; 
    
    for (int i = 0; i < file_count; i++){
        struct stat st;
        stat(input_files[i], &st);
        
        char temp_record[256];
        // Dosya adi, izinler (octal formatta), ve boyut
        // st_mode & 0777 islemi sadece okuma/yazma/calistirma izinlerini alir 
        sprintf(temp_record, "|%s,%o,%ld|", input_files[i], st.st_mode & 0777, (long)st.st_size);
        strcat(header_buffer, temp_record);
    }

    // Basligin toplam boyutunu hesapla ve ilk 10 byte'a yaz
    long header_size = strlen(header_buffer);
    // %010ld formati, sayiyi 10 karaktere tamamlayacak sekilde basina sifir ekler
    fprintf(out_file, "%010ld", header_size);

    // Baslik kismini dosyaya yaz
    fprintf(out_file, "%s", header_buffer);

    // Dosya iceriklerini araliksiz olarak sonuna ekle
    for (int i = 0; i < file_count; i++){
        FILE *in_file = fopen(input_files[i], "r");
        if (!in_file){
            fprintf(stderr, "Uyari: %s dosyasi okunurken atlandi.\n", input_files[i]);
            continue;
        }

        char buffer[4096];
        size_t bytes_read;
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), in_file)) > 0){
            fwrite(buffer, 1, bytes_read, out_file);
        }
        fclose(in_file);
    }

    fclose(out_file);
    printf("Arsivleme tamamlandi. Cikti dosyasi: %s\n", output_filename);
}

// Cikarma islemi
void extract_archive(int argc, char *argv[]){
    if (argc < 3){
        fprintf(stderr, "Hata: Cikartilacak arsiv dosyasi belirtilmedi.\n");
        exit(1);
    }

    char *archive_name = argv[2];
    char *target_dir = (argc > 3) ? argv[3] : NULL;

    FILE *arch_file = fopen(archive_name, "r");
    if (!arch_file){
        printf("Arsiv dosyasi uygunsuz veya bozuk!\n");
        exit(0);
    }

    // Hedef dizin belirtilmisse kontrol et ve olustur
    if (target_dir) {
        struct stat st = {0};
        if (stat(target_dir, &st) == -1) {
            // Dizin yoksa olustur (varsayilan izinler 0777)
            if (mkdir(target_dir, 0777) != 0) {
                fprintf(stderr, "Hata: '%s' dizini olusturulamadi.\n", target_dir);
                exit(1);
            }
        }
    }

    // Ilk 10 byte'i oku (Header kismi)
    char size_buf[11] = {0};
    if (fread(size_buf, 1, 10, arch_file) != 10){
        printf("Arsiv dosyasi uygunsuz veya bozuk!\n");
        fclose(arch_file);
        exit(0);
    }

    long header_size = atol(size_buf);
    if (header_size <= 0) {
        printf("Arsiv dosyasi uygunsuz veya bozuk!\n");
        fclose(arch_file);
        exit(0);
    }

    char *header = malloc(header_size + 1);
    fread(header, 1, header_size, arch_file);
    header[header_size] = '\0';

    // Metadata'yi parse et ve dosyalari cikart
    // strsep veya sscanf kullanarak |dosya,izin,boyut| formatini ayiriyoruz
    char *ptr = header;
    while (*ptr != '\0'){
        if (*ptr == '|'){
            ptr++;
            char filename[256] = {0};
            int permissions;
            long filesize;

            // Virgul ve pipe karakterlerine gore oku
            if (sscanf(ptr, "%255[^,],%o,%ld|", filename, &permissions, &filesize) == 3){
                
                // Tam dosya yolunu hazirla (dizin varsa)
                char full_path[512] = {0};
                if (target_dir) {
                    sprintf(full_path, "%s/%s", target_dir, filename);
                } 
                else{
                    strcpy(full_path, filename);
                }

                // Dosya icerigini oku ve yaz
                FILE *out_file = fopen(full_path, "w");
                if (out_file){
                    char *file_data = malloc(filesize);
                    
                    // fread ile siradaki veriyi oku, fwrite ile disari aktar
                    fread(file_data, 1, filesize, arch_file);
                    fwrite(file_data, 1, filesize, out_file);
                    
                    fclose(out_file);
                    free(file_data);

                    // Orjinal izinleri geri yukle
                    chmod(full_path, permissions);
                }
                
                // Pointer'i bir sonraki kayda tasi
                ptr = strchr(ptr, '|');
                if (ptr) ptr++; 
            } 
            else{
                break; // Format bozuksa donguyu kir
            }
        } 
        else{
            ptr++;
        }
    }

    free(header);
    fclose(arch_file);
    
    // basari mesaji
    if (target_dir) {
        printf("%s dizininde dosyalar acildi.\n", target_dir);
    } else {
        printf("Dosyalar bulundugunuz dizinde acildi.\n");
    }
}