
#include "SparkIntervalTimer.h"
#include "RGBmatrixPanel.h"
#include "Adafruit_mfGFX.h"   // Core graphics library
#include "fonts.h"
#include "math.h"
#include "colors.h"

/** Define RGB matrix panel GPIO pins **/
#if defined (STM32F10X_MD)	//Core
	#define CLK D6
	#define OE  D7
	#define LAT A4
	#define A   A0
	#define B   A1
	#define C   A2
	#define D	A3		// Only used for 32x32 panels
#endif

#if defined (STM32F2XX)	//Photon
	#define CLK D6
	#define OE  D7
	#define LAT A4
	#define A   A0
	#define B   A1
	#define C   A2
	#define D	A3		// Only used for 32x32 panels
#endif
/****************************************/
#define SIZE 20
#define NHL 0
#define NBA 1
#define NFL 2

struct Team {
	char* name;
	uint16_t color;
	Team(char* n, uint16_t c) {
		name = n;
		color = c;
	}
};

//Color lookup tables for each league
const Team colorLookupNHL[30] =
{
	Team("ANA", Green),
	Team("BOS", Yellow),
	Team("BUF", Yellow),
	Team("CGY", Red),
	Team("CAR", Red),
	Team("CHI", Red),
	Team("COL", Maroon),
	Team("CLB", Blue),
	Team("DAL", Green),
	Team("DET", Red),
	Team("EDM", Orange),
	Team("FLA", Yellow),
	Team("LA", DarkPurple),
	Team("MIN", DarkGreen),
	Team("MTL", Red),
	Team("NSH", White),
	Team("NJ", Red),
	Team("NYI", Orange),
	Team("NYR", Blue),
	Team("OTT", Red),
	Team("PHI", Orange),
	Team("ARI", Maroon),
	Team("PIT", Yellow),
	Team("SJ", DarkCyan),
	Team("STL", Blue),
	Team("TB", Blue),
	Team("TOR", Blue),
	Team("VAN", Blue),
	Team("WSH", Red),
	Team("WPG", Navy)
};

const Team colorLookupNBA[30] =
{
	Team("ATL", Red),
	Team("BOS", Green),
	Team("BKN", DarkGrey),
	Team("CHA", Navy),
	Team("CHI", Red),
	Team("CLE", Maroon),
	Team("DAL", Cyan),
	Team("DEN", Yellow),
	Team("DET", Red),
	Team("GS", Yellow),
	Team("HOU", Red),
	Team("IND", Yellow),
	Team("LAC", Red),
	Team("LAL", DarkPurple),
	Team("MEM", Blue),
	Team("MIA", Maroon),
	Team("MIL", DarkGreen),
	Team("MIN", Green),
	Team("NO", Navy),
	Team("NY", Orange),
	Team("OKC", Orange),
	Team("PHI", Red),
	Team("PHO", Orange),
	Team("POR", Red),
	Team("SAC", DarkPurple),
	Team("SA", LightGrey),
	Team("TOR", Red),
	Team("UTA", Yellow),
	Team("WAS", Red),
	Team("ORL", Blue)
};

const Team colorLookupNFL[32] =
{
	Team("ARI", Maroon),
	Team("ATL", Red),
	Team("BAL", DarkPurple),
	Team("BUF", Blue),
	Team("CAR", Cyan),
	Team("CHI", Orange),
	Team("CIN", Orange),
	Team("CLE", Orange),
	Team("DAL", Navy),
	Team("DEN", Orange),
	Team("DET", Cyan),
	Team("GB", Green),
	Team("HOU", Navy),
	Team("IND", Blue),
	Team("JAC", Yellow),
	Team("KC", Red),
	Team("MIA", DarkCyan),
	Team("MIN", DarkPurple),
	Team("NE", Navy),
	Team("NO", Yellow),
	Team("NYG", Blue),
	Team("NYJ", DarkGreen),
	Team("OAK", DarkGrey),
	Team("PHI", DarkCyan),
	Team("PIT", Yellow),
	Team("SD", Yellow),
	Team("SF", Red),
	Team("SEA", GreenYellow),
	Team("STL", Navy),
	Team("TB", Red),
	Team("TEN", Navy),
	Team("WAS", Maroon)
};

RGBmatrixPanel matrix(A, B, C, D, CLK, LAT, OE, false);

