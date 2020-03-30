#include <ncurses.h>
#include <string>
#include <string.h>
#include <vector>
#include <sys/stat.h>
#include <dirent.h>
#include <fstream>
#include <unistd.h>

using namespace std;

WINDOW* ventana;
DIR *directorio = NULL;
struct dirent *dir_struct = NULL;
int MaxX, MaxY, markY = 0, colors1, colors2, colors3;
string usuario, maquina, path;
int cont_pass = 0;
vector<string> deleted;
vector<string> comandos = {
    "clear", "ls", "cd", "cd/", "del", "mkdir", "file", "write",
    "read", "changeColor", "changeUser", "changeMachine", "exit"
};

void lineaPrincipal() {
    string terminal = usuario + '@' + maquina;
    
    attron(COLOR_PAIR(colors2) | A_BOLD);
    mvwaddstr(stdscr, markY, 0, terminal.c_str());
    attroff(COLOR_PAIR(colors2));
    
    attron(COLOR_PAIR(colors3));
    mvwaddstr(stdscr, markY, terminal.size(), ":");
    attroff(COLOR_PAIR(colors3));

    attron(COLOR_PAIR(colors1));
    mvwaddstr(stdscr, markY, terminal.size() + 1, path.c_str());
    attroff(COLOR_PAIR(colors1));

    attron(COLOR_PAIR(colors3));
    mvwaddstr(stdscr, markY, terminal.size() + path.size() + 1, "$ ");
    attroff(COLOR_PAIR(colors3) | A_BOLD);
    
    refresh();
}

bool analizarComando(string comando) {
    bool found, exit;
    for (int i = 0; i < comandos.size(); i++) {
        if (strstr(comando.c_str(), comandos[i].c_str())) {
            found = true;
            if (comando == "exit") {
                exit = true;
            } else {
                exit = false;
            }
        }
    }
    if (!found) {
        mvwaddstr(ventana, ++markY, 0, "Command not found");
    }
    return exit;
}

bool verificarPath(string intento) {
    bool verificacion = false;
    if (path == "~") {
        directorio = opendir("/home");
    } else {
        directorio = opendir(path.c_str());
    }
    while (dir_struct = readdir(directorio)) {
        string nombre = dir_struct -> d_name;
        if (dir_struct -> d_type == DT_DIR && nombre == intento) {
            verificacion = true;
        }
    }
    return verificacion;
}

bool verificarDelRestriction(string archivo) {
    bool vistoBueno;
    for (int i = 0; i < deleted.size(); i++) {
        if (archivo == deleted[i])
            vistoBueno = false;
    }
    return vistoBueno;
}

