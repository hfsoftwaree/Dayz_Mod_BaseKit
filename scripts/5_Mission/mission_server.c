// --------------------------------------------------
// 4 Modded MissionServer para inicializar/atualizar arquivos no start
// --------------------------------------------------
modded class MissionServer
{
    override void OnInit()
    {
        super.OnInit();

        if (!GetGame().IsServer())
            return;

        // Pasta do HF (usa a função do Flag_Base para garantir consistência)
        string folder = Flag_Base.GetHFProfileFolder();

        // Caminhos dos arquivos
        string pathPlayers = folder + "Player_Items.json";
        string pathFlags   = folder + "Player_Flag.json";
        string pathItems   = folder + "Config.json";

        // ============================================================
        // PLAYER_ITEMS.JSON  (cria ou carrega)
        // ============================================================
        array<string> players = new array<string>;

        if (!FileExist(pathPlayers))
        {
            JsonFileLoader<array<string>>.JsonSaveFile(pathPlayers, players);
            Print("[ZHFBaseKit] Player_Items.json criado!");
        }
        else
        {
            JsonFileLoader<array<string>>.JsonLoadFile(pathPlayers, players);
            Print("[ZHFBaseKit] Player_Items.json carregado: " + players.Count().ToString());
        }

        Flag_Base.RewardedPlayers = players;

        // ============================================================
        // PLAYER_FLAG.JSON (cria ou carrega)
        // ============================================================
        array<string> flags = new array<string>;

        if (!FileExist(pathFlags))
        {
            JsonFileLoader<array<string>>.JsonSaveFile(pathFlags, flags);
            Print("[ZHFBaseKit] Player_Flag.json criado!");
        }
        else
        {
            JsonFileLoader<array<string>>.JsonLoadFile(pathFlags, flags);
            Print("[ZHFBaseKit] Player_Flag.json carregado: " + flags.Count().ToString());
        }

        Flag_Base.FlaggedPlayers = flags;

        // ============================================================
        // CONFIG.JSON (cria, carrega e atualiza)
        // ============================================================
        bool configModified = false;
        StarterDataJSON loadedData = new StarterDataJSON;

        if (FileExist(pathItems))
        {
            JsonFileLoader<StarterDataJSON>.JsonLoadFile(pathItems, loadedData);
            Print("[ZHFBaseKit] Config.json carregado.");
        }
        else
        {
            loadedData = new StarterDataJSON;
            loadedData.Items = new array<ref BaseItemConfig>;
            loadedData.Webhook_Reg_Base = "";
            loadedData.Webhook_Base_Abandonada = "";
            configModified = true;
            Print("[ZHFBaseKit] Config.json inexistente — criando novo.");
        }

        if (!loadedData.Items)
        {
            loadedData.Items = new array<ref BaseItemConfig>;
            configModified = true;
        }

        // ============================================================
        // ITENS PADRÃO (usa o CreateItemConfig da Flag_Base)
        // ============================================================
        ref array<ref BaseItemConfig> defaultItems = new array<ref BaseItemConfig>;
        defaultItems.Insert(Flag_Base.CreateItemConfig("FenceKit", 1));
        defaultItems.Insert(Flag_Base.CreateItemConfig("ExpansionWallKit", 1));
        defaultItems.Insert(Flag_Base.CreateItemConfig("Pliers", 1));
        defaultItems.Insert(Flag_Base.CreateItemConfig("Handsaw", 1));
        defaultItems.Insert(Flag_Base.CreateItemConfig("Shovel", 1));
        defaultItems.Insert(Flag_Base.CreateItemConfig("Hammer", 1));
        defaultItems.Insert(Flag_Base.CreateItemConfig("WoodAxe", 1));
        defaultItems.Insert(Flag_Base.CreateItemConfig("MetalWire", 2));
        defaultItems.Insert(Flag_Base.CreateItemConfig("Whetstone", 2));
        defaultItems.Insert(Flag_Base.CreateItemConfig("ExpansionCodeLock", 2));
        defaultItems.Insert(Flag_Base.CreateItemConfig("NailBox", 3));
        defaultItems.Insert(Flag_Base.CreateItemConfig("WoodenPlank", 6));
        defaultItems.Insert(Flag_Base.CreateItemConfig("WoodenLog", 6));

        // ============================================================
        // ADICIONA ITENS QUE ESTÃO NO PADRÃO MAS NÃO NO JSON
        // ============================================================
        foreach (BaseItemConfig defItm : defaultItems)
        {
            bool found = false;

            foreach (BaseItemConfig existItm : loadedData.Items)
            {
                if (existItm && existItm.ClassName == defItm.ClassName)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                loadedData.Items.Insert(defItm);
                configModified = true;
                Print("[ZHFBaseKit] Item adicionado ao Config.json: " + defItm.ClassName);
            }
        }

        // ============================================================
        // WEBHOOK DEFAULT SE ESTIVER VAZIO
        // ============================================================
        if (!loadedData.Webhook_Reg_Base || loadedData.Webhook_Reg_Base == string.Empty)
        {
            loadedData.Webhook_Reg_Base = "COLE_AQUI_SEU_WEBHOOK";
            configModified = true;
        }

        if (!loadedData.Webhook_Base_Abandonada || loadedData.Webhook_Base_Abandonada == string.Empty)
        {
            loadedData.Webhook_Base_Abandonada = "COLE_AQUI_SEU_WEBHOOK_ABANDONADA";
            configModified = true;
        }

        // ============================================================
        // CLEANUP RADIUS (novo campo – retrocompatível)
        // ============================================================
        if (loadedData.CleanupRadius <= 0)
        {
            loadedData.CleanupRadius = 1.0;
            configModified = true;
        }

        // ============================================================
        // ADMIN STEAM IDS (novo campo – retrocompatível)
        // ============================================================
        if (!loadedData.AdminSteamIDs || loadedData.AdminSteamIDs.Count() == 0)
        {
            loadedData.AdminSteamIDs = new array<string>;
            configModified = true;
        }
		
		// ============================================================
        // MAP NAME (novo campo – retrocompatível)
        // ============================================================
		if (!loadedData.MapName || loadedData.MapName == string.Empty)
        {
            loadedData.MapName = "chernarusplus";
            configModified = true;
        }

		if (loadedData.UpdateTime <= 0)
        {
            loadedData.UpdateTime = 5;
            configModified = true;
        }
		
        // ============================================================
        // SALVA APENAS SE PRECISAR
        // ============================================================
        if (configModified || !FileExist(pathItems))
        {
            JsonFileLoader<StarterDataJSON>.JsonSaveFile(pathItems, loadedData);
            Print("[ZHFBaseKit] Config.json atualizado.");
        }

        // ============================================================
        // ATUALIZA VARIÁVEIS DO FLAG_BASE
        // ============================================================
        Flag_Base.StarterItems            = loadedData.Items;
        Flag_Base.DiscordWebhookURL       = loadedData.Webhook_Reg_Base;
        Flag_Base.DiscordWebhookAbandoned = loadedData.Webhook_Base_Abandonada;
		Flag_Base.MapName				  = loadedData.MapName;
        Flag_Base.CleanupRadius           = loadedData.CleanupRadius;
        Flag_Base.AdminSteamIDs           = loadedData.AdminSteamIDs;
		Flag_Base.UpdateTime 			  = loadedData.UpdateTime;

        Flag_Base.FILE_PATH_PLAYERS = pathPlayers;
        Flag_Base.FILE_PATH_FLAGS   = pathFlags;
        Flag_Base.FILE_PATH_ITEMS   = pathItems;

        Print("[ZHFBaseKit] Inicialização COMPLETA!");
		
		Flag_Base.StartConfigReloadLoop();
    }
}
