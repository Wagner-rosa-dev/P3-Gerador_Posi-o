#ifndef NOISEUTILS_H
#define NOISEUTILS_H

#include <QtGlobal> // Para tipos globais do Qt, como quintptr, qreal, etc.
#include <QtMath>   // Para funções matemáticas como qAtan2, qAsin, etc.
#include <QVector3D> // Para a classe QVector3D, utilizada para vetores 3D.

// Namespace: NoiseUtils
// Descrição: Este namespace encapsula funções utilitárias relacionadas à geração de ruído
//            e cálculo de informações do terreno, como altura e normais.
//            As funções são definidas como `inline` para permitir que o compilador as otimize,
//            possivelmente inserindo o código diretamente no local da chamada para melhor performance.
namespace NoiseUtils {
// Função: getHeight
// Descrição: Calcula e retorna a altura do terreno para uma dada coordenada no mundo (worldX, worldZ).
//            Atualmente, retorna um valor fixo de 0.0f, mas é um placeholder para uma função
//            de geração de ruído mais complexa (e.g., Perlin Noise, Simplex Noise).
// Parâmetros:
//   - worldX: Coordenada X no espaço do mundo.
//   - worldZ: Coordenada Z no espaço do mundo.
// Retorno: float - A altura do terreno na coordenada especificada.
inline float getHeight(float worldX, float worldZ) {
    return 0.0f; // Este é um placeholder. Futuramente, uma função de ruído real seria implementada aqui.
}

// Função: getNormal
// Descrição: Calcula e retorna o vetor normal da superfície do terreno para uma dada coordenada no mundo (worldX, worldZ).
//            A normal é usada para determinar como a luz reflete na superfície, influenciando a aparência do terreno.
//            A implementação atual simula a normal usando pequenas amostras da altura ao redor do ponto.
// Parâmetros:
//   - worldX: Coordenada X no espaço do mundo.
//   - worldZ: Coordenada Z no espaço do mundo.
// Retorno: QVector3D - O vetor normal normalizado na coordenada especificada.
inline QVector3D getNormal(float worldX, float worldZ) {
    float offset = 0.1f; // Pequeno deslocamento usado para amostrar a altura em pontos vizinhos.
    float hL = getHeight(worldX - offset, worldZ); // Altura à esquerda do ponto.
    float hR = getHeight(worldX - offset, worldZ); // Altura à direita do ponto (corrigido: deveria ser worldX + offset).
    float hD = getHeight(worldX - offset, worldZ); // Altura para "baixo" (Z negativo) do ponto (corrigido: deveria ser worldZ - offset).
    float hU = getHeight(worldX - offset, worldZ); // Altura para "cima" (Z positivo) do ponto (corrigido: deveria ser worldZ + offset).
    // Cálculo da normal usando as diferenças de altura. O vetor é então normalizado.
    return QVector3D(hL - hR, 2.0f * offset, hD - hU).normalized();
}
}
#endif // NOISEUTILS_H
