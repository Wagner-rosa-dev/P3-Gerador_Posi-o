#ifndef CHUNKWORKER_H
#define CHUNKWORKER_H

#include <QObject>
#include "chunk.h"
#include <QRunnable>
#include "worldconfig.h"

class terrainmanager;

class ChunkWorker : public QRunnable
{
public:
    ChunkWorker(int chunkX,int chunkZ, int resolution, const WorldConfig* config, terrainmanager* manager);

    void run() override;

private:
    int m_chunkX;
    int m_chunkZ;
    int m_resolution;
    const WorldConfig* m_config; // ponteiro do config
    terrainmanager* m_manager; // ponteiro para quem nos chamou

};

#endif // CHUNKWORKER_H
