#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>

using namespace std;

// Tabela de variáveis declaradas: nome -> tipo
map<string, string> variaveis_declaradas;

// Contador de linhas para log
int numero_linha = 0;

// Função para verificar se token é um operador matemático
bool eh_operador_matematico(const string& token) {
    return token == "OPMAIS" || token == "OPMENOS" || token == "OPMULTI" || token == "OPDIVI";
}

// Verifica se identificador foi declarado
bool foi_declarado(const string& id) {
    return variaveis_declaradas.count(id) > 0;
}

// Grava erro no log com a linha correspondente
void log_erro(ofstream& log, const string& mensagem) {
    log << "Linha " << numero_linha << ": " << mensagem << endl;
}

int main() {
    ifstream entrada("../processamento/lexica.tem");
    ofstream log("../processamento/erros_semanticos.log");

    if (!entrada.is_open()) {
        cerr << "Erro ao abrir arquivo de linguagem sintética!" << endl;
        return 1;
    }

    string linha;
    bool erro_encontrado = false;

    // Pilhas para verificar comandos balanceados
    int nivel_se = 0;
    int nivel_para = 0;

    while (getline(entrada, linha)) {
        numero_linha++;

        // Ignora linha vazia
        if (linha.empty()) continue;

        // Separar linha em pares (TOKEN, VALOR)
        vector<pair<string, string>> tokens;
        size_t pos = 0;

        while (pos < linha.length()) {
            size_t ini_token = linha.find('(', pos);
            size_t meio = linha.find(',', ini_token);
            size_t fim_token = linha.find(')', meio);

            if (ini_token == string::npos || meio == string::npos || fim_token == string::npos)
                break;

            string tipo = linha.substr(ini_token + 1, meio - ini_token - 1);
            string valor = linha.substr(meio + 1, fim_token - meio - 1);
            tokens.emplace_back(tipo, valor);
            pos = fim_token + 1;
        }

        if (tokens.empty()) continue;

        string primeiro_token = tokens[0].first;

        // ------------------------------
        // Declaração de variável
        // ------------------------------
        if (primeiro_token == "TIPO" && tokens.size() >= 2 && tokens[1].first == "ID") {
            string nome_var = tokens[1].second;

            // Só aceita tipo inteiro
            if (tokens[0].second != "inteiro") {
                log_erro(log, "Erro: Tipo '" + tokens[0].second + "' inválido. Apenas 'inteiro' é permitido.");
                erro_encontrado = true;
                continue;
            }

            // Verifica duplicação
            if (foi_declarado(nome_var)) {
                log_erro(log, "Erro: Variável '" + nome_var + "' já declarada.");
                erro_encontrado = true;
            }
            else {
                variaveis_declaradas[nome_var] = "inteiro";
            }
        }

        // ------------------------------
        // Atribuição
        // ------------------------------
        else if (tokens.size() >= 3 && tokens[1].first == "ATR") {
            string id_alvo = tokens[0].second;

            // Verifica se variável foi declarada
            if (!foi_declarado(id_alvo)) {
                log_erro(log, "Erro: Variável '" + id_alvo + "' usada antes de ser declarada.");
                erro_encontrado = true;
            }

            // Expressões válidas
            if (tokens.size() == 3) {
                if (tokens[2].first != "NUMINT" && (tokens[2].first != "ID" || !foi_declarado(tokens[2].second))) {
                    log_erro(log, "Erro: Valor de atribuição inválido.");
                    erro_encontrado = true;
                }
            }
            else if (tokens.size() == 5 && eh_operador_matematico(tokens[3].first)) {
                string op1 = tokens[2].second;
                string op2 = tokens[4].second;

                if ((tokens[2].first == "ID" && !foi_declarado(op1)) ||
                    (tokens[4].first == "ID" && !foi_declarado(op2))) {
                    log_erro(log, "Erro: Operando não declarado em expressão.");
                    erro_encontrado = true;
                }
            }
            else {
                log_erro(log, "Erro: Forma de atribuição inválida.");
                erro_encontrado = true;
            }
        }

        // ------------------------------
        // Comando leia
        // ------------------------------
        else if (primeiro_token == "LEIA" && tokens.size() == 4 && tokens[1].first == "PARAB" && tokens[2].first == "ID") {
            string nome = tokens[2].second;
            if (!foi_declarado(nome)) {
                log_erro(log, "Erro: Variável '" + nome + "' usada em 'leia' sem declaração.");
                erro_encontrado = true;
            }
        }

        // ------------------------------
        // Comando escreva
        // ------------------------------
        else if (primeiro_token == "ESCREVA" && tokens.size() >= 4 && tokens[1].first == "PARAB") {
            string tipo = tokens[2].first;
            string valor = tokens[2].second;

            if (tipo == "ID" && !foi_declarado(valor)) {
                log_erro(log, "Erro: Variável '" + valor + "' usada em 'escreva' sem declaração.");
                erro_encontrado = true;
            }
            else if (tipo != "ID" && tipo != "STRING" && tipo != "NUMINT") {
                log_erro(log, "Erro: Tipo inválido em 'escreva': " + tipo);
                erro_encontrado = true;
            }
        }

        // ------------------------------
        // Estruturas de controle
        // ------------------------------
        else if (primeiro_token == "SE") nivel_se++;
        else if (primeiro_token == "FIMSE") {
            if (nivel_se == 0) {
                log_erro(log, "Erro: 'fim_se' sem 'se' correspondente.");
                erro_encontrado = true;
            }
            else {
                nivel_se--;
            }
        }
        else if (primeiro_token == "PARA") nivel_para++;
        else if (primeiro_token == "FIMPARA") {
            if (nivel_para == 0) {
                log_erro(log, "Erro: 'fim_para' sem 'para' correspondente.");
                erro_encontrado = true;
            }
            else {
                nivel_para--;
            }
        }
    }

    // Verifica estruturas restantes
    if (nivel_se > 0) {
        log_erro(log, "Erro: 'se' sem 'fim_se'.");
        erro_encontrado = true;
    }
    if (nivel_para > 0) {
        log_erro(log, "Erro: 'para' sem 'fim_para'.");
        erro_encontrado = true;
    }

    entrada.close();
    log.close();

    if (erro_encontrado) {
        cout << "Analise semantica concluida com erros. Veja 'erros_semanticos.log'." << endl;
    }
    else {
        cout << "Analise semantica concluida sem erros." << endl;
    }

    return 0;
}
