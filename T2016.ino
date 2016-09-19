#include <EEPROM.h>
#include <LiquidCrystal_SR.h>

const byte pClock = 8;
const byte pLatch = 9;
const byte pData  = 10;
const byte lcdBacklight = 11;

const byte tombol = A0;

const byte BTN_SET  = 1;
const byte BTN_UP   = 2;
const byte BTN_DOWN = 3;
const byte BTN_OK   = 4;

const int btn_set_val  = 145;
const int btn_up_val   = 256;
const int btn_down_val = 439;
const int btn_ok_val   = 1023;
const byte btn_toleransi = 10;

byte btn = 0;
byte btn_bef = 0;

class XTimer{
  
  public:

    byte MAX_TIMER; // maksimal timer dalam 1 hari
    byte MAX_LAGU;  // maksimal jumlah lagu
    
    boolean status_hari; // 0 (start_timer)
    byte timer_status[20];   // start_timer + n_timer + 1
    byte timer_jam[20];      // start_timer + n_timer + 2
    byte timer_menit[20];    // start_timer + n_timer + 3
    byte timer_L1[20];       // start_timer + n_timer + 4
    byte timer_L2[20];       // start_timer + n_timer + 5
    byte timer_L3[20];       // start_timer + n_timer + 6 

    byte timer_status_sort[20];
    
    byte jumlah_timer_aktif;

    byte hari;
    int start_ram;
    int target_ram;


    XTimer(byte h){
      MAX_TIMER = 20;
      MAX_LAGU = 100;
      jumlah_timer_aktif = 0;
      
      set_hari(h);
    }

    void check_ram(){
        load_timer_full();
        String logg = "\nHari terpilih:";
        logg.concat(hari);
        logg.concat("\n");
        Serial.print(logg);
        for(byte i=0; i<MAX_TIMER; i++){
          logg = "";
          logg.concat("Timer ");
          logg.concat(i);
          logg.concat(" : ");
          logg.concat(timer_status[i]);
          logg.concat(" Pos:");
          logg.concat(timer_status_sort[i]);
          //logg.concat(" TMP:");
          //logg.concat(timer_tmpx[i]);
          logg.concat("\n");
          Serial.print(logg);
        }
        
    }

    // h = hari, start dari 0
    void set_hari(byte h){
        hari = h;
        start_ram = 100 + (h * 125);
        //Serial.print("START_RAM : ");
        //Serial.println(start_ram);

        load_timer_full();
    }

    void set_status_hari(boolean stat){
        status_hari = stat;
        EEPROM.write(start_ram, status_hari);
    }

    boolean get_status_hari(){
        if(EEPROM.read(start_ram) == 0){
          status_hari = false;
        }else{
          status_hari = true;
        }
        return status_hari;
    }

    // n_timer = timer_terpilih
    void add_timer(byte n_timer){
        byte target = n_timer;
        // jika ada timer yang dipilih
        if(n_timer > 0){
            target = n_timer - 1;

            // update timer data
        }else{
            target = jumlah_timer_aktif + 1;
            for(byte it=0; it<MAX_TIMER; it++){
                if(timer_status[it] == 0){
                  set_timer(it, target);
                  break;
                }
            }
            // update timer data
        }
        load_timer_full();
        //sort_timer_status();
    }

    // n_timer seperti pada target fungsi add_timer
    // n_timer bernilai 1-20, sesuai 
    void del_timer(byte n_timer){
        byte target = timer_status_sort[(n_timer-1)];
        boolean removed  = false;
        for(byte ta=0; ta<jumlah_timer_aktif; ta++){
            if(timer_status_sort[ta] == target){
                if(!removed){
                  removed = true;
                  set_timer(timer_status_sort[ta],0);
                  //jumlah_timer_aktif--;
                }
            }else if(timer_status_sort[ta] > target){
                set_timer(timer_status_sort[ta], (timer_status[timer_status_sort[ta]]-1));
            }
        }
        load_timer_full();
        
        /*for(byte it=0; it<MAX_TIMER; it++){
            if(timer_status[timer_status_sort[it]] == target){
                if(!removed){
                  set_timer(timer_status_sort[it], 0);
                  removed = true;
                }
            }else if(timer_status[timer_status_sort[it]] > target){
                set_timer(it, (timer_status[it]-1));
            }
        }*/
        //sort_timer_status();
    }

    void sort_timer_status(){
   
        // reset order
        byte timer_tmpx[20];
        //byte timer_tmp[20];
        for(byte it=0; it<MAX_TIMER; it++){
            if(timer_status[it] > 0){
              timer_tmpx[it] = timer_status[it];
            }else{
              timer_tmpx[it] = 99;
            }
            timer_status_sort[it] = it;
        }

        // sorting
        byte tmp = 0;
        byte tmpi= 0;
        for(byte i=0; i<MAX_TIMER-1; i++){
            for(byte j=(i+1); j<MAX_TIMER; j++){
                if(timer_tmpx[i] > timer_tmpx[j]){
                  tmp = timer_tmpx[i];
                  timer_tmpx[i] = timer_tmpx[j];
                  timer_tmpx[j] = tmp;

                  tmpi = timer_status_sort[i];
                  timer_status_sort[i] = timer_status_sort[j];
                  timer_status_sort[j] = tmpi;
                }
            }
            
        }
    }

