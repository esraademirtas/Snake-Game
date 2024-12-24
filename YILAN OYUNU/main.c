#include <stdio.h>
#include <stdlib.h>
#include <conio.h> // _kbhit() ve _getch() gibi klavye kontrol fonksiyonları için gerekli.
#include <windows.h> // Sleep(), SetConsoleCursorPosition() ve GetTickCount() gibifonksiyonlar için gerekli.
#include <string.h>


#define GENISLIK 40
#define YUKSEKLIK 20

// İkili arama agacİ için dügüm yapİSİ
typedef struct AgacDugumu {
    int x;//koordinatlar
    int y;
    char yemek;
    struct AgacDugumu* sol; //alt agaçlar
    struct AgacDugumu* sag;
} AgacDugumu;

AgacDugumu* kok = NULL;

// Yilanin dügüm yapisi
typedef struct Dugum {
    int x;
    int y;
    struct Dugum* sonraki;
} Dugum;

Dugum* bas = NULL;
Dugum* kuyruk = NULL;
int yemekX, yemekY;
char yemekKarakteri = 'A';
int puan = 0;
int oyunBitti = 0;
int yon = 'd';
unsigned long sonHareketZamani = 0;
int hareketAraligi = 100;
char oyuncuAdi[50];
char yenilenYemekler[26];//Yemek karakterleri
int yemekSayisi = 0;

// Yemekleri ikili arama agacina ekleyen fonksiyon
void agacaEkle(int x, int y, char yemek) {

    AgacDugumu* yeniDugum = (AgacDugumu*)malloc(sizeof(AgacDugumu));
    yeniDugum->x = x; //koordinatlarını atadım
    yeniDugum->y = y;
    yeniDugum->yemek = yemek;
    yeniDugum->sol = yeniDugum->sag = NULL;

    if (kok == NULL) { // agaç boşsa
        kok = yeniDugum;
        return;
    }

    AgacDugumu* mevcut = kok; // Mevcut dügüm.
    AgacDugumu* ebeveyn = NULL; // Mevcut dügümün ebeveyni.

    //agacı dolaşacak
    while (mevcut != NULL) {
        ebeveyn = mevcut;
        if (x < mevcut->x || (x == mevcut->x && y < mevcut->y)) {
            mevcut = mevcut->sol;
        } else {
            mevcut = mevcut->sag;
        }
    }

    // Yeni dügümü ebeveynin uygun alt dügümüne ekleyecek
    if (x < ebeveyn->x || (x == ebeveyn->x && y < ebeveyn->y)) {
        ebeveyn->sol = yeniDugum;
    } else {
        ebeveyn->sag = yeniDugum;
    }
}


void agaciYazdir(AgacDugumu* dugum) {
    if (dugum == NULL) return;
    agaciYazdir(dugum->sol); //sol alt agacı yazdırır
    printf("Yemek: %c, Koordinatlar: (%d, %d)\n", dugum->yemek, dugum->x, dugum->y);
    agaciYazdir(dugum->sag); //Recursive yapı
}


void oyunuBaslat() {
    printf("\t\t\t\t**************** HOSGELDINIZ ******************\n\n");
    printf("Adinizi girin: ");
    fgets(oyuncuAdi, sizeof(oyuncuAdi), stdin);
    oyuncuAdi[strcspn(oyuncuAdi, "\n")] = 0; // Sondaki yeni satir karakterini kaldir.

    printf("Zorluk secin (1 - Kolay, 2 - Orta, 3 - Zor): ");
    int zorluk;
    scanf("%d", &zorluk);

    // Zorluk seviyesine göre hareket hizini ayarlar.
    switch (zorluk) {
        case 1: hareketAraligi = 200; break; // Kolay.
        case 2: hareketAraligi = 100; break; // Orta.
        case 3: hareketAraligi = 50; break; // Zor.
        default: hareketAraligi = 100;
    }

    // yilanın başını oluşturur
    bas = (Dugum*)malloc(sizeof(Dugum));
    bas->x = GENISLIK / 2; //alanın ortasında olsun
    bas->y = YUKSEKLIK / 2;
    bas->sonraki = NULL;
    kuyruk = bas;

    // ilk yemeğin koordinatları
    yemekX = rand() % GENISLIK;
    yemekY = rand() % YUKSEKLIK;
}

void cerceveCiz() {
    system("cls");

    // Üst cerceve
    for (int i = 0; i < GENISLIK + 2; i++) printf("#");
    printf("\n");

    // Yan cerceve ve cerceve ici
    for (int i = 0; i < YUKSEKLIK; i++) {
        printf("#"); //sol.
        for (int j = 0; j < GENISLIK; j++) {
            printf(" "); //boşluklar
        }
        printf("#\n"); //sag
    }

    // Alt cerceve
    for (int i = 0; i < GENISLIK + 2; i++) printf("#");
    printf("\n");

    // Oyuncu bilgileri.
    printf("Oyuncu: %s\nPuan: %d\n", oyuncuAdi, puan);
}


