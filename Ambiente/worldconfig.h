#ifndef WORLDCONFIG_H
#define WORLDCONFIG_H

// Estrutura: WorldConfig
// Descrição: Esta estrutura define parâmetros de configuração globais para o ambiente do mundo 3D.
//            Ela centraliza valores que controlam o tamanho dos chunks, a resolução do terreno
//            em diferentes níveis de detalhe (LOD) e o tamanho da grade de renderização.
//            Isso permite ajustar facilmente as características do mundo sem modificar
//            várias partes do código.
struct WorldConfig {
    // Membro: chunkSize
    // Tipo: int
    // Descrição: Define o tamanho de um chunk individual em unidades do mundo (por exemplo, metros).
    //            Um chunk é uma seção quadrada do terreno.
    int chunkSize = 50;

    // Membro: highRes
    // Tipo: int
    // Descrição: Define a resolução (número de vértices por lado) para chunks que estão
    //            próximos à câmera e exigem um alto nível de detalhe (LOD 0).
    //            É recomendável que seja (potência de 2) + 1 para tesselação adequada.
    int highRes = 65; //(potencia de 2) + 1

    // Membro: lowRes
    // Tipo: int
    // Descrição: Define a resolução (número de vértices por lado) para chunks que estão
    //            mais distantes da câmera e podem ser renderizados com menos detalhe (LOD 1).
    //            Também é recomendável que seja (potência de 2) + 1.
    int lowRes = 17; // (potencia de 2) + 1

    // Membro: gridRenderSize
    // Tipo: int
    // Descrição: Define o número de chunks a serem renderizados ao redor do chunk central
    //            da câmera. Por exemplo, um valor de 5 significa uma grade de 5x5 chunks.
    //            Isso controla a área total do terreno visível.
    int gridRenderSize = 5;

    // Membro: lodDistanceThreshold
    // Tipo: float
    // Descrição: Define a distância da câmera a partir da qual um chunk muda seu Nível de Detalhe (LOD).
    //            Chunks mais próximos que essa distância usarão 'highRes', enquanto os mais distantes
    //            usarão 'lowRes'. É calculado com base no 'chunkSize'.
    float lodDistanceThreshold = chunkSize * 2.5f;
};

#endif // WORLDCONFIG_H
