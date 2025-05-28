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

// Verifica se variável é do tipo inteiro
bool eh_inteiro_declarado(const string& id) {
    return foi_declarado(id) && variaveis_declaradas[id] == "inteiro";
}

// Grava erro no log com a linha correspondente
void log_erro(ofstream& log, const string& mensagem) {
    log << "Linha " << numero_linha << ": " << mensagem << endl;
}

int main() {
    ifstream entrada("../processamento/sintatica.tem");
    ofstream log("../processamento/erros_semanticos.log");

    if (!entrada.is_open()) {
        cerr << "Erro ao abrir arquivo de linguagem sintatica!" << endl;
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
        // Verifica se é uma tentativa de declaração
        // ------------------------------
        bool contem_tipo_ou_dec = false;
        for (const auto& t : tokens) {
            if (t.first == "TIPO" || t.first == "DEC") {
                contem_tipo_ou_dec = true;
                break;
            }
        }

        if (contem_tipo_ou_dec) {
            if (tokens.size() == 3 &&
                tokens[0].first == "TIPO" &&
                tokens[1].first == "DEC" &&
                tokens[2].first == "ID") {

                string nome_var = tokens[2].second;

                if (tokens[0].second != "inteiro") {
                    log_erro(log, "Erro: Tipo '" + tokens[0].second + "' invalido. Apenas 'inteiro' eh permitido.");
                    erro_encontrado = true;
                    continue;
                }

                if (foi_declarado(nome_var)) {
                    log_erro(log, "Erro: Variavel '" + nome_var + "' ja declarada.");
                    erro_encontrado = true;
                }
                else {
                    variaveis_declaradas[nome_var] = "inteiro";
                }

                continue; // não processa mais nada nesta linha
            }
            else {
                log_erro(log, "Erro: Declaracao invalida. Formato esperado: TIPO DEC ID.");
                erro_encontrado = true;
                continue;
            }
        }

        // ------------------------------
        // Atribuição
        // ------------------------------
        else if (tokens.size() >= 3 && tokens[1].first == "ATR") {
            string id_alvo = tokens[0].second;

            // Verifica se variável alvo foi declarada
            if (!foi_declarado(id_alvo)) {
                log_erro(log, "Erro: Variavel '" + id_alvo + "' usada antes de ser declarada.");
                erro_encontrado = true;
            }

            // ID <- ID ou ID <- NUMINT
            if (tokens.size() == 3) {
                string tipo = tokens[2].first;
                string valor = tokens[2].second;

                if (tipo == "ID") {
                    if (!eh_inteiro_declarado(valor)) {
                        log_erro(log, "Erro: Variavel '" + valor + "' nao declarada ou nao eh inteiro.");
                        erro_encontrado = true;
                    }
                }
                else if (tipo == "NUMINT") {
                    // valor literal sempre inteiro, OK
                }
                else {
                    log_erro(log, "Erro: Valor de atribuicao invalido.");
                    erro_encontrado = true;
                }
            }

            // ID <- <NUMINT|ID> <OP> <NUMINT|ID>
            else if (tokens.size() == 5) {
                string tipo1 = tokens[2].first;
                string valor1 = tokens[2].second;
                string operador = tokens[3].first;
                string tipo2 = tokens[4].first;
                string valor2 = tokens[4].second;

                if (!eh_operador_matematico(operador)) {
                    log_erro(log, "Erro: Operador matematico invalido.");
                    erro_encontrado = true;
                }

                // Operando 1
                if (tipo1 == "ID") {
                    if (!eh_inteiro_declarado(valor1)) {
                        log_erro(log, "Erro: Variavel '" + valor1 + "' nao declarada ou nao eh inteiro.");
                        erro_encontrado = true;
                    }
                }
                else if (tipo1 != "NUMINT") {
                    log_erro(log, "Erro: Operando 1 invalido.");
                    erro_encontrado = true;
                }

                // Operando 2
                if (tipo2 == "ID") {
                    if (!eh_inteiro_declarado(valor2)) {
                        log_erro(log, "Erro: Variavel '" + valor2 + "' nao declarada ou nao eh inteiro.");
                        erro_encontrado = true;
                    }
                }
                else if (tipo2 != "NUMINT") {
                    log_erro(log, "Erro: Operando 2 invalido.");
                    erro_encontrado = true;
                }
            }

            // Demais formatos inválidos
            else {
                log_erro(log, "Erro: Forma de atribuicao invalida.");
                erro_encontrado = true;
            }
        }
        else if (tokens.size() >= 3 && tokens[1].first == "OPIGUAL") {
            log_erro(log, "Erro: Atribuicao invalida. Use '<-' em vez de '='.");
            erro_encontrado = true;
            continue;
}
        else if (tokens[0].first == "ID" && tokens.size() > 1 && tokens[1].first != "ATR") {
            log_erro(log, "Erro: Comando invalido ou fora de estrutura esperada (como SE).");
            erro_encontrado = true;
            continue;
        }

        // ------------------------------
        // Comando leia
        // ------------------------------
        else if (primeiro_token == "LEIA" && tokens.size() == 4 && tokens[1].first == "PARAB" && tokens[2].first == "ID") {
            string nome = tokens[2].second;
            if (!foi_declarado(nome)) {
                log_erro(log, "Erro: Variavel '" + nome + "' usada em 'leia' sem declaracao.");
                erro_encontrado = true;
            }
        }

        // ------------------------------
        // Comando escreva
        // ------------------------------
        else if (primeiro_token == "ESCREVA" && tokens.size() >= 4 && tokens[1].first == "PARAB") {
            string tipo = tokens[2].first;
            string valor = tokens[2].second;

            if (tipo == "ID") {
                if (!foi_declarado(valor)) {
                    log_erro(log, "Erro: Variavel '" + valor + "' usada em 'escreva' sem declaracao.");
                    erro_encontrado = true;
                }
            }
            else if (tipo != "ID" && tipo != "STRING" && tipo != "NUMINT") {
                log_erro(log, "Erro: Tipo invalida em 'escreva': " + tipo);
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