    // n_timer = 0-19, data = 1-20
    // n_timer = RAM!
    void set_timer(byte n_timer, byte data){
        timer_status[n_timer] = data;
        target_ram = start_ram + (n_timer * 6);
        EEPROM.write(target_ram + 1, timer_status[n_timer]);
    }

    void load_timer_full(){
        jumlah_timer_aktif = 0;
        for(byte it=0; it<20; it++){
            load_timer(it);
            // jumlah timer yang aktif
            if(timer_status[it] > 0){
              jumlah_timer_aktif++;
            }
        }
        //Serial.print("HARI : ");
        //Serial.println(hari);

        sort_timer_status();
    }

    // n_timer start dari 0-19
    void load_timer(byte n_timer){
        target_ram = start_ram + (n_timer * 6);
        
        /*Serial.print("RAM_STATUS_HARI : ");
        Serial.println(start_ram);
        Serial.print("Timer ke : ");
        Serial.println(n_timer);
        Serial.print("ram_status_timer: ");
        Serial.println(target_ram + 1);
        Serial.print("ram_jam: ");
        Serial.println(target_ram + 2);
        Serial.print("ram_menit: ");
        Serial.println(target_ram + 3);
        Serial.print("ram_L1: ");
        Serial.println(target_ram + 4);
        Serial.print("ram_L2: ");
        Serial.println(target_ram + 5);
        Serial.print("ram_L3: ");
        Serial.println(target_ram + 6);*/

        //for(byte it=0; it<MAX_TIMER; it++){
            timer_status[n_timer] = EEPROM.read(target_ram + 1);
            timer_jam[n_timer] = EEPROM.read(target_ram + 2);
            timer_menit[n_timer] = EEPROM.read(target_ram + 3);
            timer_L1[n_timer] = EEPROM.read(target_ram + 4);
            timer_L2[n_timer] = EEPROM.read(target_ram + 5);
            timer_L3[n_timer] = EEPROM.read(target_ram + 6);
            
            if(timer_status[n_timer] > 100){
              timer_status[n_timer] = false;
              EEPROM.write(target_ram + 1, timer_status[n_timer]);
            }
            if(timer_jam[n_timer] > 24){
              timer_jam[n_timer] = 0;
              EEPROM.write(target_ram + 2, timer_jam[n_timer]);
            }
            if(timer_menit[n_timer] > 60){
              timer_menit[n_timer] = 0;
              EEPROM.write(target_ram + 3, timer_menit[n_timer]);
            }
            if(timer_L1[n_timer] > MAX_LAGU){
              timer_L1[n_timer] = 0;
              EEPROM.write(target_ram + 4, timer_L1[n_timer]);
            }
            if(timer_L2[n_timer] > MAX_LAGU){
              timer_L2[n_timer] = 0;
              EEPROM.write(target_ram + 2, timer_L2[n_timer]);
            }
            if(timer_L3[n_timer] > MAX_LAGU){
              timer_L3[n_timer] = 0;
              EEPROM.write(target_ram + 2, timer_L3[n_timer]);
            }

            
        //}
    }
};












// data clock en/latch
LiquidCrystal_SR lcd(pData, pClock, pLatch);


// untuk tombol
const int btn_delay_limit = 250;
unsigned long btn_delay_last = 0;
unsigned long btn_delay_now = 0;


// untuk debug
const boolean debug = true;

String kembali = "[Kembali]";
byte MENU_KEMBALI = 0;

String hari[7] = {
  "Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"
};

boolean hari_aktif[7] = {
  true,true,true,true,true,true,true
};

byte timer_aktif_hari[7] = {
  0,0,0,0,0,0,0
};

byte hari_aktif_tmp = 0;
byte hari_terpilih = 0;
byte timer_terpilih = 0;

const byte MENU_UTAMA = 1;
const byte MENU_LIST_MODE = 2;
const byte MENU_LIST_DISPLAY = 3;
const byte MENU_LIST_HARI = 4;
const byte MENU_LIST_TIMER = 5;
const byte MENU_REFERENSI_HARI = 6;
const byte MENU_TIMER = 7;

const byte TETAP = 1;
const byte NAIK = 2;
const byte TURUN = 3;

const byte menu_utama_size = 3;
String menu_utama[menu_utama_size] = {
  "1.Set",
  "2.Pilih Hari",
  "3.Display"
};

const byte menu_list_mode_size = 4;
String menu_list_mode[menu_list_mode_size] = {
  "1.Umum",
  "2.Islami",
  "3.Harian",
  kembali
};

