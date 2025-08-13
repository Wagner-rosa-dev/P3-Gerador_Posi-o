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
    int chunkSize = 3;

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
    int gridRenderSize = 15;

    // Membro: lodDistanceThreshold
    // Tipo: float
    // Descrição: Define a distância da câmera a partir da qual um chunk muda seu Nível de Detalhe (LOD).
    //            Chunks mais próximos que essa distância usarão 'highRes', enquanto os mais distantes
    //            usarão 'lowRes'. É calculado com base no 'chunkSize'.
    float lodDistanceThreshold = chunkSize * 2.5f;

    // Membro: gridSquareSize
    // Tipo: float
    // Descrição: O tamanho do lado de cada quadrado na grade do terreno ( em unidades do mundo)
    float gridSquareSize = 1.0f; // um metro por quadrado

    // Membro: gridLineThickness
    // Tipo: float
    // Descrição: A espessra visual das linhas do grid
    float gridLineThickness = 0.02f; // 5 centimetros de espessura

    // Membro: gridTileSize
    // O tamanho do "tile" (peça) da grade que é desenhado. Esta peça é movida
    // para dar a ilusão de uma grade infinita. Um valor maior preenche mais a tela,
    // um valor menor é um pouco mais eficiente. 40.0 é um bom começo.
    float gridTileSize = 40.0f;

    // --- Configurações da Câmera ---
    // Descrição: Parâmetros que controlam o comportamento da câmera que segue o trator.

    // Membro: cameraFollowDistance
    // Tipo: float
    // Descrição: A distância que a câmera mantém atrás do trator.
    float cameraFollowDistance = 8.5f;

    // Membro: cameraFollowHeight
    // Tipo: float
    // Descrição: A altura da câmera em relação ao trator.
    float cameraFollowHeight = 3.0f;

    // Membro: cameraFov
    // Tipo: float
    // Descrição: O Campo de Visão (Field of View) da câmera, em graus. Funciona como um zoom.
    //            Valores menores dão mais zoom, valores maiores dão uma visão mais ampla.
    float cameraFov = 25.0f;

    // --- Configurações de Cor ---
    // Descrição: Parâmetros que controlam as cores dos elementos na cena.
    //            Os valores de cor são em formato RGB, de 0.0 a 1.0.

    // Cor do Grid de referência
    float gridColorR = 0.4f; // Vermelho
    float gridColorG = 0.4f; // Verde
    float gridColorB = 0.4f; // Azul (R=1, G=1, B=0 -> Amarelo)

    // Cor do Terreno em Baixa Altitude
    float terrainColorR = 0.6f;
    float terrainColorG = 0.6f;
    float terrainColorB = 0.6f;

    // --- Configurações da Ferramenta ---
    // Descrição: Parâmetros inseridos pelo usuário no wizard de configuração inicial.
    //            Inicializados com valores padrão.

    // Membro: toolWidth
    // Tipo: float
    // Descrição: O tamanho (largura) total da barra de ferramentas, em metros.
    float toolWidth = 12.0f;

    // Membro: sectionCount
    // Tipo: int
    // Descrição: O número de seções que a barra de ferramentas possui.
    int sectionCount = 5;

    // Membro: sectionSpacing
    // Tipo: float
    // Descrição: O espaçamento entre as seções, em centímetros.
    float sectionSpacing = 50.0f;


};

#endif // WORLDCONFIG_H
