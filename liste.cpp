#include <iostream>
#include <string>
#include <locale>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;

// Farben & Styles wie gehabt
#define RESET       "\033[0m"
#define TEXT_WHITE  "\033[97m"
#define TEXT_YELLOW "\033[93m"
#define TEXT_CYAN   "\033[96m"
#define TEXT_RED    "\033[91m"
#define BG_PINK     "\033[45m"
#define RED     "\033[38;5;196m"
#define GREEN   "\033[38;5;28m"
#define YELLOW  "\033[38;5;226m"
#define BLUE    "\033[38;5;21m"
#define MAGENTA "\033[38;5;165m"
#define PINK    "\033[38;5;204m"
#define CYAN    "\033[38;5;51m"
#define WHITE   "\033[38;5;255m"

const string jsonPath = string(getenv("HOME")) + "/.local/share/friends/list.json";

struct Freund {
    string anrede;
    string vorname;
    string nachname;
    string benutzername;
    unsigned int freundSeit;
    Freund* next = nullptr;
};

// Globale Fehlerzählung (bei Bedarf)
short error_count = 0;

template<typename T>
void sichereEingabe(const string &prompt, T &wert, short& error_count) {
    while (true) {
        cout << prompt;
        cin >> wert;
        if (!cin.fail()) break;

        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        error_count++;
        if (error_count <= 5) {
            cout << YELLOW << "Hey! Don't break the program" << RESET << RED << " >:( \a" << RESET << endl << endl;
        } else if (error_count <= 8) {
            cout << BLUE << "Are you trying to annoy me?" << RESET << PINK << " :3 \a" << RESET << endl << endl;
        } else {
            cout << RED << "Enough! Aborting... >:O\a" << RESET << endl << endl;
            exit(EXIT_FAILURE);
        }
    }
}

void anzeigen(Freund* liste) {
    system("clear");
    cout << TEXT_WHITE << BG_PINK << "\n--- Freundesliste ---\n" << RESET;
    if (!liste) {
        cout << TEXT_YELLOW << "Keine Freunde vorhanden.\n" << RESET;
        return;
    }
    int index = 1;
    while (liste) {
        cout << TEXT_RED << "[" << index << "] "
             << liste->anrede << " " << liste->vorname << " " << liste->nachname
             << " (@" << liste->benutzername << ") – seit " << liste->freundSeit << RESET << endl;
        liste = liste->next;
        index++;
    }
}

int listenLaenge(Freund* liste) {
    int count = 0;
    while (liste) {
        count++;
        liste = liste->next;
    }
    return count;
}

Freund* findeByIndex(Freund* liste, int index) {
    int i = 1;
    while (liste && i < index) {
        liste = liste->next;
        i++;
    }
    return liste;
}

void einfuegenAmEnde(Freund*& liste, Freund* neuer) {
    if (!liste) {
        liste = neuer;
    } else {
        Freund* temp = liste;
        while (temp->next) temp = temp->next;
        temp->next = neuer;
    }
    neuer->next = nullptr;
}

void loescheByIndex(Freund*& liste, int index) {
    if (!liste || index < 1) return;
    if (index == 1) {
        Freund* temp = liste;
        liste = liste->next;
        delete temp;
        return;
    }
    Freund* vorher = findeByIndex(liste, index - 1);
    if (vorher && vorher->next) {
        Freund* temp = vorher->next;
        vorher->next = temp->next;
        delete temp;
    }
}

void tausche(Freund*& liste, int i1, int i2) {
    if (i1 == i2) return;
    int len = listenLaenge(liste);
    if (i1 < 1 || i2 < 1 || i1 > len || i2 > len) return;
    if (i1 > i2) swap(i1, i2);

    Freund dummy;
    dummy.next = liste;
    Freund* prev1 = &dummy, *prev2 = &dummy;
    for (int i = 1; i < i1; i++) prev1 = prev1->next;
    for (int i = 1; i < i2; i++) prev2 = prev2->next;
    Freund* node1 = prev1->next;
    Freund* node2 = prev2->next;
    if (!node1 || !node2) return;
    if (prev1->next == prev2) {
        prev1->next = node2;
        node1->next = node2->next;
        node2->next = node1;
    } else {
        Freund* tmp = node2->next;
        prev1->next = node2;
        node2->next = node1->next;
        prev2->next = node1;
        node1->next = tmp;
    }
    liste = dummy.next;
}