const byte menu_list_display_size = 4;
String menu_list_display[]{
  "1.Off",
  "2.On",
  "3.Auto",
  kembali
};

String menu_list_hari[8] = {
  hari[0], hari[1], hari[2], hari[3], hari[4], hari[5], hari[6],
  // HARI x
  kembali
};

String menu_list_timer[3] = {
  // TIMER X
  "[Tambah Timer]",
  "[Copy Timer]",
  kembali
};

String menu_referensi_hari[2] = {
  kembali
  // HARI X
};

String menu_timer[5] = {
  "Timer:",
  "Lagu :",
  "[Simpan]",
  "[Hapus]",
  kembali
};

byte menu_level = 0;
byte menu_item = 0;
byte menu_level_max = 0;
byte menu_item_max = 0;

byte menu_item_list_hari_last = 0;
byte menu_item_list_timer_last = 0;

boolean setting_mode = false;

byte jumlah_timer = 5;
byte jumlah_hari = 7;

XTimer xt_satu = XTimer(0);
XTimer XTimer_Set = XTimer(0);

String default_display = "Hello, Default!";
boolean is_default_display = true;

unsigned long menu_timeout_now = 0;
unsigned long menu_timeout_last = 0;
unsigned long menu_timeout_limit = 5000L;


/*
 * ROM ADDR DATA 
 * 0 - 99 untuk static
 * 100 - 1023 untuk timer harian
 * 
 * 0 - mode [0:umum, 1:islami, 2:harian]
 * 1 - backlight LCD [0:off, 1:on, 2:auto]
 * 
 * 
 */

const byte ADDR_TIMER_MODE = 0;
const byte ADDR_LCD_BACKLIGHT = 1;


const byte DTIMER_MODE_UMUM = 0;
const byte DTIMER_MODE_ISLAMI = 1;
const byte DTIMER_MODE_HARIAN = 2;

const byte DLCD_BACKLIGHT_OFF = 0;
const byte DLCD_BACKLIGHT_ON = 1;
const byte DLCD_BACKLIGHT_AUTO = 2;

byte DTIMER_MODE = DTIMER_MODE_UMUM;
byte DLCD_BACKLIGHT = DLCD_BACKLIGHT_ON;


/*
 * backlight display timeout
 */
boolean lcd_bl_on = false;
unsigned long lcd_bl_timeout_now = 0;
unsigned long lcd_bl_timeout_last = 0;
unsigned long lcd_bl_timeout_limit = 5000L;

// sinkronisasi hari dan jumlah timer
boolean tersinkonisasi = false;

void setup() {
    pinMode(tombol, INPUT);
    pinMode(lcdBacklight, OUTPUT);

    // timer option (system option)
    get_timer_option();
    switch(DLCD_BACKLIGHT){
      case DLCD_BACKLIGHT_OFF:
        set_lcd_bl(false);
        break;
      case DLCD_BACKLIGHT_ON:
        set_lcd_bl(true);
        break;
      case DLCD_BACKLIGHT_AUTO:
        set_lcd_bl(true);
        menu_timeout_last = millis();
        break;
    }

    // jumlah timer
    check_hari();
    
    
    lcd.begin(16,2);
    lcd.setCursor(0,0);
    lcd.print(default_display);

    if(debug){
      Serial.begin(9600);
    }

    /*for(byte d=0; d<7; d++){
        xt_satu.set_hari(d);
        xt_satu.load_timer(0);
        Serial.print("Status : ");
        Serial.println(xt_satu.timer_status[0]);
        Serial.print("Jumlah : ");
        Serial.println(xt_satu.jumlah_timer_aktif);
        delay(5000);
        //xt_satu.load_timer(1);
        //delay(5000);
        xt_satu.load_timer(5);
        Serial.print("Status : ");
        Serial.println(xt_satu.timer_status[5]);
        delay(5000);
        //xt_satu.load_timer(19);
        //delay(60000L);
   
    }*/
}

void loop() {
    btn_listener();
    if(setting_mode == false){
        if(is_default_display == false){
            lcd.clear();
            lcd.print(default_display);
            is_default_display = true;
        }
    }else{
        menu_timeout();
    }
    if(DLCD_BACKLIGHT == DLCD_BACKLIGHT_AUTO){
        lcd_bl_timeout();
    }
}

