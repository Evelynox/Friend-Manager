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

// ANSI-Farben
#define RESET       "\033[0m"
#define TEXT_WHITE  "\033[97m"
#define TEXT_YELLOW "\033[93m"
#define TEXT_CYAN   "\033[96m"
#define TEXT_RED    "\033[91m"
#define BG_PINK     "\033[45m"

const string jsonPath = string(getenv("HOME")) + "/.local/share/friends/list.json";

// Datenstruktur
struct Freund {
    string anrede;
    string vorname;
    string nachname;
    string benutzername;
    unsigned int freundSeit;
    Freund* next = nullptr;
};

// Anzeige
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

void bearbeiten(Freund* person) {
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
        cout << "Auswahl: ";
        cin >> eingabe;

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
                cout << "Neues Jahr der Freundschaft: "; cin >> person->freundSeit; break;
            case 0: break;
            default:
                cout << TEXT_RED << "Ungültige Auswahl.\n" << RESET;
        }
    } while (eingabe != 0);
}

// JSON
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

// Main
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
        cout << "Auswahl: ";
        cin >> auswahl;

        switch (auswahl) {
            case 1: {
                system("clear");
                Freund* neuer = new Freund;
                cout << "Anrede: "; cin >> neuer->anrede;
                cout << "Vorname: "; cin >> neuer->vorname;
                cout << "Nachname: "; cin >> neuer->nachname;
                cout << "Benutzername: @"; cin >> neuer->benutzername;
                cout << "Freunde seit (Jahr): "; cin >> neuer->freundSeit;
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
                cout << "Index zum Löschen: ";
                cin >> del;
                loescheByIndex(freundesliste, del);
                break;
            }
            case 4: {
                int i1, i2;
                anzeigen(freundesliste);
                cout << "Tausche Freund: ";
                cin >> i1;
                cout << "Mit Freund: ";
                cin >> i2;
                tausche(freundesliste, i1, i2);
                break;
            }
            case 5: {
                int edit;
                anzeigen(freundesliste);
                cout << "Freund zum Bearbeiten (Index): ";
                cin >> edit;
                Freund* person = findeByIndex(freundesliste, edit);
                bearbeiten(person);
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