void bearbeiten(Freund* person, short& error_count) {
    if (!person) return;
    int eingabe = 0;
    do {
        cout << BG_PINK << TEXT_WHITE << "\n--- Bearbeiten von @" << person->benutzername << " ---\n" << RESET;
        cout << "[1] Anrede: " << person->anrede << endl;
        cout << "[2] Vorname: " << person->vorname << endl;
        cout << "[3] Nachname: " << person->nachname << endl;
        cout << "[4] Benutzername: @" << person->benutzername << endl;
        cout << "[5] Freunde seit: " << person->freundSeit << endl;
        cout << "[0] Zurück\n";
        sichereEingabe<int>("Auswahl: ", eingabe, error_count);

        switch (eingabe) {
            case 1:
                cout << "Neue Anrede: "; cin >> person->anrede; break;
            case 2:
                cout << "Neuer Vorname: "; cin >> person->vorname; break;
            case 3:
                cout << "Neuer Nachname: "; cin >> person->nachname; break;
            case 4:
                cout << "Neuer Benutzername: @"; cin >> person->benutzername; break;
            case 5:
                sichereEingabe<unsigned int>("Neues Jahr der Freundschaft: ", person->freundSeit, error_count); break;
            case 0: break;
            default:
                cout << TEXT_RED << "Ungültige Auswahl.\n" << RESET;
        }
    } while (eingabe != 0);
}

void speichereListe(Freund* liste) {
    json j;
    while (liste) {
        j.push_back({
            {"anrede", liste->anrede},
            {"vorname", liste->vorname},
            {"nachname", liste->nachname},
            {"benutzername", liste->benutzername},
            {"freundSeit", liste->freundSeit}
        });
        liste = liste->next;
    }
    string ordner = jsonPath.substr(0, jsonPath.find_last_of('/'));
    string cmd = "mkdir -p " + ordner;
    system(cmd.c_str());

    ofstream file(jsonPath);
    if (file.is_open()) {
        file << setw(4) << j;
        file.close();
    }
}

Freund* ladeListe() {
    ifstream file(jsonPath);
    if (!file.is_open()) return nullptr;
    json j;
    try {
        file >> j;
    } catch (...) {
        return nullptr;
    }
    file.close();
    Freund* kopf = nullptr;
    for (auto& f : j) {
        Freund* neuer = new Freund{
            f.value("anrede", ""),
            f.value("vorname", ""),
            f.value("nachname", ""),
            f.value("benutzername", ""),
            f.value("freundSeit", 0),
            nullptr
        };
        einfuegenAmEnde(kopf, neuer);
    }
    return kopf;
}

int main() {
    locale::global(locale("de_DE.UTF-8"));

    Freund* freundesliste = ladeListe();
    int auswahl;

    do {
        system("clear");
        cout << BG_PINK << TEXT_WHITE << "\n==== FREUNDESMANAGER ====\n" << RESET;
        cout << "[1] Freund hinzufügen\n";
        cout << "[2] Freunde anzeigen\n";
        cout << "[3] Freund löschen\n";
        cout << "[4] Reihenfolge ändern\n";
        cout << "[5] Freund bearbeiten\n";
        cout << "[6] Beenden\n";
        sichereEingabe<int>("Auswahl: ", auswahl, error_count);

        switch (auswahl) {
            case 1: {
                system("clear");
                Freund* neuer = new Freund;
                cout << "Anrede: "; cin >> neuer->anrede;
                cout << "Vorname: "; cin >> neuer->vorname;
                cout << "Nachname: "; cin >> neuer->nachname;
                cout << "Benutzername: @"; cin >> neuer->benutzername;
                sichereEingabe<unsigned int>("Freunde seit (Jahr): ", neuer->freundSeit, error_count);
                einfuegenAmEnde(freundesliste, neuer);
                break;
            }
            case 2:
                anzeigen(freundesliste);
                cin.ignore(); cin.get(); // Pause
                break;
            case 3: {
                int del;
                anzeigen(freundesliste);
                sichereEingabe<int>("Index zum Löschen: ", del, error_count);
                loescheByIndex(freundesliste, del);
                break;
            }
            case 4: {
                int i1, i2;
                anzeigen(freundesliste);
                sichereEingabe<int>("Tausche Freund: ", i1, error_count);
                sichereEingabe<int>("Mit Freund: ", i2, error_count);
                tausche(freundesliste, i1, i2);
                break;
            }
            case 5: {
                int edit;
                anzeigen(freundesliste);
                sichereEingabe<int>("Freund zum Bearbeiten (Index): ", edit, error_count);
                Freund* person = findeByIndex(freundesliste, edit);
                bearbeiten(person, error_count);
                break;
            }
            case 6:
                cout << "Speichere und beende...\n";
                speichereListe(freundesliste);
                break;
            default:
                cout << TEXT_RED << "Ungültige Auswahl.\n" << RESET;
        }
    } while (auswahl != 6);

    while (freundesliste) {
        Freund* temp = freundesliste;
        freundesliste = freundesliste->next;
        delete temp;
    }
    return 0;
}