void btn_listener() {
    int data_tombol = analogRead(tombol);
    btn = 0;
    if(data_tombol > 10){
        if((data_tombol>(btn_set_val-btn_toleransi)) && (data_tombol<(btn_set_val+btn_toleransi))){
          btn = BTN_SET;
        }else if((data_tombol>(btn_up_val-btn_toleransi)) && (data_tombol<(btn_up_val+btn_toleransi))){
          btn = BTN_UP;
        }else if((data_tombol>(btn_down_val-btn_toleransi)) && (data_tombol<(btn_down_val+btn_toleransi))){
          btn = BTN_DOWN;
        }else if((data_tombol>(btn_ok_val-btn_toleransi)) && (data_tombol<(btn_ok_val+btn_toleransi))){
          btn = BTN_OK;
        }else{
          btn = btn_bef;
        }

        btn_delay_now = millis();
        if(btn_delay_now - btn_delay_last >= btn_delay_limit){
          
            if(debug){
                Serial.print("Tombol : ");
                Serial.print(btn);
            }
          
            switch(btn) {
                case BTN_SET:
                    if(setting_mode == false){
                      menu_level = 1;
                      menu_item = 0;
                      menu_item_max = 3; // jumlah menu
                      setting_mode = true;
                      is_default_display = false;

                      if(!lcd_bl_on){
                        set_lcd_bl(true);
                      }
                    }
            
                    // level tetap, item up;
                    menu_listener(TETAP, NAIK);
                    if(debug){
                      Serial.println(" > SET ");
                    }
                    break;
              
                case BTN_UP:
                    // level tetap, item up
                    menu_listener(TETAP, NAIK);
                    if(debug){
                      Serial.println(" > UP ");
                    }
                    break;
              
                case BTN_DOWN:
                    // level tetap, item down
                    menu_listener(TETAP, TURUN);
                    if(debug){
                      Serial.println(" > DOWN ");
                    }
                    break;
              
                case BTN_OK:
                    proses_menu();
                    if(debug){
                      Serial.println(" > OK ");
                    }
                    break;
            }
            btn_delay_last = btn_delay_now;
            menu_timeout_last = btn_delay_now;
            lcd_bl_timeout_last = btn_delay_now;
        }
        btn_bef = btn;
        
    }
    
}

