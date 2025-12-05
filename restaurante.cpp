// Restaurante.cpp
#include "restaurante.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <queue>
#include <cctype>   // isspace
#include <memory>   // shared_ptr

Restaurante::Restaurante(int nChefes, int nMesas) {
    static const std::string nomes[] = {
        "Vitória","Carlos","Natálly","Chihiro","Kiki",
        "Totoro","Valesca","Luiza","Ponyo","Haru",
        "Aylla","Nadylla","Sakura","Brino","Ghibi"
    };
    // garante que não ultrapassa o número de nomes definidos
    int maxNomes = static_cast<int>(sizeof(nomes)/sizeof(nomes[0]));// 
    int total = std::min(nChefes, maxNomes);

    // reserva espaço para os chefs
    chefes.reserve(total);
    for (int i = 0; i < total; ++i) {
        chefes.push_back(std::make_shared<Chef>(i, nomes[i]));
        // enfileira o índice do chef na fila de chefs livres
        chefsLivres.push(i);
    }

    // -1 sig sem chefe 
    mesaParaChefe.assign(nMesas + 1, -1);

}

void Restaurante::processarPedido(int mesa, const std::string& pedidoRaw) {
    //  limpeza  do pedido, estética, não necessario, verificaçoes de entrada 
    std::string pedido = pedidoRaw;
    while (!pedido.empty() && isspace((unsigned char)pedido.front())) pedido.erase(pedido.begin());
    while (!pedido.empty() && isspace((unsigned char)pedido.back())) pedido.pop_back();

    if (mesa <= 0 || mesa >= mesaParaChefe.size()) {
        std::cerr << "Mesa inválida: " << mesa << std::endl;
        return;
    }

    // se já houver chef atendendo aql mesa
    if (mesaParaChefe[mesa] != -1) {
        int id = mesaParaChefe[mesa];

        // se a mesa finaliza ("fim")
        if (pedido == "fim") {
            // encerra a mesa com o chefe atual
            chefes[id]->encerrarMesa();
            mesaParaChefe[mesa] = -1;

            // coloca esse chef de volta na fila de livres 
            chefsLivres.push(id);

            // se houver alguém na fila de espera, já pega o próximo pedido e atende 
            if (!filaEspera.empty() && !chefsLivres.empty()) {
                // pega o próximo cliente em espera(mesa que está esperando ser atendida)
                auto [mesaFila, filaPedidos] = std::move(filaEspera.front());
                filaEspera.pop();

                // obtém um chef livre e pega ele
                int idxLivre = chefsLivres.front();
                chefsLivres.pop();

                // atribui e serve todos os pedidos acumulados dessa mesa
                mesaParaChefe[mesaFila] = idxLivre;
                chefes[idxLivre]->iniciarAtendimento(mesaFila);
                while (!filaPedidos.empty()) {
                    chefes[idxLivre]->prepararPedido(filaPedidos.front());
                    filaPedidos.pop();
                }
            }
            return;
        }

        // enquanto mesa já está sendo atendida
        chefes[id]->prepararPedido(pedido);
        return;
    }
    
    if (!chefsLivres.empty()) {
    int idxLivre = chefsLivres.front();   // pega o índice do chef disponível
    chefsLivres.pop();                    // remove da lista de livres

    mesaParaChefe[mesa] = idxLivre;

    chefes[idxLivre]->iniciarAtendimento(mesa);
    chefes[idxLivre]->prepararPedido(pedido);
    return;
}

    adicionarPedidoNaFila(mesa, pedido);

    // log  das mesas não atendidas 
    std::ofstream log("MesasNaoAtendidas.txt", std::ios::app);
    if (log.is_open()) {
        log << "Mesa " << mesa << " aguardando: " << pedido << "\n";
    }
}

// Função que libera um chefe após o fim
void Restaurante::liberarChefe(int mesa) {
    if (mesa < 1 || mesa >= (int)mesaParaChefe.size()) return;
    int id = mesaParaChefe[mesa];
    if (id != -1) {
        // encerra e marca mesa como sem chefe
        chefes[id]->encerrarMesa();
        mesaParaChefe[mesa] = -1;

        // coloca o chef liberado na fila de livres 
        chefsLivres.push(id);
    }

    // se houver fila de espera e houver chef livre, atende o próximo 
    if (!filaEspera.empty() && !chefsLivres.empty()) {
        auto [mesaFila, filaPedidos]= std::move(filaEspera.front());
        filaEspera.pop();

        int idxLivre = chefsLivres.front();
        chefsLivres.pop();

        mesaParaChefe[mesaFila]=idxLivre;
        chefes[idxLivre]->iniciarAtendimento(mesaFila);
        while (!filaPedidos.empty()) {
            chefes[idxLivre]->prepararPedido(filaPedidos.front());
            filaPedidos.pop();
        }
    }
}
void Restaurante::adicionarPedidoNaFila(int mesa, const std::string& pedido) {
    std::queue<std::pair<int, std::queue<std::string>>> temp;
    bool achou = false;
//percorre os elementos da fila principal
    while (!filaEspera.empty()) {
        //move o primeiro elemento da fila (par: mesa + fila de pedidos)
        auto atual =std::move(filaEspera.front());
        filaEspera.pop();
//se esta é a mesa procurada, adicionamos o pedido a fila dela
        if (atual.first== mesa) {
            atual.second.push(pedido);
            achou= true;
        }
//após verificar/modificar, coloca o elemento na fila temporaria
        temp.push(std::move(atual));
    }
//no final do loop, a fila original foi esvaziada e reconstruida na fila temporaria
    filaEspera = std::move(temp);
//se nenhuma mesa foi encontrada, então este é um novo pedido
    if (!achou) {
        std::queue<std::string> filaPed;//cria nova fila de pedidos
        filaPed.push(pedido);
        filaEspera.push({mesa, std::move(filaPed)});//adiciona mesa e pedido no final da fila de espera
    }
}

// encerra tudo 
void Restaurante::finalizar() {
    for (auto& c : chefes) {
        c->encerrarMesa();
    }
}
