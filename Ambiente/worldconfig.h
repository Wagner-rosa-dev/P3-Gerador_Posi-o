#ifndef WORLDCONFIG_H
#define WORLDCONFIG_H

struct WorldConfig {
    //parametros dos chunks
    int chunkSize = 50;
    int highRes = 65; //(potencia de 2) + 1
    int lowRes = 17; // (potencia de 2) + 1

    //parametros da grade e renderização
    int gridRenderSize = 5; //tamanho da grade de chunks a ser renderizada
    float lodDistanceThreshold = chunkSize * 2.5f;
};

#endif // WORLDCONFIG_H