void proses_menu(){
    switch(menu_level){
        case MENU_UTAMA:
            switch(menu_item){
              case 1: 
                menu_level = MENU_LIST_MODE;
                menu_item_max = 4;
                MENU_KEMBALI = 4;
                menu_item = 1;
                // level naik, item reset
                menu_listener(TETAP, TETAP);
                break;

              case 2:
                menu_level = MENU_LIST_HARI;
                // hari yang aktif + menu kembali
                menu_item_max = count_hari_aktif() + 1;
                MENU_KEMBALI = menu_item_max;
                menu_item = 1;
                // level naik, item reset
                menu_listener(TETAP, TETAP);
                break;

              case 3:
                menu_level = MENU_LIST_DISPLAY;
                menu_item_max = 4;
                MENU_KEMBALI = 4;
                menu_item = 1;
                // level naik, item reset
                menu_listener(TETAP, TETAP);
                break;
                
            }
            
            break;
            
        case MENU_LIST_MODE:
            switch(menu_item){
              case 1:
                EEPROM.write(ADDR_TIMER_MODE, DTIMER_MODE_UMUM);
                DTIMER_MODE = DTIMER_MODE_UMUM;
                sync_mode();
                load_menu(MENU_UTAMA, menu_utama_size, 1);
                break;

              case 2:
                EEPROM.write(ADDR_TIMER_MODE, DTIMER_MODE_ISLAMI);
                DTIMER_MODE = DTIMER_MODE_ISLAMI;
                sync_mode();
                load_menu(MENU_UTAMA, menu_utama_size, 1);
                break;

              case 3:
                EEPROM.write(ADDR_TIMER_MODE, DTIMER_MODE_HARIAN);
                DTIMER_MODE = DTIMER_MODE_HARIAN;
                sync_mode();
                load_menu(MENU_UTAMA, menu_utama_size, 1);
                break;

              case 4:
                load_menu(MENU_UTAMA, menu_utama_size, 1);
                break;
            }
            check_hari();
            break;

        case MENU_LIST_DISPLAY:
            switch(menu_item){
              case 1:
                EEPROM.write(ADDR_LCD_BACKLIGHT, DLCD_BACKLIGHT_OFF);
                DLCD_BACKLIGHT = DLCD_BACKLIGHT_OFF;
                set_lcd_bl(false);
                load_menu(MENU_UTAMA, menu_utama_size, 3);
                break;

              case 2:
                EEPROM.write(ADDR_LCD_BACKLIGHT, DLCD_BACKLIGHT_ON);
                DLCD_BACKLIGHT = DLCD_BACKLIGHT_ON;
                set_lcd_bl(true);
                load_menu(MENU_UTAMA, menu_utama_size, 3);
                break;

              case 3:
                EEPROM.write(ADDR_LCD_BACKLIGHT, DLCD_BACKLIGHT_AUTO);
                DLCD_BACKLIGHT = DLCD_BACKLIGHT_AUTO;
                set_lcd_bl(true);
                load_menu(MENU_UTAMA, menu_utama_size, 3);
                break;

              case 4:
                load_menu(MENU_UTAMA, menu_utama_size, 3);
                break;
            }
            break;
            
        case MENU_LIST_HARI:
            if(menu_item == MENU_KEMBALI){
              load_menu(MENU_UTAMA, menu_utama_size, 2);
              
            }else{
              // record hari yang terpilih
              menu_item_list_hari_last = menu_item;

              //String hariter = "Hari terpilih : ";
              //hariter.concat(hari[hari_terpilih]);
              //hariter.concat(" (");
              //hariter.concat(hari_terpilih);
              //hariter.concat(")");
              //Serial.println(hariter);

              //XTimer_Set = XTimer(hari_terpilih);
              XTimer_Set.set_hari(hari_terpilih);
              XTimer_Set.check_ram();

              //String jumtimer = "Jumlah timer : ";
              //jumtimer.concat(XTimer_Set.jumlah_timer_aktif);
              //Serial.println(jumtimer);
                
              menu_level = MENU_LIST_TIMER;
              menu_item_max = 3 + XTimer_Set.jumlah_timer_aktif;
              menu_item = 1;
              MENU_KEMBALI = menu_item_max;
              menu_listener(TETAP, TETAP);
            }
            break;
            
        case MENU_LIST_TIMER:

            if(menu_item == MENU_KEMBALI){
              menu_level = MENU_LIST_HARI;
              menu_item_max = count_hari_aktif()+1;
              menu_item = menu_item_list_hari_last;
              MENU_KEMBALI = menu_item_max;
              // level naik, item reset
              menu_listener(TETAP, TETAP);
              
            }else{
                // jika ada timer yang aktif
                if(XTimer_Set.jumlah_timer_aktif > 0){

                    // jika menu yang dipilih bukan timer, tapi add-timer
                    // atau copy-timer
                    if(menu_item > XTimer_Set.jumlah_timer_aktif){
                        timer_terpilih = 0;
                        if(menu_item == (XTimer_Set.jumlah_timer_aktif+1)){
                            // record timer yang terpilih
                            menu_item_list_timer_last = menu_item;

                            menu_level = MENU_TIMER;
                            menu_item_max = 5;
                            menu_item = 1;
                            MENU_KEMBALI = menu_item_max;
                            menu_listener(TETAP, TETAP);
                        }else if(menu_item == (XTimer_Set.jumlah_timer_aktif+2)){
                            // record timer yang terpilih
                            menu_item_list_timer_last = menu_item;
              
                            menu_level = MENU_REFERENSI_HARI;
                            menu_item_max = 1 + jumlah_hari;
                            menu_item = 1;
                            MENU_KEMBALI = 1;
                            menu_listener(TETAP, TETAP);
                        }
                    }else{
                        // jika ada timer yang dipilih

                        
                        // record timer yang terpilih
                        menu_item_list_timer_last = menu_item;
                        timer_terpilih = menu_item;

                        menu_level = MENU_TIMER;
                        menu_item_max = 5;
                        menu_item = 1;
                        MENU_KEMBALI = menu_item_max;
                        menu_listener(TETAP, TETAP);
                    }
                }else{
                    // add timer
                    if(menu_item == 1){
                        // record timer yang terpilih
                        menu_item_list_timer_last = menu_item;

                        menu_level = MENU_TIMER;
                        menu_item_max = 5;
                        menu_item = 1;
                        MENU_KEMBALI = menu_item_max;
                        menu_listener(TETAP, TETAP);

                    // or copy timer
                    }else if(menu_item == 2){
                        // record timer yang terpilih
                        menu_item_list_timer_last = menu_item;
              
                        menu_level = MENU_REFERENSI_HARI;
                        menu_item_max = 1 + jumlah_hari;
                        menu_item = 1;
                        MENU_KEMBALI = 1;
                        menu_listener(TETAP, TETAP);
                    }
                }
            }
            /*else if(menu_item == 1){ // disamakan
              // record timer yang terpilih
              /*menu_item_list_timer_last = menu_item;
              
              menu_level = MENU_REFERENSI_HARI;
              menu_item_max = 2 + jumlah_hari;
              menu_item = 1;
              MENU_KEMBALI = menu_item_max;
              menu_listener(TETAP, TETAP);
              
            }else{
              // record timer yang terpilih
              /*menu_item_list_timer_last = menu_item;

              menu_level = MENU_TIMER;
              menu_item_max = 4;
              menu_item = 1;
              MENU_KEMBALI = 4;
              menu_listener(TETAP, TETAP);
              
            }
            */
            break;
            
        case MENU_REFERENSI_HARI:
            if(menu_item == MENU_KEMBALI){
              menu_level = MENU_LIST_TIMER;
              menu_item_max = 3 + XTimer_Set.jumlah_timer_aktif;
              menu_item = menu_item_list_timer_last;
              MENU_KEMBALI = menu_item_max;
              // level naik, item reset
              menu_listener(TETAP, TETAP);
              
            }else{
            }
            break;

        case MENU_TIMER:

            // timer sesuai hari yang dipilih
            XTimer_Set.set_hari(hari_terpilih);
            XTimer_Set.check_ram();
            
            if(menu_item == MENU_KEMBALI){
                menu_level = MENU_LIST_TIMER;
                menu_item_max = 3 + XTimer_Set.jumlah_timer_aktif;
                menu_item = menu_item_list_timer_last;
                MENU_KEMBALI = menu_item_max;
                // level naik, item reset
                menu_listener(TETAP, TETAP);
                
            }else if(menu_item == 1){ // Timer
                
            }else if(menu_item == 2){ // Lagu
              
            }else if(menu_item == 3){ // Simpan
                // tambah timer pada ram yang terbawah dan belum dipakai
                XTimer_Set.add_timer(timer_terpilih);
                // recount jumlah timer aktif
                //XTimer_Set.set_hari(hari_terpilih);
                //XTimer_Set.check_ram();
                
                menu_level = MENU_LIST_TIMER;
                menu_item_max = 3 + XTimer_Set.jumlah_timer_aktif;
                menu_item = menu_item_list_timer_last;
                MENU_KEMBALI = menu_item_max;
                // level naik, item reset
                menu_listener(TETAP, TETAP);

                tersinkonisasi = false;
            }else if(menu_item == 4){ // hapus

                //Serial.print("Timer Terpilih : ");
                //Serial.println(timer_terpilih); 
                XTimer_Set.del_timer(timer_terpilih);
                //XTimer_Set.set_hari(hari_terpilih);
                //XTimer_Set.check_ram();

                //check_hari();
                menu_level = MENU_LIST_TIMER;
                menu_item_max = 3 + XTimer_Set.jumlah_timer_aktif;
                menu_item = menu_item_list_timer_last;
                MENU_KEMBALI = menu_item_max;
                // level naik, item reset
                menu_listener(TETAP, TETAP);

                tersinkonisasi = false;
            }
            break;
    }
}