void icerikCiz() {
    for (int i = 0; i < YUKSEKLIK; i++) {
        for (int j = 0; j < GENISLIK; j++) {
            COORD pos = {j + 1, i + 1}; // Konsolda çizim pozisyonunu belirler
            SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);

            if (i == bas->y && j == bas->x) {
                printf("*"); // yilanın başı
            } else if (i == yemekY && j == yemekX) {
                printf("%c", yemekKarakteri); // Yemek karakteri.
            } else {
                Dugum* mevcut = bas->sonraki; // yilanın diğer düğümleri
                int bulundu = 0; // Yilan düğümü bulunup bulunmadiğini kontrol eder.
                while (mevcut != NULL) {
                    if (mevcut->x == j && mevcut->y == i) {
                        printf("*");
                        bulundu = 1;
                        break;
                    }
                    mevcut = mevcut->sonraki;
                }
                if (!bulundu) {
                    printf(" "); //boş alan
                }
            }
        }
    }

    // Skor bilgisini yazdirir
    COORD puanPozisyonu = {0, YUKSEKLIK + 3};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), puanPozisyonu);
    printf("PUAN: %d", puan);
}

//
void carpismaKontrol() {
    Dugum* mevcut = bas->sonraki; // y,lanın başından sonrasını kontrol eder.
    while (mevcut != NULL) {
        if (mevcut->x == bas->x && mevcut->y == bas->y) { //kendine çarptiği senaryo
            oyunBitti = 1;
            break;
        }
        mevcut = mevcut->sonraki;
    }
}


void guncelle() {
    int yeniX = bas->x;
    int yeniY = bas->y;

    if (_kbhit()) { // Klavyeden tuşa basılıp basılmadığı
        char tus = _getch();
        if (tus == 'w' && yon != 's') yon = 'w'; // Yukari
        else if (tus == 's' && yon != 'w') yon = 's'; // Asagi
        else if (tus == 'a' && yon != 'd') yon = 'a'; // Sola
        else if (tus == 'd' && yon != 'a') yon = 'd'; // Saga
        else if (tus == 'x') oyunBitti = 1; //çıkış
    }

    // Hareket yönüne göre yeni koordinatlaribelirler
    switch (yon) {
        case 'w': yeniY--; break; // Yukari
        case 's': yeniY++; break; // Asagi
        case 'a': yeniX--; break; // Sola
        case 'd': yeniX++; break; // Saga
    }

    // Eðer yilan çerçeveye çarparsa
    if (yeniX < 0 || yeniX >= GENISLIK || yeniY < 0 || yeniY >= YUKSEKLIK) {
        oyunBitti = 1;
    }

    // Yeni baş düğümü
    Dugum* yeniBas = (Dugum*)malloc(sizeof(Dugum));
    yeniBas->x = yeniX;
    yeniBas->y = yeniY;
    yeniBas->sonraki = bas; // yeni baş düğüm olustur
    bas = yeniBas; // Yeni düğüm artik baş olur.


    carpismaKontrol();

    // Eðer yilanyemek yerse.
    if (bas->x == yemekX && bas->y == yemekY) {
        puan++;
        agacaEkle(yemekX, yemekY, yemekKarakteri); //ikili ağaca ekle
        yenilenYemekler[yemekSayisi++] = yemekKarakteri; // diziye ekle
        yemekKarakteri = (yemekKarakteri == 'Z') ? 'A' : yemekKarakteri + 1;
        yemekX = rand() % GENISLIK; // Yeni yemek koordinatlari
        yemekY = rand() % YUKSEKLIK;
    } else {
        // Yilan yemek yemezse kuyruğu sil.
        Dugum* gecici = bas;
        while (gecici->sonraki->sonraki != NULL) {
            gecici = gecici->sonraki;
        }
        free(gecici->sonraki); // Kuyrugun son dügümünü serbest birak
        gecici->sonraki = NULL; // Kuyruðu bir adim kisalt.
    }
}


int main() {
    oyunuBaslat();
    cerceveCiz();

    while (!oyunBitti) {
        icerikCiz();
        unsigned long simdikiZaman = GetTickCount(); //şu anki zamanı alır
        if (simdikiZaman - sonHareketZamani > hareketAraligi) { // Eğer belirli bir süre geçtiyse.
            guncelle();
            sonHareketZamani = simdikiZaman; // Hareket zamanini güncelle.
        }
        Sleep(10); // Küçük bir gecikme.
    }

    // Oyun bittiğinde
    COORD oyunBittiPozisyonu = {0, YUKSEKLIK + 5};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), oyunBittiPozisyonu);
    printf("\n\n\t\t\t------------------------OYUN BITTI!----------------------------\n\n");
    printf("SON PUAN: %d\n", puan);
    printf("YENILEN YEMEKLER:\n ");
    for (int i = 0; i < yemekSayisi; i++) {
        printf("%c ", yenilenYemekler[i]);
    }
    printf("\n\nYEMEK KOORDINATLARI:\n");
    agaciYazdir(kok);

    return 0;
}