void ejecutarComando(string comando) {
    if (comando == "clear") {
        clear();
        markY = 0;
    } else if (comando == "ls") {
        if (path == "~") {
            directorio = opendir("/home");
        } else {
            directorio = opendir(path.c_str());
        }
        while (dir_struct = readdir(directorio)) {
            string nombre = dir_struct -> d_name;
            if (verificarDelRestriction(nombre)) {
                switch (dir_struct -> d_type) {
                    case DT_REG: {
                        mvwaddstr(ventana, markY++, 0, nombre.c_str());
                    } break;
                    case DT_DIR: {
                        if (nombre == "." || nombre == "..") {
                        } else {
                            attron(COLOR_PAIR(colors1));
                            mvwaddstr(ventana, markY++, 0, nombre.c_str());
                            attroff(COLOR_PAIR(colors1));
                        }
                    } break;
                }
            }
        }
    } else if (strstr(comando.c_str(), "cd ")) {
        if (comando[3] == ' ') {
            mvwaddstr(ventana, markY++, 0, "INVALID COMMAND USE. EX. cd Documentos");
        } else {
            string destino;
            for (int i = 3; i < comando.size(); i++) {
                destino += comando[i];
            }
            if (verificarPath(destino) && verificarDelRestriction(destino)) {
                if (path == "~") {
                    path = "/home";
                } 
                if (path != "~") {
                    path += "/";
                    path += destino;
                }
            } else {
                mvwaddstr(ventana, markY++, 0, "DIRECTORIO NO EXISTE.");
            }
        }
    } else if (comando == "cd/") {
        if (path == "/home" || path == "~") {
        } else {
            int last = path.find_last_of("/");
            string pathTemporal;
            for (int i = 0; i < last; i++) {
                pathTemporal += path[i];
            }
            path = pathTemporal;
            directorio = opendir(path.c_str());
        }
    } else if (strstr(comando.c_str(), "del ")) {
        if (comando[4] == ' ') {
            mvwaddstr(ventana, markY++, 0, "INVALID COMMAND USE. EX. del Carpeta");
        } else {
            string auxiliar;
            for (int i = 4; i < comando.size(); i++) {
                auxiliar += comando[i];
            }
            deleted.push_back(auxiliar);
        }
    } else if (strstr(comando.c_str(), "mkdir ")) {
        if (comando[6] == ' ') {
            mvwaddstr(ventana, markY++, 0, "INVALID COMMAND USE. EX. mkdir Carpeta");
        } else {
            string nombreDirectorio;
            for (int i = 6; i < comando.size(); i++) {
                nombreDirectorio += comando[i];
            }
            string command = "mkdir " + path + "/" + nombreDirectorio;
            FILE *proc = popen(command.c_str(),"r");
        }
    } else if (strstr(comando.c_str(), "file ") && strstr(comando.c_str(), ".txt")) {
        if (comando[5] == ' ') {
            mvwaddstr(ventana, markY++, 0, "INVALID COMMAND USE. EX. file HolaMundo.txt");
        } else {
            string nombreArchivo;
            for (int i = 5; i < comando.size(); i++) {
                nombreArchivo += comando[i];
            }
            ofstream archivo(nombreArchivo,ios::out);
            string command = "mv " + nombreArchivo + " " + path;
            FILE *proc = popen(command.c_str(),"r");
        }
    } else if (strstr(comando.c_str(), "write ") && strstr(comando.c_str(), ".txt ")) {
        if (comando[6] == ' ') {
            mvwaddstr(ventana, markY++, 0, "INVALID COMMAND USE. EX. write HolaMundo.txt HOLA");
        } else {
            int found = comando.find(".txt");
            string nombreArchivo, texto;
            for (int i = 6; i < found + 4; i++) {
                nombreArchivo += comando[i];
            }

            ofstream archivo(path + "/" + nombreArchivo, ios::app);
            for (int i = found + 5; i < comando.size(); i++) {
                if (comando[i] != ' ')
                    archivo << comando[i];
                else
                    break;
            }
            archivo << " \n";
        }
    } else if (strstr(comando.c_str(), "read ") && strstr(comando.c_str(), ".txt")) {
        if (comando[5] == ' ') {
            mvwaddstr(ventana, markY++, 0, "INVALID COMMAND USE. EX. read HolaMundo.txt");
        } else {
            int found = comando.find(".txt");
            string nombreArchivo;
            for (int i = 5; i < found + 4; i++) {
                nombreArchivo += comando[i];
            }
            ifstream archivo(path + "/" + nombreArchivo, ios::in);
            string word;
            while(archivo >> word) {
                mvwaddstr(ventana, markY++, 0, word.c_str());
            }
        }
    } else if (comando == "changeColor black") {
        colors1 = 1;
        colors2 = 2;
        colors3 = 3;
        clear();
        markY = 0;
        wbkgd(ventana, COLOR_PAIR(colors3));
    } else if (comando == "changeColor white") {
        colors1 = 4;
        colors2 = 5;
        colors3 = 6;
        clear();
        markY = 0;
        wbkgd(ventana, COLOR_PAIR(colors3));
    } else if (comando == "changeColor red") {
        colors1 = 7;
        colors2 = 8;
        colors3 = 9;
        clear();
        markY = 0;
        wbkgd(ventana, COLOR_PAIR(colors3));
    } else if (strstr(comando.c_str(), "changeUser ")) {
        if (comando[11] == ' ') {
            mvwaddstr(ventana, markY++, 0, "INVALID COMMAND USE. EX. changeUser gabs05");
        } else {
            string nuevoUsuario;
            for (int i = 11; i < comando.size(); i++) {
                nuevoUsuario += comando[i];
            }
            clear();
            markY = 0;
            usuario = nuevoUsuario;
        }
    } else if (strstr(comando.c_str(), "changeMachine ")) {
        if (comando[14] == ' ') {
            mvwaddstr(ventana, markY++, 0, "INVALID COMMAND USE. EX. changeMachine myPC");
        } else {
            string nuevaMaquina;
            for (int i = 14; i < comando.size(); i++) {
                nuevaMaquina += comando[i];
            }
            clear();
            markY = 0;
            maquina = nuevaMaquina;
        }
    }
}

void obtenerComando() {
    string input = "";
    int x = getch();
    while(x != '\n') {
       input.push_back(x);
       x = getch();
    }

    while (analizarComando(input) == false) {
        markY++;
        ejecutarComando(input);
        lineaPrincipal();
        obtenerComando();
    }

    endwin();
    exit(EXIT_SUCCESS);
}

int main() {
    initscr();
    
    getmaxyx(stdscr, MaxY, MaxX);

    if (!has_colors()) {
        endwin();
        return 1;
    }

    start_color();
    init_pair(1, COLOR_BLUE, COLOR_BLACK); //background black
    init_pair(2, COLOR_GREEN, COLOR_BLACK); //background black
    init_pair(3, COLOR_WHITE, COLOR_BLACK); //dos puntos lol

    init_pair(4, COLOR_BLUE, COLOR_WHITE); //background white
    init_pair(5, COLOR_GREEN, COLOR_WHITE); //background white
    init_pair(6, COLOR_BLACK, COLOR_WHITE); //dos puntos lol

    init_pair(7, COLOR_BLUE, COLOR_RED); //background red
    init_pair(8, COLOR_GREEN, COLOR_RED); //background red
    init_pair(9, COLOR_WHITE, COLOR_RED); //dos puntos lol

    nocbreak();
    echo();
    
    ventana = newwin(MaxY, MaxX, 0, 0);
    keypad(stdscr, TRUE);

    colors1 = 1; colors2 = 2; colors3 = 3;
    usuario = "home"; maquina = "vm-gauss"; path = "~";
    lineaPrincipal();
    
    ventana = stdscr;
    touchwin(stdscr);
    refresh();

    obtenerComando();

    cbreak();
    noecho();
    
    endwin();
    return 0;
}