void menu_listener(byte level, byte item){
    String m1, m2;
    byte posisi;
    switch(menu_level){
        case MENU_UTAMA:
            menu_item = hx_constrain(menu_item,1,menu_item_max, item);

            if(menu_item <=2){
              m1 = menu_utama[0];
              switch(DTIMER_MODE){
                case DTIMER_MODE_UMUM:
                  m1.concat(" [Umum]");
                  break;
                case DTIMER_MODE_ISLAMI:
                  m1.concat(" [Islami]");
                  break;
                case DTIMER_MODE_HARIAN:
                  m1.concat(" [Harian]");
                  break;
              }
              m2 = menu_utama[1];
            }else{
              m1 = menu_utama[1];
              m2 = menu_utama[2];
              switch(DLCD_BACKLIGHT){
                case DLCD_BACKLIGHT_OFF:
                  m2.concat(" [Off]");
                  break;
                case DLCD_BACKLIGHT_ON:
                  m2.concat(" [On]");
                  break;
                case DLCD_BACKLIGHT_AUTO:
                  m2.concat(" [A]");
                  break;
              }
            }
            
            set_display(m1, m2);
            break;
            
        case MENU_LIST_MODE:
            menu_item = hx_constrain(menu_item,1,menu_item_max, item);
            
            if(menu_item <= 2){
              m1 = menu_list_mode[0];
              m2 = menu_list_mode[1];
            }else if(menu_item < menu_item_max){
              m1 = menu_list_mode[menu_item-2];
              m2 = menu_list_mode[menu_item-1];
            }else{
              m1 = menu_list_mode[menu_item_max-2];
              m2 = menu_list_mode[menu_item_max-1];
            }
            set_display(m1, m2);
            break;

        case MENU_LIST_DISPLAY:
            menu_item = hx_constrain(menu_item,1,menu_item_max, item);
            
            if(menu_item <= 2){
              m1 = menu_list_display[0];
              m2 = menu_list_display[1];
            }else if(menu_item < menu_item_max){
              m1 = menu_list_display[menu_item-2];
              m2 = menu_list_display[menu_item-1];
            }else{
              m1 = menu_list_display[menu_item_max-2];
              m2 = menu_list_display[menu_item_max-1];
            }
            set_display(m1, m2);
            break;
            
        case MENU_LIST_HARI:

            // sinkronisasi jumlah timer dan hari
            if(!tersinkonisasi){
              check_hari();
            }
            
            
            menu_item = hx_constrain(menu_item,1,menu_item_max, item);
            byte mn1,mn2;
            if(menu_item <= 2){
                if(DTIMER_MODE == DTIMER_MODE_UMUM){
                  m1 = hari[1];
                  m2 = hari[2];
                  mn1 = 1; mn2 = 2;
                }else if(DTIMER_MODE == DTIMER_MODE_ISLAMI){
                  m1 = hari[0];
                  m2 = hari[1];
                  mn1 = 0; mn2 = 1;
                }else if(DTIMER_MODE == DTIMER_MODE_HARIAN){
                  m1 = hari[0];
                  m2 = hari[1];
                  mn1 = 0; mn2 = 1;
                }
            }else if(menu_item < menu_item_max){
                if(DTIMER_MODE == DTIMER_MODE_UMUM){
                  m1 = hari[menu_item-1];
                  m2 = hari[menu_item];
                  mn1 = menu_item-1; mn2 = menu_item;
                }else if(DTIMER_MODE == DTIMER_MODE_ISLAMI){
                  if(menu_item <= 5){
                    m1 = hari[menu_item-2];
                    m2 = hari[menu_item-1];
                    mn1 = menu_item-2; mn2 = menu_item-1;
                  }else{
                    m1 = hari[menu_item-2];
                    m2 = hari[menu_item];
                    mn1 = menu_item-2; mn2 = menu_item;
                  }
                }else if(DTIMER_MODE == DTIMER_MODE_HARIAN){
                  m1 = hari[menu_item-2];
                  m2 = hari[menu_item-1];
                  mn1 = menu_item-2; mn2 = menu_item-1;
                }
            }else{
                if(DTIMER_MODE == DTIMER_MODE_UMUM){
                  m1 = hari[menu_item_max-1];
                  m2 = kembali;
                  mn1 = menu_item_max-1; mn2 = 10;
                }else if(DTIMER_MODE == DTIMER_MODE_ISLAMI){
                  m1 = hari[menu_item_max-1];
                  m2 = kembali;
                  mn1 = menu_item_max-1; mn2 = 10;
                }else if(DTIMER_MODE == DTIMER_MODE_HARIAN){
                  m1 = hari[menu_item_max-2];
                  m2 = kembali;
                  mn1 = menu_item_max-2; mn2 = 10;
                }
            }

            if(mn1 <= menu_item_max){
              m1.concat(" (t:");
              m1.concat(timer_aktif_hari[mn1]);
              m1.concat(")");
            }
            if(mn2 <= menu_item_max){
              m2.concat(" (t:");
              m2.concat(timer_aktif_hari[mn2]);
              m2.concat(")");
            }

            if(menu_item < 2){
              hari_terpilih = mn1;
            }else{
              hari_terpilih = mn2;
            }
            
            set_display(m1, m2);
            break;
            
        case MENU_LIST_TIMER:
            menu_item = hx_constrain(menu_item,1,menu_item_max, item);

            // aktifkan timer yang akan dikonfigurasi
            if(XTimer_Set.jumlah_timer_aktif > 0){
                if(menu_item <= 2){
                  if(XTimer_Set.jumlah_timer_aktif == 1){
                    m1 = "Timer ";
                    m1.concat(XTimer_Set.timer_status_sort[0]);
                    m2 = menu_list_timer[0];
                  }else{
                    m1 = "Timer ";
                    m1.concat(XTimer_Set.timer_status_sort[0]);
                    m2 = "Timer ";
                    m2.concat(XTimer_Set.timer_status_sort[1]);
                  }
                }else if(menu_item < (menu_item_max-1)){
                  m1 = "Timer ";
                  m1.concat(XTimer_Set.timer_status_sort[menu_item-2]);
                  
                  if(menu_item == (menu_item_max-2)){
                    m2 = menu_list_timer[2-(menu_item_max-menu_item)];
                  }else{
                    m2 = "Timer ";
                    m2.concat(XTimer_Set.timer_status_sort[menu_item-1]);
                  }
                }else{
                  m1 = menu_list_timer[1-(menu_item_max-menu_item)];
                  m2 = menu_list_timer[2-(menu_item_max-menu_item)];
                }
            }else{
                if(menu_item <= 2){
                  m1 = menu_list_timer[0];
                  m2 = menu_list_timer[1];
                }else{
                  m1 = menu_list_timer[menu_item_max-2];
                  m2 = menu_list_timer[menu_item_max-1];
                }
            }

            /*if(menu_item <= 2){
              m1 = menu_list_timer[0];
              m2 = "Timer 1";
            }else if(menu_item < menu_item_max){
              m1 = "Timer ";
              m1.concat(menu_item-2);
              if(menu_item <= (jumlah_timer+1)){
                m2 = "Timer ";
                m2.concat(menu_item-1);
              }else{
                m2 = menu_list_timer[1];
              }
            }else{
              m1 = menu_list_timer[1];
              m2 = menu_list_timer[2];
            }
            */
            set_display(m1, m2);
            break;
            
        case MENU_REFERENSI_HARI:
            menu_item = hx_constrain(menu_item,1,menu_item_max, item);

            if(menu_item <= 2){
              m1 = menu_referensi_hari[0];
              m2 = hari[0];
            }else if(menu_item < menu_item_max){
              m1 = hari[menu_item-3];
              if(menu_item <= (jumlah_hari+1)){
                m2 = hari[menu_item-2];
              }else{
                m2 = menu_referensi_hari[1];
              }
            }else{
              m1 = hari[jumlah_hari-2];
              m2 = hari[jumlah_hari-1];
              //m2 = menu_referensi_hari[1];
            }
            set_display(m1, m2);
            break;

        case MENU_TIMER:
            menu_item = hx_constrain(menu_item,1,menu_item_max, item);

            if(menu_item <= 2){
              m1 = menu_timer[0];
              m2 = menu_timer[1];
            }else if(menu_item < menu_item_max){
              m1 = menu_timer[menu_item-2];
              m2 = menu_timer[menu_item-1];
            }else{
              m1 = menu_timer[menu_item_max-2];
              m2 = menu_timer[menu_item_max-1];
            }
            set_display(m1, m2);
            break;
    }
}