unsigned long t = 0;
unsigned long t2 = 0;
unsigned long t3 = 0;
int pos = 0;
int sport = 0;
int step = 0;
int textWidth = 0;
int scores[SIZE] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
String teams[SIZE] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
int period[SIZE/2] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
void setup() {
	pinMode(A5, INPUT);
    pinMode(A6, INPUT);
    pinMode(A7, OUTPUT);
    matrix.begin();
    Particle.subscribe("hook-response/getNewScoresNHL", gotScoreData, MY_DEVICES);
	Particle.subscribe("hook-response/getNewScoresNBA", gotScoreData, MY_DEVICES);
	Particle.subscribe("hook-response/getNewScoresNFL", gotScoreData, MY_DEVICES);
	Time.zone(-5.0);
}

void loop() {
	//Update scores every minute
    if (millis() >= t+60000 || t == 0) {
		if (sport == NHL) {
			Particle.publish("getNewScoresNHL");
		}
		else if (sport == NBA) {
			Particle.publish("getNewScoresNBA");
		}
		else if (sport == NFL) {
			Particle.publish("getNewScoresNFL");
		}
		//gotScoreData(NULL, "VAN~7~PIT~0~1~BUF~5~MTL~5~4~LA~4~SJ~2~6~"); //For testing when there is no games
        t = millis();
    }
	//Change the game displayed every 4 seconds
    if (millis() >= t2+4000 && scores[0] != -1 && scores[1] != -1) {
		if (scores[2] != -1 && scores[3] != -1) {
			circleWipe();
		}
		matrix.setFont(GLCDFONT);
        matrix.fillScreen(matrix.Color333(0, 0, 0));
        matrix.setTextWrap(false);
        matrix.setTextSize(1);
		Particle.process();
		for (int x = 0; x < 8; x++) {
			matrix.fillScreen(Black);
			matrix.setCursor(0, (-7)+x);
			matrix.setTextColor(getColor(teams[pos*2], sport));
	        matrix.print(teams[pos*2]);
			matrix.setTextColor(getColor(teams[pos*2+1], sport));
	        matrix.setCursor(33-(teams[pos*2+1].length()*6), 31-x);
	        matrix.print(teams[pos*2+1]);
			checkForWave();
			delay(40);
		}
		Particle.process();
		delay(100);
		matrix.setTextColor(matrix.Color333(7, 7, 7));
		if (sport == NBA) {
			matrix.setFont(TEST);
			textWidth = 0;
			for (int x = 0; x < String(scores[pos*2]).length(); x++) {
				textWidth += matrix.charWidth(String(scores[pos*2])[x]);
			}
			for (int x = 0; x < 13; x++) {
				matrix.fillRect(0, 11, 32, 8, Black);
				matrix.setCursor((-(textWidth)+2)+x, 11);
				matrix.print(String(scores[pos*2]));
				matrix.setCursor(30-x, 11);
				matrix.print(String(scores[pos*2+1]));
				checkForWave();
				delay(40);
			}
			Particle.process();
			delay(100);
			matrix.drawFastHLine(14, 13, 3, White);
		}
		else {
			for (int x = 0; x < 12; x++) {
				matrix.fillRect(0, 11, 32, 8, Black);
				matrix.setCursor(((String(scores[pos*2]).length()*(-6))+2)+x, 11);
				matrix.print(String(scores[pos*2]));
				matrix.setCursor(30-x, 11);
				matrix.print(String(scores[pos*2+1]));
				checkForWave();
				delay(40);
			}
			Particle.process();
			delay(100);
			matrix.setCursor(13, 11);
			matrix.print("-");
		}
		matrix.setCursor(17, 0);
		matrix.setFont(TINIER_3);
		matrix.setTextColor(matrix.Color333(7, 7, 7));
		if (Time.minute() < 10) {
			matrix.print(String(Time.hourFormat12())+":0"+String(Time.minute()));
		} else {
			matrix.print(String(Time.hourFormat12())+":"+String(Time.minute()));
		}
		matrix.setCursor(0, 24);
		if (period[pos] == 1) {
			matrix.print("1st");
		}
		else if (period[pos] == 2) {
			matrix.print("2nd");
		}
		else if (period[pos] == 3) {
			matrix.print("3rd");
		}
		else if ((period[pos] == 4 && sport == NHL) || period[pos] == 5) {
			matrix.print("OT");
		}
		else if (period[pos] == 4) {
			matrix.print("4th");
		}
		else if (period[pos] == 6) {
			matrix.print("FIN");
		}
        pos++;
        pos %= SIZE/2;
        if (scores[pos*2] == -1 && scores[pos*2+1] == -1) {
            pos = 0;
        }
        t2 = millis();
    }
	//If there is no games, display that
    if (scores[0] == -1 && scores[1] == -1 && millis() >= t2+5000) {
		if (millis() <= t+10000) {
			matrix.setFont(TEST);
			matrix.setCursor(1, 11);
			matrix.setTextColor(Cyan);
			matrix.print("LOADING");
		} else {
			matrix.setFont(GLCDFONT);
			matrix.fillScreen(matrix.Color333(0, 0, 0));
	        matrix.setTextColor(matrix.Color333(7, 0, 0));
	        matrix.setCursor(11, 7);
	        matrix.print("NO");
			matrix.setCursor(1, 15);
	        matrix.print("GAMES");
		}
		t2 = millis();
    }
	checkForWave();
}

