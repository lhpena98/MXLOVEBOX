#include <U8g2lib.h>
#include <U8x8lib.h>


//
// OLED Test
//
#include <U8x8lib.h>
U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

const uint8_t colLow = 4;
const uint8_t colHigh = 13;
const uint8_t rowCups = 0;
const uint8_t rowState = 2; // Double spacing the Rows
const uint8_t rowTemp = 4; // Double spacing the Rows
const uint8_t rowTime = 6; // Double spacing the Rows
char *msg="Now is the time for all good men to come to the aid the party.";
byte i;
byte y = 0;

void lcdBold(bool aVal) {
if (aVal) {
u8x8.setFont(u8x8_font_victoriabold8_r); // BOLD
} else {
u8x8.setFont(u8x8_font_victoriamedium8_r); // NORMAL
}
}

void lcdRun() {
lcdBold(false);
u8x8.clear(); // ("0123456789012345")
u8x8.setCursor(0,rowCups); 
for (i=0;i<strlen(msg);i++) {
  if ((i % 16)==0 && i!=0) y++;
     u8x8.setCursor(i % 16,y);
     u8x8.print(msg[i]);
  }
}

////////////////////////////////////////////////////////////////////////////////
void setup() {
Serial.begin(115200);
u8x8.begin();
lcdBold(false); // You MUST make this call here to set a Font
lcdRun();
}
////////////////////////////////////////////////////////////////////////////////
void loop() {
}