void menu_timeout(){
    menu_timeout_now = millis();
    if(menu_timeout_now - menu_timeout_last >= menu_timeout_limit){
        setting_mode = false;
        btn_bef = 0;
    }
}

void set_lcd_bl(boolean stat){
    digitalWrite(lcdBacklight, stat);
    lcd_bl_on = stat;
}

/*
 * Jika Backlight dibuat auto
 */
void lcd_bl_timeout(){
    lcd_bl_timeout_now = millis();
    if(lcd_bl_timeout_now - lcd_bl_timeout_last >= lcd_bl_timeout_limit){
        if(lcd_bl_on){
            set_lcd_bl(false);
        }
    }
}

void load_menu(byte LV, byte LV_MAX, byte LV_ITEM){
    menu_level = LV;
    menu_item_max = LV_MAX;
    menu_item = LV_ITEM;
    // level naik, item reset
    menu_listener(TETAP, TETAP);
}

void set_display(String m1, String m2){
    lcd.clear();
    lcd.setCursor(1,0);
    lcd.print(m1);
    lcd.setCursor(1,1);
    lcd.print(m2);
    
    if(menu_item < 2){
        lcd.setCursor(0,0);
    }else{
        lcd.setCursor(0,1);
    }
    lcd.print('>');
}

int hx_constrain(int val, int mini, int maxi, byte stat){

    if(stat == TETAP) return val;
    
    int val_bef = val;
    if(stat == NAIK){
      val++;
    }else if(stat == TURUN){
      val--;
    }

    if(val > val_bef){
      if(val > maxi) val = maxi;
    }
    if(val < val_bef){
      if(val < mini) val = mini;
    }

    return val;
}


