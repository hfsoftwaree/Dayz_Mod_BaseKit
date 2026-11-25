# 🏗️ BaseKit – Kit de Construção e Controle de Bandeiras para DayZ

Mod de **scripts (Enforce Script)** para **DayZ Standalone** que gerencia o ciclo de vida de **bandeiras de território** em servidores com **DayZ Expansion**: entrega um **kit inicial de construção**, limita **uma bandeira ativa por jogador**, **limpa bases abandonadas** e envia **logs para o Discord** via webhook.

---

## 📦 Dependências

- **DayZ Expansion** (`TerritoryFlag`, `ExpansionActionDismantleFlag`, `ExpansionWallKit`, `ExpansionCodeLock`)
- Item customizado **`ZHF_Barrel_Red`**, o barril que recebe o kit (deve existir no servidor)

---

## ✨ Funcionalidades

- 🎁 **Kit inicial de construção:** ao montar a primeira bandeira, o jogador recebe um barril com itens configuráveis (cercas, ferramentas, pregos, madeira etc.). O kit é entregue **uma única vez por jogador**.
- 🚩 **Uma bandeira ativa por jogador:** impede montar uma nova bandeira enquanto houver outra ativa (validação em `TerritoryFlagKit.CanBePlaced`).
- 🧹 **Limpeza de base abandonada:** quando o Expansion remove a bandeira por inatividade, os objetos ao redor são deletados e os registros do jogador são limpos, permitindo receber o kit novamente numa futura base.
- 🔔 **Logs no Discord** (embeds com jogador, SteamID, coordenadas e link do iZurvive):
  - 🟢 Bandeira montada
  - 🔴 Bandeira desmontada
  - 🔴 Base abandonada, território deletado
- 🛡️ **Exceção para administradores:** SteamIDs listados no `Config.json` ignoram as regras do mod.
- 🔄 **Recarga automática da configuração** a cada `UpdateTime` minutos, sem reiniciar o servidor.
- 🗺️ **Compatível com vários mapas** via `MapName` (usado no link do iZurvive).

---

## ⚙️ Como funciona

| Situação | Comportamento |
|---|---|
| Jogador monta a bandeira | Cria o barril com o kit (se ainda não recebeu), registra o jogador e a posição, e envia o log ao Discord |
| Jogador tenta montar outra bandeira | Bloqueado com mensagem, se já tiver uma ativa |
| Jogador desmonta a bandeira (Expansion) | Remove o registro de bandeira ativa, envia o log ao Discord e libera nova montagem |
| Bandeira deletada por inatividade | Limpa os objetos no raio configurado, apaga os registros do jogador e envia o log de base abandonada |

> Para diferenciar a desmontagem do jogador de uma deleção por inatividade, o mod registra a posição da bandeira desmontada e ignora a limpeza se o `EEDelete` ocorrer em até 10 segundos.

---

## 🗂️ Arquivos gerados

Criados automaticamente no perfil do servidor, em `HF/BaseKit/`:

| Arquivo | Conteúdo |
|---|---|
| `Config.json` | Configurações gerais, webhooks e itens do kit |
| `Player_Items.json` | Jogadores que já receberam o kit |
| `Player_Flag.json` | Jogadores com bandeira ativa |
| `FlagsData.json` | Nome, SteamID e posição de cada bandeira |

### Exemplo de `Config.json`

```json
{
    "Webhook_Reg_Base": "COLE_AQUI_SEU_WEBHOOK",
    "Webhook_Base_Abandonada": "COLE_AQUI_SEU_WEBHOOK_ABANDONADA",
    "MapName": "chernarusplus",
    "CleanupRadius": 1.0,
    "UpdateTime": 5,
    "AdminSteamIDs": [],
    "Items": [
        { "ClassName": "FenceKit", "Count": 1 },
        { "ClassName": "Hammer", "Count": 1 },
        { "ClassName": "NailBox", "Count": 3 },
        { "ClassName": "WoodenPlank", "Count": 6 }
    ]
}
```

| Campo | Descrição |
|---|---|
| `Webhook_Reg_Base` | Webhook do Discord para bandeira montada/desmontada |
| `Webhook_Base_Abandonada` | Webhook do Discord para bases abandonadas |
| `MapName` | Nome do mapa usado no link do iZurvive |
| `CleanupRadius` | Raio (em metros) da limpeza de base abandonada |
| `UpdateTime` | Intervalo (minutos) da recarga automática do `Config.json` |
| `AdminSteamIDs` | SteamIDs isentos das regras do mod |
| `Items` | Lista de itens (`ClassName` e `Count`) entregues no barril |

Se o barril ficar cheio, os itens excedentes são criados no chão ao redor.

---

## 📁 Estrutura do projeto

```
WelcomePackBase/
├── config.cpp
└── scripts/
    ├── 4_World/
    │   ├── Flag_Base (kit, registro e arquivos)
    │   ├── Flag_Base (montagem/desmontagem e webhooks)
    │   └── TerritoryFlag (controle de deleção e limpeza)
    └── 5_Mission/
        └── MissionServer (inicialização dos arquivos)
```

> Ajuste os nomes conforme os arquivos reais do seu projeto.

---

## 🧰 Tecnologias

- Enforce Script (DayZ)
- `JsonFileLoader` para persistência
- `RestApi` para webhooks do Discord
- Integração com **DayZ Expansion**

---

## 👤 Autor

Desenvolvido por **HF**.