//Parses the data returned by the webhook
void gotScoreData(const char *name, const char *data) {
    String str = String(data);
    for (int x = 0; x < SIZE; x++) {
        scores[x] = -1;
        teams[x] = NULL;
		period[x/2] = -1;
    }
    int start = 0;
    int idx = 0;
    for (unsigned int x = 1; x < str.length() && idx < (SIZE*5)/2; x++) {
        if (str[x] == '~' && (idx % 5 == 0 || idx % 5 == 2)) {
			if (idx % 5 == 0) {
            	teams[idx*2/5] = ""+str.substring(start, x);
			} else {
				teams[idx*2/5+1] = ""+str.substring(start, x);
			}
            start = x+1;
            idx++;
        }
        else if (str[x] == '~' && (idx % 5 == 1 || idx % 5 == 3)) {
            scores[idx*2/5] = str.substring(start, x).toInt();
            start = x+1;
            idx++;
        }
		else if (str[x] == '~' && idx % 5 == 4) {
            period[idx/5] = str.substring(start, x).toInt();
            start = x+1;
            idx++;
        }
    }
}

//Checks the IR sensor for activity
void checkForWave() {
	int l, r = 0;
	digitalWrite(A7, HIGH);
    delay(10);
    l = analogRead(A6);
    r = analogRead(A5);
    digitalWrite(A7, LOW);
    delay(10);
    l -= analogRead(A6);
    r -= analogRead(A5);
	//In order to get the reflected IR, you must take a reading with the IR leds on, then turn them off and subtract the ambient light
	if (millis() > t3+200 || t3 == 0) {
		step = 0;
	}
	if (l > 900 && step == 0) {
		step = 1;
		t3 = millis();
	}
	if (r > 900 && step == 0) {
		step = 2;
		t3 = millis();
	}
	if (r > 900 && step == 1) {
		step = 0;
		sport--;
		if (sport < 0) {
			sport = 2;
		}
		t = 0;
		t2 -= 4000;
		newSport(sport);
	}
	if (l > 900 && step == 2) {
		step = 0;
		sport++;
		sport %= 3;
		t = 0;
		t2 -= 4000;
		newSport(sport);
	}
}

//Called when a new sport is set, displays the sport name then fades it out
void newSport(int s) {
	for (int x = 0; x < SIZE; x++) {
        scores[x] = -1;
        teams[x] = NULL;
		period[x/2] = -1;
    }
	matrix.setFont(GLCDFONT);
	matrix.fillScreen(Black);
	matrix.setCursor(7,11);
	matrix.setTextColor(Red);
	if (s == NHL) {
		matrix.print("NHL");
	}
	else if (s == NBA) {
		matrix.print("NBA");
	}
	else if (s == NFL) {
		matrix.print("NFL");
	}
	delay(500);
	circleWipe();
}

//Draws a black circle increasing in size
void circleWipe() {
    for (int x = 0; x < 23; x++) {
        matrix.fillCircle(16, 16, x, matrix.Color333(0, 0, 0));
		if (x % 3 == 0) {
			Particle.process();
		}
		checkForWave();
        delay(10);
    }
}

//Uses the color lookup tables to determine the color of a given team
uint16_t getColor(String team, int s) {
	if (s == NHL) {
	    for (int x = 0; x < 30; x++) {
			if (strcmp(colorLookupNHL[x].name, team.c_str()) == 0) {
				return colorLookupNHL[x].color;
			}
		}
	} else if (s == NBA) {
	    for (int x = 0; x < 30; x++) {
			if (strcmp(colorLookupNBA[x].name, team.c_str()) == 0) {
				return colorLookupNBA[x].color;
			}
		}
	} else if (s == NFL) {
	    for (int x = 0; x < 32; x++) {
			if (strcmp(colorLookupNFL[x].name, team.c_str()) == 0) {
				return colorLookupNFL[x].color;
			}
		}
	}
	return White;
}