/*
 * Global timer options
 */
void get_timer_option(){
    DTIMER_MODE = EEPROM.read(ADDR_TIMER_MODE);
    DLCD_BACKLIGHT = EEPROM.read(ADDR_LCD_BACKLIGHT);
    if(DTIMER_MODE > 200){
      DTIMER_MODE = DTIMER_MODE_UMUM;
      EEPROM.write(ADDR_TIMER_MODE,DTIMER_MODE);
    }
    if(DLCD_BACKLIGHT > 200){
      DLCD_BACKLIGHT = DLCD_BACKLIGHT_ON;
      EEPROM.write(ADDR_LCD_BACKLIGHT,DLCD_BACKLIGHT);
    }
}


/*
 * setting ram hari
 */
void sync_mode(){
    switch(DTIMER_MODE){
      case DTIMER_MODE_UMUM:
        set_mode_hari(0, false); // minggu tidak aktif
        for(byte tmx=1; tmx<7; tmx++){
          set_mode_hari(tmx, true); // senin-sabtu aktif
        }
        break;
        
      case DTIMER_MODE_ISLAMI:
        for(byte tmx=0; tmx<7; tmx++){
          set_mode_hari(tmx, true); // minggu-sabtu aktif
        }
        set_mode_hari(5, false); // jumat tidak aktif
        break;
        
      case DTIMER_MODE_HARIAN:
        for(byte tmx=0; tmx<7; tmx++){
          set_mode_hari(tmx, true); // aktifkan semua (nanti dipilih manual)
        }
        break;
    }
}

void set_mode_hari(byte tHari, boolean data){
    //XTimer_Set = XTimer(tHari);
    XTimer_Set.set_hari(tHari);
    XTimer_Set.set_status_hari(data);
}

void check_hari(){
    for(byte hx=0; hx<7; hx++){
        //XTimer_Set = XTimer(hx);
        XTimer_Set.set_hari(hx);
        //String msgc = "Status Hari ";
        //msgc.concat(hari[hx]);
        //msgc.concat("(");
        //msgc.concat(hx);
        //msgc.concat(") = ");
        //msgc.concat(XTimer_Set.get_status_hari());
        //Serial.println(msgc);
        timer_aktif_hari[hx] = XTimer_Set.jumlah_timer_aktif;
    }
    tersinkonisasi = true;
}

byte count_hari_aktif(){
    if(DTIMER_MODE == DTIMER_MODE_UMUM){
      return 6;
    }else if(DTIMER_MODE == DTIMER_MODE_ISLAMI){
      return 6;
    }else if(DTIMER_MODE == DTIMER_MODE_HARIAN){
      return 7;
    }
}

