#ifndef CHUNKWORKER_H
#define CHUNKWORKER_H

#include <QObject>
#include "chunk.h"

class ChunkWorker : public QObject
{
    Q_OBJECT // Macro obrigatorio para sinais e slots

public:
    explicit ChunkWorker(QObject *parent = nullptr);

public slots:
    // este e o gatilho que a thread pincipal usara para pedir a geração de um chunk
    //ele precisa receber todas as informações necessarias para o calculo
    void generateChunkMesh(int chunkX, int chunkZ, int resolution);

signals:
    //este e o anuncio que o trabalho fara quando terminar
    //ele envia de volta os dados da malha prontos para a thread principal
    void meshReady(int chunkX, int chunkZ, const chunk::MeshData& meshData);
};

#endif // CHUNKWORKER_H
