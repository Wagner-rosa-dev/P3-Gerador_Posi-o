#ifndef CHUNKWORKER_H
#define CHUNKWORKER_H

#include <QObject>      // Classe base para o sistema de sinais/slots do Qt (embora QRunnable não exija QObject diretamente, é bom ter em mente para contexto).
#include "chunk.h"      // Inclui a definição da classe Chunk e sua estrutura MeshData.
#include <QRunnable>    // Classe base para objetos que podem ser executados por um QThreadPool em uma thread separada.
#include "worldconfig.h"// Inclui a estrutura WorldConfig para acessar parâmetros do mundo.

// Declaração antecipada da classe terrainmanager
// Descrição: Usada para evitar inclusões circulares e para declarar que ChunkWorker
//            terá um ponteiro para terrainmanager, sem precisar de sua definição completa aqui.
class terrainmanager;

// Classe: ChunkWorker
// Descrição: Esta classe é um "trabalhador" (worker) que herda de QRunnable,
//            projetada para executar o cálculo pesado de geração de malha de chunk
//            em uma thread separada (gerenciada por QThreadPool).
//            Após a geração, ela envia os dados da malha de volta para a thread principal
//            (via um slot no `terrainmanager`) para o upload na GPU.
class ChunkWorker : public QRunnable
{
public:
    // Construtor: ChunkWorker
    // Descrição: Inicializa o worker com os dados necessários para gerar um chunk específico.
    //            Define `setAutoDelete(true)` para que o objeto seja automaticamente deletado
    //            pelo QThreadPool após a execução do método `run()`.
    // Parâmetros:
    //   - chunkX: Coordenada X do chunk na grade.
    //   - chunkZ: Coordenada Z do chunk na grade.
    //   - resolution: A resolução da malha a ser gerada para este chunk.
    //   - config: Ponteiro constante para a configuração do mundo (WorldConfig).
    //   - manager: Ponteiro para o `terrainmanager` que solicitou a geração da malha.
    ChunkWorker(int chunkX,int chunkZ, int resolution, const WorldConfig* config, terrainmanager* manager);

    // Método: run
    // Descrição: O método principal que é executado quando o QRunnable é iniciado por um QThreadPool.
    //            Contém a lógica para gerar os dados da malha do chunk na CPU
    //            e enviá-los de volta para a thread principal.
    void run() override;

private:
    // Membro: m_chunkX
    // Tipo: int
    // Descrição: A coordenada X do chunk que este worker é responsável por gerar.
    int m_chunkX;
    // Membro: m_chunkZ
    // Tipo: int
    // Descrição: A coordenada Z do chunk que este worker é responsável por gerar.
    int m_chunkZ;
    // Membro: m_resolution
    // Tipo: int
    // Descrição: A resolução da malha que será gerada para este chunk.
    int m_resolution;
    // Membro: m_config
    // Tipo: const WorldConfig*
    // Descrição: Ponteiro constante para a configuração global do mundo, contendo parâmetros como o tamanho do chunk.
    const WorldConfig* m_config;
    // Membro: m_manager
    // Tipo: terrainmanager*
    // Descrição: Ponteiro para o `terrainmanager` que iniciou este worker.
    //            Usado para chamar um slot na thread principal e enviar os dados da malha gerada.
    terrainmanager* m_manager;
};

#endif // CHUNKWORKER_H
