#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>

using namespace std;

// Lista de tokens fixos em ordem de inserção
vector<pair<string, string>> tokens_ordem = {
    {"até", "ATE"},
    {"<-", "ATR"},
    {":", "DEC"},
    {"e", "E"},
    {"então", "ENTAO"},
    {"escreva", "ESCREVA"},
    {"fim_para", "FIMPARA"},
    {"fim_se", "FIMSE"},
    {"leia", "LEIA"},
    {"<>", "LOGDIFF"},
    {"=", "LOGIGUAL"},
    {">", "LOGMAIOR"},
    {">=", "LOGMAIORIGUAL"},
    {"<", "LOGMENOR"},
    {"<=", "LOGMENORIGUAL"},
    {"não", "NAO"},
    {"/", "OPDIVI"},
    {"+", "OPMAIS"},
    {"-", "OPMENOS"},
    {"*", "OPMULTI"},
    {"ou", "OU"},
    {"para", "PARA"},
    {"(", "PARAB"},
    {")", "PARFE"},
    {"passo", "PASSO"},
    {"se", "SE"},
    {"senão", "SENAO"},
    {"string", "STRING"},
    {"inteiro", "TIPO"},
};

// Mapa auxiliar para busca rápida
map<string, string> mapa_tokens;

// Verifica se é um número inteiro
bool eh_inteiro(const string& s) {
    for (char c : s) {
        if (c < '0' || c > '9') return false;
    }
    return !s.empty();
}

// Verifica se é uma string entre aspas
bool eh_string(const string& s) {
    return s.length() >= 2 && s.front() == '"' && s.back() == '"';
}

// Verifica se é um identificador válido
bool eh_identificador_valido(const string& s) {
    if (s.empty()) return false;
    if (!isalpha(static_cast<unsigned char>(s[0]))) return false;

    for (char c : s) {
        if (!isalnum(static_cast<unsigned char>(c)) && c != '_') {
            return false;
        }
    }
    return true;
}

// Separa linha em lexemas
vector<string> separar_lexemas(const string& linha) {
    vector<string> resultado;
    string palavra;
    size_t i = 0;

    while (i < linha.size()) {
        char c = linha[i];

        // operadores de 2 caracteres
        if (i + 1 < linha.size()) {
            string dois = linha.substr(i, 2);
            if (dois == "<-" || dois == "<=" || dois == ">=" || dois == "<>") {
                if (!palavra.empty()) {
                    resultado.push_back(palavra);
                    palavra.clear();
                }
                resultado.push_back(dois);
                i += 2;
                continue;
            }
        }

        // operadores de 1 caractere
        if (string("()+-*/=<>:").find(c) != string::npos) {
            if (!palavra.empty()) {
                resultado.push_back(palavra);
                palavra.clear();
            }
            resultado.push_back(string(1, c));
            i++;
            continue;
        }

        // strings entre aspas
        if (c == '"') {
            if (!palavra.empty()) {
                resultado.push_back(palavra);
                palavra.clear();
            }
            string aspas = "\"";
            i++;
            while (i < linha.size() && linha[i] != '"') {
                aspas += linha[i++];
            }
            if (i < linha.size()) {
                aspas += '"';
                i++;
            }
            resultado.push_back(aspas);
            continue;
        }

        // espaços
        if (isspace(static_cast<unsigned char>(c))) {
            if (!palavra.empty()) {
                resultado.push_back(palavra);
                palavra.clear();
            }
        }
        else {
            palavra += c;
        }
        i++;
    }

    if (!palavra.empty()) {
        resultado.push_back(palavra);
    }
    return resultado;
}

// Classifica token
string classificar_token(const string& lexema) {
    auto it = mapa_tokens.find(lexema);
    if (it != mapa_tokens.end()) return it->second;
    if (eh_string(lexema)) return "STRING";
    if (eh_inteiro(lexema)) return "NUMINT";
    if (eh_identificador_valido(lexema)) return "ID";
    return "ERRO";
}

int main() {
    ifstream entrada("../processamento/TESTE.por");
    ofstream saida("../processamento/sintatica.tem");

    if (!entrada.is_open() || !saida.is_open()) {
        cerr << "Erro ao abrir arquivos!" << endl;
        return 1;
    }

    // Preenche mapa auxiliar
    for (const auto& par : tokens_ordem) {
        mapa_tokens[par.first] = par.second;
    }

    set<string> tokens_ja_emitidos; // rastrear tokens já listados

    string linha;
    while (getline(entrada, linha)) {
        vector<string> lexemas = separar_lexemas(linha);
        for (const string& lex : lexemas) {
            string token = classificar_token(lex);

            // Emite forma sintatica
            saida << "(" << token << "," << lex << ") ";
        }
        saida << "\n";
    }

    entrada.close();
    saida.close();

    cout << "Analise sintatica concluida. Veja os arquivos 'sintatica.tem'." << endl;

    return 0;
}
