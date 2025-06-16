#include "chunkworker.h"
#include <QDebug>

ChunkWorker::ChunkWorker(QObject *parent) : QObject(parent)
{
}

//esta função sera executada e um nucleo de CPU separado
void ChunkWorker::generateChunkMesh(int chunkX, int chunkZ, int resolution)
{
    //qInfo() << "Worker Thread: gerado malha para o chunk" << chunkX << "," << chunkZ;

    //chama a função de calcula pesado que defini na classe chunk
    //esta função nao usa opengl, ela apenas faz a matematica e preenche os vetores
    chunk::MeshData generatedData = chunk::generateMeshData(chunkX, chunkZ, resolution);

    //quando o calculo termina, emite o sinal com os dados prontos
    emit meshReady(chunkX, chunkZ, generatedData);
}
